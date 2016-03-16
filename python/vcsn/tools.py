import tempfile
from vcsn_cxx import weight


def _extend(*classes):
    """
    Decorator that extends all the given classes with the contents
    of the class currently being defined.
    """
    def wrap(this):
        for cls in classes:
            for (name, val) in this.__dict__.items():
                if name not in ('__dict__', '__weakref__') \
                   and not (name == '__doc__' and val is None):
                    setattr(cls, name, val)
        return classes[0]
    return wrap


def _info_to_dict(info):
    '''Convert a "key: value" list of lines into a dictionary.
    Convert Booleans into bool, and likewise for integers.
    '''
    res = dict()
    for l in info.splitlines():
        (k, v) = l.split(':', 1)
        v = v.strip()
        # Beware that we may display "N/A" for Boolean (e.g., "is
        # ambiguous" for non LAL), and for integers (e.g., "number of
        # deterministic states" for non LAL).
        try:
            # Don't convert "0" and "1", which are used for false and
            # true, as integer: special case categories that start
            # with "is ".
            if k.startswith('is '):
                if v in ['false', '0']:
                    v = False
                elif v in ['true', '1']:
                    v = True
            # Otherwise, if it passes the conversion into a number,
            # make it a number.
            else:
                v = int(v)
        except:
            pass
        res[k] = v
    return res


def _format(self, spec, syntax, syntaxes):
    """Format `self` weight according to `spec`.

    Parameters
    ----------
    spec : str, optional
        a list of letters that specify how the label
        should be formatted.

    """

    while spec:
        c, spec = spec[0], spec[1:]
        if c in syntaxes:
            syntax = syntaxes[c]
        elif c == ':':
            break
        else:
            raise ValueError("unknown format specification: " + c + spec)

    s = self.format(syntax)
    return s.__format__(spec)


# FIXME: Get rid of this.
def _is_equal(lhs, rhs):
    "A stupid string-based comparison.  Must be eliminated once we DRT."
    return isinstance(rhs, lhs.__class__) and str(lhs) == str(rhs)


def _left_mult(self, lhs):
    '''Support "aut * weight".  Also serves for expressions and expansions.'''
    return self.left_mult(self.context().weight(str(lhs)))


def _right_mult(self, rhs):
    '''Support both "aut * aut" and "aut * weight".  Also serves for
    expressions.'''
    if isinstance(rhs, type(self)):
        return self.multiply(rhs)
    elif isinstance(rhs, weight):
        return self.right_mult(rhs)
    else:
        return self.right_mult(self.context().weight(str(rhs)))


def _tmp_file(suffix, **kwargs):
    '''A NamedTemporaryFile suitable for Vcsn.'''
    return tempfile.NamedTemporaryFile(prefix='vcsn-',
                                       suffix=('.' + suffix),
                                       **kwargs)
