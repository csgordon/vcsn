#ifndef VCSN_ALGOS_LEFT_MULT_HH
# define VCSN_ALGOS_LEFT_MULT_HH

# include <vcsn/dyn/automaton.hh> // dyn::make_automaton
# include <vcsn/dyn/weight.hh>

namespace vcsn
{

  /*-----------.
  | left-mult  |
  `-----------*/

  template <class Aut>
  Aut&
  left_mult_here(Aut& res, const typename Aut::context_t::weight_t& w)
  {
    assert(is_standard(res));

    using automaton_t = Aut;
    using context_t = typename automaton_t::context_t;
    using weightset_t = typename context_t::weightset_t;
    using state_t = typename automaton_t::state_t;

    weightset_t ws(*res.context().weightset());
    state_t initial = res.dst_of(res.initial_transitions().front());

    if (!ws.is_one(w))
      for (auto t: res.all_out(initial))
        res.lmul_weight(t, w);
    return res;
  }

  template <class Aut>
  Aut
  left_mult(const Aut& aut, const typename Aut::context_t::weight_t& w)
  {
    auto res = copy(aut);
    left_mult_here(res, w);
    return res;
  }

  namespace dyn
  {
    namespace detail
    {
      /*-----------------.
      | dyn::left_mult.  |
      `-----------------*/

      template <typename Aut, typename WeightSet>
      automaton
      left_mult(const automaton& aut, const weight& weight)
      {
        const auto& a = aut->as<Aut>();
        const auto& w = weight->as<WeightSet>().weight();
        return make_automaton(a.context(), left_mult(a, w));
      }

      using left_mult_t =
        auto (const automaton& aut, const weight& weight) -> automaton;
      bool left_mult_register(const std::string& lctx, const std::string& rctx,
                              left_mult_t fn);
    }
  }
}

#endif // !VCSN_ALGOS_LEFT_MULT_HH
