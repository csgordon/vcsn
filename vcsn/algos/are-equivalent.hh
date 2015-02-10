#pragma once

#include <vcsn/algos/accessible.hh> // is_useless
#include <vcsn/algos/complement.hh>
#include <vcsn/algos/complete.hh>
#include <vcsn/algos/determinize.hh>
#include <vcsn/algos/left-mult.hh>
#include <vcsn/algos/product.hh>
#include <vcsn/algos/reduce.hh>
#include <vcsn/algos/strip.hh>
#include <vcsn/algos/union.hh>
#include <vcsn/dyn/automaton.hh>
#include <vcsn/dyn/expression.hh>

namespace vcsn
{

  /*---------------------------------------.
  | are_equivalent(automaton, automaton).  |
  `---------------------------------------*/

  /// Check equivalence between Boolean automata on a free labelset.
  template <typename Aut1, typename Aut2>
  auto
  are_equivalent(const Aut1& a1, const Aut2& a2)
    -> vcsn::enable_if_t<(labelset_t_of<Aut1>::is_free()
                          && std::is_same<weightset_t_of<Aut1>, b>::value
                          && labelset_t_of<Aut2>::is_free()
                          && std::is_same<weightset_t_of<Aut2>, b>::value),
                         bool>
  {
    return (is_useless(difference(a1, a2))
            && is_useless(difference(a2, a1)));
  }


  /// Check equivalence between automata on fields, or Z.
  template <typename Aut1, typename Aut2>
  auto
  are_equivalent(const Aut1& a1, const Aut2& a2)
    -> decltype((enable_if_t<!std::is_same<weightset_t_of<Aut1>, b>::value>(),
                 a2->weightset()->sub(a2->weightset()->zero(),
                                      a2->weightset()->one()),
                 true))
  {
    const auto& ws2 = *a2->weightset();
    // d = a1 U -a2.
    auto d = union_a(a1,
                     left_mult(ws2.sub(ws2.zero(), ws2.one()),
                               a2));
    return is_empty(reduce(d));
  }


  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename Aut1, typename Aut2>
      bool
      are_equivalent(const automaton& aut1, const automaton& aut2)
      {
        const auto& a1 = aut1->as<Aut1>();
        const auto& a2 = aut2->as<Aut2>();
        return ::vcsn::are_equivalent(a1, a2);
      }

      REGISTER_DECLARE(are_equivalent,
                       (const automaton&, const automaton&) -> bool);
    }
  }


  /*-----------------------------------.
  | difference(automaton, automaton).  |
  `-----------------------------------*/

  /// An automaton that computes weights of \a lhs, but not by \a rhs.
  template <typename Lhs, typename Rhs>
  typename Lhs::element_type::automaton_nocv_t
  difference(const Lhs& lhs, const Rhs& rhs)
  {
    // Meet complement()'s requirements.
    auto r = strip(rhs);
    if (!is_deterministic(r))
      r = complete(strip(determinize(r)));
    else if (!is_complete(r))
      r = complete(r);
    return strip(product(lhs, complement(r)));
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename Lhs, typename Rhs>
      automaton
      difference(const automaton& lhs, const automaton& rhs)
      {
        const auto& l = lhs->as<Lhs>();
        const auto& r = rhs->as<Rhs>();
        return make_automaton(::vcsn::difference(l, r));
      }

      REGISTER_DECLARE(difference,
                       (const automaton&, const automaton&) -> automaton);
    }
  }

  /*--------------------------------------.
  | difference(expression, expression).   |
  `--------------------------------------*/

  /// Difference of expressions.
  template <typename ExpSet>
  inline
  typename ExpSet::value_t
  difference(const ExpSet& rs,
             const typename ExpSet::value_t& lhs,
             const typename ExpSet::value_t& rhs)
  {
    return rs.conjunction(lhs, rs.complement(rhs));
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename ExpSetLhs, typename ExpSetRhs>
      expression
      difference_expression(const expression& lhs, const expression& rhs)
      {
        const auto& l = lhs->as<ExpSetLhs>();
        const auto& r = rhs->as<ExpSetLhs>();
        return make_expression(l.expressionset(),
                               ::vcsn::difference(l.expressionset(),
                                                  l.expression(),
                                                  r.expression()));
      }

      REGISTER_DECLARE(difference_expression,
                       (const expression&, const expression&) -> expression);
    }
  }
}
