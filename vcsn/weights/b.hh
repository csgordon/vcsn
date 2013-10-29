#ifndef VCSN_WEIGHTS_B_HH
# define VCSN_WEIGHTS_B_HH

# include <cassert>
# include <ostream>
# include <sstream>
# include <stdexcept>
# include <string>

# include <vcsn/misc/escape.hh>
# include <vcsn/misc/stream.hh>
# include <vcsn/misc/star_status.hh>

namespace vcsn
{
  class b
  {
  public:
    static std::string sname()
    {
      return "b";
    }

    std::string vname(bool = true) const
    {
      return sname();
    }

    /// Build from the description in \a is.
    static b make(std::istream& is)
    {
      eat(is, sname());
      return {};
    }

    using value_t = bool;

    static value_t
    zero()
    {
      return false;
    }

    static value_t
    one()
    {
      return true;
    }

    static value_t
    add(const value_t l, const value_t r)
    {
      return l || r;
    }

    static value_t
    mul(const value_t l, const value_t r)
    {
      return l && r;
    }

    static value_t
    star(const value_t)
    {
      return one();
    }

    static bool
    equals(const value_t l, const value_t r)
    {
      return l == r;
    }

    /// Whether \a lhs < \a rhs.
    static bool less_than(value_t lhs, value_t rhs)
    {
      return lhs < rhs;
    }

    static bool
    is_one(const value_t v)
    {
      return v;
    }

    static bool
    is_zero(const value_t v)
    {
      return !v;
    }

    static constexpr bool is_commutative_semiring() { return true; }

    static constexpr bool show_one() { return false; }
    static constexpr star_status_t star_status()
    {
      return star_status_t::STARRABLE;
    }

    static value_t
    transpose(const value_t v)
    {
      return v;
    }

    static value_t
    conv(std::istream& stream)
    {
      int i;
      if ((stream >> i) && (i == 0 || i == 1))
        return i;
      else
        {
          stream.clear();
          throw std::domain_error(sname() + ": invalid value: " + std::to_string(i));
        }
    }

    static value_t
    conv(const std::string& str)
    {
      return ::vcsn::conv(b(), str);
    }

    static std::ostream&
    print(std::ostream& o, const value_t v)
    {
      return o << format(v);
    }

    static std::string
    format(const value_t v)
    {
      return v ? "1" : "0";
    }
  };

  /// The intersection of two weightsets.
  inline
  b intersection(const b&, const b&)
  {
    return {};
  }

  /// The union of two weightsets.
  inline
  b get_union(const b&, const b&)
  {
    return {};
  }
}

#endif // !VCSN_WEIGHTS_B_HH
