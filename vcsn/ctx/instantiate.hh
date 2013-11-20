#ifndef VCSN_CTX_INSTANTIATE_HH
# define VCSN_CTX_INSTANTIATE_HH

# include <vcsn/algos/accessible.hh>
# include <vcsn/algos/are-equivalent.hh> // difference
# include <vcsn/algos/aut-to-exp.hh>
# include <vcsn/algos/complete.hh>
# include <vcsn/algos/concatenate.hh>
# include <vcsn/algos/constant-term.hh>
# include <vcsn/algos/copy.hh>
# include <vcsn/algos/derivation.hh>
# include <vcsn/algos/dot.hh>
# include <vcsn/algos/edit-automaton.hh>
# include <vcsn/algos/efsm.hh>
# include <vcsn/algos/enumerate.hh>
# include <vcsn/algos/eval.hh>
# include <vcsn/algos/expand.hh>
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
# include <vcsn/algos/read.hh>
# include <vcsn/algos/standard.hh>
# include <vcsn/algos/star.hh>
# include <vcsn/algos/star-normal-form.hh>
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

# include <vcsn/weights/b.hh>
# include <vcsn/weights/polynomialset.hh>

# include <vcsn/misc/name.hh>

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
  LIBVCSN_API                                                           \
  std::ostream& Format<Aut>(const Aut& aut, std::ostream& out)

  /*-------------------------------------------------------.
  | Instantiate the function that work for every context.  |
  `-------------------------------------------------------*/

# define VCSN_CTX_INSTANTIATE_1(Ctx)                                    \
  MAYBE_EXTERN template                                                 \
  class LIBVCSN_API mutable_automaton<Ctx>;                             \
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
  VCSN_CTX_INSTANTIATE_PRINT(efsm, mutable_automaton<Ctx>);             \
  VCSN_CTX_INSTANTIATE_PRINT                                            \
  (efsm, vcsn::detail::transpose_automaton<mutable_automaton<Ctx>>);    \
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
  VCSN_CTX_INSTANTIATE_PRINT(tikz, mutable_automaton<Ctx>);             \
                                                                        \
  /* transpose. */                                                      \
  MAYBE_EXTERN template                                                 \
  class LIBVCSN_API detail::transpose_automaton<mutable_automaton<Ctx>>; \
  MAYBE_EXTERN template                                                 \
  class LIBVCSN_API detail::transposer<ratexpset<Ctx>>


  /*----------------------------------.
  | Register the abstract functions.  |
  `----------------------------------*/

#define REGISTER(Algo, ...)                                     \
  Algo ## _register(sname<__VA_ARGS__>(), Algo<__VA_ARGS__>)

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

        // Same labelset, but over Booleans.
        using bool_ctx_t = context<typename Ctx::labelset_t, b>;
        using bool_aut_t = mutable_automaton<bool_ctx_t>;

        using namespace dyn::detail;

        REGISTER(complete, aut_t);
        REGISTER(de_bruijn, Ctx, unsigned);
        REGISTER(derivation, rs_t, const std::string);
        REGISTER(derived_term, rs_t);
        REGISTER(difference, aut_t, bool_aut_t);
        REGISTER(divkbaseb, Ctx, unsigned, unsigned);
        REGISTER(double_ring, Ctx);
        REGISTER(enumerate, aut_t, unsigned);
        REGISTER(eval, aut_t, const std::string);
        REGISTER(infiltration, aut_t, aut_t);
        REGISTER(is_ambiguous, aut_t);
        REGISTER(is_complete, aut_t);
        REGISTER(is_deterministic, aut_t);
        REGISTER(ladybird, Ctx, unsigned);
        REGISTER(list_polynomial, wps_t, std::ostream);
        REGISTER(print_polynomial, wps_t, std::ostream);
        REGISTER(power, aut_t, unsigned);
        REGISTER(product, aut_t, aut_t);
        REGISTER(product, aut_t, bool_aut_t);
        REGISTER(product, bool_aut_t, aut_t);
        REGISTER(random, Ctx);
        REGISTER(shortest, aut_t, unsigned);
        REGISTER(shuffle, aut_t, aut_t);
        REGISTER(u, Ctx, unsigned);

        return true;
      }

      template <typename Ctx>
      bool
      register_kind_functions(labels_are_nullable)
      {
        using aut_t = mutable_automaton<Ctx>;
        using rs_t = ratexpset<Ctx>;
        using namespace dyn::detail;

        REGISTER(de_bruijn, Ctx, unsigned);
        REGISTER(divkbaseb, Ctx, unsigned, unsigned);
        REGISTER(double_ring, Ctx);
        REGISTER(ladybird, Ctx, unsigned);
        REGISTER(proper, aut_t);
        REGISTER(random, Ctx);
        REGISTER(thompson, rs_t);
        REGISTER(u, Ctx, unsigned);

        return true;
      }

      template <typename Ctx>
      bool
      register_kind_functions(labels_are_one)
      {
        using aut_t = mutable_automaton<Ctx>;
        using namespace dyn::detail;

        REGISTER(eliminate_state, aut_t, int);

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

        // label polynomialset.
        using lps_t = typename vcsn::polynomialset<Ctx>;
        // ratexp polynomialset.
        using rps_t = typename vcsn::rat::ratexp_polynomialset_t<rs_t>;

        using namespace dyn::detail;

        REGISTER(accessible, aut_t);
        REGISTER(aut_to_exp, aut_t);
        REGISTER(chain, aut_t, unsigned);
        REGISTER(coaccessible, aut_t);
        REGISTER(concatenate, aut_t, aut_t);
        REGISTER(constant_term, rs_t);
        REGISTER(copy, aut_t);
        REGISTER(dot, aut_t, std::ostream);
        REGISTER(dot, taut_t, std::ostream);
        REGISTER(efsm, aut_t, std::ostream);
        REGISTER(efsm, taut_t, std::ostream);
        REGISTER(expand, rs_t);
        REGISTER(info, aut_t, std::ostream);
        REGISTER(info, taut_t, std::ostream);
        REGISTER(info_exp, rs_t, std::ostream);
        REGISTER(is_empty, aut_t);
        REGISTER(is_eps_acyclic, aut_t);
        REGISTER(is_normalized, aut_t);
        REGISTER(is_proper, aut_t);
        REGISTER(is_standard, aut_t);
        REGISTER(is_trim, aut_t);
        REGISTER(is_useless, aut_t);
        REGISTER(is_valid, aut_t);
        REGISTER(is_valid_exp, rs_t);
        REGISTER(left_mult, aut_t, ws_t);
        REGISTER(lift_automaton, aut_t);
        REGISTER(lift_exp, rs_t);
        REGISTER(list_polynomial, rps_t, std::ostream);
        REGISTER(make_automaton_editor, Ctx);
        REGISTER(make_context, Ctx);
        REGISTER(make_ratexpset, Ctx);
        REGISTER(print_exp, rs_t, std::ostream);
        REGISTER(print_polynomial, lps_t, std::ostream);
        REGISTER(print_polynomial, rps_t, std::ostream);
        REGISTER(print_weight, ws_t, std::ostream);
        REGISTER(read_polynomial, Ctx, std::istream);
        REGISTER(read_weight, Ctx, std::istream);
        REGISTER(right_mult, aut_t, ws_t);
        REGISTER(split, rs_t);
        REGISTER(standard, aut_t);
        REGISTER(standard_exp, rs_t);
        REGISTER(star, aut_t);
        REGISTER(star_normal_form, rs_t);
        REGISTER(sum, aut_t, aut_t);
        REGISTER(tikz, aut_t, std::ostream);
        REGISTER(tikz, taut_t, std::ostream);
        REGISTER(transpose, aut_t);
        REGISTER(transpose_exp, rs_t);
        REGISTER(trim, aut_t);
        REGISTER(union_a, aut_t, aut_t);

        register_kind_functions<Ctx>(typename Ctx::kind_t());
        return true;
      }
    }
  }

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
