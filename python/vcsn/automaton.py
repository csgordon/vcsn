## ----------- ##
## automaton.  ##
## ----------- ##

import re
from vcsn_cxx import automaton, label, weight
from vcsn import is_equal, info_to_dict, dot_to_svg

def automaton_mul(self, rhs):
    """Support both "aut * aut" and "aut * weight"."""
    if isinstance(rhs, weight):
        return self.right_mult(rhs)
    else:
        return self.concatenate(rhs)

def one_epsilon(s):
    s = re.sub(r'\\\\e', '&epsilon;', s)
    return s

automaton.__eq__ = lambda self, other: str(self) == str(other)
automaton.__add__ = automaton.sum
automaton.__and__ = lambda l, r: conjunction(l, r)
automaton.__invert__ = automaton.complement
automaton.__mul__ = automaton_mul
automaton.__mod__ = automaton.difference
automaton.__or__ = automaton.union
automaton.__pow__ = automaton.power
automaton.__repr__ = lambda self: self.info()['type']
automaton.__str__ = lambda self: self.format('dot')
automaton.__sub__ = automaton.difference
automaton._repr_svg_ = lambda self: dot_to_svg(one_epsilon(self.format('dot')))

class conjunction(object):
    """A proxy class that delays calls to the & operator in order
    to turn a & b & c into a variadic evaluation of
    automaton.product_real(a, b, c)."""
    def __init__(self, *auts):
        self.auts = auts
    def __and__(self, aut):
        self.auts += (aut,)
        return self
    def value(self):
        if isinstance(self.auts, tuple):
            self.auts = automaton.product_real(self.auts)
        return self.auts
    def __nonzero__(self):
        return bool(self.value())
    def __str__(self):
        return str(self.value())
    def __repr__(self):
        return repr(self.value())
    def __getattr__(self, name):
        return getattr(self.value(), name)
    def __hasattr__(self, name):
        return hasattr(self.value(), name)

def automaton_eval(self, w):
    c = self.context()
    if not isinstance(w, label):
        w = c.word(str(w))
    return self.eval_(w)
automaton.eval = automaton_eval

def automaton_load(file, format = "dot"):
    return automaton(open(file, "r").read(), format)
automaton.load = staticmethod(automaton_load)

def automaton_fst(aut, cmd):
    open("/tmp/in.efsm", "w").write(aut.format("efsm"))
    from subprocess import check_call, check_output
    check_call(['efstcompile',   '/tmp/in.efsm', '/tmp/in.fst'])
    check_call([cmd,             '/tmp/in.fst',  '/tmp/out.fst'])
    res = check_output(['efstdecompile', '/tmp/out.fst'])
    return automaton(res, "efsm")

automaton.fstdeterminize = lambda self: automaton_fst(self, "fstdeterminize")
automaton.fstminimize = lambda self: automaton_fst(self, "fstminimize")

automaton.info = lambda self: info_to_dict(self.format('info'))

def automaton_is_synchronized_by(self, w):
    c = self.context()
    if not isinstance(w, label):
        w = c.word(str(w))
    return self.is_synchronized_by_(w)
automaton.is_synchronized_by = automaton_is_synchronized_by

automaton.lan_to_lal = \
  lambda self: automaton(re.sub(r'"lan<(lal_char\(.*?\))>', r'"\1',
                         self.format('dot')), 'dot')

# Somewhat cheating: in Python, proper returns a LAL, not a LAN.
# proper_real is the genuine binding to dyn::proper.
automaton.proper = lambda self, prune = True: self.proper_real(prune).lan_to_lal()
