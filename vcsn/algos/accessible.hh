#ifndef VCSN_ALGOS_ACCESSIBLE_HH
# define VCSN_ALGOS_ACCESSIBLE_HH

# include <deque>
# include <queue>
# include <map>

# include <vcsn/algos/copy.hh>
# include <vcsn/algos/transpose.hh>
# include <vcsn/dyn/fwd.hh>
# include <vcsn/misc/attributes.hh>
# include <vcsn/misc/set.hh>

namespace vcsn
{

  /*--------------------------------------------------.
  | Sets of accessible, coaccessible, useful states.  |
  `--------------------------------------------------*/

  // The set of accessible states, including pre(), and possibly post().
  template <typename AutPtr>
  std::set<state_t_of<AutPtr>>
  accessible_states(const AutPtr& aptr)
  {
    using automaton_ptr = AutPtr;
    using automaton_t = typename automaton_ptr::element_type;
    using state_t = state_t_of<automaton_t>;

    // Reachable states.
    const auto& a = *aptr;
    std::set<state_t> res{a.pre()};

    // States work list.
    using worklist_t = std::queue<state_t>;
    worklist_t todo;
    todo.emplace(a.pre());

    while (!todo.empty())
    {
      const state_t src = todo.front();
      todo.pop();

      for (auto tr : a.all_out(src))
      {
        state_t dst = a.dst_of(tr);
        // If we have not seen it already, explore its successors.
        if (res.emplace(dst).second)
          todo.emplace(dst);
      }
    }

    return res;
  }

  // The set of coaccessible states, including post(), and possibly pre().
  template <typename AutPtr>
  std::set<state_t_of<AutPtr>>
  coaccessible_states(const AutPtr& a)
  {
    return accessible_states(transpose(a));
  }

  // The set of coaccessible states, including post(), and possibly pre().
  template <typename AutPtr>
  std::set<state_t_of<AutPtr>>
  useful_states(const AutPtr& a)
  {
    auto accessible = accessible_states(a);
    auto coaccessible = coaccessible_states(a);
    return intersection(accessible, coaccessible);
  }


  /*----------------------------------------------------.
  | Number of accessible, coaccessible, useful states.  |
  `----------------------------------------------------*/

  /// Number of accessible states, not counting pre() and post().
  template <typename AutPtr>
  size_t
  num_accessible_states(const AutPtr& a)
  {
    auto set = accessible_states(a);
    size_t res = set.size();
    // Don't count pre().
    res -= 1;
    // Don't count post().
    if (has(set, a->post()))
      res -= 1;
    return res;
  }

  /// Number of accessible states, not counting pre() and post().
  template <typename AutPtr>
  size_t
  num_coaccessible_states(const AutPtr& a)
  {
    return num_accessible_states(transpose(a));
  }

  /// Number of accessible states, not counting pre() and post().
  template <typename AutPtr>
  size_t
  num_useful_states(const AutPtr& a)
  {
    auto set = useful_states(a);
    size_t res = set.size();
    // Don't count pre().
    if (has(set, a->pre()))
      res -= 1;
    // Don't count post().
    if (has(set, a->post()))
      res -= 1;
    return res;
  }


  /*-----------------------------------------------.
  | accessible, coaccessible, useful subautomata.  |
  `-----------------------------------------------*/

  template <typename AutPtr>
  AutPtr accessible(const AutPtr& a)
  {
    return vcsn::copy(a, accessible_states(a));
  }

  template <typename AutPtr>
  AutPtr coaccessible(const AutPtr& a)
  {
    return vcsn::copy(a, coaccessible_states(a));
  }

  template <typename AutPtr>
  AutPtr trim(const AutPtr& a)
  {
    return vcsn::copy(a, useful_states(a));
  }

  /*----------------------------------------------------------------.
  | is_trim, is_accessible, is_coaccessible, is_empty, is_useless.  |
  `----------------------------------------------------------------*/

  template <typename AutPtr>
  bool is_trim(const AutPtr& a)
  {
    return num_useful_states(a) == a->num_states();
  }

  template <typename AutPtr>
  bool is_useless(const AutPtr& a)
  {
    return num_useful_states(a) == 0;
  }

  template <typename AutPtr>
  bool is_accessible(const AutPtr& a)
  {
    return num_accessible_states(a) == a->num_states();
  }

  template <typename AutPtr>
  bool is_coaccessible(const AutPtr& a)
  {
    return num_coaccessible_states(a) == a->num_states();
  }

  template <typename AutPtr>
  bool is_empty(const AutPtr& a) ATTRIBUTE_PURE;

  template <typename AutPtr>
  bool is_empty(const AutPtr& a)
  {
    // FIXME: Beware of the case where there is a transition from
    // pre() to post().
    return a->num_states() == 0;
  }

  namespace dyn
  {
    namespace detail
    {

      /*------------------.
      | dyn::accessible.  |
      `------------------*/

      template <typename Aut>
      automaton
      accessible(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return make_automaton(accessible(a));
      }

      REGISTER_DECLARE(accessible,
                       (const automaton&) -> automaton);

      /*--------------------.
      | dyn::coaccessible.  |
      `--------------------*/

      template <typename Aut>
      automaton
      coaccessible(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return make_automaton(coaccessible(a));
      }

      REGISTER_DECLARE(coaccessible,
                       (const automaton&) -> automaton);

      /*------------.
      | dyn::trim.  |
      `------------*/

      template <typename Aut>
      automaton
      trim(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return make_automaton(trim(a));
      }

      REGISTER_DECLARE(trim,
                       (const automaton&) -> automaton);

      /*---------------------.
      | dyn::is_accessible.  |
      `---------------------*/

      template <typename Aut>
      bool
      is_accessible(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return is_accessible(a);
      }

      REGISTER_DECLARE(is_accessible,
                       (const automaton&) -> bool);

      /*-----------------------.
      | dyn::is_coaccessible.  |
      `-----------------------*/

      template <typename Aut>
      bool
      is_coaccessible(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return is_coaccessible(a);
      }

      REGISTER_DECLARE(is_coaccessible,
                       (const automaton&) -> bool);

      /*---------------.
      | dyn::is_trim.  |
      `---------------*/

      template <typename Aut>
      bool
      is_trim(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return is_trim(a);
      }

      REGISTER_DECLARE(is_trim,
                       (const automaton&) -> bool);

      /*------------------.
      | dyn::is_useless.  |
      `------------------*/

      template <typename Aut>
      bool
      is_useless(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return is_useless(a);
      }

      REGISTER_DECLARE(is_useless,
                       (const automaton&) -> bool);

      /*----------------.
      | dyn::is_empty.  |
      `----------------*/

      template <typename Aut>
      bool
      is_empty(const automaton& aut)
      {
        const auto& a = aut->as<Aut>();
        return is_empty(a);
      }

      REGISTER_DECLARE(is_empty,
                       (const automaton&) -> bool);
    }
  }
}

#endif // !VCSN_ALGOS_ACCESSIBLE_HH
