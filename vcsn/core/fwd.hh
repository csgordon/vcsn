#ifndef VCSN_CORE_FWD_HH
# define VCSN_CORE_FWD_HH

# include <vcsn/misc/memory.hh> // make_shared_ptr.

// Doxygen hack: we need to comment a namespace, just once, in order
// for its members to show up in the documentation; if we don't do
// this then only classes will be shown, but we have many important
// naked functions as well --- at least one per algorithm.

/// The Vaucanson Static layer, containing data structures and
/// algorithm implementation at the lowest level.
namespace vcsn
{

  /// The Vaucanson Dyn layer, containing data structures and
  /// algorithm implementation at the lowest level.
  namespace dyn
  {}
}

namespace vcsn
{

  // vcsn/core/crange.hh
  template <class C>
  struct container_range;

  template <class C>
  struct container_filter_range;


  // vcsn/core/mutable-automaton.hh
  namespace detail
  {
    template <typename Context>
    class mutable_automaton_impl;
  }
  template <typename Context>
  using mutable_automaton
    = std::shared_ptr<detail::mutable_automaton_impl<Context>>;


  // vcsn/core/permutation-automaton.hh.
  namespace detail
  {
    template <typename Aut>
    class permutation_automaton_impl;
  }

  /// A permutation automaton as a shared pointer.
  template <typename Aut>
  using permutation_automaton
    = std::shared_ptr<detail::permutation_automaton_impl<Aut>>;


  // vcsn/core/ratexp-automaton.hh.
  namespace detail
  {
    template <typename Aut>
    class ratexp_automaton_impl;
  }

  /// A ratexp automaton as a shared pointer.
  template <typename Aut>
  using ratexp_automaton
    = std::shared_ptr<detail::ratexp_automaton_impl<Aut>>;

} // namespace vcsn

#endif // !VCSN_CORE_FWD_HH
