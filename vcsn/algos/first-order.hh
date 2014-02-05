#ifndef VCSN_ALGOS_FIRST_ORDER_HH
# define VCSN_ALGOS_FIRST_ORDER_HH

# include <stack>
# include <map>
# include <unordered_map>

# include <vcsn/core/rat/visitor.hh>
# include <vcsn/ctx/fwd.hh>
# include <vcsn/dyn/polynomial.hh>
# include <vcsn/dyn/ratexp.hh>
# include <vcsn/misc/indent.hh>
# include <vcsn/misc/map.hh>
# include <vcsn/misc/raise.hh>
# include <vcsn/misc/unordered_set.hh>
# include <vcsn/weights/polynomialset.hh>

//# define DEBUG 1

namespace vcsn
{

  /*----------------------.
  | first_order(ratexp).  |
  `----------------------*/

  namespace rat
  {
    template <typename RatExpSet>
    class first_order_visitor
      : public RatExpSet::const_visitor
    {
    public:
      using ratexpset_t = RatExpSet;
      using context_t = typename ratexpset_t::context_t;
      using labelset_t = typename context_t::labelset_t;
      using label_t = typename context_t::label_t;
      using ratexp_t = typename ratexpset_t::value_t;
      using weightset_t = typename ratexpset_t::weightset_t;
      using weight_t = typename weightset_t::value_t;

      using polynomialset_t = ratexp_polynomialset_t<ratexpset_t>;
      using polynomial_t = typename polynomialset_t::value_t;
      using monomial_t = typename polynomialset_t::monomial_t;

      using super_type = typename ratexpset_t::const_visitor;
      using node_t = typename super_type::node_t;
      using inner_t = typename super_type::inner_t;

      constexpr static const char* me() { return "first_order"; }

      // Keep it sorted to ensure determinism, and better looking
      // results.  Anyway, rough benches show no difference between
      // map and unordered_map here.
      using polys_t = std::map<label_t, polynomial_t, vcsn::less<labelset_t>>;

      /// Store the result.
      struct value_t
      {
        weight_t constant;
        polys_t polynomials;
      };

      first_order_visitor(const ratexpset_t& rs, bool use_spontaenous = false)
        : rs_(rs)
        , use_spontaenous_(use_spontaenous)
      {}

      void operator()(const ratexp_t& v)
      {
        // FIXME: make a libc++ bug report: "{ws_.zero(), {}}" should
        // suffice.
        res_ = {ws_.zero(), polys_t{}};
        v->accept(*this);
      }

      std::unordered_map<ratexp_t, value_t,
                         vcsn::hash<ratexpset_t>,
                         vcsn::equal_to<ratexpset_t>> cache_;

      value_t first_order(const ratexp_t& e)
      {
        auto insert = cache_.emplace(e, value_t{ws_.zero(), polys_t{}});
        auto& res = insert.first->second;
        if (insert.second)
          {
            std::swap(res, res_);
#if DEBUG
            rs_.print(std::cerr, e) << "..." << incendl;
#endif
            e->accept(*this);
            std::swap(res, res_);
#if DEBUG
            rs_.print(std::cerr, e) << " => ";
            print_(std::cerr, res) << decendl;
#endif
          }
        else
          {
#if DEBUG
            rs_.print(std::cerr, e) << " -> ";
            print_(std::cerr, res) << iendl;
#endif
          }
        return res;
      }

      polynomial_t first_order_as_polynomial(const ratexp_t& e)
      {
        operator()(e);
        return as_polynomial(res_);
      }

      polynomial_t as_polynomial(const value_t& v)
      {
        // FIXME: polynomial_t{{rs_.one(), constant}} is wrong,
        // because the (default) ctor will not eliminate the monomial
        // when constant is zero.
        polynomial_t res;
        ps_.add_weight(res, rs_.one(), v.constant);
        for (const auto& p: v.polynomials)
          // We may add a label on our maps, and later map it to 0.
          // In this case polynomialset builds '\z -> 1', i.e., it
          // does insert \z as a label in the polynomial.  Avoid this.
          //
          // FIXME: shouldn't polynomialset do that itself?
          if (!ps_.is_zero(p.second))
            ps_.add_weight(res,
                           rs_.mul(rs_.atom(p.first), ratexp_(p.second)),
                           ws_.one());
        return res;
      }

      // FIXME: duplicate with expand.
      ratexp_t ratexp_(const polynomial_t p)
      {
        ratexp_t res = rs_.zero();
        for (const auto& m: p)
          res = rs_.add(res, rs_.lmul(m.second, m.first));
         return res;
      }

      /// Print a first order development.
      std::ostream& print_(std::ostream& o, const value_t& v) const
      {
        ws_.print(o, v.constant);
        for (const auto& p: v.polynomials)
          {
            o << " + ";
            rs_.labelset()->print(o, p.first) << ".[";
            ps_.print(o, p.second) << ']';
          }
        return o;
      }

      /// In place addition.
      void add_(value_t& lhs, const value_t& rhs) const
      {
        lhs.constant = ws_.add(lhs.constant, rhs.constant);
        for (const auto& p: rhs.polynomials)
          ps_.add_weight(lhs.polynomials[p.first], p.second);
      }

      VCSN_RAT_VISIT(zero,)
      {
        res_ = {ws_.zero(), polys_t{}};
      }

      VCSN_RAT_VISIT(one,)
      {
        res_ = {ws_.one(), polys_t{}};
      }

      VCSN_RAT_VISIT(atom, e)
      {
        res_ = {ws_.zero(), {{e.value(), ps_.one()}}};
      }

      VCSN_RAT_VISIT(sum, e)
      {
        res_ = {ws_.zero(), polys_t{}};
        for (const auto& v: e)
          add_(res_, first_order(v));
      }

      VCSN_RAT_VISIT(prod, e)
      {
        res_ = {ws_.one(), polys_t{}};
        for (const auto& r: e)
          {
            // fo(l) = c(l) + a.A(l) + ...
            // fo(r) = c(r) + a.A(r) + ...
            // fo(l.r) = (c(l) + a.A(l) + ...) (c(r) + a.A(r) + ...)
            // c(fo(lr)) = c(l).c(r)
            // A(fo(lr)) = A(l).r + c(l).A(r)

            // (i): A(fo(lr)) = A(l).r
            for (auto& p: res_.polynomials)
              p.second = ps_.rmul(p.second, r);

            // Don't leave \z polynomials.
            if (!ws_.is_zero(res_.constant))
              {
                value_t rhs = first_order(r);
                // (ii) A(fo(lr)) += c(l).A(r)
                for (const auto& p: rhs.polynomials)
                  ps_.add_weight(res_.polynomials[p.first],
                                 ps_.lmul(res_.constant, p.second));

                // (iii) c(fo(lr)) = c(l).c(r)
                res_.constant = ws_.mul(res_.constant, rhs.constant);
              }
          }
      }

      VCSN_RAT_VISIT(intersection, e)
      {
        res_ = first_order(e.head());
        for (const auto& r: e.tail())
          {
            // Save current result in lhs, and compute the result in res.
            value_t lhs = {ws_.zero(), polys_t{}};
            std::swap(res_, lhs);
            value_t rhs = first_order(r);

            for (auto& p: lhs.polynomials)
              {
                auto i = rhs.polynomials.find(p.first);
                if (i != std::end(rhs.polynomials))
                  res_.polynomials[p.first]
                    = ps_.intersection(p.second, i->second);
              }
            res_.constant = ws_.mul(lhs.constant, rhs.constant);
          }
      }

      // FO(E:F) = FO(E):F + E:FO(F)
      VCSN_RAT_VISIT(shuffle, e)
      {
        value_t res = {ws_.one(), polys_t{}};
        // The shuffle-product of the previously traversed siblings.
        // Initially the neutral element: \e.
        ratexp_t prev = rs_.one();
        for (const auto& rhs: e)
          {
            // Save current result in lhs, and compute the result in res.
            value_t lhs; lhs.constant = ws_.zero();
            std::swap(res, lhs);

            value_t r = first_order(rhs);
            res.constant = ws_.mul(lhs.constant, r.constant);

            // (i) fo(lhs) -> fo(lhs):r, that is, shuffle-multiply the
            // current result by the current child (rhs).
            for (const auto& p: lhs.polynomials)
              for (const auto& m: p.second)
                res.polynomials[p.first].emplace(rs_.shuffle(m.first, rhs),
                                                 m.second);
            // (ii) prev:fo(rhs)
            for (const auto& p: r.polynomials)
              for (const auto& m: p.second)
                ps_.add_weight(res.polynomials[p.first],
                               rs_.shuffle(prev, m.first), m.second);

            prev = rs_.shuffle(prev, rhs);
          }
        res_ = res;
      }

      const labelset_t& letters_(std::true_type)
      {
        return *rs_.labelset();
      }

      std::vector<label_t> letters_(std::false_type)
      {
        raise(me(), ": cannot handle complement with generators");
      }

      VCSN_RAT_VISIT(complement, e)
      {
        value_t res = first_order(e.sub());
        res_.constant = ws_.is_zero(res.constant) ? ws_.one() : ws_.zero();

        // Turn the polynomials into a ratexp, and complement it.
        // Here, we need to iterate over the set of letters (obviously
        // required to complement).
        auto letters
          = letters_(std::integral_constant<bool,
                     context_t::is_lal || context_t::is_lan>());
        for (auto l: letters)
          ps_.add_weight
            (res_.polynomials[l],
             polynomial_t{{rs_.complement(ratexp_(res.polynomials[l])),
                           ws_.one()}});
      }

      VCSN_RAT_VISIT(star, e)
      {
        value_t res = first_order(e.sub());
        res_.constant = ws_.star(res.constant);
        for (const auto& p: res.polynomials)
          res_.polynomials[p.first]
            = ps_.lmul(res_.constant,
                       ps_.rmul(p.second, e.shared_from_this()));
      }

      VCSN_RAT_VISIT(lweight, e)
      {
        value_t res = first_order(e.sub());
        res_.constant = ws_.mul(e.weight(), res.constant);
        for (const auto& p: res.polynomials)
          res_.polynomials[p.first] = ps_.lmul(e.weight(), p.second);
      }

      VCSN_RAT_VISIT(rweight, e)
      {
        value_t res = first_order(e.sub());
        res_ = {ws_.mul(res.constant, e.weight()), polys_t{}};
        for (auto& p: res.polynomials)
          for (const auto& m: p.second)
            ps_.add_weight(res_.polynomials[p.first],
                           rs_.rmul(m.first, e.weight()), m.second);
      }

      // private:
      ratexpset_t rs_;
      /// Whether to use spontaneous transitions.
      bool use_spontaenous_;
      /// Shorthand to the weightset.
      weightset_t ws_ = *rs_.weightset();
      polynomialset_t ps_ = make_ratexp_polynomialset(rs_);
      /// The result.
      value_t res_;
    };

  } // rat::

  /// First order expansion.
  template <typename RatExpSet>
  inline
  rat::ratexp_polynomial_t<RatExpSet>
  first_order(const RatExpSet& rs, const typename RatExpSet::ratexp_t& e,
              bool use_spontaenous = false)
  {
    rat::first_order_visitor<RatExpSet> first_order{rs, use_spontaenous};
    return first_order.first_order_as_polynomial(e);
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename RatExpSet, typename Bool>
      polynomial
      first_order(const ratexp& exp, bool use_spontaenous = false)
      {
        const auto& e = exp->as<RatExpSet>();
        const auto& rs = e.get_ratexpset();
        auto ps = vcsn::rat::make_ratexp_polynomialset(rs);
        return make_polynomial(ps,
                               first_order<RatExpSet>(rs, e.ratexp(),
                                                      use_spontaenous));
      }

      REGISTER_DECLARE(first_order,
                       (const ratexp& e, bool use_spontaenous) -> polynomial);
    }
  }
} // vcsn::

#endif // !VCSN_ALGOS_FIRST_ORDER_HH
