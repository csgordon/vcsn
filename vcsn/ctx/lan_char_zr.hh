#ifndef VCSN_CTX_LAN_CHAR_ZR_HH
# define VCSN_CTX_LAN_CHAR_ZR_HH

# include <vcsn/ctx/ctx.hh>
# include <vcsn/ctx/lan_char.hh>
# include <vcsn/weights/z.hh>

namespace vcsn
{
  namespace ctx
  {
    using lan_char_z  = context<lan_char, vcsn::z>;
    using lan_char_zr = context<lan_char, vcsn::ratexpset<lan_char_z>>;
  }
}

# include <vcsn/ctx/instantiate.hh>

namespace vcsn
{
  VCSN_CTX_INSTANTIATE(lan_char_zr);
};

#endif // !VCSN_CTX_LAN_CHAR_ZR_HH
