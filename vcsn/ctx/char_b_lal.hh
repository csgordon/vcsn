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

# include <vcsn/algos/aut_to_exp.hh>
# include <vcsn/algos/determinize.hh>
# include <vcsn/algos/dotty.hh>
# include <vcsn/algos/edit-automaton.hh>
# include <vcsn/algos/eval.hh>
# include <vcsn/algos/lift.hh>
# include <vcsn/algos/make-context.hh>
# include <vcsn/algos/standard_of.hh>
# include <vcsn/algos/transpose.hh>

# ifndef MAYBE_EXTERN
#  define MAYBE_EXTERN extern
# endif

namespace vcsn
{
  VCSN_CTX_INSTANTIATE(char_b_lal);

  MAYBE_EXTERN template
  auto
  determinize(const mutable_automaton<ctx::char_b_lal>& aut)
    -> mutable_automaton<ctx::char_b_lal>;

  MAYBE_EXTERN template
  class details::evaluator<mutable_automaton<ctx::char_b_lal>>;

  MAYBE_EXTERN template
  bool
  eval(const mutable_automaton<ctx::char_b_lal>& aut, const std::string& w);

# if VCSN_INSTANTIATION
  namespace
  {
    static bool register_char_b_lal =
      dyn::details::determinize_register
      (mutable_automaton<ctx::char_b_lal>::sname(),
       dyn::details::determinize<mutable_automaton<ctx::char_b_lal>>)
      &
      dyn::details::eval_register
      (mutable_automaton<ctx::char_b_lal>::sname(),
       dyn::details::eval<mutable_automaton<ctx::char_b_lal>>)
      ;
  }
#endif
};

#endif // !VCSN_CTX_CHAR_B_LAL_HH
