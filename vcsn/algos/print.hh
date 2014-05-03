#ifndef VCSN_ALGOS_PRINT_HH
# define VCSN_ALGOS_PRINT_HH

# include <iosfwd>

# include <vcsn/ctx/context.hh>
# include <vcsn/dyn/context.hh>
# include <vcsn/dyn/expansion.hh>
# include <vcsn/dyn/label.hh>
# include <vcsn/dyn/fwd.hh>
# include <vcsn/dyn/polynomial.hh>
# include <vcsn/dyn/ratexp.hh>
# include <vcsn/dyn/weight.hh>
# include <vcsn/misc/raise.hh>

namespace vcsn
{

  /*-------------------------.
  | print(context, stream).  |
  `-------------------------*/

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename Context, typename Ostream, typename String>
      std::ostream& print_ctx(const context& ctx, std::ostream& o,
			      const std::string& format)
      {
        const auto& c = ctx->as<Context>();
        return c.print_set(o, format);
      }

      REGISTER_DECLARE(print_ctx,
                       (const context& c, std::ostream& o,
                        const std::string& format) -> std::ostream&);
    }
  }

  /*---------------------------.
  | print(expansion, stream).  |
  `---------------------------*/

  template <typename ValueSet>
  inline
  std::ostream&
  print(std::ostream& o,
        const ValueSet& vs, const typename ValueSet::value_t& v,
        const std::string& format)
  {
    return vs.print(o, v, format);
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename ExpansionSet, typename Ostream, typename String>
      std::ostream& print_expansion(const expansion& expansion, std::ostream& o,
                                    const std::string& format)
      {
        const auto& e = expansion->as<ExpansionSet>();
        return vcsn::print<ExpansionSet>(o, e.expansionset(), e.expansion(), format);
      }

      REGISTER_DECLARE(print_expansion,
                       (const expansion& l, std::ostream& o,
                        const std::string& format) -> std::ostream&);
    }
  }

  /*-----------------------.
  | print(stream, label).  |
  `-----------------------*/

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename Ostream, typename LabelSet, typename String>
      std::ostream& print_label(std::ostream& o, const label& label,
                                const std::string& format)
      {
        const auto& l = label->as<LabelSet>();
        return vcsn::print<LabelSet>(o, l.labelset(), l.label(), format);
      }

      REGISTER_DECLARE(print_label,
                       (std::ostream& o, const label& l,
                        const std::string& format) -> std::ostream&);
    }
  }

  /*---------------------------.
  | list(polynomial, stream).  |
  `---------------------------*/

  template <typename PolynomialSet>
  inline
  std::ostream&
  list(const PolynomialSet& ps, const typename PolynomialSet::value_t& p,
       std::ostream& o)
  {
    bool first = true;
    for (const auto& m: p)
      {
        if (!first)
          o << std::endl;
        first = false;
        ps.print(o, m);
      }
    return o;
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename PolynomialSet, typename Ostream>
      std::ostream& list_polynomial(const polynomial& polynomial,
                                    std::ostream& o)
      {
        const auto& p = polynomial->as<PolynomialSet>();
        return vcsn::list<PolynomialSet>(p.polynomialset(),
                                         p.polynomial(), o);
      }

      REGISTER_DECLARE(list_polynomial,
                       (const polynomial& p, std::ostream& o) -> std::ostream&);
    }
  }

  /*----------------------------.
  | print(polynomial, stream).  |
  `----------------------------*/

  /// Actually applies to (ValueSet, Value, ostream, string): for
  /// polynomialset, ratexpset, and weightset.
  template <typename PolynomialSet>
  inline
  std::ostream&
  print(const PolynomialSet& ps, const typename PolynomialSet::value_t& p,
        std::ostream& o, const std::string& format)
  {
    return ps.print(o, p, format);
  }

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename PolynomialSet, typename Ostream, typename String>
      std::ostream& print_polynomial(const polynomial& polynomial,
                                     std::ostream& o, const std::string& format)
      {
        const auto& p = polynomial->as<PolynomialSet>();
        return vcsn::print<PolynomialSet>(p.polynomialset(),
                                          p.polynomial(), o, format);
      }

      REGISTER_DECLARE(print_polynomial,
                       (const polynomial& p, std::ostream& o,
                        const std::string& format) -> std::ostream&);
    }
  }


  /*------------------------.
  | print(ratexp, stream).  |
  `------------------------*/

#if 0
  /// See PolynomialSet.
  template <typename RatExpSet>
  inline
  std::ostream&
  print(const RatExpSet& rs, const typename RatExpSet::ratexp_t& e,
        std::ostream& o, const std::string& format)
  {
    return rs.print(o, e, format);
  }
#endif

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename RatExpSet, typename Ostream, typename String>
      std::ostream& print_ratexp(const ratexp& exp, std::ostream& o,
                                 const std::string& format)
      {
        const auto& e = exp->as<RatExpSet>();
        return vcsn::print(e.ratexpset(), e.ratexp(), o, format);
      }

      REGISTER_DECLARE(print_ratexp,
                       (const ratexp& aut, std::ostream& o,
			const std::string& format) -> std::ostream&);
    }
  }

  /*------------------------.
  | print(weight, stream).  |
  `------------------------*/

#if 0
  /// See PolynomialSet.
  template <typename WeightSet>
  inline
  std::ostream&
  print(const WeightSet& ws, const typename WeightSet::value_t& w,
        std::ostream& o)
  {
    return ws.print(o, w);
  }
#endif

  namespace dyn
  {
    namespace detail
    {
      /// Bridge.
      template <typename WeightSet, typename Ostream, typename String>
      std::ostream& print_weight(const weight& weight, std::ostream& o,
                                 const std::string& format)
      {
        const auto& w = weight->as<WeightSet>();
        return vcsn::print<WeightSet>(w.weightset(), w.weight(), o, format);
      }

      REGISTER_DECLARE(print_weight,
                       (const weight& aut, std::ostream& o,
                        const std::string& format) -> std::ostream&);
    }
  }

}

#endif // !VCSN_ALGOS_PRINT_HH
