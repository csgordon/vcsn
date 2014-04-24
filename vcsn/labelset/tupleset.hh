#ifndef VCSN_LABELSET_TUPLESET_HH
# define VCSN_LABELSET_TUPLESET_HH

# include <iosfwd>
# include <istream>
# include <tuple>

# include <vcsn/config.hh> // VCSN_HAVE_CORRECT_LIST_INITIALIZER_ORDER
# include <vcsn/labelset/fwd.hh>
# include <vcsn/misc/escape.hh>
# include <vcsn/misc/raise.hh>
# include <vcsn/misc/stream.hh>
# include <vcsn/misc/tuple.hh>
# include <vcsn/weightset/b.hh>

namespace vcsn
{

  template <typename... ValueSets>
  class tupleset
  {
  public:
    using valuesets_t = std::tuple<ValueSets...>;
    using indices_t = vcsn::detail::make_index_sequence<sizeof...(ValueSets)>;
    static constexpr indices_t indices{};
    template <std::size_t... I>
    using seq = vcsn::detail::index_sequence<I...>;

    /// The Ith valueset type.
    template <std::size_t I>
    using valueset_t = typename std::tuple_element<I, valuesets_t>::type;

  public:
    using self_type = tupleset;
    using value_t = std::tuple<typename ValueSets::value_t...>;
    using kind_t = labels_are_tuples;

    tupleset(valuesets_t vs)
      : sets_(vs)
    {}

    tupleset(ValueSets... ls)
      : sets_(ls...)
    {}

    static std::string sname()
    {
      return sname_(indices);
    }

    std::string vname(bool full = true) const
    {
      return vname_(full, indices);
    }

    /// Build from the description in \a is.
    static tupleset make(std::istream& is)
    {
      // name: lat<law_char(abc), law_char(xyz)>
      kind_t::make(is);
      auto res = make_(is, indices);
      eat(is, '>');
      return res;
    }

    const valuesets_t& sets() const
    {
      return sets_;
    }

    template <size_t I>
    const valueset_t<I>& set() const
    {
      return std::get<I>(sets());
    }

    static constexpr bool is_ltl()
    {
      return is_ltl_(indices);
    }

    /// Iterate over the letters of v.
    ///
    /// Templated by Value so that we work for both word_t and label_t.
    /// Besides, avoids the problem of instantiation with weighset that
    /// do not provide a word_t type.
    template <typename Value>
    static auto
    letters_of(const Value& v)
      -> decltype(zip_sequences_tuple(v))
    {
      return zip_sequences_tuple(v);
    }

    static bool
    equals(const value_t& l, const value_t& r)
    {
      return equals_(l, r, indices);
    }

    /// Whether \a l < \a r.
    static bool
    less_than(const value_t& l, const value_t& r)
    {
      return less_than_(l, r, indices);
    }

    static value_t
    special()
    {
      return special_(indices);
    }

    static bool
    is_special(const value_t& l)
    {
      return is_special_(l, indices);
    }

    value_t
    zero() const
    {
      return zero_(indices);
    }

    bool
    is_zero(const value_t& l) const
    {
      return is_zero_(l, indices);
    }

    static constexpr bool
    has_one()
    {
      return has_one_(indices);
    }

    static value_t
    one()
    {
      return one_(indices);
    }

    static bool
    is_one(const value_t& l)
    {
      return is_one_(l, indices);
    }

    static bool
    show_one()
    {
      return show_one_(indices);
    }

    bool
    is_letter(const value_t&) const
    {
      return false;
    }

    value_t
    add(const value_t& l, const value_t& r) const
    {
      return add_(l, r, indices);
    }

    value_t
    mul(const value_t& l, const value_t& r) const
    {
      return mul_(l, r, indices);
    }

    value_t
    rdiv(const value_t& l, const value_t& r) const
    {
      return rdiv_(l, r, indices);
    }

    value_t
    ldiv(const value_t& l, const value_t& r) const
    {
      return ldiv_(l, r, indices);
    }

    value_t
    star(const value_t& l) const
    {
      return star_(l, indices);
    }

    value_t
    delimit(const value_t& l) const
    {
      return delimit_(l, indices);
    }

    value_t
    undelimit(const value_t& l) const
    {
      return undelimit_(l, indices);
    }

    // FIXME: this needs to be computed.
    static constexpr star_status_t star_status()
    {
      return star_status_t::STARRABLE;
    }

    template <typename LhsValue, typename RhsValue>
    value_t
    concat(const LhsValue& l, const RhsValue& r) const
    {
      return concat_(l, r, indices);
    }

    value_t
    transpose(const value_t& l) const
    {
      return transpose_(l, indices);
    }

    static size_t
    hash(const value_t& v)
    {
      return hash_(v, indices);
    }

    static value_t
    conv(self_type, value_t v)
    {
      return v;
    }

    value_t
    conv(b, b::value_t v) const
    {
      return v ? one() : zero();
    }

    /// Read one letter from i, return the corresponding value.
    value_t
    conv(std::istream& i) const
    {
      value_t res = conv_(i, indices);
      eat(i, ')');
      return res;
    }

    std::set<value_t> convs(std::istream&) const
    {
      raise("tupleset: ranges not implemented");
    }

    std::ostream&
    print_set(std::ostream& o, const std::string& format) const
    {
      return print_set_(o, format, indices);
    }

    std::ostream&
    print(std::ostream& o, const value_t& l,
          const std::string& format = "text") const
    {
      return print_(o, l, format, indices);
    }

  private:
    template <std::size_t... I>
    static std::string sname_(seq<I...>)
    {
      std::string res = "lat<";
      const char *sep = "";
      for (auto n: {valueset_t<I>::sname()...})
        {
          res += sep;
          res += n;
          sep = ",";
        }
      res.push_back('>');
      return res;
    }

    template <std::size_t... I>
    std::string vname_(bool full, seq<I...>) const
    {
      std::string res = "lat<";
      const char *sep = "";
      for (auto n: {set<I>().vname(full)...})
        {
          res += sep;
          res += n;
          sep = ",";
        }
      res.push_back('>');
      return res;
    }

    template <std::size_t... I>
    static tupleset make_(std::istream& i, seq<I...>)
    {
#  if VCSN_HAVE_CORRECT_LIST_INITIALIZER_ORDER
      return tupleset{(eat_separator_<I>(i, '<', ','),
                       valueset_t<I>::make(i))...};
#  else
      return make_gcc_tuple
        ((eat_separator_<sizeof...(ValueSets)-1 -I>(i, '<', ','),
          valueset_t<sizeof...(ValueSets)-1 -I>::make(i))...);
#  endif
    }

    template <std::size_t... I>
    static constexpr bool
    is_ltl_(seq<I...>)
    {
      return all_<valueset_t<I>::is_ltl()...>();
    }

    template <std::size_t... I>
    static bool
    equals_(const value_t& l, const value_t& r, seq<I...>)
    {
      for (auto n: {valueset_t<I>::equals(std::get<I>(l),
                                          std::get<I>(r))...})
        if (!n)
          return false;
      return true;
    }

    template <std::size_t... I>
    static bool
    less_than_(const value_t& l, const value_t& r, seq<I...>)
    {
      for (auto n: {valueset_t<I>::less_than(std::get<I>(l),
                                             std::get<I>(r))...})
        if (n)
          return true;
      return false;
    }

    template <std::size_t... I>
    static std::size_t
    hash_(const value_t& v, seq<I...>)
    {
      std::size_t res = 0;
      for (auto h: {valueset_t<I>::hash(std::get<I>(v))...})
        std::hash_combine(res, h);
      return res;
    }

    template <std::size_t... I>
    static value_t
    special_(seq<I...>)
    {
      return std::make_tuple(valueset_t<I>::special()...);
    }

    template <std::size_t... I>
    static bool
    is_special_(const value_t& l, seq<I...>)
    {
      for (auto n: {valueset_t<I>::is_special(std::get<I>(l))...})
        if (!n)
          return false;
      return true;
    }

    template <std::size_t... I>
    value_t
    zero_(seq<I...>) const
    {
      return value_t{set<I>().zero()...};
    }

    template <std::size_t... I>
    bool
    is_zero_(const value_t& l, seq<I...>) const
    {
      for (auto n: {set<I>().is_zero(std::get<I>(l))...})
        if (!n)
          return false;
      return true;
    }

    template <std::size_t... I>
    static constexpr bool
    has_one_(seq<I...>)
    {
      return all_<valueset_t<I>::has_one()...>();
    }

    template <std::size_t... I>
    static value_t
    one_(seq<I...>)
    {
      return value_t{valueset_t<I>::one()...};
    }

    template <std::size_t... I>
    static bool
    is_one_(const value_t& l, seq<I...>)
    {
      for (auto n: {valueset_t<I>::is_one(std::get<I>(l))...})
        if (!n)
          return false;
      return true;
    }

    template <std::size_t... I>
    static bool
    show_one_(seq<I...>)
    {
      for (auto n: {valueset_t<I>::show_one()...})
        if (n)
          return true;
      return false;
    }

    template <std::size_t... I>
    value_t
    add_(const value_t& l, const value_t& r, seq<I...>) const
    {
      return value_t{set<I>().add(std::get<I>(l), std::get<I>(r))...};
    }

    template <std::size_t... I>
    value_t
    mul_(const value_t& l, const value_t& r, seq<I...>) const
    {
      return value_t{set<I>().mul(std::get<I>(l), std::get<I>(r))...};
    }

    template <std::size_t... I>
    value_t
    rdiv_(const value_t& l, const value_t& r, seq<I...>) const
    {
      return value_t{set<I>().rdiv(std::get<I>(l), std::get<I>(r))...};
    }

    template <std::size_t... I>
    value_t
    ldiv_(const value_t& l, const value_t& r, seq<I...>) const
    {
      return value_t{set<I>().ldiv(std::get<I>(l), std::get<I>(r))...};
    }

    template <std::size_t... I>
    value_t
    star_(value_t const& l, seq<I...>) const
    {
      return value_t{set<I>().star(std::get<I>(l))...};
    }

    template <std::size_t... I>
    value_t
    delimit_(value_t const& l, seq<I...>) const
    {
      return value_t{set<I>().delimit(std::get<I>(l))...};
    }

    template <std::size_t... I>
    value_t
    undelimit_(value_t const& l, seq<I...>) const
    {
      return value_t{set<I>().undelimit(std::get<I>(l))...};
    }

    template <typename LhsValue, typename RhsValue, std::size_t... I>
    value_t
    concat_(const LhsValue& l, const RhsValue& r, seq<I...>) const
    {
      return value_t{set<I>().concat(std::get<I>(l), std::get<I>(r))...};
    }

    template <std::size_t... I>
    value_t
    conv_(std::istream& i, seq<I...>) const
    {
#  if VCSN_HAVE_CORRECT_LIST_INITIALIZER_ORDER
      return std::make_tuple((eat_separator_<I>(i, '(', ','),
                              set<I>().conv(i))...);
#  else
      return
        detail::make_gcc_tuple((eat_separator_<sizeof...(ValueSets)-1 - I>(i, '(', ','),
                                std::get<sizeof...(ValueSets)-1 - I>(sets_).conv(i))...);
#  endif
    }

    /// Read the separator from the input stream \a i.
    /// If \a I is 0, then the separator is '(',
    /// otherwise it is ',' (possibly followed by spaces).
    template <std::size_t I>
    static void
    eat_separator_(std::istream& i, char first, char tail)
    {
      eat(i, I == 0 ? first : tail);
      while (isspace(i.peek()))
        i.ignore();
    }


    template <std::size_t... I>
    std::ostream&
    print_(std::ostream& o, value_t const& l,
           const std::string& format, seq<I...>) const
    {
      if (!is_special(l))
        {
          using swallow = int[];
          (void) swallow
            {
              (o << (I == 0 ? "(" : ", "),
               set<I>().print(o, std::get<I>(l), format),
               0)...
            };
          o << ')';
        }
      return o;
    }

    template <std::size_t... I>
    std::ostream&
    print_set_(std::ostream& o, const std::string& format,
               seq<I...>) const
    {
      const char *sep = "";
      if (format == "latex")
        sep = " \\times ";
      else if (format == "text")
        {
          o << "lat<";
          sep = ",";
        }
      else
        raise("invalid format: ", format);
      using swallow = int[];
      (void) swallow
        {
          (o << (I == 0 ? "" : sep),
           set<I>().print_set(o, format),
           0)...
        };
      if (format == "text")
        o << '>';
      return o;
    }

    template <std::size_t... I>
    value_t
    transpose_(value_t const& l, seq<I...>) const
    {
      return value_t{(set<I>().transpose(std::get<I>(l)))...};
    }

    /// The intersection with another tupleset.
    template <std::size_t... I>
    tupleset
    meet_(const tupleset& rhs, seq<I...>) const
    {
      return tupleset{meet(set<I>(), rhs.set<I>())...};
    }

    /// The join with another tupleset.
    template <std::size_t... I>
    tupleset
    join_(const tupleset& rhs, seq<I...>) const
    {
      return tupleset{join(set<I>(), rhs.set<I>())...};
    }

    /// The meet with another tupleset.
    friend
    tupleset
    meet(const tupleset& lhs, const tupleset& rhs)
    {
      return lhs.meet_(rhs, indices);
    }

    /// The meet with the B weightset.
    friend
    tupleset
    meet(const tupleset& lhs, const b&)
    {
      return lhs;
    }

    /// The meet with the B weightset.
    friend
    tupleset
    meet(const b&, const tupleset& rhs)
    {
      return rhs;
    }

    /// The join with another tupleset.
    friend
    tupleset
    join(const tupleset& lhs, const tupleset& rhs)
    {
      return lhs.join_(rhs, indices);
    }

    /// The join with the B weightset.
    friend
    tupleset
    join(const tupleset& lhs, const b&)
    {
      return lhs;
    }

    /// The join with the B weightset.
    friend
    tupleset
    join(const b&, const tupleset& rhs)
    {
      return rhs;
    }

    valuesets_t sets_;
  };

}
#endif // !VCSN_LABELSET_TUPLESET_HH
