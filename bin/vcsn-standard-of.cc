#include <iostream>
#include <cassert>

#include <vcsn/algos/dyn.hh>
#include "parse-args.hh"

int main(int argc, char * const argv[])
{
  auto opts = parse_args(&argc, &argv);

  // Input.
  using namespace vcsn::dyn;
  if (!opts.is_automaton)
    {
      vcsn::dyn::context* ctx = make_context(opts.context,
                                             opts.labelset_describ);
      vcsn::dyn::ratexp exp =read_ratexp_string(opts.file, *ctx,
                                                opts.input_format);

      // Process.
      vcsn::dyn::automaton aut = standard_of(exp);

      // Output.
      print(aut, std::cout, opts.output_format) << std::endl;
    }
}
