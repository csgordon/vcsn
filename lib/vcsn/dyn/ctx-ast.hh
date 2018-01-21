#pragma once

#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace vcsn::dyn::ast
{
  namespace x3 = boost::spirit::x3;

  struct printer
  {
    std::ostream& o;
    template <typename T>
    void operator()(const x3::forward_ast<T>& t) const
    {
      operator()(t.get());
    }
    template <typename T>
    void operator()(const T& t) const
    {
      o << t;
    }
    template <typename T>
    void operator()(const std::vector<T>& t) const
    {
      const char* sep = "lat<";
      for (const auto& a: t)
        {
          o << sep << a;
          sep = ", ";
        }
      o << '>';
    }
  };

  struct expressionset;
  std::ostream& operator <<(std::ostream& o, const expressionset& es);

  struct oneset
  {
    friend std::ostream& operator <<(std::ostream& o, const oneset&)
    {
      return o << "oneset";
    }
  };

  struct letterset
  {
    std::string letter_type;
    std::string gens;
    friend std::ostream& operator <<(std::ostream& o, const letterset& ls)
    {
      o << "letterset<" << ls.letter_type << "_letters";
      if (!ls.gens.empty())
        o << '(' << vcsn::str_escape(ls.gens, "[]()") << ')';
      return o << '>';
    }
  };

  struct wordset
  {
    std::string letter_type;
    std::string gens;
    friend std::ostream& operator <<(std::ostream& o, const wordset& ls)
    {
      o << "wordset<" << ls.letter_type << "_letters";
      if (!ls.gens.empty())
        o << '(' << vcsn::str_escape(ls.gens, "[]()") << ')';
      return o << '>';
    }
  };

  struct weightset
    // Order matters: to avoid infinit recursion at creation.
    : x3::variant<std::string,
                  x3::forward_ast<expressionset>,
                  std::vector<weightset>>
  {
    using base_type::base_type;
    using base_type::operator=;
    friend std::ostream& operator <<(std::ostream& o, const weightset& ws)
    {
      boost::apply_visitor(printer{o}, ws);
      return o;
    }
  };

  struct labelset
  // Order matters: to avoid infinit recursion at creation.
    : x3::variant<oneset,
                  letterset,
                  wordset,
                  x3::forward_ast<expressionset>,
                  std::vector<labelset>>
  {
    using base_type::base_type;
    using base_type::operator=;
    friend std::ostream& operator <<(std::ostream& o, const labelset& ls)
    {
      boost::apply_visitor(printer{o}, ls);
      return o;
    }
  };

  // Need to obfuscate because there are macros that use the name `Context`.
  struct ConText
  {
    labelset ls;
    weightset ws;
    friend std::ostream& operator <<(std::ostream& o, const ConText& ctx)
    {
      return o << ctx.ls << ", " << ctx.ws;
    }
  };

  struct expressionset
  {
    ConText ctx;
    std::string identities = "";
    friend std::ostream& operator <<(std::ostream& o, const expressionset& es)
    {
      o << "expressionset<" << es.ctx << '>';
      if (!es.identities.empty())
	o << '(' << es.identities << ')';
      return o;
    }
  };
}

BOOST_FUSION_ADAPT_STRUCT(vcsn::dyn::ast::oneset)
BOOST_FUSION_ADAPT_STRUCT(vcsn::dyn::ast::letterset, letter_type, gens)
BOOST_FUSION_ADAPT_STRUCT(vcsn::dyn::ast::wordset, letter_type, gens)
BOOST_FUSION_ADAPT_STRUCT(vcsn::dyn::ast::ConText, ls, ws)
BOOST_FUSION_ADAPT_STRUCT(vcsn::dyn::ast::expressionset, ctx, identities)