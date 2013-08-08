#ifndef LIB_VCSN_RAT_READ_HH
# define LIB_VCSN_RAT_READ_HH

# include <vcsn/dyn/fwd.hh>

namespace vcsn
{
  namespace rat
  {
    /// The ratexp in file \a f, with \a ctx as default context.
    dyn::ratexp read_file(const std::string& f, const dyn::context& ctx);

    /// The ratexp in string \a s, with \a ctx as default context.
    dyn::ratexp read_string(const std::string& s, const dyn::context& ctx);
  }
}

#endif // ! LIB_VCSN_RAT_READ_HH
