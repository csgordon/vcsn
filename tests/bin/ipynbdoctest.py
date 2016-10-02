#! /usr/bin/env python

import argparse
import os
import pprint
import re
import subprocess
import sys

from test import *

try:
    from queue import Empty
    from difflib import ndiff as diff
    from jupyter_client import KernelManager
    from nbformat import v4 as formatter
except ImportError as e:
    SKIP('cannot run ipynbdoctest: ', e)
    exit(0)

def truncate(text):
    '''Truncate a too long text to its first characters.'''
    text = str(text)
    # Dicts: keep curly brackets, truncate inside.
    text = re.sub(r'\{(.{60}).*\}', r'{\1...}', text)
    # Lists: same with square brackets.
    text = re.sub(r'\[(.{60}).*\]', r'[\1...]', text)
    # Other types: truncate as well.
    text = re.sub(r'(?s)^([^\{\[].{60}).*', r'\1...', text)
    # Keep it one line.
    text = text.replace('\n', '\\n')
    return text


def canonicalize(s):
    '''Canonicalize a string `s` for comparison.

    Fix universal newlines, strip trailing newlines, and normalize likely
    random values (memory addresses and UUIDs).
    '''
    s = str(s)
    # Normalize newline.
    s = s.replace('\r\n', '\n')

    # Ignore trailing newlines (but not space).
    s = s.rstrip('\n')

    # Remove hex addresses.
    s = re.sub(r'at 0x[a-f0-9]+', 'object', s)

    # Normalize UUIDs.
    s = re.sub(r'[a-f0-9]{8}(\-[a-f0-9]{4}){3}\-[a-f0-9]{12}', 'U-U-I-D', s)

    # Normalize Graphviz version.
    s = re.sub(r'(Generated by graphviz version ).*', r'\1VERSION', s)

    # SVG generated by graphviz may put note at different positions
    # depending on the graphviz build.  Let's just strip anything that
    # looks like a position or size.
    s = re.sub(r'<path[^/]* d="[^"]*"', '<path', s)
    s = re.sub(r'points="[^"]*"', 'points=""', s)
    s = re.sub(r'x="[0-9.-]+"', 'x=""', s)
    s = re.sub(r'y="[0-9.-]+"', 'y=""', s)
    s = re.sub(r'width="[0-9.]+pt"', 'width=""', s)
    s = re.sub(r'height="[0-9.]+pt"', 'height=""', s)
    s = re.sub(r'viewBox="[0-9 .-]*"', 'viewbox=""', s)
    s = re.sub(r'transform="[^"]*"', 'transform=""', s)
    s = re.sub(r'font-size="[^"]*"', 'font-size=""', s)

    # Normalize whether data was already there or not.
    s = re.sub(r'(Over)?[Ww]rit', 'Writ', s)

    # Normalize paths.
    # s = re.sub('/([^/]+/){2,}', '', s)

    return s


def canonical_dict(dict):
    '''Neutralize gratuitous differences in a Jupyter dictionary.

    For instance, neutralize different Graphviz layouts in SVG.
    '''

    if 'evalue' in dict:
        if dict['evalue'] == "No module named 'pandas'":
            SKIP('pandas not installed')
            exit(0)
        if dict['evalue'] == "cannot open /usr/share/dict/words for reading: No such file or directory":
            SKIP('/usr/share/dict/words not installed')
            exit(0)

    if 'text' in dict:
        if 'fstcompile: command not found' in dict['text']:
            SKIP('OpenFST not installed')
            exit(0)
        if 'pygmentize: command not found' in dict['text']:
            SKIP('pygmentize (from python-pygments) not installed')
            exit(0)
        # Normalize newline.
        dict['text'] = dict['text'].replace('\r\n', '\n')
        # TAF-Kit path.
        dict['text'] = re.sub(r'usage: .*?vcsn-tafkit', 'usage: vcsn-tafkit',
                              dict['text'])
        # %%file writes `Writing`, or `Overwriting` if the file exists.
        dict['text'] = re.sub(r'^Overwriting ', 'Writing ',
                              dict['text'])

    if 'data' in dict:
        for k in ['image/svg+xml', 'text/html']:
            if k in dict['data']:
                dict['data'][k] = canonicalize(dict['data'][k])
    return dict


def compare_outputs(ref, test):
    exp = pprint.pformat([canonical_dict(d) for d in ref], width=132)
    eff = pprint.pformat([canonical_dict(d) for d in test], width=132)
    if exp == eff:
        return True
    else:
        rst_file("Expected output", exp)
        rst_file("Effective output", eff)
        rst_diff(exp, eff)
        return False


def run_cell(kc, cell):
    kc.execute(cell['source'])
    # This is useful but will make the notebook crash on long
    # executions, such as when the cache is empty, which can cause
    # problems when doing bulk testing.
    #
    # kc.get_shell_msg(timeout=20)
    kc.get_shell_msg()
    outs = []
    while True:
        msg = kc.get_iopub_msg(timeout=0.2)
        msg_type = msg['msg_type']
        content = msg['content']
        if msg_type == 'status' and content['execution_state'] == 'idle':
            break
        if msg_type in ('status', 'pyin', 'execute_input',
                        'comm_open', 'comm_msg'):
            continue
        # Use stream and should be kept:
        #   print, %%file, !
        # Use stream and should be ignored:
        #   widgets that crashes
        if msg_type == 'stream':
            if 'Widget' in content['text']:
                continue
            # If the last stream had the same name,
            # then outputs are appended.
            if len(outs):
                last = outs[-1]
                if last['output_type'] == 'stream' and \
                   last['name'] == content['name']:
                    last['text'] += content['text']
                    continue
        if msg_type == 'clear_output':
            outs = []
            continue
        content['output_type'] = msg_type
        outs.append(content)
    return outs

def test_notebook(ipynb):
    print('\nTesting notebook {}'.format(ipynb))
    with open(ipynb) as f:
        nb = formatter.reads_json(f.read())
    km = KernelManager()
    # Do not save the history to disk, as it can yield spurious lock errors.
    # See https://github.com/ipython/ipython/issues/2845
    km.start_kernel(extra_arguments=['--HistoryManager.hist_file=:memory:'],
                    stderr=subprocess.DEVNULL)
    kc = km.client()
    kc.start_channels()
    kc.wait_for_ready()
    npass, nfail, nerror = 0, 0, 0
    i = 0
    for n, cell in enumerate(nb['cells']):
        if cell['cell_type'] != 'code':
            continue
        # `%timeit`s shall count in execution count
        if cell['source'].startswith('%timeit'):
            run_cell(kc, {'source': 'pass'})
            continue
        i = cell['execution_count']
        try:
            outs = run_cell(kc, cell)
        except Empty:
            print('Failed to run cell #{} ({}):'.format(i, n),
                  '    Kernel Client is Empty; this is most likely due to a',
                  '    timeout issue. Check with `vcsn ps` or run the notebook',
                  '    manually, then retry.', file=sys.stderr, sep='\n')
            print('Source was:\n', cell['source'])
            FAIL('failed to run cell #{}'.format(i))
            nerror += 1
            continue
        except Exception as e:
            print('Failed to run cell #{} ({}):'.format(i, n), repr(e),
                  file=sys.stderr)
            print('Source was:', cell['source'], sep='\n')
            FAIL('failed to run cell #{}'.format(i))
            nerror += 1
            continue
        failed = not compare_outputs(cell.outputs, outs)
        print('cell #{} ({}): '.format(i, n), end='')
        if failed:
            print('FAIL')
            FAIL('cell #{}'.format(i))
            nfail += 1
        else:
            print('OK')
            PASS('cell #{}'.format(i))
            npass += 1
    print("Tested notebook {}".format(ipynb))
    print("    {:3} cells successfully replicated".format(npass))
    if nfail:
        print("    {:3} cells mismatched output".format(nfail))
    if nerror:
        print("    {:3} cells failed to complete".format(nerror))
    if i == 0:
        # The TAP protocol does not like empty test suite.
        PASS('no tests')

    kc.stop_channels()
    km.shutdown_kernel()
    del km
    return False if nfail or nerror else True


if __name__ == '__main__':
    p = argparse.ArgumentParser(description='Update test cases.')
    p.add_argument('--tap', action='store_true', help='enable TAP mode')
    p.add_argument('notebooks', nargs='+', help='IPython notebook to check')
    args = p.parse_args()

    # Set the locale to something simple, so that we don't have
    # surprises on translated error messages.
    os.environ['LC_MESSAGES'] = 'C'

    success = True
    cwd = os.getcwd()
    for ipynb in args.notebooks:
        # Go into the directory of the notebook, so that the path to
        # the rest of the package is really identical as if we were
        # running the notebook.
        os.chdir(os.path.dirname(ipynb))
        success &= test_notebook(os.path.basename(ipynb))
        os.chdir(cwd)
    sys.exit(0 if success or args.tap else 1)
