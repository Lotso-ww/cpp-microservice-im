// file      : libcutl/fs/exception.cxx
// license   : MIT; see accompanying LICENSE file

#include <libcutl/fs/exception.hxx>

namespace cutl
{
  namespace fs
  {
    char const* error::
    what () const noexcept
    {
      return "filesystem error";
    }
  }
}
