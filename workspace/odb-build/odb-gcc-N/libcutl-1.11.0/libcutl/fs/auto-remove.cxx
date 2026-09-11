// file      : libcutl/fs/auto-remove.cxx
// license   : MIT; see accompanying LICENSE file

#include <cstdio> // std::remove
#include <cerrno>

#include <libcutl/fs/auto-remove.hxx>

namespace cutl
{
  namespace fs
  {
    auto_remove::
    ~auto_remove ()
    {
      if (!canceled_)
        std::remove (path_.string ().c_str ()); // Ignore error.
    }

    auto_removes::
    ~auto_removes ()
    {
      if (!canceled_)
      {
        for (paths::iterator i (paths_.begin ()); i != paths_.end (); ++i)
          std::remove (i->string ().c_str ()); // Ignore error.
      }
    }
  }
}
