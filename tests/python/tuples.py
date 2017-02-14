#! /usr/bin/env python

import vcsn
from test import *

# FIXME: we should probably merge transducers.py in here.

def check(ctx, exp):
    c = vcsn.context(ctx)
    try:
        r = c.expression(exp)
    except RuntimeError:
        FAIL("error parsing " + exp)
    else:
        CHECK_EQ(exp, r)

check('lal_char(ab), lat<q, expressionset<lal_char(xyz), b>, z>',
      'a')
# Yes, once we worked properly with 3 tapes, but not 4.  So check 5.
check('lat<lan, lan, lan, lan, lan>, q',
      '1|2|3|4|5')
check('lal_char(ab), lat<q, expressionset<lal_char(xyz), b>, z>',
      '<2/3,x,-3>a')
check('lal_char(ab), lat<q, expressionset<lal_char(xyz), b>, z>',
      '<2/3,x+y*,-3>a')
check('lal_char(ab), lat<expressionset<lal_char(xyz), lat<q, q>>, lat<q, q>>',
      '<<(1,2)>x+<(2,1/3)>y*,(2,3)>a')


## ------------------ ##
## tuple(automaton).  ##
## ------------------ ##

ind = lambda e: vcsn.context('lan, q').expression(e).inductive()
a1 = ind('<2>[ab]')
a2 = ind('x')
CHECK_EQ('''context = lat<nullableset<letterset<char_letters(ab)>>, nullableset<letterset<char_letters(x)>>>, q
$ -> 0
0 -> 1 <2>a|x
0 -> 2 <2>b|x
1 -> $
2 -> $''', (a1 | a2).format('daut'))


## ------------------- ##
## tuple(expression).  ##
## ------------------- ##

# Regression: Make sure the expressions of one-tuple contexts are
# indeed one-tuple expressions.
c = vcsn.context('lat<lan(abc)>, b')
e = c.expression('[^]')
CHECK_EQ(c, e.context())

# FIXME: this shows that we really need to visit (in the sense of
# visitors) the context to parse it properly.
#
# c = vcsn.context('lat<lat<lan(abc)>>, b')
# e = c.expression('[^]')
# CHECK_EQ(c, e.context())
