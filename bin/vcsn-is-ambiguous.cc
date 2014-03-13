#include <iostream>
#include <stdexcept>

#include <vcsn/dyn/algos.hh>

#include "parse-args.hh"

struct is_ambiguous: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);

    // Process.
    bool res = vcsn::dyn::is_ambiguous(aut);

    // Output.
    opts.print(res);
    return res ? 0 : 2;
  }
};

int main(int argc, char* const argv[])
{
  return vcsn_main(argc, argv, is_ambiguous{});
}
