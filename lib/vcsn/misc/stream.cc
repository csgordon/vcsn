#include <vcsn/misc/stream.hh>

#include <cassert>
#include <cstring> // strerror
#include <istream>
#include <fstream>

namespace vcsn
{

  std::string
  bracketed(std::istream& i, const char lbracket, const char rbracket)
  {
    assert(i.peek() == lbracket);
    i.ignore();
    size_t level = 1;
    std::ostringstream o;
    int c;
    while ((c = i.get()) != -1)
      {
        if (c == lbracket)
          ++level;
        else if (c == rbracket
                 && !--level)
          return o.str();
        o << char(c);
      }
    throw std::runtime_error("missing  " + str_escape(rbracket)
                             + " after " + str_escape(lbracket)
                             + o.str());
  }

  void eat(std::istream& is, char c)
  {
    if (is.peek() != c)
      throw std::runtime_error("unexpected: "
                               + str_escape(is.peek())
                               + ": expected " + str_escape(c));
    is.ignore();
  }

  void eat(std::istream& is, const std::string& expect)
  {
    std::string s;
    char c;
    size_t cnt = expect.size();
    while (cnt && is >> c)
      {
        s.append(1, c);
        --cnt;
      }
    if (s != expect)
      throw std::runtime_error("unexpected: "
                               + str_escape(s)
                               + ": expected " + str_escape(expect));
  }

  std::shared_ptr<std::istream>
  open_input_file(const std::string& file)
  {
    std::shared_ptr<std::istream> res;
    if (file.empty() || file == "-")
      {
        struct noop
        {
          void operator()(...) const {}
        };
        res.reset(&std::cin, noop());
      }
    else
      {
        res.reset(new std::ifstream(file.c_str()));
        if (!res->good())
          throw std::runtime_error("cannot open " + file
                                   + " for reading: "
                                   + strerror(errno));
      }
    return res;
  }

  std::shared_ptr<std::ostream>
  open_output_file(const std::string& file)
  {
    std::shared_ptr<std::ostream> res;
    if (file.empty() || file == "-")
      {
        struct noop
        {
          void operator()(...) const {}
        };
        res.reset(&std::cout, noop());
      }
    else
      {
        res.reset(new std::ofstream(file.c_str()));
        if (!res->good())
          throw std::runtime_error("cannot open " + file
                                   + " for writing: "
                                   + strerror(errno));
      }
    return res;
  }
}
