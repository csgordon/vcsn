#include <vcsn/algos/dyn.hh>
#include <vcsn/algos/lift.hh>
#include <lib/vcsn/algos/registry.hh>

namespace vcsn
{

  namespace dyn
  {
    namespace details
    {
      Registry<lift_t>&
      lift_registry()
      {
        static Registry<lift_t> instance{"lift"};
        return instance;
      }

      bool lift_register(const std::string& ctx, const lift_t& fn)
      {
        return lift_registry().set(ctx, fn);
      }
    }

    automaton
    lift(const automaton& aut)
    {
      return details::lift_registry().call(aut->vname(),
                                           aut);
    }
  }
}
