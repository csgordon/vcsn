#ifndef VCSN_ALGOS_ENUMERATE_HH
# define VCSN_ALGOS_ENUMERATE_HH

# include <algorithm>
# include <map>
# include <queue>

# include <vcsn/ctx/context.hh>
# include <vcsn/dyn/automaton.hh>
# include <vcsn/dyn/fwd.hh>
# include <vcsn/dyn/polynomial.hh>
# include <vcsn/labelset/word-polynomialset.hh>

namespace vcsn
{

  /*-----------------------.
  | enumerate(automaton).  |
  `-----------------------*/

  namespace detail
  {
    template <typename Aut>
    class enumerater
    {
    public:
      using automaton_t = Aut;
      using context_t = context_t_of<Aut>;
      static_assert(context_t::labelset_t::is_free(),
                    "enumerate: requires free labelset");

      using labelset_t = labelset_t_of<automaton_t>;
      using weightset_t = weightset_t_of<automaton_t>;
      using wordset_context_t = word_context_t<context_t>;
      using polynomialset_t = polynomialset<wordset_context_t>;
      using polynomial_t = typename polynomialset_t::value_t;
      using label_t = label_t_of<automaton_t>;
      using weight_t = weight_t_of<automaton_t>;
      using state_t = state_t_of<automaton_t>;
      using word_t = word_t_of<automaton_t>;

      /// Same as polynomial_t::value_type.
      using monomial_t = std::pair<word_t, weight_t>;
      using queue_t = std::queue<std::pair<state_t, monomial_t>>;

      enumerater(const automaton_t& aut)
        : aut_(aut)
      {
        past_[aut_->pre()] = ps_.one();
      }

      /// The weighted accepted word with length at most \a max.
      polynomial_t enumerate(unsigned max)
      {
        queue_t queue;
        queue.emplace(aut_->pre(), ps_.monomial_one());

        // We match words that include the initial and final special
        // characters.
        max += 2;
        for (size_t i = 0; i < max && !queue.empty(); ++i)
          propagate_(queue);

        // Return the past of post(), but remove the initial and final
        // special characters for the words.
        polynomial_t res;
        for (const auto& m: past_[aut_->post()])
          ps_.add_here(res, ls_.undelimit(m.first), m.second);
        return res;
      }

      /// The shortest accepted weighted words, or throw an exception.
      // FIXME: code duplication.
      polynomial_t shortest(unsigned num)
      {
        queue_t queue;
        queue.emplace(aut_->pre(), ps_.monomial_one());

        while (past_[aut_->post()].size() < num && !queue.empty())
          propagate_(queue);

        // Return the past of post(), but remove the initial and final
        // special characters for the words.
        polynomial_t res;
        for (const auto& m: past_[aut_->post()])
          {
            ps_.add_here(res, ls_.undelimit(m.first), m.second);
            if (--num == 0)
              break;
          }
        return res;
      }

    private:
      /// Process once all the states of \a q1.
      /// Save into q1 the new states to visit.
      void propagate_(queue_t& q1)
      {
        queue_t q2;
        while (!q1.empty())
          {
            state_t s;
            monomial_t m;
            tie(s, m) = std::move(q1.front());
            q1.pop();
            for (const auto t: aut_->all_out(s))
              {
                // FIXME: monomial mul.
                monomial_t n(ls_.mul(m.first, aut_->label_of(t)),
                             ws_.mul(m.second, aut_->weight_of(t)));
                ps_.add_here(past_[aut_->dst_of(t)], n);
                q2.emplace(aut_->dst_of(t), n);
              }
          }
        q1.swap(q2);
      }

      const automaton_t& aut_;
      const weightset_t& ws_ = *aut_->weightset();
      const polynomialset_t ps_ = make_word_polynomialset(aut_->context());
      const labelset_t_of<polynomialset_t>& ls_ = *ps_.labelset();
      /// For each state, the first orders of its past.
      std::map<state_t, polynomial_t> past_;
    };
  }

  template <typename Automaton>
  inline
  typename detail::enumerater<Automaton>::polynomial_t
  enumerate(const Automaton& aut, unsigned max)
  {
    detail::enumerater<Automaton> enumerater(aut);
    return enumerater.enumerate(max);
  }

  template <typename Automaton>
  inline
  typename detail::enumerater<Automaton>::polynomial_t
  shortest(const Automaton& aut, unsigned num)
  {
    detail::enumerater<Automaton> enumerater(aut);
    return enumerater.shortest(num);
  }


  namespace dyn
  {
    namespace detail
    {

      /*-----------------.
      | dyn::enumerate.  |
      `-----------------*/

      template <typename Aut, typename Unsigned>
      polynomial
      enumerate(const automaton& aut, unsigned max)
      {
        const auto& a = aut->as<Aut>();
        auto ps = vcsn::detail::make_word_polynomialset(a->context());
        return make_polynomial(ps, enumerate(a, max));
      }

      REGISTER_DECLARE
      (enumerate,
       (const automaton& aut, unsigned max) -> polynomial);


      /*----------------.
      | dyn::shortest.  |
      `----------------*/

      template <typename Aut, typename Unsigned>
      polynomial
      shortest(const automaton& aut, unsigned num)
      {
        const auto& a = aut->as<Aut>();
        auto ps = vcsn::detail::make_word_polynomialset(a->context());
        return make_polynomial(ps, shortest(a, num));
      }

      REGISTER_DECLARE(shortest,
                       (const automaton& aut, unsigned num) -> polynomial);
    }
  }
}

#endif // !VCSN_ALGOS_ENUMERATE_HH
