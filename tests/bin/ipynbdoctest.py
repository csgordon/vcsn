#! /usr/bin/env python

import argparse
import os
import pprint
import re
import subprocess
import sys
import tempfile

from test import *

# Useful sources of inspiration:
#
# - https://gist.github.com/minrk/2620735
# The original version.
#
# - https://github.com/chembl/mychembl/blob/master/ipython_notebooks/ipnbdoctest.py
# With Travis support, a lot of OO modeling, options, context manager
# to close properly the kernel, etc.

try:
    from queue import Empty
    from jupyter_client import KernelManager
    from nbformat import v4 as formatter
except ImportError as e:
    SKIP('cannot run ipynbdoctest: ', e)
    exit(0)

# Verbosity level.
verbose = 0

def log(*msg, level=1):
    if level <= verbose:
        print('ipynbdoctest:', *msg, file=sys.stderr, flush=True)

def is_libcpp():
    '''Whether `vcsn compile` uses libc++.  Matters when we use random
    number generation, otherwise we get different sequences.

    '''
    with tempfile.NamedTemporaryFile(mode='w+', suffix='.cc') as f:
        print('''#include <cstddef>
        #ifndef _LIBCPP_VERSION
        # error "this is not libc++"
        #endif
        int main() {}''', file=f)
        f.flush()
        log("vcsn compile: run", level=2)
        res = subprocess.call(['vcsn', 'compile', f.name]) == 0
        log("vcsn compile: result: ", res, level=2)
        return res


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


def canonicalize(s, type, ignores):
    '''Canonicalize the `data` field of a `image/svg+xml` or `text/html`
    output for comparison.

    Fix newlines, strip trailing newlines, and normalize likely
    random values (memory addresses and UUIDs).
    '''
    in_ = str(s)
    s = in_

    # Ignore trailing newlines (but not space).
    s = s.rstrip('\n')

    # Remove hex addresses.
    s = re.sub(r'at 0x[a-f0-9]+', 'object', s)

    # Normalize UUIDs.
    s = re.sub(r'[a-f0-9]{8}(\-[a-f0-9]{4}){3}\-[a-f0-9]{12}', 'U-U-I-D', s)

    # Normalize whether data was already there or not.
    s = re.sub(r'(Over)?[Ww]rit', 'Writ', s)

    if type in ['text/html']:
        # Depending on the version of Pandas, the tables are presented in
        # different ways.
        s = s.replace('<div style="max-height:1000px;max-width:1500px;overflow:auto;">',
                      '<div>')
        s = re.sub('<td> +', '<td>', s)
        s = re.sub(r'<style>\s*.dataframe thead tr:only-child.*?</style>\n', '', s,
                   flags = re.S)
        s = re.sub(r'<style scoped>.*?</style>\n', '', s,
                   flags = re.S)

    if type in ['image/svg+xml', 'text/html']:
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

        # The following patterns from Graphviz 2.40 are rewritten as they used to
        # be in 2.38.  This was shamelessly stolen from Spot.
        s = re.sub(r'"#000000"', '"black"', s)
        s = re.sub(r'"#00ff00"', '"green"', s)
        s = re.sub(r'"#696969"', '"DimGray"', s)
        s = re.sub(r'"#d3d3d3"', '"lightgray"', s)
        s = re.sub(r'"#ff0000"', '"red"', s)
        s = re.sub(r'"#ffa500"', '"orange"', s)
        s = re.sub(r'"#ffffff"', '"white"', s)
        s = re.sub(r' fill="black"', '', s)
        s = re.sub(r' stroke="transparent"', ' stroke="none"', s)
        s = re.sub(r'><title>', '>\n<title>', s)

    # Run the user's canonicalization.
    for n, p in enumerate(ignores):
        s = re.sub(p, 'IGN{}'.format(n), s)
    log('canonicalize:', type, ': ', in_, '->', s, level=3)
    return s


def canonical_dict(dict, ignores):
    '''Neutralize gratuitous differences in a Jupyter dictionary.

    For instance, neutralize different Graphviz layouts in SVG.
    '''
    log('canonical_dict: run', level=2)
    if 'evalue' in dict:
        if dict['evalue'] == "No module named 'pandas'":
            SKIP('pandas not installed')
            exit(0)
        if dict['evalue'] in ["No module named 'ipywidgets'",
                              "module 'vcsn' has no attribute 'ipython'",
                              "'module' object has no attribute 'ipython'"]:
            SKIP('ipywidgets not installed')
            exit(0)
        if dict['evalue'] ==\
        "cannot open /usr/share/dict/words for reading: No such file or directory":
            SKIP('/usr/share/dict/words not installed')
            exit(0)
        if dict['evalue'] == "module 'ipywidgets' has no attribute 'Label'":
            SKIP('cannot use ipywidgets.Label for some reason...')
            exit(0)

    if 'text' in dict:
        if dict['text'].startswith('SKIP: '):
            SKIP('On demand from the notebook: ' + dict['text'][len('SKIP: '):])
            exit(0)
        if re.search('fstcompile: (command )?not found', dict['text']):
            SKIP('OpenFST not installed')
            exit(0)
        if re.search('pygmentize: (command )?not found', dict['text']):
            SKIP('pygmentize (from python-pygments) not installed')
            exit(0)
        if dict['text'] in ['UsageError: Cell magic `%%automaton` not found.\n',
                            'UsageError: Line magic function `%demo` not found.\n',
                            'ERROR:root:Cell magic `%%automaton` not found.\n',
                            'ERROR:root:Line magic function `%demo` not found']:
            SKIP("No cell/line magic support")
            exit(0)
        if re.search('Use of this header .* is deprecated', dict['text']):
            SKIP('spurious warnings about deprecated header')
            exit(0)

        # Normalize newline.  For some reason, out of our 400+ lines
        # containing \r\n in its stdout, we have one line with two \r, on
        # separate lines:
        #
        #  "  -D            input is an identity\r\n",
        #  "  -E            input is a rational expression\r",
        #  "\r\n",
        #  "  -F            input is a float\r\n",
        #  "  -L            input is a label (or a word)\r\n",
        dict['text'] = re.sub(r'\r+\n', r'\n', dict['text'])

        # Tools path.
        dict['text'] = re.sub(r'usage: .*?vcsn-tools', 'usage: vcsn-tools',
                              dict['text'])
        # %%file writes `Writing`, or `Overwriting` if the file exists.
        dict['text'] = re.sub(r'^Overwriting ', 'Writing ',
                              dict['text'])
        # Pygmentize 2.0 seems to produce less efficient sequences for
        # character litterals.
        dict['text'] = re.sub(r'\x1b\[39;49;00m\x1b\[33m', '', dict['text'])

    if 'data' in dict:
        for k in dict['data']:
            dict['data'][k] = canonicalize(dict['data'][k], k, ignores)
        # FIXME: Probably not the best way to deal with this, but right know
        # I want the test skip to be green again
        if 'text/plain' in dict['data']:
            if '\\\\x85\\\\xa1\\\\xa2\\\\xa4\\\\xa5\\\\xa7\\\\xa8\\\\xa9' \
               '\\\\xaa\\\\xad\\\\xb1\\\\xb3\\\\xb4\\\\xb6\\\\xbb\\\\xbc' \
               '\\\\xc3)>>,' in dict['data']['text/plain']:
                SKIP('dictionary file contains unknown characters')
                exit(0)
    # It is unclear what this is made for, but `'transient': {}`
    # entries recently appeared in the outputs.
    if 'transient' in dict:
        del dict['transient']
    log('canonical_dict: ran', level=2)
    return dict


def check_outputs(ref, test, ignores):
    '''Check that two lists of outputs are equivalent and report the
    result.'''

    log('check_outputs', 'run', level=2)
    # The embedding of widgets does not seem to work the same way
    # everywhere.
    for ds in [ref, test]:
        for d in ds:
            if 'data' in d and \
               'application/vnd.jupyter.widget-view+json' in d['data']:
                log('check_outputs', 'skip', level=2)
                SKIP('widgets are used')
                return

    # There can be several outputs.  For instance wnen the cell both
    # prints a result (goes to "stdout") and displays an automaton
    # (goes to "data").
    exp = pprint.pformat([canonical_dict(d, ignores) for d in ref],  width=132)
    eff = pprint.pformat([canonical_dict(d, ignores) for d in test], width=132)

    if exp == eff:
        log('check_outputs', 'pass', level=2)
        PASS()
    else:
        rst_file("Expected output", exp)
        rst_file("Effective output", eff)
        rst_diff(exp, eff)
        log('check_outputs', 'fail', level=2)
        FAIL()


def run_cell(kc, source):
    log("run_cell: execute:", source)
    kc.execute(source)
    log("run_cell: done")
    # SACS-2017 has a cell with many compilation and timings.  This
    # can be really long.
    kc.get_shell_msg(timeout=900)
    log("run_cell: get_shell_msg done")
    outs = []
    while True:
        log('run_cell: get_iopub_msg')
        msg = kc.get_iopub_msg(timeout=0.2)
        log('run_cell: get_iopub_msg: done:', msg)
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
            # If the last stream had the same name, then outputs are
            # appended.
            if outs:
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
    log('run_cell: return')
    return outs

def test_notebook(ipynb):
    print('\nTesting notebook {}'.format(ipynb))
    global_ignores = []
    with open(ipynb) as f:
        nb = formatter.reads_json(f.read())
    log('test_notebook: create KernelManager', level=2)
    km = KernelManager()
    # Do not save the history to disk, as it can yield spurious lock errors.
    # See https://github.com/ipython/ipython/issues/2845
    log('test_notebook: start_kernel', level=2)
    km.start_kernel(extra_arguments=['--HistoryManager.hist_file=:memory:'],
                    stderr=subprocess.DEVNULL)
    kc = km.client()
    log('test_notebook: start_channels', level=2)
    kc.start_channels()
    try:
        log('test_notebook: wait_for_ready', level=2)
        kc.wait_for_ready()
        log('test_notebook: wait_for_ready: done', level=2)
    except RuntimeError as e:
        SKIP('cannot start Jupyter kernel:', repr(e))
        exit(0)
    nerror = 0
    for i, cell in enumerate(nb['cells']):
        if cell['cell_type'] != 'code':
            continue
        # i counts all the cells (included those without code), n
        # counts only the executable cells.
        n = cell['execution_count']
        print('cell [{}] ({}): '.format(n, i))
        source = cell['source']
        # `%timeit`s shall count in execution count but its result
        # cannot be compared.
        if source.startswith('%timeit'):
            run_cell(kc, 'pass')
            continue
        if (('VCSN_SEED' in source or 'vcsn.setenv(SEED=1)' in source)
            and not is_libcpp()):
            SKIP('random number generation not on libc++')
            # Of course, we can't run the remainder as we certainly
            # have skipped definitions used later in the notebook.
            exit(0)
        # Adjust the paths to files used in the notebooks.
        source = source.replace('../../tests/demo',
                                os.environ['abs_top_srcdir'] + '/tests/demo')
        # Check if there are neutralization patterns to apply.
        global_ignores += re.findall('^# global ignore: (.*)$', source, re.M)
        ignores = global_ignores + re.findall('^# ignore: (.*)$', source, re.M)
        log('Ignores: ', ignores)
        try:
            outs = run_cell(kc, source)
        except Empty:
            print('Failed to run cell [{}] ({}):'.format(n, i),
                  '    Kernel Client is Empty; this is most likely due to a',
                  '    timeout issue. Check with `vcsn ps` or run the notebook',
                  '    manually, then retry.', sep='\n')
            print('Source was:\n', source)
            FAIL('failed to run cell [{}]'.format(n))
            nerror += 1
            continue
        except RuntimeError as e:
            print('Failed to run cell [{}] ({}):'.format(n, i), repr(e))
            print('Source was:', source, sep='\n')
            FAIL('failed to run cell [{}]'.format(n))
            nerror += 1
            continue
        if re.search('^# ignore cell$', source, re.M):
            SKIP('ignore cell request')
            continue
        check_outputs(cell.outputs, outs, ignores)
    print("Tested notebook {}".format(ipynb))
    print("    {:3} cells successfully replicated".format(num_pass()))
    if num_fail():
        print("    {:3} cells mismatched output".format(num_fail()))
    if nerror:
        print("    {:3} cells failed to complete".format(nerror))
    if num_test() == 0:
        # The TAP protocol does not like empty test suite.
        PASS('no test')

    log('test_notebook: stop_channels', level=2)
    kc.stop_channels()
    log('test_notebook: shutdown_kernel', level=2)
    km.shutdown_kernel()
    del km
    log('test_notebook: return', level=2)
    return False if nfail or nerror else True


if __name__ == '__main__':
    p = argparse.ArgumentParser(
        description='Check IPython notebooks.',
        epilog=r'''Cells may use some special comments:

        `# ignore cell` -- will cause the cell to be run, but
        its output will not be checked.

        `# ignore: RE` -- will cause each match of the regex
        RE to be neutralized to a constant string.  Use this for instance
        for timers `# ignore: \d+ms`.  Of course this is easier if the output
        is easy to recognize, here thanks to the unit `ms`.

        `# global ignore: RE` -- same as above, but applies to all the
        following cells.
        '''
    )
    opt = p.add_argument
    opt('--tap', action='store_true', help='enable TAP mode')
    opt('-v', '--verbose', help='be more verbose', action='count', default=0)
    opt('notebooks', nargs='+', help='IPython notebook to check')
    args = p.parse_args()
    verbose = args.verbose

    # Set the locale to something simple, so that we don't have
    # surprises on translated error messages.
    os.environ['LC_MESSAGES'] = 'C'

    success = True
    for ipynb in args.notebooks:
        success &= test_notebook(ipynb)
    log('done, exiting')
    sys.exit(0 if success or args.tap else 1)
