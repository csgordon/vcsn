#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>

#include <boost/optional.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>

#include <vcsn/algos/focus.hh> // FIXME: Move elsewhere than algos.

#include <vcsn/ctx/context.hh> // We need context to define join.
#include <vcsn/ctx/traits.hh>
#include <vcsn/labelset/wordset.hh>
#include <vcsn/misc/algorithm.hh> // front
#include <vcsn/misc/attributes.hh>
#include <vcsn/misc/functional.hh>
#include <vcsn/misc/math.hh>
#include <vcsn/misc/raise.hh>
#include <vcsn/misc/star-status.hh>
#include <vcsn/misc/stream.hh>
#include <vcsn/misc/wet.hh>
#include <vcsn/misc/zip-maps.hh>
#include <vcsn/weightset/fwd.hh>
#include <vcsn/weightset/z.hh>

namespace vcsn
{
  // http://llvm.org/bugs/show_bug.cgi?id=18571
#if defined __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-value"
#endif
  template <typename LabelSet>
  auto label_is_zero(const LabelSet& ls, const typename LabelSet::value_t* l)
    -> decltype(ls.is_zero(l), bool())
  {
    return ls.is_zero(*l);
  }

#if defined __clang__
# pragma clang diagnostic pop
#endif

  template <typename LabelSet>
  bool label_is_zero(const LabelSet&, ...)
  ATTRIBUTE_CONST;

  template <typename LabelSet>
  bool label_is_zero(const LabelSet&, ...)
  {
    return false;
  }

  namespace detail
  {
    template <typename WeightSet>
    struct is_division_ring
      : std::true_type
    {};

    template <>
    struct is_division_ring<z>
      : std::false_type
    {};

    template <typename Context, wet_kind_t Kind>
    struct is_division_ring<polynomialset<Context, Kind>>
      : std::false_type
    {};
  }

  /// Linear combination of labels: map labels to weights.
  /// \tparam Context  the LabelSet and WeightSet types.
  template <typename Context,
            wet_kind_t Kind = detail::wet_kind<labelset_t_of<Context>,
                                               weightset_t_of<Context>>()>
  class polynomialset
  {
  public:
    using self_t = polynomialset<Context, Kind>;
    using context_t = Context;
    using labelset_t = labelset_t_of<context_t>;
    using weightset_t = weightset_t_of<context_t>;
    using polynomialset_t = polynomialset<context_t>;

    using labelset_ptr = typename context_t::labelset_ptr;
    using weightset_ptr = typename context_t::weightset_ptr;
    /// Polynomials over labels.
    using label_t = typename labelset_t::value_t;
    using weight_t = weight_t_of<context_t>;

    using value_t = wet_of<context_t, Kind>;
    /// A pair <label, weight>.
    using monomial_t = typename value_t::value_type;

    polynomialset() = delete;
    polynomialset(const polynomialset&) = default;
    polynomialset(const context_t& ctx)
      : ctx_{ctx}
    {}

    /// The static name.
    static symbol sname()
    {
      static symbol res("polynomialset<" + context_t::sname() + '>');
      return res;
    }

    const context_t& context() const { return ctx_; }
    const labelset_ptr& labelset() const { return ctx_.labelset(); }
    const weightset_ptr& weightset() const { return ctx_.weightset(); }

    static constexpr bool is_commutative() { return false; }

    /// Remove the monomial of \a l in \a v.
    value_t&
    del_weight(value_t& v, const label_t& l) const
    {
      v.erase(l);
      return v;
    }

    /// Set the monomial of \a l in \a v to weight \a k.
    /// \pre  w is not null
    value_t&
    new_weight(value_t& v, const label_t& l, const weight_t w) const
    {
      assert(!weightset()->is_zero(w));
      v.set(l, w);
      return v;
    }

    /// Set the monomial of \a l in \a v to weight \a w.
    value_t&
    set_weight(value_t& v, const label_t& l, const weight_t w) const
    {
      if (weightset()->is_zero(w))
        return del_weight(v, l);
      else
        return new_weight(v, l, w);
    }

    const weight_t
    get_weight(const value_t& v, const label_t& l) const ATTRIBUTE_PURE
    {
      auto i = v.find(l);
      if (i == v.end())
        return weightset()->zero();
      else
        return weight_of(*i);
    }


    /*---------.
    | clear.   |
    `---------*/

    /// Set to zero.
    void clear(value_t& v)
    {
      v.clear();
    }


    /*-------.
    | add.   |
    `-------*/

    /// `v += <k>l`.
    value_t&
    add_here(value_t& v, const label_t& l, const weight_t k) const
    {
      if (!label_is_zero(*labelset(), &l))
        {
          auto i = v.find(l);
          if (i == v.end())
            {
              set_weight(v, l, k);
            }
          else
            {
              // Do not use set_weight() because it would lookup l
              // again and we already have the right iterator.
              auto w2 = weightset()->add(weight_of(*i), k);
              if (weightset()->is_zero(w2))
                v.erase(i);
              else
                v.set(i, w2);
            }
        }
      return v;
    }

    /// `v += m`.
    value_t&
    add_here(value_t& v, const monomial_t& m) const
    {
      return add_here(v, label_of(m), weight_of(m));
    }

    /// `v += p`, default case.
    template <wet_kind_t WetType>
    auto
    add_here_impl(value_t& l, const value_t& r) const
      -> enable_if_t<!(WetType == wet_kind_t::bitset
                       && std::is_same<weightset_t, b>::value),
                     value_t&>
    {
      for (const auto& m: r)
        add_here(l, m);
      return l;
    }

    /// `v += p`, B and bitsets.
    template <wet_kind_t WetType>
    auto
    add_here_impl(value_t& l, const value_t& r) const
      -> enable_if_t<WetType == wet_kind_t::bitset
                     && std::is_same<weightset_t, b>::value,
                     value_t&>
    {
      l.set() += r.set();
      return l;
    }

    value_t&
    add_here(value_t& l, const value_t& r) const
    {
      return add_here_impl<value_t::kind>(l, r);
    }

    /// The sum of polynomials \a l and \a r.
    value_t add(const value_t& l, const value_t& r) const
    {
      value_t res = l;
      add_here(res, r);
      return res;
    }


    /*-------.
    | sub.   |
    `-------*/

    /// `v -= m`.
    value_t&
    sub_here(value_t& v, const monomial_t& m) const
    {
      if (!label_is_zero(*labelset(), &label_of(m)))
        {
          auto i = v.find(label_of(m));
          if (i == v.end())
            {
              raise(sname(), ": sub_here: invalid arguments: ",
                    to_string(*this, v), ", ", to_string(*this, m));
            }
          else
            {
              // Do not use set_weight() because it would lookup w
              // again and we already have the right iterator.
              auto w2 = weightset()->sub(weight_of(*i), weight_of(m));
              if (weightset()->is_zero(w2))
                v.erase(i);
              else
                weight_set(*i, w2);
            }
        }
      return v;
    }

    /// The subtraction of polynomials \a l and \a r.
    value_t
    sub(const value_t& l, const value_t& r) const
    {
      value_t res = l;
      for (const auto& rm: r)
        sub_here(res, rm);
      return res;
    }


    /*-------.
    | mul.   |
    `-------*/

    /// The product of monomials \a l and \a r.
    monomial_t
    mul(const monomial_t& l, const monomial_t& r) const
    {
      return {labelset()->mul(label_of(l), label_of(r)),
              weightset()->mul(weight_of(l), weight_of(r))};
    }

    /// The product of polynomials \a l and \a r.
    /// General case.
    template <wet_kind_t WetType>
    auto
    mul_impl(const value_t& l, const value_t& r) const
      -> enable_if_t<WetType != wet_kind_t::bitset,
                     value_t>
    {
      value_t res;
      for (const auto& lm: l)
        for (const auto& rm: r)
          add_here(res,
                   labelset()->mul(label_of(lm), label_of(rm)),
                   weightset()->mul(weight_of(lm), weight_of(rm)));
      return res;
    }

    /// The product of polynomials \a l and \a r.
    /// Case of bitsets.
    template <wet_kind_t WetType>
    auto
    mul_impl(const value_t& l, const value_t& r) const
      -> enable_if_t<WetType == wet_kind_t::bitset,
                     value_t>
    {
      return l.set() & r.set();
    }

    /// The product of polynomials \a l and \a r.
    auto
    mul(const value_t& l, const value_t& r) const
      -> value_t
    {
      return mul_impl<value_t::kind>(l, r);
    }

    /// The product of polynomials \a l and \a r.
    auto
    mul(const value_t& p, const label_t& l, const weight_t& w) const
      -> value_t
    {
      value_t res;
      for (const auto& m: p)
        add_here(res,
                 labelset()->mul(label_of(m), l),
                 weightset()->mul(weight_of(m), w));
      return res;
    }


    /*---------------.
    | conjunction.   |
    `---------------*/

    /// The conjunction of polynomials \a l and \a r.
    /// Not valid for all the labelsets.
    value_t
    conjunction(const value_t& l, const value_t& r) const
    {
      value_t res;
      for (const auto& lm: l)
        for (const auto& rm: r)
          add_here(res,
                   labelset()->conjunction(label_of(lm), label_of(rm)),
                   weightset()->mul(weight_of(lm), weight_of(rm)));
      return res;
    }

    /// The infiltration of polynomials \a l and \a r.
    /// Not valid for all the labelsets.
    value_t
    infiltration(const value_t& l, const value_t& r) const
    {
      value_t res;
      for (const auto& lm: l)
        for (const auto& rm: r)
          add_here(res,
                   labelset()->infiltration(label_of(lm), label_of(rm)),
                   weightset()->mul(weight_of(lm), weight_of(rm)));
      return res;
    }

    /// The sum of the weights of the common labels.
    /// FIXME: Should probably be a special case of conjunction.
    weight_t
    scalar_product(const value_t& l, const value_t& r) const
    {
      weight_t res = weightset()->zero();
      for (const auto& p: zip_maps<vcsn::as_tuple>(l, r))
        res = weightset()->add(res,
                               weightset()->mul(weight_of(std::get<0>(p)),
                                                weight_of(std::get<1>(p))));
      return res;
    }

    /// The star of polynomial \a v.
    value_t
    star(const value_t& v) const
    {
      // The only starrable polynomials are scalars (if they are
      // starrable too).
      auto s = v.size();
      if (s == 0)
        return one();
      else if (s == 1)
        {
          auto i = v.find(labelset()->one());
          if (i != v.end())
            return {{i->first, weightset()->star(i->second)}};
        }
      raise(sname(), ": star: invalid value: ", to_string(*this, v));
    }

    /// Left exterior product.
    value_t
    lmul(const weight_t& w, const value_t& v) const
    {
      value_t res;
      if (!weightset()->is_zero(w))
        for (const auto& m: v)
          add_here(res, label_of(m), weightset()->mul(w, weight_of(m)));
      return res;
    }

    /// Left product by a label.
    value_t
    lmul_label(const label_t& lhs, const value_t& v) const
    {
      value_t res;
      for (const auto& m: v)
        add_here(res,
                 labelset()->mul(lhs, label_of(m)),
                 weight_of(m));
      return res;
    }

    /// Left product by a monomial.
    value_t
    mul(const monomial_t& lhs, const value_t& v) const
    {
      value_t res;
      for (const auto& m: v)
        add_here(res,
                 labelset()->mul(label_of(lhs), label_of(m)),
                 weightset()->mul(weight_of(lhs), weight_of(m)));
      return res;
    }

    /// Right exterior product.
    ///
    /// Beware that we do not multiply the weight here, but the label.
    /// It seems that this routine is used _only_ when calling
    /// "split", which is done only on polynomials of expressions, so
    /// it is valid to rmul a label by a weight.  If some day we need
    /// an rmul between weights, we will need additional properties to
    /// allow it.
    value_t
    rmul(const value_t& v, const weight_t& w) const
    {
      value_t res;
      if (!weightset()->is_zero(w))
        for (const auto& m: v)
          add_here(res, labelset()->rmul(label_of(m), w), weight_of(m));
      return res;
    }

    /// Right product.
    value_t
    rmul_label(const value_t& v, const label_t& rhs) const
    {
      value_t res;
      for (const auto& lhs: v)
        add_here(res,
                 labelset()->mul(label_of(lhs), rhs),
                 weight_of(lhs));
      return res;
    }

    /// Right product by a monomial.
    value_t
    mul(const value_t& l, const monomial_t& rhs) const
    {
      value_t res;
      for (const auto& lhs: l)
        add_here(res,
                 labelset()->mul(label_of(lhs), label_of(rhs)),
                 weightset()->mul(weight_of(lhs), weight_of(rhs)));
      return res;
    }

    value_t
    rdiv(const value_t& l, const value_t& r) const
    {
      raise(sname(), ": rdiv: not implemented (",
            to_string(*this, l), ", ", to_string(*this, r), ")");
    }

    monomial_t
    ldiv(const monomial_t& l, const monomial_t& r) const
    {
      return {labelset()->ldiv(label_of(l), label_of(r)),
              weightset()->ldiv(weight_of(l), weight_of(r))};
    }

    /// Left division by a monomial.
    value_t
    ldiv(const monomial_t& l, const value_t& r) const
    {
      value_t res;
      for (const auto& m: r)
        add_here(res, ldiv(l, m));
      return res;
    }

    value_t
    ldiv(const value_t& l, const value_t& r) const
    {
      value_t res;
      if (is_zero(l))
        raise(sname(), ": ldiv: division by zero");
      else
        {
          value_t remainder = r;
#if DEBUG
          std::cerr << "ldiv(";
          print(l, std::cerr) << ", ";
          print(r, std::cerr) << "\n";
#endif
          while (!is_zero(remainder))
            {
              auto factor = ldiv(detail::front(l), detail::front(remainder));
#if DEBUG
              std::cerr << "factor = "; print(factor, std::cerr) << "\n";
#endif
              add_here(res, factor);
#if DEBUG
              std::cerr << "res = "; print(res, std::cerr) << "\n";
              std::cerr << "sub = "; print(mul(l, factor), std::cerr) << "\n";
#endif
              remainder = sub(remainder, mul(l, factor));
#if DEBUG
              std::cerr << "rem = "; print(remainder, std::cerr) << "\n";
#endif
            }
#if DEBUG
          std::cerr << "ldiv(";
          print(l, std::cerr) << ", ";
          print(r, std::cerr) << ") = ";
          print(res, std::cerr) << " rem: ";
          print(remainder, std::cerr) << "\n";
#endif
          if (!is_zero(remainder))
            raise(sname(), ": ldiv: not implemented (",
                  to_string(*this, l), ", ", to_string(*this, r), ")");
        }
      return res;
    }

    /// Left exterior division.
    value_t&
    ldiv_here(const weight_t& w, value_t& v) const
    {
      if (!weightset()->is_one(w))
        for (auto&& m: v)
          weight_set(m, weightset()->ldiv(w, weight_of(m)));
      return v;
    }

    /// Right exterior division.
    value_t&
    rdiv_here(value_t& v, const weight_t& w) const
    {
      if (!weightset()->is_one(w))
        for (auto& m: v)
          weight_set(m, weightset()->rdiv(weight_of(m), w));
      return v;
    }

    /// LGCD between two polynomials.
    ///
    /// Based only on weights.
    /// For instance <2>a+<4>b, <3>a+<6>b => <1>a+<2>b.
    /// And ab, a => 1.
    /// We could try to have ab, a => a in the future.
    value_t lgcd(const value_t& lhs, const value_t& rhs) const
    {
      using std::begin;
      using std::end;
      value_t res;
      // For each monomial, look for the matching GCD of the weight.
      auto i = begin(lhs), i_end = end(lhs);
      auto j = begin(rhs), j_end = end(rhs);
      for (;
           i != i_end && j != j_end
             && labelset()->equal(i->first, j->first);
           ++i, ++j)
        res.set(i->first, weightset()->lgcd(i->second, j->second));
      // If the sets of labels are different, the polynomials
      // cannot be "colinear", and the GCD is just 1.
      if (i != i_end || j != j_end)
        res = one();
      return res;
    }

    /*--------.
    | norm.   |
    `--------*/

    /// In the general case, normalize by the first (non null) weight.
    template <typename WeightSet, typename Dummy = void>
    struct norm_
    {
      typename WeightSet::value_t operator()(const value_t& v) const
      {
        return weight_of(front(v));
      }
      const WeightSet& ws_;
    };

    /// For Z, take the GCD, with the sign of the first value.
    template <typename Dummy>
    struct norm_<z, Dummy>
    {
      typename z::value_t operator()(const value_t& v) const
      {
        int sign = 0 < weight_of(detail::front(v)) ? 1 : -1;
        auto res = abs(weight_of(detail::front(v)));
        for (const auto& m: v)
          res = z_.lgcd(res, abs(weight_of(m)));
        res *= sign;
        return res;
      }
      const z& z_;
    };

    /// Compute the left GCD of weights which are polynomials.
    template <typename Ctx, typename Dummy>
    struct norm_<polynomialset<Ctx>, Dummy>
    {
      using ps_t = polynomialset<Ctx>;

      typename ps_t::value_t operator()(const value_t& v) const
      {
        typename ps_t::value_t res = weight_of(detail::front(v));
        for (const auto& p: v)
          res = ps_.lgcd(res, weight_of(p));
        return res;
      }
      const ps_t& ps_;
    };

    auto norm(const value_t& v) const
      -> decltype(norm_<weightset_t>{*this->weightset()}(v))
    {
      return norm_<weightset_t>{*weightset()}(v);
    }


    /*-------------.
    | normalize.   |
    `-------------*/

    /// Normalization: general case: just divide by the GCD of the
    /// weights.
    template <typename LabelSet>
    struct normalize_here_
    {
      weight_t operator()(value_t& v)
      {
        auto res = ps_.norm(v);
        ps_.ldiv_here(res, v);
        return res;
      }
      const polynomialset& ps_;
    };

    /// Normalization: when labels are words: left-factor the longest
    /// common prefix.
    template <typename GenSet>
    struct normalize_here_<wordset<GenSet>>
    {
      label_t operator()(value_t& v)
      {
        label_t res = label_of(detail::front(v));
        for (const auto& m: v)
          res = ps_.labelset()->lgcd(res, label_of(m));
        for (auto& m: v)
          label_of(m) = ps_.labelset()->ldiv(res, label_of(m));
        return res;
      }
      const polynomialset& ps_;
    };

    /// Normalize v in place, and return the factor which was divided.
    auto normalize_here(value_t& v) const
      -> decltype(normalize_here_<labelset_t>{*this}(v))
    {
      auto n = normalize_here_<labelset_t>{*this};
      return n(v);
    }



    /*---------------.
    | tuple(v...).   |
    `---------------*/

    /// Build a tuple of polynomials: (e.E+f.F)|(g.G+h.H)
    /// => eg.(E|G) + eh.(E|H) + fg.(F|G) + fh.(F|H).
    template <typename... Polys>
    auto
    tuple(Polys&&... vs) const
      -> value_t
    {
      auto res = value_t{};
      detail::cross([&res, this](auto... ms)
                    {
                      this->add_here(res,
                                     this->labelset()->tuple(ms.first...),
                                     this->weightset()->mul(ms.second...));
                    },
                    std::forward<Polys>(vs)...);
      return res;
    }



    /*---------------.
    | equal(l, r).   |
    `---------------*/

    ATTRIBUTE_PURE
    static bool monomial_equal(const monomial_t& lhs,
                               const monomial_t& rhs)
    {
      return (labelset_t::equal(label_of(lhs), label_of(rhs))
              && weightset_t::equal(weight_of(lhs), weight_of(rhs)));
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    equal_impl(const value_t& l, const value_t& r)
      -> enable_if_t<WetType != wet_kind_t::bitset,
                     bool>
    {
      return boost::equal(l, r, monomial_equal);
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    equal_impl(const value_t& l, const value_t& r)
      -> enable_if_t<WetType == wet_kind_t::bitset,
                     bool>
    {
      return l.set() == r.set();
    }

    ATTRIBUTE_PURE
    static bool
    equal(const value_t& l, const value_t& r)
    {
      return equal_impl<value_t::kind>(l, r);
    }

    const value_t&
    one() const
    {
      static value_t res{monomial_one()};
      return res;
    }

    const monomial_t&
    monomial_one() const
    {
      static monomial_t res{labelset()->one(), weightset()->one()};
      return res;
    }

    bool
    is_one(const value_t& v) const ATTRIBUTE_PURE
    {
      if (v.size() != 1)
        return false;
      auto i = v.find(labelset()->one());
      if (i == v.end())
        return false;
      return weightset()->is_one(i->second);
    }

    const value_t&
    zero() const
    {
      static value_t res;
      return res;
    }

    bool
    is_zero(const value_t& v) const
    {
      return v.empty();
    }

    static constexpr bool show_one() { return false; }
    static constexpr star_status_t star_status()
    {
      return weightset_t::star_status();
    }

    /// Conversion from (this and) other weightsets.
    static value_t
    conv(self_t, value_t v)
    {
      return v;
    }

    /// FIXME: use enable_if to prevent this from being instantiated
    /// when WS is a polynomialset.  Then use this same technique for
    /// expressions.
    template <typename WS>
    value_t
    conv(const WS& ws, const typename WS::value_t& v) const
    {
      return {{labelset()->one(), weightset()->conv(ws, v)}};
    }

    /// Convert from another polynomialset to type_t.
    template <typename C>
    value_t
    conv(const polynomialset<C>& sps,
         const typename polynomialset<C>::value_t& v) const
    {
      value_t res;
      typename C::labelset_t sls = * sps.labelset();
      typename C::weightset_t sws = * sps.weightset();
      labelset_t tls = * labelset();
      weightset_t tws = * weightset();
      for (const auto& m: v)
        add_here(res, tls.conv(sls, label_of(m)), tws.conv(sws, weight_of(m)));
      return res;
    }


    /*--------------.
    | less(l, r).   |
    `--------------*/

    ATTRIBUTE_PURE
    static bool monomial_less(const monomial_t& lhs, const monomial_t& rhs)
    {
      if (labelset_t::less(label_of(lhs), label_of(rhs)))
        return true;
      else if (labelset_t::less(label_of(rhs), label_of(lhs)))
        return false;
      else
        return weightset_t::less(weight_of(lhs), weight_of(rhs));
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    less_impl(const value_t& l, const value_t& r)
      -> enable_if_t<WetType != wet_kind_t::bitset,
                     bool>
    {
      return boost::range::lexicographical_compare(l, r, monomial_less);
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    less_impl(const value_t& l, const value_t& r)
      -> enable_if_t<WetType == wet_kind_t::bitset,
                     bool>
    {
      return l.set() < r.set();
    }

    ATTRIBUTE_PURE
    static bool
    less(const value_t& l, const value_t& r)
    {
      return less_impl<value_t::kind>(l, r);
    }


    value_t
    transpose(const value_t& v) const
    {
      value_t res;
      for (const auto& i: v)
        res.set(labelset()->transpose(label_of(i)),
                weightset()->transpose(weight_of(i)));
      return res;
    }


    /*--------.
    | hash.   |
    `--------*/
    ATTRIBUTE_PURE
    static size_t hash(const monomial_t& m, size_t res = 0)
    {
      hash_combine(res, labelset_t::hash(label_of(m)));
      hash_combine(res, weightset_t::hash(weight_of(m)));
      return res;
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    hash_impl(const value_t& p)
      -> enable_if_t<WetType != wet_kind_t::bitset,
                     size_t>
    {
      size_t res = 0;
      for (const auto& m: p)
        res = hash(m, res);
      return res;
    }

    template <wet_kind_t WetType>
    ATTRIBUTE_PURE
    static auto
    hash_impl(const value_t& p)
      -> enable_if_t<WetType == wet_kind_t::bitset,
                     size_t>
    {
      return hash_value(p.set());
    }

    ATTRIBUTE_PURE
    static size_t hash(const value_t& v)
    {
      return hash_impl<value_t::kind>(v);
    }


    /// Build from the description in \a is.
    static self_t make(std::istream& is)
    {
      // name is, for instance, "polynomialset<lal_char(abcd), z>".
      eat(is, "polynomialset<");
      auto ctx = Context::make(is);
      eat(is, '>');
      return {ctx};
    }

    std::ostream&
    print_set(std::ostream& o, format fmt = {}) const
    {
      if (fmt == format::latex)
        {
          o << "\\mathsf{Poly}[";
          labelset()->print_set(o, fmt);
          o << " \\to ";
          weightset()->print_set(o, fmt);
          o << "]";
        }
      else
        {
          o << "polynomialset<";
          labelset()->print_set(o, fmt);
          o << "_";
          weightset()->print_set(o, fmt);
          o << ">";
        }
      return o;
    }

    /// Read a label, if there is one.
    ///
    /// Does not handle `\z`, nor letter classes.
    ///
    /// \returns  none if there is no label.
    boost::optional<label_t>
    conv_label(std::istream& i, bool weighted, const char sep = '+') const
    {
      int peek = i.peek();
      assert(peek != '[');
      if (peek == '\\')
        {
          i.ignore();
          if (i.peek() == 'z')
            {
              i.ignore();
              return boost::none;
            }
          else
            i.unget();
        }

      // The label is not \z.
      // Check if there is a label that comes.  Or rather, check if
      // there is something else than EOF or the separator, in which
      // case it must be a label.
      label_t res;
      if (peek == EOF || peek == sep || isspace(peek))
        {
          // There is no label.  This counts as '$', the special
          // label.
          //
          // Indeed, that's how we represent the initial and final
          // transitions: '$ -> 0 "<2>"'.  Using the one label is
          // tempting, but it does not exist for lal_char for
          // instance.  And it would be wrong to have '\e' when we
          // can, and '$' otherwise...
          //
          // However, we must have at least a weight: a completely
          // empty mononial ($ -> 0 "<2>,") is invalid.
          require(weighted,
                  sname(), ": conv: invalid monomial: ",
                  str_escape(peek),
                  " (did you mean \\e or \\z?)");
          res = labelset()->special();
        }
      else
        res = labelset()->conv(i);
      return res;
    }

    /// Read a weight, if there is one, bracketed.
    weight_t
    conv_weight(std::istream& i) const
    {
      if (i.peek() == langle)
        // FIXME: convert to use conv(std::istream).
        //
        // The problem is when we have a rational expression as a
        // weight: in that case, conv expect to parse up to EOF, not
        // up to '>'.  We first need to fix the parsing of expression
        // to work on a flow, to be able to use weightset()->conv
        // here.  Which means to get back the stream from a Flex
        // scanner.  It might not be easy.
        return ::vcsn::conv(*weightset(), bracketed(i, langle, rangle));
      else
        return weightset()->one();
    }

    /// Read a monomial from a stream.
    ///
    /// \param i    the stream to parse
    /// \param sep  the separator between monomials.
    ///
    /// \returns boost::none on EOF
    boost::optional<monomial_t>
    conv_monomial(std::istream& i, const char sep = '+') const
    {
#define SKIP_SPACES()                           \
      while (isspace(i.peek()))                 \
        i.ignore()

      // Possibly a weight in braces.
      SKIP_SPACES();
      if (i.peek() == -1)
        return boost::none;

      bool weighted = i.peek() == langle;
      weight_t w = conv_weight(i);

      // Possibly, a label.
      SKIP_SPACES();
      auto l = conv_label(i, weighted, sep);
      require(l != boost::none,
              "\\z is invalid for monomials");
      return monomial_t{l.get(), w};
#undef SKIP_SPACES
    }

    /// Read a polynomial from a stream.
    ///
    /// Somewhat more general than a mere reversal of "format",
    /// in particular "a+a" is properly understood as "<2>a" in
    /// char_z.
    ///
    /// \param i    the stream to parse
    /// \param sep  the separator between monomials.
    value_t
    conv(std::istream& i, const char sep = '+') const
    {
      value_t res;
#define SKIP_SPACES()                           \
      while (isspace(i.peek()))                 \
        i.ignore()

      do
        {
          // Possibly a weight in braces.
          SKIP_SPACES();
          bool weighted = i.peek() == langle;
          weight_t w = conv_weight(i);

          SKIP_SPACES();
          // Possibly, a label.
          // Handle label classes.
          if (i.peek() == '[')
            labelset()->convs(i, [this, &res, &w](const label_t& l)
                              {
                                add_here(res, l, w);
                              });
          else if (auto l = conv_label(i, weighted, sep))
            add_here(res, l.get(), w);

          // sep (e.g., '+'), or stop parsing.
          SKIP_SPACES();
          if (i.peek() == sep)
            i.ignore();
          else
            break;
        }
      while (true);
#undef SKIP_SPACES

      return res;
    }

    /// Print a monomial.
    std::ostream&
    print(const monomial_t& m, std::ostream& out,
          format fmt = {}) const
    {
      static bool parens = getenv("VCSN_PARENS");
      print_weight_(weight_of(m), out, fmt);
      if (parens)
        out << (fmt == format::latex ? "\\left(" : "(");
      labelset()->print(label_of(m), out, fmt.for_labels());
      if (parens)
        out << (fmt == format::latex ? "\\right)" : ")");
      return out;
    }

    /// Print a value (a polynomial).
    ///
    /// \param v       the polynomial
    /// \param out     the output stream
    /// \param fmt     the format: "text" or "latex"
    /// \param sep     the separator between monomials
    std::ostream&
    print(const value_t& v, std::ostream& out,
          format fmt = {},
          const std::string& sep = " + ") const
    {
      bool latex = fmt == format::latex;
      if (is_zero(v))
        out << (latex ? "\\emptyset" : "\\z");
      else
        print_<context_t>(v, out, fmt,
                          latex && sep == " + " ? " \\oplus " : sep);
      return out;
    }

  private:
    /// Print a weight.
    std::ostream&
    print_weight_(const weight_t& w, std::ostream& out,
                  format fmt) const
    {
      static bool parens = getenv("VCSN_PARENS");
      if (parens || weightset()->show_one() || !weightset()->is_one(w))
        {
          out << (fmt == format::latex ? "\\left\\langle " : std::string{langle});
          weightset()->print(w, out, fmt.for_weights());
          out << (fmt == format::latex ? "\\right\\rangle " : std::string{rangle});
        }
      return out;
    }

    /// Print a polynomial value without classes.
    std::ostream&
    print_without_classes_(const value_t& v, std::ostream& out,
                           format fmt,
                           const std::string& sep) const
    {
      bool first = true;
      for (const auto& m: v)
        {
          if (!first)
            out << sep;
          first = false;
          print(m, out, fmt);
        }
      return out;
    }

    /// Print a polynomial value with classes.
    std::ostream&
    print_with_classes_(const value_t& v, std::ostream& out,
                        format fmt,
                        const std::string& sep) const
    {
      using std::begin;
      using std::end;

      // No classes if not at least 3 elements.
      if (sep == " + " || v.size() <= 2)
        return print_without_classes_(v, out, fmt, sep);

      // No classes if the weights of the letters aren't all the same.
      auto first_letter
        = boost::find_if(v,
                         [this](const monomial_t& m)
                         {
                           return !labelset()->is_one(label_of(m));
                         });
      auto w = weight_of(*first_letter);
      if (!std::all_of(std::next(first_letter), end(v),
                       [this, w](const monomial_t& m)
                       {
                         return weightset()->equal(weight_of(m), w);
                       }))
        return print_without_classes_(v, out, fmt, sep);

      // Print with classes.  First, the constant-term.
      if (first_letter != begin(v))
        {
          print(detail::front(v), out, fmt);
          if (1 < v.size())
            out << sep;
        }

      // The weight.
      print_weight_(w, out, fmt);

      // Gather the letters.  We can use a vector, as we know that the
      // labels are already sorted, and random access iteration will
      // be handy below.
      std::vector<label_t> letters;
      for (const auto& m: v)
        if (!labelset()->is_one(label_of(m)))
          letters.push_back(label_of(m));

      // Print the character class.  'letters' are sorted, since
      // polynomials are shortlex-sorted on the labels.
      print_label_class(*labelset(), letters, out, fmt.for_labels());
      return out;
    }

    /// Print a non-null value for a non letterized labelset.
    template <typename Ctx>
    vcsn::enable_if_t<!labelset_t_of<Ctx>::is_letterized(),
                      std::ostream&>
    print_(const value_t& v, std::ostream& out,
           format fmt = {},
           const std::string& sep = " + ") const
    {
      return print_without_classes_(v, out, fmt, sep);
    }

    /// Print a non-null value for a letterized labelset (e.g., letterset
    /// or nullableset.
    template <typename Ctx>
    vcsn::enable_if_t<labelset_t_of<Ctx>::is_letterized(),
                      std::ostream&>
    print_(const value_t& v, std::ostream& out,
           format fmt = {},
           const std::string& sep = " + ") const
    {
      return print_with_classes_(v, out, fmt, sep);
    }


  private:
    context_t ctx_;

    /// Left marker for weight in concrete syntax.
    constexpr static char langle = '<';
    /// Right marker for weight in concrete syntax.
    constexpr static char rangle = '>';
  };

  namespace detail
  {
    template <typename Ctx1, typename Ctx2>
    struct join_impl<polynomialset<Ctx1>, polynomialset<Ctx2>>
    {
      using type = polynomialset<join_t<Ctx1, Ctx2>>;
      static type join(const polynomialset<Ctx1>& ps1,
                       const polynomialset<Ctx2>& ps2)
      {
        return {vcsn::join(ps1.context(), ps2.context())};
      }
    };

    template <typename WS1, typename Ctx2>
    struct join_impl<WS1, polynomialset<Ctx2>>
    {
      using type
        = polynomialset<context<typename Ctx2::labelset_t,
                                join_t<WS1, typename Ctx2::weightset_t>>>;
      static type join(const WS1& ws1, const polynomialset<Ctx2>& ps2)
      {
        return {*ps2.labelset(), vcsn::join(ws1, *ps2.weightset())};
      }
    };
  }
}
