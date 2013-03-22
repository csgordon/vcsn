#include <iostream>
#include <stdexcept>

#include <vcsn/dyn/algos.hh>

#include "parse-args.hh"

struct product: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    automaton lhs = read_automaton(opts);
    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    automaton rhs = read_automaton(opts2);

    // Process.
    auto res = vcsn::dyn::product(lhs, rhs);

    // Output.
    print(opts, res);
    return 0;
  }
};

int main(int argc, char* const argv[])
{
  return vcsn_main(argc, argv, product{});
}
