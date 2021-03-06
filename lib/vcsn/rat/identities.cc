#include <cctype>

#include <vcsn/core/rat/identities.hh>
#include <vcsn/misc/builtins.hh>
#include <vcsn/misc/getargs.hh>

namespace vcsn
{

  namespace rat
  {

    identities::identities(const std::string& s)
    {
      static const auto map = getarg<identities::ids_t>
        {
          "identities",
          {
            {"agressive",    identities::agressive},
            {"associative",  identities::associative},
            {"auto",         identities::deflt},
            {"binary",       "trivial"},
            {"default",      identities::deflt},
            {"distributive", identities::distributive},
            {"linear",       identities::linear},
            {"none",         identities::none},
            {"series",       "distributive"},
            {"trivial",      identities::trivial},
          }
        };
      ids_ = map[s];
    }

    identities::identities(const char* cp)
      : identities(std::string{cp})
    {}

    std::string to_string(identities i)
    {
      switch (i.ids())
        {
        case identities::associative:
          return "associative";
        case identities::agressive:
          return "agressive";
        case identities::linear:
          return "linear";
        case identities::distributive:
          return "distributive";
        case identities::trivial:
          return "trivial";
        case identities::none:
          return "none";
        }
      BUILTIN_UNREACHABLE();
    }

    std::ostream& operator<<(std::ostream& os, identities i)
    {
      return os << to_string(i);
    }

    std::istream& operator>>(std::istream& is, identities& ids)
    {
      std::string buf;
      while (is && isalnum(is.peek()))
        buf += is.get();
      ids = identities{buf};
      return is;
    }

    identities meet(identities i1, identities i2)
    {
      return std::max(i1, i2);
    }
  } // namespace rat
} // namespace vcsn
