#pragma once

#include <limits>

#include <boost/optional.hpp>

#include <vcsn/ctx/traits.hh>

namespace vcsn
{
  template <Automaton Aut>
  class dijkstra_node
  {
    using automaton_t = Aut;
    using weight_t = weight_t_of<automaton_t>;
    using state_t = state_t_of<automaton_t>;
    /// We have 2 alternatives to raw pointer:
    /// - Reference: Not possible as the dijkstra node is used in a boost
    ///   container that requires default contructible objects.
    /// - Shared pointer: Using the shared pointer returned by `aut->weightset()`
    ///   would work but slows the algorithm down as a lot of dijkstra nodes
    ///   are created.
    /// Hence, we stick with the raw pointer for now.
    using weightset_ptr = const weightset_t_of<automaton_t>*;
  public:
    dijkstra_node() = default;

    dijkstra_node(const automaton_t& aut, state_t state,
                  boost::optional<weight_t> weight, state_t parent,
                  unsigned depth = std::numeric_limits<unsigned>::max())
      : depth_{depth}
      , state_{state}
      , parent_{parent}
      , weight_{weight}
      , ws_{aut->weightset().get()}
    {}

    bool
    operator<(const dijkstra_node& other) const
    {
      if (!weight_)
        return true;
      else if (!other.weight_)
        return false;
      else
        return ws_->less(*weight_, *other.weight_);
    }

    weight_t
    get_weight() const
    {
      return weight_ ? *weight_ : ws_->max();
    }

    void
    set_weight(weight_t weight)
    {
      weight_ = weight;
    }

    // FIXME: private
    unsigned depth_;
    state_t state_;
    state_t parent_;
  private:
    boost::optional<weight_t> weight_;
    weightset_ptr ws_;
  };
}
