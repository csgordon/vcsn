# Vaucanson 2, a generic library for finite state machines.
# Copyright (C) 2012-2013 Vaucanson Group.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# The complete GNU General Public Licence Notice can be found as the
# `COPYING' file in the root directory.
#
# The Vaucanson Group consists of people listed in the `AUTHORS' file.

tafkit_TESTS =                                  \
  tafkit/accessible.chk                         \
  tafkit/aut_to_exp.chk                         \
  tafkit/cat.chk                                \
  tafkit/de-bruijn.chk                          \
  tafkit/determinize.chk                        \
  tafkit/eps-removal.chk                        \
  tafkit/eval.chk                               \
  tafkit/is-deterministic.chk                   \
  tafkit/is-deterministic-fail.chk              \
  tafkit/is_complete.chk                        \
  tafkit/ladybird.chk                           \
  tafkit/lift.chk                               \
  tafkit/product.chk                            \
  tafkit/standard_of.chk
dist_TESTS += $(tafkit_TESTS)

XFAIL_TESTS += tafkit/is-deterministic-fail.chk

.PHONY: check-tafkit
check-tafkit:
	$(MAKE) $(AM_MAKEFLAGS) check TESTS='$(tafkit_TESTS)'
