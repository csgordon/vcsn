#ifndef VCSN_CTX_INSTANTIATE_HH
# define VCSN_CTX_INSTANTIATE_HH

# include <vcsn/algos/accessible.hh>
# include <vcsn/algos/aut-to-exp.hh>
# include <vcsn/algos/complete.hh>
# include <vcsn/algos/concatenate.hh>
# include <vcsn/algos/constant-term.hh>
# include <vcsn/algos/copy.hh>
# include <vcsn/algos/derive.hh>
# include <vcsn/algos/dot.hh>
# include <vcsn/algos/edit-automaton.hh>
# include <vcsn/algos/efsm.hh>
# include <vcsn/algos/enumerate.hh>
# include <vcsn/algos/eval.hh>
# include <vcsn/algos/info.hh>
# include <vcsn/algos/is-ambiguous.hh>
# include <vcsn/algos/is-complete.hh>
# include <vcsn/algos/is-deterministic.hh>
# include <vcsn/algos/is-eps-acyclic.hh>
# include <vcsn/algos/is-normalized.hh>
# include <vcsn/algos/is-proper.hh>
# include <vcsn/algos/is-valid.hh>
# include <vcsn/algos/left-mult.hh>
# include <vcsn/algos/lift.hh>
# include <vcsn/algos/make-context.hh>
# include <vcsn/algos/print.hh>
# include <vcsn/algos/product.hh>
# include <vcsn/algos/proper.hh>
# include <vcsn/algos/read-weight.hh>
# include <vcsn/algos/standard.hh>
# include <vcsn/algos/star.hh>
# include <vcsn/algos/sum.hh>
# include <vcsn/algos/thompson.hh>
# include <vcsn/algos/tikz.hh>
# include <vcsn/algos/transpose.hh>
# include <vcsn/algos/union.hh>

# include <vcsn/factory/de-bruijn.hh>
# include <vcsn/factory/divkbaseb.hh>
# include <vcsn/factory/double-ring.hh>
# include <vcsn/factory/ladybird.hh>
# include <vcsn/factory/random.hh>
# include <vcsn/factory/u.hh>

# include <vcsn/weights/polynomialset.hh>

/* The purpose of this file is manyfold:

   - *prevent* the instantiation of the algorithms that we will
      provided by the context library.  This is what the INSTANTIATE
      macros do.  In this case MAYBE_EXTERN is "extern".

   - instantiate them when building the context libraries.  In this
     context, MAYBE_EXTERN is "".

   - register the dyn functions.  This is what the register function
     templates do.

*/

namespace vcsn
{
# define VCSN_CTX_INSTANTIATE_PRINT(Format, Aut)                        \
  MAYBE_EXTERN template                                                 \
  std::ostream& Format<Aut>(const Aut& aut, std::ostream& out);         \
                                                                        \
  MAYBE_EXTERN template                                                 \
  std::string Format<Aut>(const Aut& aut);

  /*-------------------------------------------------------.
  | Instantiate the function that work for every context.  |
  `-------------------------------------------------------*/

# define VCSN_CTX_INSTANTIATE_1(Ctx)                                    \
  MAYBE_EXTERN template                                                 \
  class mutable_automaton<Ctx>;                                         \
                                                                        \
  /* aut_to_exp. */                                                     \
  MAYBE_EXTERN template                                                 \
  Ctx::ratexp_t                                                         \
  aut_to_exp<mutable_automaton<Ctx>>                                    \
  (const mutable_automaton<Ctx>& aut,                                   \
    const state_chooser_t<mutable_automaton<Ctx>>& next_state);         \
                                                                        \
  /* dot. */                                                            \
  VCSN_CTX_INSTANTIATE_PRINT(dot, mutable_automaton<Ctx>);              \
  VCSN_CTX_INSTANTIATE_PRINT                                            \
  (dot, vcsn::detail::transpose_automaton<mutable_automaton<Ctx>>);     \
                                                                        \
  /* efsm. */                                                           \
  MAYBE_EXTERN template                                                 \
  std::ostream& efsm<mutable_automaton<Ctx>>                            \
  (const mutable_automaton<Ctx>& aut, std::ostream& out);               \
  MAYBE_EXTERN template                                                 \
  std::ostream&                                                         \
  efsm<vcsn::detail::transpose_automaton<mutable_automaton<Ctx>>>       \
  (const vcsn::detail::transpose_automaton<mutable_automaton<Ctx>>& aut,\
   std::ostream& out);                                                  \
                                                                        \
  /* lift. */                                                           \
  MAYBE_EXTERN template                                                 \
  detail::lifted_automaton_t<mutable_automaton<Ctx>>                    \
  lift<mutable_automaton<Ctx>>(const mutable_automaton<Ctx>& aut);      \
                                                                        \
  /* print. */                                                          \
  MAYBE_EXTERN template                                                 \
  std::ostream&                                                         \
  print<Ctx::ratexpset_t>(const Ctx::ratexpset_t& rs,                   \
                          const Ctx::ratexp_t& e, std::ostream& o);     \
  MAYBE_EXTERN template                                                 \
  std::ostream&                                                         \
  print<typename Ctx::weightset_t>                                      \
  (const typename Ctx::weightset_t& ws,                                 \
   const typename Ctx::weight_t& e,                                     \
   std::ostream& o);                                                    \
                                                                        \
  /* standard. */                                                       \
  MAYBE_EXTERN template                                                 \
  class rat::standard_visitor<mutable_automaton<Ctx>, Ctx>;             \
                                                                        \
  /* tikz. */                                                           \
  MAYBE_EXTERN template                                                 \
  std::ostream&                                                         \
  tikz<mutable_automaton<Ctx>>(const mutable_automaton<Ctx>& aut,       \
                               std::ostream& out);                      \
                                                                        \
  /* transpose. */                                                      \
  MAYBE_EXTERN template                                                 \
  class detail::transpose_automaton<mutable_automaton<Ctx>>;            \
  MAYBE_EXTERN template                                                 \
  class detail::transposer<ratexpset<Ctx>>


  /*----------------------------------.
  | Register the abstract functions.  |
  `----------------------------------*/

#define REGISTER(Algo, Type)                    \
  Algo ## _register(Type::sname(), Algo<Type>)

#define REGISTER2(Algo, Type1, Type2)           \
  Algo ## _register(Type1::sname(), Type2::sname(), Algo<Type1, Type2>)

  namespace ctx
  {
    namespace detail
    {
      template <typename Ctx>
      bool
      register_kind_functions(labels_are_letters)
      {
        using aut_t = mutable_automaton<Ctx>;
        // Word polynomialset.
        using wps_t = typename vcsn::detail::law_traits<Ctx>::polynomialset_t;
        using rs_t = ratexpset<Ctx>;
        using namespace dyn::detail;

        REGISTER(complete, aut_t);
        REGISTER(de_bruijn, Ctx);
        REGISTER(derive, rs_t);
        REGISTER(divkbaseb, Ctx);
        REGISTER(double_ring, Ctx);
        REGISTER(enumerate, aut_t);
        REGISTER(eval, aut_t);
        REGISTER(is_ambiguous, aut_t);
        REGISTER(is_complete, aut_t);
        REGISTER(is_deterministic, aut_t);
        REGISTER(ladybird, Ctx);
        list_polynomial_register(wps_t::sname(), list<wps_t>);
        REGISTER(power, aut_t);
        print_polynomial_register(wps_t::sname(), print<wps_t>);
        REGISTER2(product, aut_t, aut_t);
        REGISTER(random, Ctx);
        REGISTER(shortest, aut_t);
        REGISTER(u, Ctx);

        return true;
      }

      template <typename Ctx>
      bool
      register_kind_functions(labels_are_nullable)
      {
        using aut_t = mutable_automaton<Ctx>;
        using rs_t = ratexpset<Ctx>;
        using namespace dyn::detail;

        REGISTER(de_bruijn, Ctx);
        REGISTER(derive, rs_t);
        REGISTER(divkbaseb, Ctx);
        REGISTER(double_ring, Ctx);
        REGISTER(ladybird, Ctx);
        REGISTER(proper, aut_t);
        REGISTER(random, Ctx);
        REGISTER(thompson, rs_t);
        REGISTER(u, Ctx);

        return true;
      }

      template <typename Ctx>
      bool
      register_kind_functions(labels_are_one)
      {
        return true;
      }

      template <typename Ctx>
      bool
      register_kind_functions(labels_are_words)
      {
        using aut_t = mutable_automaton<Ctx>;
        using rs_t = ratexpset<Ctx>;

        using namespace dyn::detail;

        REGISTER(proper, aut_t);
        REGISTER(thompson, rs_t);

        return true;
      }

      template <typename Ctx>
      bool
      register_functions()
      {
        using aut_t = mutable_automaton<Ctx>;
        using taut_t = vcsn::detail::transpose_automaton<aut_t>;
        using rs_t = ratexpset<Ctx>;
        using ws_t = typename Ctx::weightset_t;

        // ratexp polynomialset.
        using rps_t = typename vcsn::rat::ratexp_polynomialset_t<rs_t>;

        using namespace dyn::detail;

        REGISTER(accessible, aut_t);
        REGISTER(aut_to_exp, aut_t);
        REGISTER(chain, aut_t);
        REGISTER(coaccessible, aut_t);
        REGISTER2(concatenate, aut_t, aut_t);
        REGISTER(copy, aut_t);

        // dot.
        dot_stream_register(aut_t::sname(),
                            static_cast<dot_stream_t&>(dot<aut_t>));
        dot_string_register(aut_t::sname(),
                            static_cast<dot_string_t&>(dot<aut_t>));
        dot_stream_register(taut_t::sname(),
                            static_cast<dot_stream_t&>(dot<taut_t>));
        dot_string_register(taut_t::sname(),
                            static_cast<dot_string_t&>(dot<taut_t>));

        // edit-automaton.
        make_automaton_editor_register(Ctx::sname(),
                                       make_automaton_editor<aut_t>);

        REGISTER(efsm, aut_t);
        REGISTER(efsm, taut_t);
        REGISTER(constant_term, rs_t);
        REGISTER(info, aut_t);
        REGISTER(info, taut_t);
        REGISTER(info_exp, rs_t);
        REGISTER(is_empty, aut_t);
        REGISTER(is_eps_acyclic, aut_t);
        REGISTER(is_normalized, aut_t);
        REGISTER(is_proper, aut_t);
        REGISTER(is_standard, aut_t);
        REGISTER(is_trim, aut_t);
        REGISTER(is_useless, aut_t);

        is_valid_register(aut_t::sname(),
                          static_cast<is_valid_t&>(is_valid<aut_t>));
        is_valid_exp_register(rs_t::sname(),
                              static_cast<is_valid_exp_t&>(is_valid<rs_t>));

        REGISTER2(left_mult, aut_t, ws_t);
        // lift.
        lift_automaton_register(aut_t::sname(), lift<aut_t>);
        lift_exp_register(rs_t::sname(), lift<rs_t>);

        list_polynomial_register(rps_t::sname(), list<rps_t>);
        REGISTER(make_context, Ctx);
        REGISTER(make_ratexpset, Ctx);

        // print
        print_exp_register(rs_t::sname(), print<rs_t>);
        print_polynomial_register(rps_t::sname(), print<rps_t>);
        print_weight_register(ws_t::sname(), print<ws_t>);

        REGISTER(read_weight, Ctx);
        REGISTER2(right_mult, aut_t, ws_t);

        standard_register(aut_t::sname(),
                          static_cast<standard_t&>(standard<aut_t>));
        standard_exp_register(rs_t::sname(),
                              static_cast<standard_exp_t&>(standard<rs_t>));

        REGISTER(star, aut_t);
        REGISTER2(sum, aut_t, aut_t);
        REGISTER(tikz, aut_t);
        REGISTER(tikz, taut_t);
        REGISTER(transpose, aut_t);
        REGISTER(transpose_exp, rs_t);
        REGISTER(trim, aut_t);
        REGISTER2(union_a, aut_t, aut_t);

        register_kind_functions<Ctx>(typename Ctx::kind_t());
        return true;
      }
    }
  }

# undef REGISTER2
# undef REGISTER

# if VCSN_INSTANTIATION
#  define VCSN_CTX_INSTANTIATE_2(Ctx)                           \
  namespace ctx                                                 \
  {                                                             \
    namespace detail                                            \
    {                                                           \
      static bool registered = register_functions<Ctx>();       \
    }                                                           \
  }
# else
#  define VCSN_CTX_INSTANTIATE_2(Ctx)
# endif

# define VCSN_CTX_INSTANTIATE(Ctx)              \
  VCSN_CTX_INSTANTIATE_1(ctx::Ctx);             \
  VCSN_CTX_INSTANTIATE_2(Ctx)

# ifndef MAYBE_EXTERN
#  define MAYBE_EXTERN extern
# endif

}

#endif // !VCSN_CTX_INSTANTIATE_HH
