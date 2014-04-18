#ifndef VCSN_MISC_ZIP_HH
# define VCSN_MISC_ZIP_HH

# include <type_traits>

# include <vcsn/misc/raise.hh> // pass
# include <vcsn/misc/tuple.hh>

namespace vcsn
{

  template <typename... Sequences>
  struct zip_sequences
  {
    /// Type of the tuple of all the maps.
    using sequences_t = std::tuple<Sequences...>;

    /// Type of index sequences.
    template <std::size_t... I>
    using seq = vcsn::detail::index_sequence<I...>;

    static constexpr size_t size = sizeof...(Sequences);

    /// Index sequence for our maps.
    using indices_t = vcsn::detail::make_index_sequence<sizeof...(Sequences)>;

    zip_sequences(const sequences_t& sequences)
      : sequences_(sequences)
    {}

    zip_sequences(Sequences... sequences)
      : sequences_(sequences...)
    {}

    /// Composite iterator.
    struct iterator
    {
      using iterators_t
        = std::tuple<typename std::remove_reference<Sequences>::type::iterator...>;
      using values_t
        = std::tuple<typename std::remove_reference<Sequences>::type::value_type...>;

      iterator(typename std::remove_reference<Sequences>::type::iterator... is,
               typename std::remove_reference<Sequences>::type::iterator... ends)
        : is_{is...}
        , ends_{ends...}
      {}

      /// The current position.
      iterators_t is_;
      /// The ends.
      iterators_t ends_;

      /// Advance to next position.
      iterator& operator++()
      {
        if (!next_())
          done_();
        return *this;
      }

      bool operator!=(const iterator& that) const
      {
        return not_equal_(that, indices_t{});
      }

      values_t operator*()
      {
        return dereference_(indices_t{});
      }

    private:
      /// We have reached the end, move all the cursors to this end.
      void done_()
      {
        is_ = ends_;
      }

      /// Move to the next position.  Return the index of the lastest
      /// iterator that could move, -1 if we reached the end.
      bool next_()
      {
        return next_(indices_t{});
      }

      template <std::size_t... I>
      bool next_(seq<I...>)
      {
        bool res = true;
        using swallow = int[];
        swallow
          {
            res
            && (++std::get<I>(is_) == std::get<I>(ends_)
                ? res = false
                : true)
              ...
          };
        return res;
      }

      template <std::size_t... I>
      bool not_equal_(const iterator& that, seq<I...>) const
      {
        for (auto n: {(std::get<I>(is_) != std::get<I>(that.is_))...})
          if (n)
            return true;
        return false;
      }

      /// Tuple of values.
      template <std::size_t... I>
      values_t dereference_(seq<I...>) const
      {
        return values_t{(*std::get<I>(is_))...};
      }
    };

    iterator begin()
    {
      auto res = begin_(indices_t{});
      return res;
    }

    iterator end()
    {
      return end_(indices_t{});
    }

  private:
    template <std::size_t... I>
    iterator begin_(seq<I...>)
    {
      return iterator(std::get<I>(sequences_).begin()...,
                      std::get<I>(sequences_).end()...);
    }

    template <std::size_t... I>
    iterator end_(seq<I...>)
    {
      return iterator(std::get<I>(sequences_).end()...,
                      std::get<I>(sequences_).end()...);
    }

    sequences_t sequences_;
  };

  template <typename... Sequences>
  zip_sequences<Sequences...>
  zip(Sequences&&... seqs)
  {
    return {std::forward<Sequences>(seqs)...};
  }

  template <typename... Sequences>
  zip_sequences<Sequences...>
  zip_sequences_tuple(const std::tuple<Sequences...>& seqs)
  {
    return {seqs};
  }
}

#endif // !VCSN_MISC_ZIP_HH
