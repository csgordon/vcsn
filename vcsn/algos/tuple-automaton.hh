#pragma once

#include <vcsn/algos/tuple.hh>
#include <vcsn/core/lazy-tuple-automaton.hh>
#include <vcsn/dyn/automaton.hh>

namespace vcsn
{
  namespace detail
  {
    /*-------------------------------.
    | tuple_automata_impl<Aut...>.   |
    `-------------------------------*/

    /// Build the (accessible part of the) Cartesian product of
    /// automata.
    ///
    /// Warning: beware of naming issues: do not confuse it with
    /// tuple_automaton_impl which is actually an automaton decorator
    /// whose states are tuples of states of other automata.  This
    /// class, tuple_automata_impl, derives (indirectly) from
    /// tuple_automaton_impl.
    template <Automaton Aut, Automaton... Auts>
    class tuple_automata_impl
      : public lazy_tuple_automaton<tuple_automata_impl<Aut, Auts...>,
                                    false, false, Aut, Auts...>
    {
      /// The type of the resulting automaton.
      using automaton_t = Aut;
      using self_t = tuple_automata_impl;
      using super_t = lazy_tuple_automaton<self_t, false, false, Aut, Auts...>;

    public:
      using state_name_t = typename super_t::state_name_t;
      using state_t = typename super_t::state_t;

      template <size_t... I>
      using seq = typename super_t::template seq<I...>;

      using super_t::ws_;
      using super_t::transition_maps_;

      /// The context of the result.
      using context_t = context_t_of<Aut>;
      using labelset_t = labelset_t_of<context_t>;
      using weightset_t = weightset_t_of<context_t>;

      using label_t = typename labelset_t::value_t;
      using weight_t = typename weightset_t::value_t;

      /// The type of input automata.
      using automata_t = std::tuple<Auts...>;

      /// The type of the Ith input automaton, unqualified.
      template <size_t I>
      using input_automaton_t = base_t<tuple_element_t<I, automata_t>>;

      using super_t::aut_;

      /// Build a tuple automaton.
      /// \param aut   the automaton to build.
      /// \param auts  the input automata.
      tuple_automata_impl(Aut aut, const Auts&... auts)
        : super_t{aut, auts...}
      {
        aut_->todo_.emplace_back(aut_->pre_(), aut_->pre());
      }

      /// Compute the (accessible part of the) tuple.
      void tuple()
      {
        try
          {
            while (!aut_->todo_.empty())
              {
                const auto& p = aut_->todo_.front();
                add_tuple_transitions(std::get<1>(p), std::get<0>(p));
                aut_->todo_.pop_front();
              }
          }
        catch (const std::runtime_error& e)
          {
            raise(e, "  while tupling automata");
          }
      }

    private:
      using super_t::out_;
      using super_t::state;
      /// Add transitions to the result automaton, starting from the
      /// given result input state, which must correspond to the given
      /// pair of input state automata.  Update the worklist with the
      /// needed source-state pairs.
      void add_tuple_transitions(const state_t src, const state_name_t& psrc)
      {
        add_tuple_transitions_(src, psrc, aut_->indices);
      }

      template <std::size_t... I>
      void add_tuple_transitions_(const state_t src, const state_name_t& psrc,
                                  seq<I...>)
      {
        // If this is post, we are done.
        if (!all((std::get<I>(psrc) == std::get<I>(aut_->auts_)->post())...))
          {
            const auto& ls = *aut_->labelset();
            const auto& ws = *aut_->weightset();
            // A blank label: the labels of each tape will be inserted
            // one after the other.  Using `one` instead `special` is
            // tempting, but `one` is not always available.
            auto label = ls.special();
            auto weight = ws.one();
            auto dst = psrc;
            add_tape_transitions_<0>(src, psrc, dst, label, weight);
          }
      }

      template <std::size_t I>
      void add_tape_transitions_(const state_t src, const state_name_t& psrc,
                                 state_name_t dst,
                                 label_t label, weight_t weight)
      {
        const auto& ls = *aut_->labelset();
        const auto& ws = *aut_->weightset();
        const auto& aut = std::get<I>(aut_->auts_);
        // FIXME: too much code duplication between both branches,
        // something is missing.  FIXME: a lot of passing by value on
        // dst, label and weight.  We can probably be more economical.
        if (std::get<I>(psrc) == aut->post())
          {
            std::get<I>(label) = label_one(ls.template set<I>());
            static_if<I + 1 == sizeof...(Auts)>
              ([this](auto src, auto, auto dst, auto label, auto weight)
               {
                 if (state(dst) == aut_->post())
                   // The label is actually a tuple of \e, don't use it.
                   aut_->set_final(src, weight);
                 else
                   aut_->new_transition(src, state(dst), label, weight);
               },
               [this](auto src, auto psrc, auto dst, auto label, auto weight)
               {
                 add_tape_transitions_<I + 1>(src, psrc, dst, label, weight);
               })
              (src, psrc, dst, label, weight);
          }
        else
          for (auto t: aut->all_out(std::get<I>(psrc)))
            {
              std::get<I>(label)
                = aut->dst_of(t) == aut->post()
                ? label_one(ls.template set<I>())
                : aut->label_of(t);
              weight = ws.mul(weight, aut->weight_of(t));
              std::get<I>(dst) = aut->dst_of(t);
              static_if<I + 1 == sizeof...(Auts)>
                ([this,&ls,&ws](auto src, auto, auto dst, auto label, auto weight)
                 {
                   if (state(dst) == aut_->post())
                     // The label is actually a tuple of \e, don't use it.
                     aut_->set_final(src, weight);
                   else
                     aut_->new_transition(src, state(dst), label, weight);
                 },
                 [this](auto src, auto psrc, auto dst, auto label, auto weight)
                 {
                   add_tape_transitions_<I + 1>(src, psrc, dst, label, weight);
                 })
                (src, psrc, dst, label, weight);
            }
      }
    };


    /*-----------------------.
    | tuple(automaton...).   |
    `-----------------------*/

    /// Build the (accessible part of the) tuple.
    template <Automaton... Auts>
    auto
    tuple(const Auts&... as)
    {
      auto ctx = tuple_context(as->context()...);
      auto aut = make_mutable_automaton(ctx);
      auto res = tuple_automata_impl<decltype(aut), Auts...>(aut, as...);
      res.tuple();
      return res.strip();
    }
  }

  using detail::tuple;

  namespace dyn
  {
    namespace detail
    {
      /// Bridge helper.
      template <typename Auts, size_t... I>
      automaton
      tuple_(const std::vector<automaton>& as,
             vcsn::detail::index_sequence<I...>)
      {
        return tuple(as[I]->as<tuple_element_t<I, Auts>>()...);
      }

      /// Bridge.
      template <typename Auts>
      automaton
      tuple(const std::vector<automaton>& as)
      {
        auto indices
          = vcsn::detail::make_index_sequence<std::tuple_size<Auts>::value>{};
        return tuple_<Auts>(as, indices);
      }
    }
  }
}
