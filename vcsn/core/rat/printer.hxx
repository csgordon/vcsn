#ifndef VCSN_CORE_RAT_PRINTER_HXX
# define VCSN_CORE_RAT_PRINTER_HXX

namespace vcsn
{
  namespace rat
  {

    template <typename GenSet, typename WeightSet, typename Kind>
    inline
    printer<GenSet, WeightSet, Kind>::printer(std::ostream& out,
                                              const genset_t& gs,
                                              const weightset_t& ws,
                                              const bool debug)
      : out_(out)
      , gs_(gs)
      , ws_(ws)
      , debug_(debug)
    {}


# define DEFINE                                                         \
    template <typename GenSet, typename WeightSet, typename Kind>       \
    inline                                                              \
    auto                                                                \
    printer<GenSet, WeightSet, Kind>

# define VISIT(Type)                          \
    DEFINE::visit(const Type ## _t& v)        \
      -> void

    VISIT(sum)
    {
      print(v, '+');
    }

    VISIT(prod)
    {
      print(v, '.');
    }

    VISIT(star)
    {
      const auto& sub = v.sub();
      assert(sub);

      bool parens = parens_(v);
      if (debug_)
        out_ << '*';
      if (parens)
        out_ << '(';
      print(v.left_weight());

      {
        // If our argument shows weight, then add inner parens, so
        // that ({2}a)* => ({2}a)*, not => {2}a* (which is actually
        // read as {2}(a*)).
        bool innerp = shows_weight_(*sub);
        if (innerp)
          out_ << '(';
        sub->accept(*this);
        if (innerp)
          out_ << ')';
      }
      out_ << "*";
      print(v.right_weight());
      if (parens)
        out_ << ')';
    }

    VISIT(zero)
    {
      print(v.left_weight());
      out_ << "\\z";
    }

    VISIT(one)
    {
      print(v.left_weight());
      out_ << "\\e";
    }

    VISIT(atom)
    {
      print(v.left_weight());
      bool p = parens_(v);
      if (p)
        out_ << '(';
      gs_.output(out_, v.value());
      if (p)
        out_ << ')';
    }

    DEFINE::print(const weight_t& w)
      -> void
    {
      if (ws_.show_unit() || !ws_.is_unit(w))
        {
          out_ << '{';
          ws_.print(out_, w);
          out_ << '}';
        }
    }

    DEFINE::print(const nary_t& n, const char op)
      -> void
    {
      assert(n.size());

      print(n.left_weight());

      bool parens = parens_(n);
      if (debug_)
        out_ << op;
      if (parens)
        out_ << '(';
      bool first = true;
      for (auto i: n)
        {
          if (!first)
            out_ << op;
          first = false;
          i->accept(*this);
        }
      if (parens)
        out_ << ')';

      print(n.right_weight());
    }

# undef VISIT
# undef DEFINE

  } // namespace rat
} // namespace vcsn

#endif // !VCSN_CORE_RAT_PRINTER_HXX
