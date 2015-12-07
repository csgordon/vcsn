#pragma once

#include <unordered_map>

#include <vcsn/algos/copy.hh>
#include <vcsn/algos/standard.hh>
#include <vcsn/algos/star.hh>
#include <vcsn/algos/sum.hh>
#include <vcsn/core/join.hh>
#include <vcsn/core/rat/expressionset.hh>
#include <vcsn/dyn/automaton.hh> // dyn::make_automaton
#include <vcsn/dyn/label.hh>
#include <vcsn/dyn/polynomial.hh>
#include <vcsn/dyn/weight.hh>
#include <vcsn/misc/raise.hh> // require
#include <vcsn/misc/vector.hh> // make_vector

namespace vcsn
{
  /*----------------------------------.
  | multiply(automaton, automaton).   |
  `----------------------------------*/

  /// Append automaton \a b to \a res for non standard automata.
  ///
  /// \pre The context of \a res must include that of \a b.
  template <typename A, typename B>
  A&
  multiply_here(A& res, const B& b, general_tag = {})
  {
    const auto& ls = *res->labelset();
    const auto& ws = *res->weightset();
    const auto& bws = *b->weightset();

    // The set of the current (left-hand side) final transitions.
    // Store these transitions by copy.
    auto ftr = detail::make_vector(res->final_transitions());

    // The set of the current (right-hand side) initial transitions.
    // Store these transitions by copy.
    auto init_ts = detail::make_vector(b->initial_transitions());

    auto copy = make_copier(b, res);
    copy([](state_t_of<B>) { return true; },
         // Import all the B transitions, except the initial ones.
         [b] (transition_t_of<B> t) { return b->src_of(t) != b->pre(); });
    const auto& map = copy.state_map();

    // Branch all the final transitions of res to the initial ones in b.
    for (auto t1: ftr)
      {
        auto s1 = res->src_of(t1);
        auto w1 = res->weight_of(t1);
        res->del_transition(t1);
        for (auto t2: init_ts)
          res->new_transition(s1,
                              map.at(b->dst_of(t2)),
                              ls.one(),
                              ws.mul(w1,
                                     ws.conv(bws, b->weight_of(t2))));
      }
    return res;
  }

  /// Append automaton \a b to \a res.
  ///
  /// \pre The context of \a res must include that of \a b.
  /// \pre both are standard.
  template <typename A, typename B>
  A&
  multiply_here(A& res, const B& b, standard_tag)
  {
    require(is_standard(b), __func__, ": rhs must be standard");

    const auto& ls = *res->labelset();
    const auto& bls = *b->labelset();
    const auto& ws = *res->weightset();
    const auto& bws = *b->weightset();

    // The set of the current (left-hand side) final transitions.
    // Store these transitions by copy.
    auto final_ts = detail::make_vector(res->final_transitions());

    state_t_of<B> b_initial = b->dst_of(b->initial_transitions().front());

    auto copy = make_copier(b, res);
    copy(// The initial state of b is not copied.
         [b_initial](state_t_of<B> s) { return s != b_initial; },
         // Import all the B transitions, except the initial ones (and
         // those from its (genuine) initial state).
         [b] (transition_t_of<B> t) { return b->src_of(t) != b->pre(); });
    const auto& map = copy.state_map();

    // Branch all the final transitions of res to the successors of
    // b's initial state.
    for (auto t1: final_ts)
      {
        // Remove the previous final transition first, as we might add
        // a final transition for the same state later.
        //
        // For instance on "<2>a+(<3>\e+<5>a)", the final state s1 of
        // <2>a will be made final thanks to <3>\e.  So if we compute
        // the new transitions from s1 and then remove t1, we will
        // have removed the fact that s1 is final thanks to <3>\e.
        //
        // Besides, s1 will become final with weight <3>, which might
        // interfere with <5>a too.
        auto s1 = res->src_of(t1);
        auto w1 = res->weight_of(t1);
        res->del_transition(t1);
        for (auto t2: b->all_out(b_initial))
          res->set_transition(s1,
                              map.at(b->dst_of(t2)),
                              ls.conv(bls, b->label_of(t2)),
                              ws.mul(w1,
                                     ws.conv(bws, b->weight_of(t2))));
      }
    return res;
  }

  /// Concatenate two automata.
  template <typename A, typename B, typename Tag = general_tag>
  inline
  auto
  multiply(const A& lhs, const B& rhs, Tag tag = {})
    -> decltype(detail::make_join_automaton(tag, lhs, rhs))
  {
    auto res = detail::make_join_automaton(tag, lhs, rhs);
    ::vcsn::copy_into(lhs, res);
    multiply_here(res, rhs, tag);
    return res;
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename Lhs, typename Rhs, typename String>
      automaton
      multiply(const automaton& lhs, const automaton& rhs,
               const std::string& algo)
      {
        const auto& l = lhs->as<Lhs>();
        const auto& r = rhs->as<Rhs>();
        return ::vcsn::detail::dispatch_standard(algo,
            [l, r](auto tag)
            {
              return make_automaton(::vcsn::multiply(l, r, tag));
            },
        r);
      }
    }
  }

  /*---------------------------------.
  | multiply(automaton, min, max).   |
  `---------------------------------*/

  /// Repeated concatenation of an automaton.
  ///
  /// The return type, via SFINAE on aut->null_state(), makes the
  /// difference with another overload, `<ValueSet>(ValueSet, value,
  /// value)`, which coincides in the case ValueSet = Z, hence value =
  /// int.
  ///
  /// Unfortunately, fresh_automaton_t_of, which uses
  /// context_t_of<Aut>, is not SFINAE transparent: it causes a hard
  /// failure instead of being ignored.
  ///
  /// FIXME: if you know how to use fresh_automaton_t_of instead, let
  /// me know.
  template <typename Aut, typename Tag = general_tag>
  auto
  multiply(const Aut& aut, int min, int max, Tag tag = {})
    -> decltype(aut->null_state(), // SFINAE.
                detail::make_join_automaton(tag, aut))
  {
    auto res = detail::make_join_automaton(tag, aut);
    if (min == -1)
      min = 0;
    if (max == -1)
    {
      res = star(aut, tag);
      if (min)
        res = multiply(multiply(aut, min, min, tag), res, tag);
    }
    else
    {
      require(min <= max,
              "multiply: invalid exponents: ", min, ", ", max);
      if (min == 0)
      {
        //automatonset::one().
        auto s = res->new_state();
        res->set_initial(s);
        res->set_final(s);
      }
      else
      {
        auto tag_aut = detail::make_join_automaton(tag, aut);
        copy_into(aut, res);
        copy_into(aut, tag_aut);
        for (int n = 1; n < min; ++n)
          res = multiply(res, tag_aut, tag);
      }
      if (min < max)
      {
        // Aut sum = automatonset.one();
        auto sum = detail::make_join_automaton(tag, aut);
        {
          auto s = sum->new_state();
          sum->set_initial(s);
          sum->set_final(s);
        }
        for (int n = 1; n <= max - min; ++n)
          sum = vcsn::sum(sum, multiply(aut, n, n, tag), tag);
        res = vcsn::multiply(res, sum, tag);
      }
    }
    return res;
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename Aut, typename Int1, typename Int2, typename String>
      automaton
      multiply_repeated(const automaton& a, int min, int max,
                        const std::string& algo)
      {
        const auto& aut = a->as<Aut>();
        return ::vcsn::detail::dispatch_standard(algo,
            [aut, min, max](auto tag)
            {
              return make_automaton(::vcsn::multiply(aut, min, max, tag));
            },
        aut);
      }
    }
  }


  /*------------------------------------.
  | multiply(expression, expression).   |
  `------------------------------------*/

  /// Product (concatenation) of expressions/labels/polynomials/weights.
  template <typename ValueSet>
  inline
  typename ValueSet::value_t
  multiply(const ValueSet& vs,
           const typename ValueSet::value_t& lhs,
           const typename ValueSet::value_t& rhs)
  {
    return vs.mul(lhs, rhs);
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename ExpSetLhs, typename ExpSetRhs>
      expression
      multiply_expression(const expression& lhs, const expression& rhs)
      {
        auto join_elts = join<ExpSetLhs, ExpSetRhs>(lhs, rhs);
        return make_expression(std::get<0>(join_elts),
                               ::vcsn::multiply(std::get<0>(join_elts),
                                                std::get<1>(join_elts),
                                                std::get<2>(join_elts)));
      }
    }
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (concatenate).
      template <typename ExpSetLhs, typename ExpSetRhs>
      expression
      concatenate_expression(const expression& lhs, const expression& rhs)
      {
        auto join_elts = join<ExpSetLhs, ExpSetRhs>(lhs, rhs);
        auto res = std::get<0>(join_elts).concat(std::get<1>(join_elts),
                                                 std::get<2>(join_elts));
        return make_expression(std::get<0>(join_elts), res);
      }
    }
  }


  /*----------------------------------.
  | multiply(expression, min, max).   |
  `----------------------------------*/

  template <typename ExpSet>
  typename ExpSet::value_t
  multiply(const ExpSet& rs, const typename ExpSet::value_t& r,
           int min, int max)
  {
    typename ExpSet::value_t res;
    if (min == -1)
      min = 0;
    if (max == -1)
      {
        res = rs.star(r);
        if (min)
          res = rs.mul(rs.power(r, min), res);
      }
    else
      {
        require(min <= max,
                "multiply: invalid exponents: ", min, ", ", max);
        res = rs.power(r, min);
        if (min < max)
          {
            typename ExpSet::value_t sum = rs.one();
            for (int n = 1; n <= max - min; ++n)
              sum = rs.add(sum, rs.power(r, n));
            res = rs.mul(res, sum);
          }
      }
    return res;
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename ExpSet, typename Int1, typename Int2>
      expression
      multiply_expression_repeated(const expression& re, int min, int max)
      {
        const auto& r = re->as<ExpSet>();
        return make_expression(r.expressionset(),
                               ::vcsn::multiply(r.expressionset(),
                                                r.expression(),
                                                min, max));
      }
    }
  }


  /*--------------------------.
  | multiply(label, label).   |
  `--------------------------*/

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename LabelSetLhs, typename LabelSetRhs>
      label
      multiply_label(const label& lhs, const label& rhs)
      {
        const auto& l = lhs->as<LabelSetLhs>();
        const auto& r = rhs->as<LabelSetRhs>();
        auto rs = join(l.labelset(), r.labelset());
        auto lr = rs.conv(l.labelset(), l.label());
        auto rr = rs.conv(r.labelset(), r.label());
        return make_label(rs, multiply(rs, lr, rr));
      }
    }
  }


  /*------------------------------------.
  | multiply(polynomial, polynomial).   |
  `------------------------------------*/

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename PolynomialSetLhs, typename PolynomialSetRhs>
      polynomial
      multiply_polynomial(const polynomial& lhs, const polynomial& rhs)
      {
        auto join_elts = join<PolynomialSetLhs, PolynomialSetRhs>(lhs, rhs);
        return make_polynomial(std::get<0>(join_elts),
                               multiply(std::get<0>(join_elts),
                                        std::get<1>(join_elts),
                                        std::get<2>(join_elts)));
      }
    }
  }

  /*----------------------------.
  | multiply(weight, weight).   |
  `----------------------------*/

  namespace dyn
  {
    namespace detail
    {
      /// Bridge (multiply).
      template <typename WeightSetLhs, typename WeightSetRhs>
      weight
      multiply_weight(const weight& lhs, const weight& rhs)
      {
        const auto& l = lhs->as<WeightSetLhs>();
        const auto& r = rhs->as<WeightSetRhs>();
        auto ws = join(l.weightset(), r.weightset());
        auto lw = ws.conv(l.weightset(), l.weight());
        auto rw = ws.conv(r.weightset(), r.weight());
        return make_weight(ws, ::vcsn::multiply(ws, lw, rw));
      }

      /// Bridge (multiply).
      template <typename WeightSet, typename Int1, typename Int2>
      weight
      multiply_weight_repeated(const weight& wgt, int min, int max)
      {
        const auto& w = wgt->as<WeightSet>();
        return make_weight(w.weightset(),
                           ::vcsn::multiply(w.weightset(),
                                            w.weight(),
                                            min, max));
      }
    }
  }
}
