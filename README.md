Vcsn
====

[Vcsn](http://vcsn.lrde.epita.fr) is a platform for weighted automata and
rational expressions.

It is composed of an efficient C++ 17 generic library, shell tools, Python
bindings, and a graphical interactive environment on top of Jupyter/IPython.

Copyright (C) 2012-2018 The Vcsn Group.

Overview
--------

The Vcsn platform enables the development of C++ programs manipulating
weighted finite automata in an abstract and general way with, at the same
time, a large specialization power.  On the one hand, we can write
algorithms working on every automaton with weights in any semiring and with
words from any free monoids.  And on the other hand, a particular algorithm
can be specialized for a particular data structure.

The Python bindings, and especially the IPython interface, make Vcsn a tool
particularly well suited for practical sessions in courses of Formal
Language Theory.  More generally, it proves to be a handy means to explore
compositions of algorithms on automata from small sizes to "real world"
cases.

Although it is now quite mature, Vcsn is an ongoing development
project. Therefore some algorithms and data structures may change in the
future.

Please send any question or comments to <vcsn@lrde.epita.fr>, and report
bugs to either our issue tracker
<https://gitlab.lrde.epita.fr/vcsn/vcsn/issues>, or via emails to
<vcsn@lrde.epita.fr>.

Using Vcsn
----------

Documentation about Vcsn can be found in several places:

- [The Python interface](http://vcsn-sandbox.lrde.epita.fr/tree/Doc) is
  available online.  In particular, be sure to read the [introduction to
  Vcsn](http://vcsn-sandbox.lrde.epita.fr/notebooks/Doc/!Read-me-first.ipynb).

- This documentation is also available in the directory `doc/notebooks`.
  Once Vcsn installed, run `vcsn doc automaton.determinize` for example.

- The file `NEWS.md` includes many examples of how to run commands and
    algorithms.

- The directory `tests/python` contains tons of test cases written in
  Python.

- The [documentation of the C++ low-level
  interface](https://www.lrde.epita.fr/dload/vcsn/latest/vcsn.htmldir/) is
  generated by Doxygen from the comments in the header files
  (`vcsn/algos/*.hh`).  To generate the C++ documentation, run `make
  doxygen`.  For a given algorithm it is useful to read the Python
  documentation, since it provides many examples.

- The dyn:: C++ interface is documented in `vcsn/dyn/algos.hh`.  Look for
  the namespace `vcsn/dyn` in the Doxygen generated documentation.


Quick Start
-----------

Vcsn must be installed to be used.  Once you installed it, here are a few
commands to help you start using it.

- `vcsn python` or `vcsn ipython`

  Start a Python/IPython session.  Then, for instance:

    $ vcsn ipython
    Python 3.6.4 (default, Dec 21 2017, 20:33:21)
    Type 'copyright', 'credits' or 'license' for more information
    IPython 6.2.1 -- An enhanced Interactive Python. Type '?' for help.

    In [1]: import vcsn

    In [2]: vcsn.B.expression('[ab]{4}a[ab]').standard().determinize().num_states()
    Out[2]: 12

- `vcsn notebook`

  Start a Jupyter notebook in a web browser.  Then you can run the same
  Python commands as above.  Requires that you have installed IPython.

- `vcsn doc`

  Opens the documentation.

- `vcsn diagnose`

  Checks that Vcsn is installed properly, and generates diagnostics.

- `vcsn --help`

  List the available commands.


Installation
------------

To install Vcsn on your system, type in the classical sequence at the
command prompt:

    ./configure
    make
    make install (as root)

Do not hesitate to run `make -j3` if, for instance, your CPU features 4
threads.  To enable the generation of the Doxygen documentation, pass
`--enable-doxygen` to configure.

Note that an installation is specific to the compiler used to install
it. Indeed, the call to `./configure` enables some workarounds and,
consequently, users must compile with the same compiler to avoid
compatibility problems.

Between `make` and `make install`, you may also want to run:

    make check

It run the test suite to check the whole platform. Beware that checking
Vcsn is a very long process, also consider `-j3`.

### Build Requirements

#### Packages needed

Vcsn was tested with the [GNU Compiler Collection (GCC)](http://gcc.gnu.org)
versions 7 and [Clang](http://clang.llvm.org) 5 and 6.

[Boost](http://www.boost.org) is a C++ library which provides many useful
features.  You must install this library on your system.  Vcsn should
support any version after 1.49.  The following Boost components are used:

- Boost.Algorithm
- Boost.DynamicBitset
- Boost.IOStreams
- Boost.Filesystem
- Boost.Flyweight
- Boost.Heap
- Boost.Iterator
- Boost.Python
- Boost.Range
- Boost.Regex
- Boost.System
- Boost.Tokenizer

[Ccache](http://ccache.samba.org) saves the user from repeated compilations.

To load plugins, Vcsn relies on libltdl, which is a component of the [GNU
Libtool](http://www.gnu.org/software/libtool/) project.  Depending on your
distribution/packaging system, you may have to install `libltdl-dev` (e.g.,
Debian) or `libtool` (MacPorts).

Vcsn uses the Dot format to save automaton in a human readable file. You
should install [Graphviz](http://www.research.att.com/sw/tools/graphviz) to
visualize these `.gv` files.

To provide safe support for ℚ, Vcsn relies on [The GNU Multiple Precision
Arithmetic Library](https://gmplib.org).

[Doxygen](http://doxygen.org) is used to generate the C++ reference
documentation.

[yaml-cpp](https://github.com/jbeder/yaml-cpp) is used to handle the
configuration files.  Beware that version 0.5.2 is buggy and will not work
properly.  Use 0.5.1, or 0.5.3 or more recent.

#### Ubuntu/Debian packages

Please, help us keep this list up-to-date!

  ccache
  dot2tex
  g++
  graphviz
  imagemagick
  ipython3-notebook
  libboost-all-dev
  libgmp-dev
  libzmq3-dev
  locales
  pdf2svg
  python3-colorama
  python3-dev
  python3-matplotlib
  python3-pandas
  python3-pip
  python3-psutil
  python3-regex
  python3-setuptools
  libyaml-cpp-dev

Vcsn expects to be built and to run in an UTF-8 environment.  This requires
the `locales` package, and that en_US.UTF-8 be supported:

    sudo 'echo "en_US.UTF-8 UTF-8" >>/etc/locale.gen'
    sudo locale-gen
    export LANG=en_US.UTF-8   \
           LANGUAGE=en_US:en  \
           LC_ALL=en_US.UTF-8

#### MacPorts
First install these packages:

    sudo port install  boost +python36  py36-notebook  python36

Make sure that Boost is configured to be used with Python 3.6:

    $ port variant boost | grep -F +
    [+]no_single: Disable building single-threaded libraries
    [+]no_static: Disable building static libraries
    (+)python36: Build Boost.Python for Python 3.6

Then install Vcsn.

    sudo port install vcsn

### Libraries installed in non-standard directories

If you have installed Boost in a non-standard directory (i.e., a directory
that is not searched by default by your C++ compiler), you will have to set
the `CPPFLAGS` and `LDFLAGS` variables to pass the necessary `-I` and `-L`
options to the preprocessor and linker.

For instance if you installed Boost in `/opt/boost/` you should run
`./configure` as follows:

    ./configure CPPFLAGS="-I/opt/boost" LDFLAGS="-L/opt/boost"

### Layout of the tarball

The project directory layout is as follows:

build-aux
:   Auxiliary tools used by the GNU Build System during `configure` and
    `make` stages.

doc
:   Doxygen documentation, and IPython notebooks.

share
:   Data files to be installed on your system.

lib
:   Various libraries, including instantiation of some contexts.

vcsn
:   The Vcsn C++ Library headers.

python
:   The Python binding.

bin
:   Various programs to install. In particular the program vcsn, which
    provides access to all the other programs. See vcsn --help.

tests
:   The test suites.

Starting from the repository
----------------------------

To contribute to Vcsn, or to build it from its Git repository, you need
more tools:

- Automake 1.14 or newer
- Autoconf 2.69 or newer
- Bison 3.0.4 or newer
- Flex 2.5.35 or newer

Before the configuration steps, run:

    ./bootstrap

to set up the GNU Build system.

### Ubuntu

In addition of the packages above, you need:

autoconf automake libtool flex bison

License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option)
any later version.

The complete GNU General Public License Notice can be found as the
`COPYING.txt` file in the root directory.

Contacts
--------

The team can be reached by mail at <vcsn@lrde.epita.fr>.  Snail mail
address:

- Vcsn - LRDE

  Akim Demaille & Alexandre Duret-Lutz\
  Laboratoire de Recherche et Développement de l'EPITA\
  14-16 rue Voltaire\
  94276 Le Kremlin-Bicêtre CEDEX\
  France


<!--

LocalWords:  Vcsn Vaucanson Sakarovitch Télécom ParisTech EPITA LRDE automata
LocalWords:  semiring monoids Vcsn's txt vcsn algos hh dyn GCC DynamicBitset
LocalWords:  Regex Tokenizer Ccache libltdl Libtool dev libtool Graphviz gv de
LocalWords:  Doxygen CPPFLAGS LDFLAGS preprocessor IPython instantiation Akim
LocalWords:  Automake Autoconf Demaille Alexandre Duret Lutz Laboratoire et
LocalWords:  Développement l'EPITA Bicêtre CEDEX rst API ipynb ispell md
LocalWords:  Sylvain american Jupyter determinize doxygen namespace ipython
LocalWords:  IOStreams Filesystem yaml cpp ccache tex graphviz libgmp
LocalWords:  imagemagick libboost libzmq pdf svg colorama matplotlib
LocalWords:  psutil setuptools libyaml UTF sudo autoconf automake utf

Local Variables:
coding: utf-8
ispell-dictionary: "american"
fill-column: 76
mode: markdown
End:

-->
