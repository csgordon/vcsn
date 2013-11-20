#include <vcsn/factory/divkbaseb.hh>
#include <vcsn/dyn/algos.hh>
#include <lib/vcsn/algos/registry.hh>

namespace vcsn
{
  namespace dyn
  {

    REGISTER_DEFINE(divkbaseb);

    automaton
    divkbaseb(const context& ctx, unsigned k, unsigned b)
    {
      return detail::divkbaseb_registry().call(ctx, k, b);
    }
  }
}
