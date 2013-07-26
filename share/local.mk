# Vaucanson 2, a generic library for finite state machines.
# Copyright (C) 2013 Vaucanson Group.
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

lal_char_bdir = $(pkgdatadir)/lal_char_b
dist_lal_char_b_DATA =                          \
  share/vcsn/lal_char_b/a1.gv                   \
  share/vcsn/lal_char_b/b1.gv                   \
  share/vcsn/lal_char_b/evena.gv                \
  share/vcsn/lal_char_b/oddb.gv

lal_char_zdir = $(pkgdatadir)/lal_char_z
dist_lal_char_z_DATA =                          \
  share/vcsn/lal_char_z/b1.gv                   \
  share/vcsn/lal_char_z/binary.gv               \
  share/vcsn/lal_char_z/c1.gv                   \
  share/vcsn/lal_char_z/d1.gv

lal_char_zmindir = $(pkgdatadir)/lal_char_zmin
dist_lal_char_zmin_DATA =                       \
  share/vcsn/lal_char_zmin/minab.gv             \
  share/vcsn/lal_char_zmin/minblocka.gv         \
  share/vcsn/lal_char_zmin/slowgrow.gv
