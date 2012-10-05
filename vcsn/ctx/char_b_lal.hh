#ifndef VCSN_CTX_CHAR_B_LAL_HH
# define VCSN_CTX_CHAR_B_LAL_HH

# include <vcsn/ctx/char.hh>
# include <vcsn/weights/b.hh>

namespace vcsn
{
  namespace ctx
  {
    using char_b_lal = char_<vcsn::b, labels_are_letters>;
  }
}

# include <vcsn/algos/determinize.hh>
# include <vcsn/algos/dotty.hh>
# include <vcsn/algos/eval.hh>
# include <vcsn/algos/lift.hh>
# include <vcsn/algos/standard_of.hh>

# ifndef MAYBE_EXTERN
#  define MAYBE_EXTERN extern
# endif

namespace vcsn
{
  VCSN_CTX_INSTANTIATE(ctx::char_b_lal);

  MAYBE_EXTERN template
  auto
  determinize(const mutable_automaton<ctx::char_b_lal>& aut)
    -> mutable_automaton<ctx::char_b_lal>;

  MAYBE_EXTERN template
  class details::evaluator<mutable_automaton<ctx::char_b_lal>>;

  MAYBE_EXTERN template
  bool
  eval(const mutable_automaton<ctx::char_b_lal>& aut, const std::string& w);

#if VCSN_INSTANTIATION
  inline
  bool
  register_char_b_lal_functions()
  {
    dotty_register("char_b_lal",
                   abstract_dotty<mutable_automaton<ctx::char_b_lal>>);
    return true;
  }

  static bool registered = register_char_b_lal_functions();
#endif
};

#endif // !VCSN_CTX_CHAR_B_LAL_HH
