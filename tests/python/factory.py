#! /usr/bin/env python

# Check the factories (of automata, expressions, etc.)
import re
import vcsn
from test import *

def get_labels(aut):
    'A set of the labels used in aut.'
    return set(re.findall(r'\w+ -> \w+ (.*)$', a.format('daut'), re.M))
vcsn.automaton.labels = get_labels

def check_is_deterministic(exp, aut, value):
    CHECK_EQ(exp, aut)
    CHECK_EQ(value, aut.info('is deterministic'))

## ------- ##
## cerny.  ##
## ------- ##

a = vcsn.context('lal(abc), b').cerny(6)
a_info = a.info()
CHECK_EQ(6, a_info['number of states'])
CHECK_EQ(True, a_info['is deterministic'])
CHECK_EQ(meaut('cerny-6.gv'), a)

## ----------- ##
## de_bruijn.  ##
## ----------- ##

def check_de_bruijn(exp, aut):
    check_is_deterministic(exp, aut, False)

check_de_bruijn(meaut('de-bruijn-2.gv'),
                vcsn.context('lal(ab), b').de_bruijn(2))

check_de_bruijn(meaut('de-bruijn-3.gv'),
                vcsn.context('lal(xyz), b').de_bruijn(3))

## ---------------- ##
## div/quotkbaseb.  ##
## ---------------- ##

b = vcsn.context('lal(0-9), b')

XFAIL(lambda: b.divkbaseb(0, 2), "divkbaseb: divisor cannot be 0")
XFAIL(lambda: b.divkbaseb(2, 0), "divkbaseb: base (0) must be at least 2")
XFAIL(lambda: b.divkbaseb(2, 1), "divkbaseb: base (1) must be at least 2")
XFAIL(lambda: b.divkbaseb(2, 11),
      "divkbaseb: base (11) must be less than " +
      "or equal to the alphabet size (10)")

b2 = vcsn.context('lat<lal(0-9), lal(0-2)>, b')

XFAIL(lambda: b2.quotkbaseb(0, 2), "quotkbaseb: divisor cannot be 0")
XFAIL(lambda: b2.quotkbaseb(2, 1), "quotkbaseb: base (1) must be at least 2")
XFAIL(lambda: b2.quotkbaseb(2, 4),
      "quotkbaseb: base (4) must be less than " +
      "or equal to the right alphabet size (3)")
XFAIL(lambda: b2.quotkbaseb(2, 11),
      "quotkbaseb: base (11) must be less than " +
      "or equal to the left alphabet size (10)")

b3 = vcsn.context('lat<lal(0-9), lal(0-9)>, b')

def check(k, base, exp):
    a = b3.quotkbaseb(k, base)
    CHECK_EQ(exp, a.shortest(10))
    CHECK_EQ(a.project(0), b3.project(0).divkbaseb(k, base))

# FIXME: we don't parse polynomials yet.
check(2, 2,
      r'\e|\e + 0|0 + 00|00 + 10|01 + 000|000 + 010|001 + 100|010 + 110|011 + 0000|0000 + 0010|0001')
check(10, 10,
      r'\e|\e + 0|0 + 00|00 + 10|01 + 20|02 + 30|03 + 40|04 + 50|05 + 60|06 + 70|07')
check(5, 10,
      r'\e|\e + 0|0 + 5|1 + 00|00 + 05|01 + 10|02 + 15|03 + 20|04 + 25|05 + 30|06')
check(3, 10,
      r'\e|\e + 0|0 + 3|1 + 6|2 + 9|3 + 00|00 + 03|01 + 06|02 + 09|03 + 12|04')



## ------------- ##
## double_ring.  ##
## ------------- ##

ctx = vcsn.context('lal(abcd), b')
CHECK_EQ(ctx.double_ring(0, []),
         vcsn.automaton('''
digraph
{
  vcsn_context = "lal(abcd), b"
  rankdir = LR
}
'''))

CHECK_EQ(ctx.double_ring(1, [0]),
         meaut('double-ring-1-0.gv'))

CHECK_EQ(ctx.double_ring(4, [2, 3]),
         meaut('double-ring-4-2-3.gv'))


## ---------- ##
## ladybird.  ##
## ---------- ##

def check_ladybird(exp, aut):
    check_is_deterministic(exp, aut, False)

b = vcsn.context('lal(abc), b')
z = vcsn.context('lal(abc), z')

exp = meaut('ladybird-2.gv')
check_ladybird(exp, b.ladybird(2))

exp = vcsn.automaton(str(exp).replace(', b', ', z'))
check_ladybird(exp, z.ladybird(2))

check_ladybird(meaut('ladybird-2-zmin.gv'),
               vcsn.context('lal(abc), zmin').ladybird(2))


## ------------- ##
## levenshtein.  ##
## ------------- ##

nmin = vcsn.context('lat<lal(abc), lal(bcd)>, nmin')
CHECK_EQ(meaut('levenshtein.gv'), nmin.levenshtein())


## ------------------ ##
## random_automaton.  ##
## ------------------ ##

# Expect a clique.
c1 = vcsn.context('lal(), b').random_automaton(4, 1, 4, 4)
c2 = meaut('clique-a-4.gv')
CHECK_EQ(c1, c2)


# Expect the right number of states.
a = vcsn.context('lal(a), b').random_automaton(100, .1, 20, 30)
CHECK_EQ('mutable_automaton<letterset<char_letters(a)>, b>',
         a.info('type'))
CHECK_EQ(100, a.info('number of states'))
CHECK_EQ(20, a.info('number of initial states'))
CHECK_EQ(30, a.info('number of final states'))


# For a while, we were only able to get matching letters (a|A, b|B,
# etc.).  Don't use too many labels (max_labels), as it results in
# label classes, unrecognized by a.labels().
ctx = vcsn.context('lat<lal(abc), lal(abc)>, b')
a = ctx.random_automaton(num_states=100, density=1, max_labels=2)
# Get all the labels.
print("random_automaton: {:d}".format(a))
labels = a.labels()
print("labels: {}".format(labels))
# Make sure there are \e|a and a|\e.
CHECK_NE([l for l in labels if re.match(r'\\e\|[abc]', l)], [])
CHECK_NE([l for l in labels if re.match(r'[abc]\|\\e', l)], [])
# Make sure there are a|b labels.
CHECK_NE([l for l in labels
          if (re.match(r'[abc]\|[abc]', l)
              and not re.match(r'([abc])\|\1', l))],
         [])
# Make sure there are \e|\e labels.
CHECK(r'\e|\e' in labels)


# Check that we check that max_labels is <= number of generators.
# lal has one so the maximum value for max_labels is 2.
XFAIL(lambda: vcsn.context('lal(a), b').random_automaton(2, max_labels=3),
      "random_automaton: max number of labels cannot be greater than "
      "the number of generators")

# Check that max_labels is honored.
ctx = vcsn.context('lal(a-z), b')
a = ctx.random_automaton(num_states=10, max_labels=5, density=1)
labels = a.labels()
print("random_automaton: {:d}".format(a))
print("labels: {}".format(labels))
# Expect from 1 to 5 labels per entry.
for n in range(1, 7):
    if n == 1:
        pat = r'^(\\e|[a-z])$'
    elif n == 2:
        pat = r'^((\\e|[a-z]), [a-z])$'
    elif n == 3:
        pat = r'^(((\\e|[a-z])(, [a-z]){2})|(\[[a-z-]{3}\]))$'
    else:
        pat = r'^((\\e, \[[a-z-]{{{}}}\])|(\[[a-z-]{{{}}}\]))$'.format(n - 1, n)
    ls = [l for l in labels if re.match(pat, l)]
    print("Number of labels:", n, ls)
    if n == 6:
        CHECK_EQ([], ls)
    else:
        CHECK_NE([], ls)

# Check that we do generate weights on transitions
ctx = vcsn.context('lal(a), z')
# Generating an automaton with new lal can introduce spontaneous cycle and
# evaluate will never terminate.
a = ctx.random_automaton(num_states=2, loop_chance=0, weights='5=1')
print("random_automaton: {:d}".format(a))
if a.is_eps_acyclic():
    CHECK(a('a') != ctx.weight_one())
else:
    SKIP("Cannot evaluate random automaton, require is_eps_acyclic")


## ------------------- ##
## random_expression.  ##
## ------------------- ##

def randexp(c, *args, **kwargs):
    '''Generate a random expression.'''

    if not isinstance(c, vcsn.context):
        c = vcsn.context(c)
    res = c.random_expression(*args, **kwargs)
    print('randexp:', res)
    return res

operators = [
    'add',
    'atom',
    'complement',
    'compose',
    'conjunction',
    'infiltrate',
    'ldivide',
    'lweight',
    'mul',
    'one',
    'rweight',
    'shuffle',
    'star',
    'transposition',
    'tuple',
    'zero'
]

def check_operators(e, ops):
    info = e.info()
    print('Info:', info)
    ks = [k for k in operators if info[k] != 0]
    CHECK_EQ(ops, ks)

# A random expression without any operator is an error.
XFAIL(lambda: randexp('lal(a-z), b', 'l=0'))

# Check that operators are present only if the user has specified them.
exp = randexp('lal(a), b',
              '+=1, *=0.5, {c}=1, {\\}=0', length=100, identities='none')
check_operators(exp, ['add', 'atom', 'complement', 'star'])

# Check the length.
for _ in range(10):
    exp = randexp('lal(a), b',
                  '+, *=.1', length=15, identities='none')
    CHECK_EQ(15, exp.info('length'))

# Check rweight and lweight.
exp = randexp('lal(abc), b',
              '+=1, w.=1', length=50, identities='none')
check_operators(exp, ['add', 'atom', 'lweight'])


# Check the weight generation on expression.
exp = randexp('lal(abc), b',
              '+, w., w="1=1"', length=20, identities='none')
CHECK_NE(str(exp).find('<1>'), -1)
CHECK_EQ(str(exp).find('<0>'), -1)

# Check `w=specs`.
exp = randexp('lal(abc), z',
              'w., .w, w="min=-10, max=10"', length=100, identities='none')
for m in re.findall('<(.*?)>', str(exp)):
    CHECK(-10 <= int(m))
    CHECK(int(m) <= 10)


# Check that we generate valid expressions on multitape contexts.
for i in range(100):
    randexp('lat<lan(abc), lan(abc)>, q',
            '+,*,.', length=20)
# FIXME: We did not ask for tuple, but we will have some anyway, as
# when we generate multitape labels, we pretty-print them, which turns
# them into multitape expressions. And with identities = none, that's
# all we have left: tupling operator on single-tape labels.
exp = randexp('lat<lan(abc), lan(abc)>, q',
              '@,+,*,.,\e', length=1000, identities='none')
check_operators(exp, ['add', 'atom', 'compose', 'mul', 'one', 'star', 'tuple'])

# Check that we use | correctly.
for i in range(100):
    randexp('lat<lan(abc), lan(abc)>, q',
            '+,*=.2,.,|,@', length=20)
# Since @ can only appear on top of |, it is less likely to appear.
# So make a very large expression.
exp = randexp('lat<lan(abc), lan(abc)>, q',
              '+,*=.2,.,|=.1,@=10,\e', length=2000, identities='none')
check_operators(exp, ['add', 'atom', 'compose', 'mul', 'one', 'star', 'tuple'])


## ---------------------- ##
## random_deterministic.  ##
## ---------------------- ##

a = vcsn.context('lal(a), b').random_deterministic(100)
CHECK_EQ('mutable_automaton<letterset<char_letters(a)>, b>',
         a.info('type'))
print('automaton {:d}'.format(a))
CHECK_EQ(100, a.info('number of states'))
CHECK_EQ(1, a.info('number of initial states'))
CHECK_EQ(1, a.info('number of final states'))
CHECK(a.is_complete())



## --- ##
## u.  ##
## --- ##

CHECK_EQ(meaut('u-5.gv'),
         vcsn.context('lal(abc), b').u(5))
