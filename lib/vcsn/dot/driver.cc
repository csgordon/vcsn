#include <cstring> // strerror
#include <sstream>

#include <lib/vcsn/dot/driver.hh>
#include <lib/vcsn/dot/parse.hh>

namespace vcsn
{
  namespace dot
  {

    driver::driver()
      : kratexpset_{nullptr}
    {}

    auto
    driver::parse_(const location& l)
      -> automaton_t*
    {
      location_ = l;
      // Parser.
      parser p(*this);
      p.set_debug_level(!!getenv("YYDEBUG"));
      if (p.parse())
        {
          delete aut_;
          aut_ = nullptr;
        }
      scan_close_();
      return aut_;
    }

    auto
    driver::parse_file(const std::string& f)
      -> automaton_t*
    {
      FILE *yyin = f == "-" ? stdin : fopen(f.c_str(), "r");
      if (!yyin)
        {
          std::cerr << f << ": cannot open: " << strerror(errno) << std::endl;
          exit(1);
        }
      scan_open_(yyin);
      auto res = parse_();
      if (f != "-")
        fclose(yyin);
      return res;
    }

    auto
    driver::parse_string(const std::string& e, const location& l)
      -> automaton_t*
    {
      scan_open_(e);
      return parse_(l);
    }

    void
    driver::make_kratexpset()
    {
      if (!kratexpset_)
        {
          if (context_.empty())
            throw std::domain_error("no vcsn_context defined");
          if (letters_.empty())
            throw std::domain_error("no vcsn_letters defined");
          std::set<char> ls;
          for (auto l: letters_)
            ls.insert(l);
          if (context_ == "char_b_lal")
            {
              using ctx_t = ctx::char_b_lal;
              auto ctx = new ctx_t{ls};
              kratexpset_ = new concrete_abstract_kratexpset<ctx_t>{*ctx};
              aut_ = new mutable_automaton<ctx_t>{*ctx};
            }
          else
            throw std::domain_error("unknown context: " + context_);
        }
    }

    void
    driver::error(const location& l, const std::string& m)
    {
      std::ostringstream er;
      er  << l << ": " << m;
      if (!!getenv("YYDEBUG"))
        std::cerr << er.str() << std::endl;
      errors += (errors.empty() ? "" : "\n") + er.str();
    }

    void
    driver::invalid(const location& l, const std::string& s)
    {
      error(l, "invalid character: " + s);
    }

    auto
    driver::state_(const std::string& s)
      -> state_t
    {
      assert(aut_);
      state_t res;
      // The pre state has multiple instances with names starting by
      // "I", and likewise for the post state with "F".
      if (s[0] == 'I')
        res = aut_->pre();
      else if (s[0] == 'F')
        res = aut_->post();
      else
        {
          auto i = stmap_.find(s);
          if (i == std::end(stmap_))
            // FIXME: optimize.
            res = stmap_[s] = aut_->new_state();
          else
            res = i->second;
        }
      return res;
    }
  }
}
