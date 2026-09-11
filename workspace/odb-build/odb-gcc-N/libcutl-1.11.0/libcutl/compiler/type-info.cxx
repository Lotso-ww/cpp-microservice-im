// file      : libcutl/compiler/type-info.cxx
// license   : MIT; see accompanying LICENSE file

#include <libcutl/compiler/type-info.hxx>

namespace cutl
{
  namespace compiler
  {
    using namespace bits;

    type_info const&
    lookup (type_id const& tid)
    {
      type_info_map::const_iterator i (type_info_map_->find (tid));

      if (i == type_info_map_->end ())
        throw no_type_info ();

      return i->second;
    }

    void
    insert (type_info const& ti)
    {
      type_info_map_->insert (type_info_map::value_type (ti.type_id (), ti));
    }
  }
}
