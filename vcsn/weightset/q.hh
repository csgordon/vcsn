#pragma once

#include <string>
#include <ostream>

#include <vcsn/core/join.hh>
#include <vcsn/misc/format.hh>
#include <vcsn/misc/functional.hh> // hash_combine
#include <vcsn/misc/math.hh>
#include <vcsn/misc/raise.hh>
#include <vcsn/misc/star-status.hh>
#include <vcsn/misc/stream.hh> // eat
#include <vcsn/misc/symbol.hh>
#include <vcsn/weightset/b.hh>
#include <vcsn/weightset/fwd.hh>
#include <vcsn/weightset/weightset.hh>
#include <vcsn/weightset/z.hh>

namespace vcsn
{
  namespace detail
  {
  class q_impl
  {
  public:
    using self_t = q;

    static symbol sname()
    {
      static symbol res("q");
      return res;
    }

    /// Build from the description in \a is.
    static q make(std::istream& is)
    {
      eat(is, sname());
      return {};
    }

    struct value_t
    {
      value_t(int n = 0, unsigned d = 1)
        : num(n)
        , den(d)
      {}

      /// Put it in normal form.
      value_t& reduce()
      {
        int gc = gcd(abs(num), den);
        num /= gc;
        den /= gc;
        return *this;
      }

      int num;
      unsigned int den;
    };

    static unsigned int abs(int a)
    {
      return a < 0 ? -a : a;
    }

    static value_t zero()
    {
      return value_t{0, 1};
    }

    static value_t one()
    {
      return value_t{1, 1};
    }

    static value_t add(const value_t l, const value_t r)
    {
      unsigned int cm = lcm(l.den, abs(r.den));
      return value_t{l.num * int (cm / l.den) + r.num * int (cm / r.den),
                     cm}.reduce();
    }

    static value_t sub(const value_t l, const value_t r)
    {
      unsigned int cm = lcm(l.den, abs(r.den));
      return value_t{l.num * int (cm / l.den) - r.num * int (cm / r.den),
                     cm}.reduce();
    }

    static value_t mul(const value_t l, const value_t r)
    {
      return value_t{l.num * r.num, l.den * r.den}.reduce();
    }

    /// GCD: arbitrarily the first argument.
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
      if (0 < r.num)
        return value_t{l.num * int(r.den), l.den * r.num}.reduce();
      else
        return value_t{-l.num * int(r.den), l.den * -r.num}.reduce();
    }

    static value_t
    ldiv(const value_t l, const value_t r)
    {
      return rdiv(r, l);
    }

    value_t star(const value_t v) const
    {
      if (abs(v.num) < v.den)
        // No need to reduce: numerator and denominator are coprime.
        return {int(v.den), v.den - v.num};
      else
        raise(sname(), ": star: invalid value: ", to_string(*this, v));
    }

    static bool is_special(const value_t) // C++11: cannot be constexpr.
    {
      return false;
    }

    static bool is_zero(const value_t v)
    {
      return v.num == 0;
    }

    static bool is_one(const value_t v)
    {
      // All values are normalized.
      return v.num == 1 && v.den == 1;
    }

    static bool equal(const value_t l, const value_t r)
    {
      return l.num == r.num && l.den == r.den;
    }

    /// Whether \a lhs < \a rhs.
    static bool less(const value_t lhs, const value_t rhs)
    {
      return lhs.num * rhs.den < rhs.num * lhs.den;
    }

    static constexpr bool is_commutative() { return true; }

    static constexpr bool show_one() { return false; }
    static constexpr star_status_t star_status() { return star_status_t::ABSVAL; }

    static value_t
    abs(const value_t v)
    {
      return v.num < 0 ? (value_t{-v.num, v.den}) : v;
    }

    static value_t
    transpose(const value_t v)
    {
      return v;
    }

    static size_t hash(const value_t v)
    {
      size_t res = 0;
      hash_combine(res, hash_value(v.num));
      hash_combine(res, hash_value(v.den));
      return res;
    }

    static value_t
    conv(self_t, const value_t v)
    {
      return v;
    }

    static value_t
    conv(z, const z::value_t v)
    {
      return {v, 1};
    }

    static value_t
    conv(b, const b::value_t v)
    {
      return {v, 1};
    }

    static value_t
    conv(std::istream& i, bool = true)
    {
      int num;
      if (!(i >> num))
        raise(sname(), ": invalid numerator: ", i);

      // If we have a slash after the numerator then we have a
      // denominator as well.
      if (i.peek() == '/')
        {
          eat(i, '/');

          // operator>> with an istream and an unsigned int silently
          // mangles a negative number into its two's complement
          // representation as a positive number.
          if (i.peek() == '-')
            {
              num = - num;
              eat(i, '-');
            }

          unsigned int den;
          if (!(i >> den))
            raise(sname(), ": invalid denominator: ", i);
          // Make sure our rational respects our constraints.
          require(den, sname(), ": null denominator");
          return value_t{num, den}.reduce();
        }
      else
        return value_t{num, 1};
    }

    static std::ostream&
    print(const value_t v, std::ostream& o,
          format fmt = {})
    {
      if (fmt == format::latex)
        {
          if (v.den == 1)
            o << v.num;
          else
            o << "\\frac{" << v.num << "}{" << v.den << '}';
        }
      else
        {
          o << v.num;
          if (v.den != 1)
            o << '/' << v.den;
        }
      return o;
    }

    std::ostream&
    print_set(std::ostream& o, format fmt = {}) const
    {
      if (fmt == format::latex)
        o << "\\mathbb{Q}";
      else if (fmt == format::text)
        o << sname();
      else
        raise("invalid format: ", fmt);
      return o;
    }
  };

    /*-------.
    | join.  |
    `-------*/

    VCSN_JOIN_SIMPLE(b, q);
    VCSN_JOIN_SIMPLE(z, q);
    VCSN_JOIN_SIMPLE(q, q);
  }
}
