#ifndef VCSN_ALGOS_FWD_HH
# define VCSN_ALGOS_FWD_HH

# include <memory>

namespace vcsn
{
  template <typename Aut>
  bool in_situ_remover(Aut& aut, bool prune = true);

  template <typename Aut>
  bool is_proper(const Aut& aut);

  template <typename Aut>
  bool is_valid(const Aut& aut);

  namespace detail
  {
    template <std::size_t Band, typename Aut>
    class blind_automaton_impl;

    template <std::size_t Band, typename AutPtr>
    using blind_automaton
      = std::shared_ptr<blind_automaton_impl<Band, AutPtr>>;
  }

  // vcsn/algos/first-order.hh.
  namespace detail
  {
    template <typename Aut>
    struct linear_automaton_impl;
  }

  template <typename Aut>
  using linear_automaton
    = std::shared_ptr<detail::linear_automaton_impl<Aut>>;

  // vcsn/algos/transpose.hh
  namespace detail
  {
    template <typename Aut>
    class transpose_automaton_impl;
  }

  template <typename AutPtr>
  using transpose_automaton
    = std::shared_ptr<detail::transpose_automaton_impl<AutPtr>>;
}

#endif // !VCSN_ALGOS_FWD_HH
