#ifndef VCSN_CORE_RAT_PRINTER_HXX
# define VCSN_CORE_RAT_PRINTER_HXX

# include <vcsn/misc/escape.hh>
# include <vcsn/misc/indent.hh>
# include <vcsn/misc/raise.hh>

namespace vcsn
{
  namespace rat
  {

    inline
    std::ostream&
    operator<<(std::ostream& o, type_t t)
    {
      switch (t)
        {
# define CASE(T) case type_t::T: o << #T; break
          CASE(zero);
          CASE(one);
          CASE(atom);
          CASE(sum);
          CASE(prod);
          CASE(ldiv);
          CASE(conjunction);
          CASE(shuffle);
          CASE(star);
          CASE(transposition);
          CASE(lweight);
          CASE(rweight);
          CASE(complement);
# undef CASE
        }
      return o;
    }

    template <typename RatExpSet>
    inline
    printer<RatExpSet>::printer(std::ostream& out,
                                const ratexpset_t& rs,
                                const bool debug)
      : out_(out)
      , ctx_(rs.context())
      , debug_(debug)
    {}


# define DEFINE                                 \
    template <typename RatExpSet>               \
    inline                                      \
    auto                                        \
    printer<RatExpSet>

    DEFINE::operator()(const node_t& v)
      -> std::ostream&
    {
      static bool print = !! getenv("VCSN_PRINT");
      if (print)
        out_ << '<' << v.type() << '>' << vcsn::incendl;
      v.accept(*this);
      if (print)
        out_ << vcsn::decendl << "</" << v.type() << '>';
      return out_;
    }

    DEFINE::format(const std::string& format)
      -> void
    {
      format_ = format;
      if (format_ == "latex")
        {
          lgroup_       = "{";
          rgroup_       = "}";
          langle_       = " \\langle ";
          rangle_       = " \\rangle ";
          lparen_       = "\\left(";
          rparen_       = "\\right)";
          star_         = "^{*}";
          complement_   = "^{c}";
          transposition_ = "^{T}";
          conjunction_  = " \\& ";
          shuffle_      = " \\between ";
          product_      = " \\, ";
          sum_          = " + ";
          zero_         = "\\emptyset";
          one_          = "\\varepsilon";
          lmul_         = "\\,";
          rmul_         = "\\,";
          ldiv_         = " \\backslash ";
        }
      else if (format_ == "text")
        {
          lgroup_       = "";
          rgroup_       = "";
          langle_       = "<";
          rangle_       = ">";
          lparen_       = "(";
          rparen_       = ")";
          star_         = "*";
          complement_   = "{c}";
          transposition_ = "{T}";
          conjunction_  = "&";
          shuffle_      = ":";
          product_      = "";
          sum_          = "+";
          zero_         = "\\z";
          one_          = "\\e";
          lmul_         = "";
          rmul_         = "";
          ldiv_         = "{\\}";
        }
      else
        raise("invalid output format for ratexp: ", str_escape(format));
    }

    DEFINE::precedence(const node_t& v) const
      -> precedence_t
    {
      const atom_t* atom = dynamic_cast<const atom_t*>(&v);
      if (atom && ! ctx_.labelset()->is_letter(atom->value()))
        return precedence_t::word;
      else
        switch (v.type())
          {
# define CASE(Type)                             \
            case exp::type_t::Type:             \
              return precedence_t::Type;
            CASE(atom);
            CASE(complement);
            CASE(conjunction);
            CASE(ldiv);
            CASE(lweight);
            CASE(one);
            CASE(prod);
            CASE(rweight);
            CASE(shuffle);
            CASE(star);
            CASE(sum);
            CASE(transposition);
            CASE(zero);
# undef CASE
          }
      abort(); // Unreachable.
    }

# define VISIT(Type)                          \
    DEFINE::visit(const Type ## _t& v)        \
      -> void

    VISIT(lweight)
    {
      out_ << langle_;
      ctx_.weightset()->print(out_, v.weight(), format_);
      out_ << rangle_ << lmul_;
      print_child(*v.sub(), v);
    }

    VISIT(rweight)
    {
      print_child(*v.sub(), v);
      out_ << rmul_ << langle_;
      ctx_.weightset()->print(out_, v.weight(), format_);
      out_ << rangle_;
    }

    VISIT(zero)
    {
      (void) v;
      out_ << zero_;
    }

    VISIT(one)
    {
      (void) v;
      out_ << one_;
    }

    VISIT(atom)
    {
      ctx_.labelset()->print(out_, v.value(), format_);
    }

    DEFINE::print_child(const node_t& child, const node_t& parent,
                        bool force_parens)
      -> void
    {
      static bool force = !! getenv("VCSN_PARENS");
      bool parent_has_precedence = precedence(child) <= precedence(parent);
      bool needs_parens =
        (force
         || force_parens
         || (parent_has_precedence && ! (parent.is_unary() && child.is_unary())));
      if (needs_parens)
        out_ << lparen_;
      else if (parent.is_unary())
        out_ << lgroup_;
      operator()(child);
      if (needs_parens)
        out_ << rparen_;
      else if (parent.is_unary())
        out_ << rgroup_;
    }

    template <typename RatExpSet>
    template <type_t Type>
    inline
    auto
    printer<RatExpSet>::print(const unary_t<Type>& v, const char* op)
      -> void
    {
      // Force parens around the child if it is a left weight.  This
      // is not needed for right weights: compare e<w>* with (<w>e)*.
      const node_t& child = *v.sub();
      print_child(child, v, child.type() == rat::type_t::lweight);
      out_ << op;
    }

    template <typename RatExpSet>
    template <type_t Type>
    inline
    auto
    printer<RatExpSet>::print(const variadic_t<Type>& n, const char* op)
      -> void
    {
      bool first = true;
      for (auto i: n)
        {
          if (! first)
            out_ << op;
          print_child(*i, n);
          first = false;
        }
    }

# undef VISIT
# undef DEFINE

  } // namespace rat
} // namespace vcsn

#endif // !VCSN_CORE_RAT_PRINTER_HXX
