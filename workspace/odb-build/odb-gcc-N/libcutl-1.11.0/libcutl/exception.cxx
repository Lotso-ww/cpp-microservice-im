// file      : libcutl/exception.cxx
// license   : MIT; see accompanying LICENSE file

#include <typeinfo>

#include <libcutl/exception.hxx>

namespace cutl
{
  char const* exception::
  what () const noexcept
  {
    return typeid (*this).name ();
  }
}
