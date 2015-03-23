#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>

#include <vcsn/ctx/context.hh> // We need context to define join.
#include <vcsn/weightset/fwd.hh>

#include <vcsn/ctx/traits.hh>
#include <vcsn/misc/attributes.hh>
#include <vcsn/misc/functional.hh>
#include <vcsn/misc/map.hh>
#include <vcsn/misc/math.hh>
#include <vcsn/misc/raise.hh>
#include <vcsn/misc/star_status.hh>
#include <vcsn/misc/stream.hh>
#include <vcsn/misc/zip-maps.hh>

#include <vcsn/labelset/wordset.hh>
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

    template <typename Context>
    struct is_division_ring<polynomialset<Context>>
      : std::false_type
    {};
  }

  /// Linear combination of labels: map labels to weights.
  /// \tparam Context  the LabelSet and WeightSet types.
  template <typename Context>
  class polynomialset
  {
  public:
    using self_type = polynomialset<Context>;
    using context_t = Context;
    using labelset_t = labelset_t_of<context_t>;
    using weightset_t = weightset_t_of<context_t>;
    using polynomialset_t = polynomialset<context_t>;

    using labelset_ptr = typename context_t::labelset_ptr;
    using weightset_ptr = typename context_t::weightset_ptr;
    /// Polynomials over labels.
    using label_t = typename labelset_t::value_t;
    using weight_t = weight_t_of<context_t>;

    using value_t = std::map<label_t, weight_t, vcsn::less<labelset_t>>;
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
    value_t&
    set_weight(value_t& v, const label_t& l, const weight_t k) const
    {
      if (weightset()->is_zero(k))
        del_weight(v, l);
      else
        v[l] = k;
      return v;
    }

    /// `v += p`.
    value_t&
    add_here(value_t& v, const value_t& p) const
    {
      for (const auto& m: p)
        add_here(v, m);
      return v;
    }

    /// `v += m`.
    value_t&
    add_here(value_t& v, const monomial_t& p) const
    {
      return add_here(v, p.first, p.second);
    }

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
              // Do not use set_weight() because it would lookup w
              // again and we already have the right iterator.
              auto w2 = weightset()->add(i->second, k);
              if (weightset()->is_zero(w2))
                v.erase(i);
              else
                i->second = w2;
            }
        }
      return v;
    }

    /// `v -= <k>l`.
    value_t&
    sub_here(value_t& v, const monomial_t& r) const
    {
      if (!label_is_zero(*labelset(), &r.first))
        {
          auto i = v.find(r.first);
          if (i == v.end())
            {
              raise(sname(), ": sub_here: invalid arguments: ",
                    to_string(*this, v), ", ", to_string(*this, r));
            }
          else
            {
              // Do not use set_weight() because it would lookup w
              // again and we already have the right iterator.
              auto w2 = weightset()->sub(i->second, r.second);
              if (weightset()->is_zero(w2))
                v.erase(i);
              else
                i->second = w2;
            }
        }
      return v;
    }

    const weight_t
    get_weight(const value_t& v, const label_t& l) const ATTRIBUTE_PURE
    {
      auto i = v.find(l);
      if (i == v.end())
        return weightset()->zero();
      else
        return i->second;
    }

    /// The sum of polynomials \a l and \a r.
    value_t
    add(const value_t& l, const value_t& r) const
    {
      value_t res = l;
      add_here(res, r);
      return res;
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

    /// The product of monomials \a l and \a r.
    monomial_t
    mul(const monomial_t& l, const monomial_t& r) const
    {
      return {labelset()->mul(l.first, r.first),
              weightset()->mul(l.second, r.second)};
    }

    /// The product of polynomials \a l and \a r.
    value_t
    mul(const value_t& l, const value_t& r) const
    {
      value_t res;
      for (const auto& lm: l)
        for (const auto& rm: r)
          add_here(res,
                   labelset()->mul(lm.first, rm.first),
                   weightset()->mul(lm.second, rm.second));
      return res;
    }

    /// The conjunction of polynomials \a l and \a r.
    /// Not valid for all the labelsets.
    value_t
    conjunction(const value_t& l, const value_t& r) const
    {
      value_t res;
      for (auto i: l)
        for (auto j: r)
          add_here(res,
                   labelset()->conjunction(i.first, j.first),
                   weightset()->mul(i.second, j.second));
      return res;
    }

    /// The sum of the weights of the common labels.
    weight_t
    scalar_product(const value_t& l, const value_t& r) const
    {
      weight_t res = weightset()->zero();
      for (auto i: zip_maps<vcsn::as_tuple>(l, r))
        res = weightset()->add(res,
                               weightset()->mul(std::get<0>(i).second,
                                                std::get<1>(i).second));
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
          add_here(res, m.first, weightset()->mul(w, m.second));
      return res;
    }

    /// Left product by a label.
    value_t
    lmul_label(const label_t& lhs, const value_t& v) const
    {
      value_t res;
      for (const auto& m: v)
        add_here(res,
                 labelset()->mul(lhs, m.first),
                 m.second);
      return res;
    }

    /// Left product by a monomial.
    value_t
    mul(const monomial_t& lhs, const value_t& v) const
    {
      value_t res;
      for (const auto& m: v)
        add_here(res,
                 labelset()->mul(lhs.first, m.first),
                 weightset()->mul(lhs.second, m.second));
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
          add_here(res, labelset()->rmul(m.first, w), m.second);
      return res;
    }

    /// Right product.
    value_t
    rmul_label(const value_t& v, const label_t& rhs) const
    {
      value_t res;
      for (auto i: v)
        add_here(res,
                 labelset()->mul(i.first, rhs),
                 i.second);
      return res;
    }

    /// Right product by a monomial.
    value_t
    mul(const value_t& l, const monomial_t& rhs) const
    {
      value_t res;
      for (const auto& lhs: l)
        add_here(res,
                 labelset()->mul(lhs.first, rhs.first),
                 weightset()->mul(lhs.second, rhs.second));
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
      return {labelset()->ldiv(l.first, r.first),
              weightset()->ldiv(l.second, r.second)};
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
              auto factor = ldiv(*begin(l), *begin(remainder));
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
        for (auto& m: v)
          m.second = weightset()->ldiv(w, m.second);
      return v;
    }

    /// Right exterior division.
    value_t&
    rdiv_here(value_t& v, const weight_t& w) const
    {
      if (!weightset()->is_one(w))
        for (auto& m: v)
          m.second = weightset()->rdiv(m.second, w);
      return v;
    }

    /// In the general case, normalize by the first (non null) weight.
    template <typename WeightSet, typename Dummy = void>
    struct norm_
    {
      typename WeightSet::value_t operator()(const value_t& v) const
      {
        return begin(v)->second;
      }
      const WeightSet& ws_;
    };

    /// For Z, take the GCD, with the sign of the first value.
    template <typename Dummy>
    struct norm_<z, Dummy>
    {
      typename z::value_t operator()(const value_t& v) const
      {
        int sign = 0 < begin(v)->second ? 1 : -1;
        auto res = abs(begin(v)->second);
        for (const auto& m: v)
          res = z_.lgcd(res, abs(m.second));
        res *= sign;
        return res;
      }
      const z& z_;
    };

    value_t lgcd(const value_t& lhs, const value_t& rhs) const
    {
      value_t res;
      // For each monomial, look for the matching GCD of the weight.
      auto i = begin(lhs), i_end = end(lhs);
      auto j = begin(rhs), j_end = end(rhs);
      for (;
           i != i_end && j != j_end
             && labelset()->equal(i->first, j->first);
           ++i, ++j)
        res[i->first] = weightset()->lgcd(i->second, j->second);
      // If the sets of labels are different, the polynomials
      // cannot be "colinear", and the GCD is just 1.
      if (i != i_end || j != j_end)
        res = one();
      return res;
    }

    /// Compute the left GCD of weights which are polynomials.
    template <typename Ctx, typename Dummy>
    struct norm_<polynomialset<Ctx>, Dummy>
    {
      using ps_t = polynomialset<Ctx>;

      typename ps_t::value_t operator()(const value_t& v) const
      {
        typename ps_t::value_t res = begin(v)->second;
        for (const auto& p: v)
          res = ps_.lgcd(res, p.second);
        return res;
      }
      const ps_t& ps_;
    };

    auto norm(const value_t& v) const
      -> decltype(norm_<weightset_t>{*this->weightset()}(v))
    {
      return norm_<weightset_t>{*weightset()}(v);
    }

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
        label_t res = begin(v)->first;
        for (const auto& m: v)
          res = ps_.labelset()->lgcd(res, m.first);
        for (auto& m: v)
          m.first = ps_.labelset()->ldiv(res, m.first);
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

    ATTRIBUTE_PURE
    static bool monomial_equal(const monomial_t& lhs,
                                const monomial_t& rhs)
    {
      return (labelset_t::equal(lhs.first, rhs.first)
              && weightset_t::equal(lhs.second, rhs.second));
    }

    static bool
    equal(const value_t& l, const value_t& r) ATTRIBUTE_PURE
    {
      return l.size() == r.size()
        && std::equal(l.begin(), l.end(), r.begin(),
                      monomial_equal);
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
    conv(self_type, value_t v)
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
        add_here(res, tls.conv(sls, m.first), tws.conv(sws, m.second));
      return res;
    }


    ATTRIBUTE_PURE
    static bool monomial_less(const monomial_t& lhs,
                                   const monomial_t& rhs)
    {
      if (labelset_t::less(lhs.first, rhs.first))
        return true;
      else if (labelset_t::less(rhs.first, lhs.first))
        return false;
      else
        return weightset_t::less(lhs.second, rhs.second);
    }

    static bool less(const value_t& lhs,
                          const value_t& rhs)
    {
      return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                          rhs.begin(), rhs.end(),
                                          monomial_less);
    }

    value_t
    transpose(const value_t& v) const
    {
      value_t res;
      for (const auto& i: v)
        res[labelset()->transpose(i.first)] = weightset()->transpose(i.second);
      return res;
    }

    ATTRIBUTE_PURE
    static size_t hash(const monomial_t& m)
    {
      size_t res = 0;
      hash_combine(res, labelset_t::hash(m.first));
      hash_combine(res, weightset_t::hash(m.second));
      return res;
    }

    ATTRIBUTE_PURE
    static size_t hash(const value_t& v)
    {
      size_t res = 0;
      for (const auto& m: v)
        {
          hash_combine(res, labelset_t::hash(m.first));
          hash_combine(res, weightset_t::hash(m.second));
        }
      return res;
    }

    /// Build from the description in \a is.
    static self_type make(std::istream& is)
    {
      // name is, for instance, "polynomialset<lal_char(abcd), z>".
      eat(is, "polynomialset<");
      auto ctx = Context::make(is);
      eat(is, '>');
      return {ctx};
    }

    std::ostream&
    print_set(std::ostream& o, const std::string& format = "text") const
    {
      if (format == "latex")
        {
          o << "\\mathsf{Poly}[";
          labelset()->print_set(o, format);
          o << " \\to ";
          weightset()->print_set(o, format);
          o << "]";
        }
      else
        {
          o << "polynomialset<";
          labelset()->print_set(o, format);
          o << "_";
          weightset()->print_set(o, format);
          o << ">";
        }
      return o;
    }

    /// Construct from a string.
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
          weight_t w = weightset()->one();
          bool default_w = true;
          if (i.peek() == langle)
            {
              // FIXME: convert to use conv(std::istream).
              w = ::vcsn::conv(*weightset(), bracketed(i, langle, rangle));
              default_w = false;
            }

          // Possibly, a label.
          SKIP_SPACES();
          // Whether the label is \z.
          bool is_zero = false;
          if (i.peek() == '\\')
            {
              i.ignore();
              if (i.peek() == 'z')
                {
                  is_zero = true;
                  i.ignore();
                }
              else
                i.unget();
            }

          if (!is_zero)
            {
              // The label is not \z.
              int peek = i.peek();
              // Handle ranges
              if (peek == '[')
                for (auto c : labelset()->convs(i))
                  add_here(res, c, w);
              else
                {
                  // Check if there is a label that comes.  Or rather,
                  // check if there is something else than EOF or the
                  // separator, in which case it must be a label.
                  label_t label;
                  if (peek == EOF || peek == sep || isspace(peek))
                    {
                      // There is no label.  This counts as '$', the
                      // special label.
                      //
                      // Indeed, that how we represent the initial and
                      // final transitions: '$ -> 0 "<2>"'.  Using the
                      // one label is tempting, but it does not exist
                      // for lal_char for instance.  And it would be
                      // wrong to have '\e' when we can, and '$'
                      // otherwise...
                      //
                      // However, we must have at least a weight: a
                      // completely empty mononial ($ -> 0 "<2>,") is
                      // invalid.
                      require(!default_w,
                              sname(), ": conv: invalid monomial: ",
                              str_escape(peek),
                              " (did you mean \\e or \\z?)");
                      label = labelset()->special();
                    }
                  else
                    label = labelset()->conv(i);
                  add_here(res, label, w);
                }
            }

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
          const std::string& format = "text") const
    {
      static bool parens = getenv("VCSN_PARENS");
      print_weight_(m.second, out, format);
      if (parens)
        out << (format == "latex" ? "\\left(" : "(");
      labelset()->print(m.first, out, format);
      if (parens)
        out << (format == "latex" ? "\\right)" : ")");
      return out;
    }

    /// Print a value (a polynomial).
    ///
    /// \param v       the polynomial
    /// \param out     the output stream
    /// \param format  the format: "text" or "latex"
    /// \param sep     the separator between monomials
    std::ostream&
    print(const value_t& v, std::ostream& out,
          const std::string& format = "text",
          const std::string& sep = " + ") const
    {
      bool latex = format == "latex";
      if (v.empty())
        out << (latex ? "\\emptyset" : "\\z");
      else
        print_<context_t>(v, out, format,
                          latex && sep == " + " ? " \\oplus " : sep);
      return out;
    }

  private:
    /// Print a weight.
    std::ostream&
    print_weight_(const weight_t& w, std::ostream& out,
                  const std::string& format = "text") const
    {
      static bool parens = getenv("VCSN_PARENS");
      if (parens || weightset()->show_one() || !weightset()->is_one(w))
        {
          out << (format == "latex" ? "\\left\\langle " : std::string{langle});
          weightset()->print(w, out, format);
          out << (format == "latex" ? "\\right\\rangle " : std::string{rangle});
        }
      return out;
    }

    /// Print a polynomial value without ranges.
    std::ostream&
    print_without_ranges_(const value_t& v, std::ostream& out,
                          const std::string& format = "text",
                          const std::string& sep = " + ") const
    {
      bool first = true;
      for (const auto& m: v)
        {
          if (!first)
            out << sep;
          first = false;
          print(m, out, format);
        }
      return out;
    }

    /// Print a polynomial value with ranges.
    std::ostream&
    print_with_ranges_(const value_t& v, std::ostream& out,
                       const std::string& format = "text",
                       const std::string& sep = " + ") const
    {
      if (sep == " + " || v.size() <= 2)
        return print_without_ranges_(v, out, format, sep);

      // No ranges if the weights of the letters aren't all the same.
      //
      // While at it, gather the letters.  We can use a vector, as we
      // know that the labels are already sorted, and random access
      // iteration will be handy below.
      std::vector<label_t> letters;
      weight_t first_w = weightset()->zero();
      for (const auto& m: v)
        if (!labelset()->is_one(m.first))
          {
            if (weightset()->is_zero(first_w))
              first_w = m.second;
            else if (!weightset()->equal(m.second, first_w))
              return print_without_ranges_(v, out, format, sep);

            letters.push_back(m.first);
          }

      // Print with ranges.  First, the constant-term.
      if (labelset()->is_one(std::begin(v)->first))
        {
          print(*std::begin(v), out, format);
          if (1 < v.size())
            out << sep;
        }

      // The weight.
      print_weight_(first_w, out, format);

      // Print the character class.  letters are sorted, since
      // polynomials are shortlex-sorted on the labels.
      out << '[';
      std::vector<label_t> alphabet;
      for (auto l : labelset()->genset())
        alphabet.emplace_back(labelset()->value(l));
      for (auto it = std::begin(letters), letters_end = std::end(letters);
           it != letters_end; ++it)
        {
          auto end
            = std::mismatch(it, letters_end,
                            std::find(std::begin(alphabet), std::end(alphabet),
                                      *it)).first;
          labelset()->print(*it, out, format);
          // No ranges for two letters or less.
          auto width = std::distance(it, end);
          if (2 < width)
            {
              it += width - 1;
              out << '-';
              labelset()->print(*it, out, format);
            }
        }
      out << ']';

      return out;
    }

    /// Print a non-null value for a non letterized labelset.
    template <typename Ctx>
    vcsn::enable_if_t<!labelset_t_of<Ctx>::is_letterized(),
                      std::ostream&>
    print_(const value_t& v, std::ostream& out,
           const std::string& format = "text",
           const std::string& sep = " + ") const
    {
      return print_without_ranges_(v, out, format, sep);
    }

    /// Print a non-null value for a letterized labelset (e.g., letterset
    /// or nullableset.
    template <typename Ctx>
    vcsn::enable_if_t<labelset_t_of<Ctx>::is_letterized(),
                      std::ostream&>
    print_(const value_t& v, std::ostream& out,
           const std::string& format = "text",
           const std::string& sep = " + ") const
    {
      return print_with_ranges_(v, out, format, sep);
    }


  private:
    context_t ctx_;

    /// Left marker for weight in concrete syntax.
    constexpr static char langle = '<';
    /// Right marker for weight in concrete syntax.
    constexpr static char rangle = '>';
  };

  /// The entry between two states of an automaton.
  template <typename Aut>
  typename polynomialset<context_t_of<Aut>>::value_t
  get_entry(const Aut& aut,
            state_t_of<Aut> s, state_t_of<Aut> d)
  {
    using automaton_t = Aut;
    using context_t = context_t_of<automaton_t>;
    using polynomialset_t = polynomialset<context_t>;
    using polynomial_t = typename polynomialset_t::value_t;

    polynomial_t res;
    for (auto t : aut->outin(s, d))
      // Bypass set_weight(), because we know that the weight is
      // nonzero, and that there is only one weight per letter.
      res[aut->label_of(t)] = aut->weight_of(t);
    return res;
  }

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
