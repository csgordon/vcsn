#ifndef VCSN_MISC_DIRECTION_HH
# define VCSN_MISC_DIRECTION_HH

# include <iosfwd>
# include <string>

namespace vcsn
{
  /// Orientation.
  enum class direction
  {
    /// Looking downstream.
    forward,
    /// Looking upstream.
    backward,
  };

  /// Conversion to string.
  std::string to_string(direction d);

  /// Parsing.
  std::istream& operator>>(std::istream& is, direction& d);

  /// Pretty-printing.
  std::ostream& operator<<(std::ostream& os, direction d);
};

#endif // !VCSN_MISC_DIRECTION_HH
