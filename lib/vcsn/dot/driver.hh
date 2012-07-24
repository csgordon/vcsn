#ifndef LIB_VCSN_DOT_DRIVER_HH
# define LIB_VCSN_DOT_DRIVER_HH

# include <vcsn/core/rat/kratexp.hh>
# include <vcsn/core/rat/fwd.hh>
# include <vcsn/ctx/char_b_lal.hh>

# include <lib/vcsn/dot/location.hh>

namespace vcsn
{
  namespace dot
  {

    /// State and public interface for Dot parsing.
    class driver
    {
    public:
      using exp_t = vcsn::rat::exp_t;
      using automaton_t = mutable_automaton<ctx::char_b_lal>;
      driver();

      automaton_t parse_file(const std::string& f);
      automaton_t parse_string(const std::string& e,
                               const location& l = location());

      /// From context_ and letters_, build kratexpset_.
      /// \throw std::
      void make_kratexpset();

      /// Report an error \a m at \a l.
      void error(const location& l, const std::string& m);
      /// The string \a s is invalid at \a l.
      void invalid(const location& l, const std::string& s);

      /// The error messages.
      std::string errors;

      /// The inital location.
      location location_;

    private:
      /// Prepare scanner to load file f.
      void scan_open_(FILE *f);
      /// Prepare scanner to read string e.
      void scan_open_(const std::string& e);
      /// Parse this stream.
      automaton_t parse_(const location& l = location{});
      /// Close the scanner.
      void scan_close_();

      /// The name of the context.
      std::string context_;
      /// The letters gathered so far.
      std::set<char> letters_;
      /// Defined when context_ and letters_ are known.
      abstract_kratexpset* kratexpset_ = nullptr;
      friend class parser;
    };

  }
}
#endif // !LIB_VCSN_DOT_DRIVER_HH
