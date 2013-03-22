#include <iostream>
#include <stdexcept>

#include <vcsn/dyn/algos.hh>

#include "parse-args.hh"

struct accessible: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);

    // Process.
    auto res = vcsn::dyn::accessible(aut);

    // Output.
    print(opts, res);
    return 0;
  }
};

int main(int argc, char* const argv[])
{
  return vcsn_main(argc, argv, accessible{});
}
