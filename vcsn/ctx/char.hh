#ifndef VCSN_CTX_CHAR_HH
# define VCSN_CTX_CHAR_HH

# include <vcsn/alphabets/char.hh>
# include <vcsn/alphabets/setalpha.hh>
# include <vcsn/ctx/ctx.hh>

namespace vcsn
{
  namespace ctx
  {
    template <typename WeightSet, typename Kind = labels_are_letters>
    using char_ = context<vcsn::set_alphabet<vcsn::char_letters>,
                          WeightSet,
                          Kind>;
  }

  /*-------------------------------------------------------.
  | Instantiate the function that work for every context.  |
  `-------------------------------------------------------*/

# define VCSN_CTX_INSTANTIATE_1(Ctx)                                    \
  MAYBE_EXTERN template                                                 \
  class polynomialset<Ctx>;                                             \
                                                                        \
  MAYBE_EXTERN template                                                 \
  class mutable_automaton<Ctx>;                                         \
                                                                        \
  MAYBE_EXTERN template                                                 \
  void                                                                  \
  dotty<mutable_automaton<Ctx>>(const mutable_automaton<Ctx>& aut,      \
                                std::ostream& out);                     \
                                                                        \
  MAYBE_EXTERN template                                                 \
  std::string                                                           \
  dotty<mutable_automaton<Ctx>>(const mutable_automaton<Ctx>& aut);     \
                                                                        \
  MAYBE_EXTERN template                                                 \
  void                                                                  \
  abstract_dotty<mutable_automaton<Ctx>>                                \
  (const abstract_mutable_automaton& aut,                               \
   std::ostream& out);                                                  \
                                                                        \
  MAYBE_EXTERN template                                                 \
  std::string                                                           \
  abstract_dotty<mutable_automaton<Ctx>>                                \
  (const abstract_mutable_automaton& aut);                              \
                                                                        \
  MAYBE_EXTERN template                                                 \
  details::lifted_automaton_t<mutable_automaton<Ctx>>                   \
  lift<mutable_automaton<Ctx>>(const mutable_automaton<Ctx>& aut);      \
                                                                        \
  MAYBE_EXTERN template                                                 \
  class rat::standard_of_visitor<mutable_automaton<Ctx>>;


  /*----------------------------------.
  | Register the abstract functions.  |
  `----------------------------------*/

# if VCSN_INSTANTIATION
#  define VCSN_CTX_INSTANTIATE_2(Ctx)                                   \
  namespace ctx                                                         \
  {                                                                     \
    namespace                                                           \
    {                                                                   \
      bool                                                              \
      register_functions()                                              \
      {                                                                 \
        using aut_t = mutable_automaton<Ctx>;                           \
        /* dotty. */                                                    \
        dotty_register                                                  \
          (#Ctx,                                                        \
           static_cast<const dotty_stream_t&>(abstract_dotty<aut_t>));  \
        dotty_register                                                  \
          (#Ctx,                                                        \
           static_cast<const dotty_string_t&>(abstract_dotty<aut_t>));  \
                                                                        \
        /* edit-automaton. */                                           \
        make_automaton_editor_register                                  \
          (#Ctx,                                                        \
           abstract_make_automaton_editor<aut_t>);                      \
                                                                        \
        /* make-context. */                                             \
        make_context_register                                           \
          (#Ctx,                                                        \
           abstract_make_context<Ctx>);                                 \
                                                                        \
        /* standard_of. */                                              \
        standard_of_register                                            \
          (#Ctx,                                                        \
           abstract_standard_of<aut_t>);                                \
                                                                        \
        /* transpose. */                                                \
        transpose_register                                              \
          (#Ctx,                                                        \
           abstract_transpose<aut_t>);                                  \
                                                                        \
        return true;                                                    \
      }                                                                 \
                                                                        \
      static bool registered = register_functions();                    \
    }                                                                   \
  }
# else
#  define VCSN_CTX_INSTANTIATE_2(Ctx)
# endif


# define VCSN_CTX_INSTANTIATE(Ctx)              \
  VCSN_CTX_INSTANTIATE_1(ctx::Ctx)              \
  VCSN_CTX_INSTANTIATE_2(Ctx)

}

#endif // !VCSN_CTX_CHAR_HH
