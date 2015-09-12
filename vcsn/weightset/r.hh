#pragma once

#include <string>
#include <ostream>

#include <vcsn/core/join.hh>
#include <vcsn/misc/format.hh>
#include <vcsn/misc/functional.hh> // hash_value
#include <vcsn/misc/raise.hh>
#include <vcsn/misc/star-status.hh>
#include <vcsn/misc/stream.hh>
#include <vcsn/weightset/fwd.hh>
#include <vcsn/weightset/b.hh>
#include <vcsn/weightset/q.hh>
#include <vcsn/weightset/qmp.hh>
#include <vcsn/weightset/z.hh>
#include <vcsn/weightset/weightset.hh>

namespace vcsn
{
  namespace detail
  {
  class r_impl
  {
  public:
    using self_t = r;

    static symbol sname()
    {
      static symbol res("r");
      return res;
    }

    /// Build from the description in \a is.
    static r make(std::istream& is)
    {
      eat(is, sname());
      return {};
    }

    using value_t = double;

    static value_t
    zero()
    {
      return 0.;
    }

    static value_t
    one()
    {
      return 1.;
    }

    static value_t
    add(const value_t l, const value_t r)
    {
      return l + r;
    }

    static value_t
    sub(const value_t l, const value_t r)
    {
      return l - r;
    }

    static value_t
    mul(const value_t l, const value_t r)
    {
      return l * r;
    }

    static value_t
    lgcd(const value_t l, const value_t r)
    {
      require(!is_zero(l), sname(), ": lgcd: invalid lhs: zero");
      require(!is_zero(r), sname(), ": lgcd: invalid rhs: zero");
      return l;
    }

    static value_t
    rgcd(const value_t l, const value_t r)
    {
      return lgcd(l, r);
    }

    static value_t
    rdiv(const value_t l, const value_t r)
    {
      require(!is_zero(r), "div: division by zero");
      return l / r;
    }

    static value_t
    ldiv(const value_t l, const value_t r)
    {
      return rdiv(r, l);
    }

    value_t
    star(const value_t v) const
    {
      if (-1 < v && v < 1)
        return 1/(1-v);
      else
        raise(sname(), ": star: invalid value: ", v);
    }

    constexpr static bool is_special(const value_t)
    {
      return false;
    }

    static bool
    is_zero(const value_t v)
    {
      return v == 0;
    }

    static bool
    is_one(const value_t v)
    {
      return v == 1;
    }

    static bool
    equal(const value_t l, const value_t r)
    {
      return l == r;
    }

    /// Whether \a lhs < \a rhs.
    static bool less(const value_t lhs, const value_t rhs)
    {
      return lhs < rhs;
    }

    static constexpr bool is_commutative() { return true; }
    static constexpr bool is_idempotent() { return false; }

    static constexpr bool show_one() { return false; }
    static constexpr star_status_t star_status() { return star_status_t::ABSVAL; }

    static value_t
    abs(const value_t v)
    {
      return v < 0 ? -v : v;
    }

    static value_t
    transpose(const value_t v)
    {
      return v;
    }

    static size_t hash(const value_t v)
    {
      return hash_value(v);
    }

    static value_t
    conv(self_t, const value_t v)
    {
      return v;
    }

    static value_t
    conv(q, const q::value_t v)
    {
      return value_t(v.num) / value_t(v.den);
    }

    static value_t
    conv(z, const z::value_t v)
    {
      return v;
    }

    static value_t
    conv(b, const b::value_t v)
    {
      return v;
    }

    static value_t
    conv(std::istream& i, bool = true)
    {
      value_t res;
      if (i >> res)
        return res;
      else
        raise(sname(), ": invalid value: ", i);
    }

    static std::ostream&
    print(const value_t v, std::ostream& o,
          format = {})
    {
      return o << v;
    }

    std::ostream&
    print_set(std::ostream& o, format fmt = {}) const
    {
      if (fmt == format::latex)
        o << "\\mathbb{R}";
      else if (fmt == format::text)
        o << sname();
      else
        raise("invalid format: ", fmt);
      return o;
    }
  };

    VCSN_JOIN_SIMPLE(b, r);
    VCSN_JOIN_SIMPLE(z, r);
    VCSN_JOIN_SIMPLE(q, r);
    VCSN_JOIN_SIMPLE(qmp, r);
    VCSN_JOIN_SIMPLE(r, r);
  }

}
