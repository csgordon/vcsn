#ifndef VCSN_ALGOS_EPS_REMOVAL_HH
# define VCSN_ALGOS_EPS_REMOVAL_HH

# include <utility>
# include <list>
# include <unordered_map>
# include <stdexcept>

# include <vcsn/algos/copy.hh>
# include <vcsn/algos/is-eps-acyclic.hh>
# include <vcsn/core/kind.hh>
# include <vcsn/misc/star_status.hh>
# include <vcsn/misc/direction.hh>

namespace vcsn
{

  /**
    @class epsilon_remover
    @brief This class contains the core of the epsilon-removal algorithm.

    It contains also statics methods that deal with close notions:
    is_valid, is_proper.
    This class is specialized for labels_are_letter automata since all these
    methods become trivial.

  */

  template <typename Aut, typename Kind>
  class epsilon_remover
  {
    using automaton_t = Aut;
    using state_t = typename automaton_t::state_t;
    using word_t = typename automaton_t::labelset_t::word_t;
    using weightset_t = typename automaton_t::weightset_t;
    using weight_t = typename weightset_t::value_t;
    using label_t = typename automaton_t::label_t;
    using transition_t = typename automaton_t::transition_t;

    /**
      @brief The core of the epsilon-removal

      For each state s
        if s has an epsilon-loop with weight w
            if w is not starable, return false
              else compute ws = star(w)
            endif
            remove the loop
        else
          ws = 1
        endif
        for each incoming epsilon transition e:p-->s with weight h
          for each outgoing transition s--a|k-->q
            add (and not set) transition p-- a | h.ws.k --> q
          endfor
          if s is final with weight h
            add final weight h.ws to p
          endif
          remove e
        endfor
      endfor
      return true

      If the method returns false, the input is corrupted.

      @param input The automaton in which epsilon-transitions while be removed
      @return true if the epsilon-removal succeeds, or false otherwise.
      */

    public:
    static bool in_situ_remover(automaton_t & input)
    {
      label_t empty_word= input.labelset()->identity();
      auto weightset_ptr=input.weightset();
      using state_weight_t = std::pair<state_t, weight_t>;
      std::list<state_weight_t> closure;
      std::list<transition_t> to_erase;
      /*
         For each state (s), the incoming epsilon-transitions
         are put into the to_erase list; in the same time,
         for each of these transitions (t), if (t) is a loop,
         the star of its weight is computed, otherwise,
         (t) is stored into the closure list.
         */
      for (auto s: input.states())
      {
        weight_t star = weightset_ptr->unit();// if there is no eps-loop
        to_erase.clear();
        closure.clear();
        for (auto t: input.in(s, empty_word))
        {
          weight_t t_weight = input.weight_of(t);
          state_t src = input.src_of(t);
          if (src == s){  //loop
            try
            {
              star = weightset_ptr->star(t_weight);
            }
            catch(std::domain_error& e)
            {
              return false;
            }
          }
          else
            closure.push_back(state_weight_t(src, t_weight));
          to_erase.push_back(t);
        }
        //Incoming epsilon transitions are deleted from input
        for (auto t: to_erase)
          input.del_transition(t);
        /*
           For each transition (t : s -- label|weight --> dst),
           for each former
           epsilon transition closure->first -- e|closure->second --> s
           a transition (closure->first -- label | closure->second*star*weight --> dst
           is added to the automaton (add, not set !!)
           */
        for (auto t: input.out(s))
        {
          weight_t s_weight=weightset_ptr->mul(star, input.weight_of(t));
          label_t label = input.label_of(t);
          state_t dst = input.dst_of(t);
          for (auto pair: closure)
            input.add_transition(pair.first, dst, label,
                weightset_ptr->mul(pair.second, s_weight));
        }
        /*
           If (s) is final with weight (weight),
           for each former
           epsilon transition closure->first -- e|closure->second --> s
           pair-second * star * weigt is added to the final weight of closure->first
           */

        if (input.is_final(s))
        {
          weight_t s_weight=weightset_ptr->mul(star,
              input.get_final_weight(s));
          for (auto pair: closure)
            input.add_final(pair.first,
                weightset_ptr->mul(pair.second, s_weight));
        }
      }
      return true;
    }

    /**@brief Test whether an automaton is proper.

      An automaton is proper if and only if it contains no epsilon-transition.

      @param input The tested automaton
      @return true iff the automaton is proper
      */
    static bool is_proper(const automaton_t& input)
    {
      for (auto t: input.transitions())
        if (input.labelset()->is_identity(input.label_of(t)))
          return false;
      return true;
    }

    /**@brief Test whether an automaton is valid.

      The behaviour of this method depends on the star_status of the weight_set:
      -- starable : return true;
      -- tops : copy the input and return the result of epsilon_removal on the copy;
      -- non_starable : return true iff the automaton is epsilon-acyclic
WARNING: for weight_sets with zero divisor, should test whether the weight of
every simple circuit is zero;
-- absval : build a copy of the input where each weight is replaced by its absolute value
and eturn the result of epsilon_removal on the copy.
@param input The tested automaton
@return true iff the automaton is valid
*/
    static bool is_valid(const automaton_t& input);

    /**@brief Remove the epsilon-transitions of the input
      The behaviour of this method depends on the star_status of the weight_set:
      -- starable : always valid, does not throw any exception
      -- tops : the epsilon-removal algo is directly launched on the input;
      if it returns false, an exception is launched
      -- non_starable / absval:
      is_valid is called before launching the algorithm.
      @param input The automaton in which epsilon-transitions will be removed
      @throw domain_error if the input is not valid
      */
    static void eps_removal_here(automaton_t& input);

    static automaton_t eps_removal(const automaton_t& input);
  };

  /*
     The implementation of is_valide depends on star_status;
     the different versions are implemented in EpsilonDispatcher.
     */
  template <typename Aut, star_status_t Status>
  class EpsilonDispatcher {};

  /// TOPS : valid iff the epsilon removal succeeds

  template <typename Aut>
  class EpsilonDispatcher<Aut, star_status_t::TOPS>
  {
    using automaton_t = Aut;
    using remover_t = epsilon_remover<Aut, typename Aut::kind_t>;
    public:
    static bool is_valid(const automaton_t& input)
    {
      if (remover_t::is_proper(input))
        return true;
      if (is_eps_acyclic<Aut>(input))
        return true;
      automaton_t tmp = copy(input);
      return remover_t::in_situ_remover(tmp);
    }

    static void eps_removal_here(automaton_t& input)
    {
      bool t = remover_t::in_situ_remover(input);
      if (!t)
        throw std::domain_error("Non valid automaton");
    }

    static Aut eps_removal(const automaton_t& input)
    {
      Aut tmp= copy(input);
      bool t = remover_t::in_situ_remover(tmp);
      if (!t)
        throw std::domain_error("Non valid automaton");
      return tmp;
    }

  };

  /// !TOPS

  /// ABSVAL : valid iff the epsilon-removal succeeds on the "absolute value"
  /// of the automaton
  template <typename Aut>
  class EpsilonDispatcher<Aut, star_status_t::ABSVAL>
  {
    using automaton_t = Aut;
    using state_t = typename automaton_t::state_t;
    using weightset_t = typename automaton_t::weightset_t;
    using remover_t = epsilon_remover<Aut, typename Aut::kind_t>;
    public:
    static bool is_valid(const automaton_t& input)
    {
      if (remover_t::is_proper(input))
        return true;
      if (epsilon_acyclic<Aut>::test(input))
        return true;
      automaton_t tmp_aut = copy(input);
      //Apply absolute value to the weight of each transition
      weightset_t* weightset_ptr=input.weightset();
      for (auto t: tmp_aut.transitions())
        tmp_aut.set_weight(t, weightset_ptr->abs(tmp_aut.weight_of(t)));
      //Apply eps-removal
      return remover_t::in_situ_remover(tmp_aut);
    }

    static void eps_removal_here(automaton_t& input)
    {
      bool t=is_valid(input);
      if (!t)
        throw std::domain_error("Non valid automaton");
      remover_t::in_situ_remover(input);
    }

    static Aut eps_removal(const Aut & input)
    {
      bool t = is_valid(input);
      if (!t)
        throw std::domain_error("Non valid automaton");
      Aut tmp=copy(input);
      remover_r::in_situ_remover(tmp);
      return tmp;
    }
  };
  /// !ABSVAL

  // STARABLE : always valid
  template <typename Aut>
  class EpsilonDispatcher<Aut, star_status_t::STARABLE>
  {
    public:
      static bool is_valid(const Aut&)
      {
        return true;
      }

      static void eps_removal_here(Aut & input)
      {
        epsilon_remover<Aut, typename Aut::kind_t>::in_situ_remover(input);
      }

      static Aut eps_removal(const Aut & input)
      {
        Aut tmp = copy(input);
        epsilon_remover<Aut, typename Aut::kind_t>::in_situ_remover(tmp);
        return tmp;
      }
  };

  //!STARABLE

  // NON_STARABLE : valid iff there is no epsilon-circuit with weight zero
  // Warning: the property tested here is the acyclicity, which is equivalent
  // only in zero divisor free semirings

  template <typename Aut>
  class EpsilonDispatcher<Aut, star_status_t::NON_STARABLE >
  {
    public:
      static bool is_valid(const Aut & input)
      {
        if (epsilon_remover<Aut, typename Aut::kind_t>::is_proper(input))
          return true;
        return is_eps_acyclic<Aut>(input);
      }

      static void eps_removal_here(Aut & input)
      {
        bool t = is_valid(input);
        if (!t)
          throw std::domain_error("Non valid automaton");
        epsilon_remover<Aut, typename Aut::kind_t>::in_situ_remover(input);
      }

      static Aut eps_removal(const Aut & input)
      {
        bool t = is_valid(input);
        if (!t)
          throw std::domain_error("Non valid automaton");
        Aut tmp = copy(input);
        epsilon_remover<Aut, typename Aut::kind_t>::in_situ_remover(tmp);
        return tmp;
      }
  };

  // !NON_STARABLE

  template <typename Aut, typename Kind>
  inline
  bool epsilon_remover<Aut,Kind>::is_valid(const Aut & input)
  {
    return EpsilonDispatcher<Aut, Aut::weightset_t::star_status()>
      ::is_valid(input);
  }

  template <typename Aut, typename Kind>
  inline
  void epsilon_remover<Aut,Kind>::eps_removal_here(Aut & input)
  {
    if (is_proper(input))
      return;
    EpsilonDispatcher<Aut, Aut::weightset_t::star_status()>
      ::eps_removal_here(input);
  }

  template <typename Aut, typename Kind>
  inline
  Aut epsilon_remover<Aut,Kind>::eps_removal(const Aut & input)
  {
    if (is_proper(input))
      return copy(input);
    return EpsilonDispatcher<Aut, Aut::weightset_t::star_status()>
      ::eps_removal(input);
  }

  /*
     Special case of labels_are_letters
     */

  template <typename Aut>
  class epsilon_remover<Aut, labels_are_letters>
  {
    public:
      static bool is_proper(Aut &)
      {
        return true;
      }

      static bool is_valid(Aut &)
      {
        return true;
      }

      static void eps_removal_here(Aut &)
      {
        return;
      }

      static Aut eps_removal(const Aut & input)
      {
        Aut tmp = copy(input);
        return tmp;
      }
  };

  /*--------------------.
    | eps_removal handler |
    `--------------------*/

  template <class Aut>
  inline
  bool is_proper(Aut & input)
  {
    return epsilon_remover<Aut, typename Aut::kind_t>::is_proper(input);
  }

  template <class Aut>
  inline
  bool is_valid(Aut & input)
  {
    return epsilon_remover<Aut, typename Aut::kind_t>::is_valid(input);
  }

  template <class Aut>
  inline
  void eps_removal_here(Aut & input, direction_t dir = direction_t::FORWARD)
  {
    switch (dir)
    {
      case direction_t::FORWARD :
        epsilon_remover<Aut, typename Aut::kind_t>::eps_removal_here(input);
        return;
      case direction_t::BACKWARD :
        auto tr_input = transpose(input);
        using tr_aut_t = decltype(tr_input);
        epsilon_remover<tr_aut_t, typename tr_aut_t::kind_t>
          ::eps_removal_here(tr_input);
        return;
    }
  }

  template <class Aut>
  inline
  Aut eps_removal(const Aut & input, direction_t dir = direction_t::FORWARD)
  {
    switch (dir)
    {
      case direction_t::FORWARD :
        return epsilon_remover<Aut, typename Aut::kind_t>::eps_removal(input);
      default : //direction_t::BACKWARD
        Aut tmp=copy(input);
        eps_removal_here(tmp, direction_t::BACKWARD);
        return tmp;
    }
  }

  /*----------------.
    | abstract ePS-REM  |
    `----------------*/

  // TODO
  /*

     namespace dyn
     {
     namespace details
     {
     template <typename Aut>
     auto
     eval(const automaton& aut, const std::string& s)
     -> std::string
     {
     auto res = ::vcsn::eval(dynamic_cast<const Aut&>(*aut), s);
     return boost::lexical_cast<std::string>(res);
     }

     using eval_t = auto (const automaton& aut, const std::string& s)
     -> std::string;

     bool eval_register(const std::string& ctx, const eval_t& fn);
     }
     }
     */

} // namespace vcsn

#endif // !VCSN_ALGOS_EPS_REMOVAL_HH
