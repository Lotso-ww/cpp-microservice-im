// file      : tests/compiler/sloc-counter/driver.cxx
// license   : MIT; see accompanying LICENSE file

#include <fstream>
#include <sstream>
#include <iostream>

#include <libcutl/compiler/code-stream.hxx>
#include <libcutl/compiler/sloc-counter.hxx>

#undef NDEBUG
#include <cassert>

using namespace std;
using namespace cutl::compiler;

int
main (int argc, char* argv[])
{
  ifstream ifs;

  if (argc > 1)
    ifs.open (argv[1]);

  istream& in (ifs.is_open () ? ifs : cin);

  ostringstream os1, os2;
  ostream_filter<sloc_counter, char> filt (os1);

  for (istream::int_type i (in.get ());
       i != istream::traits_type::eof ();
       i = in.get ())
  {
    char c (istream::traits_type::to_char_type (i));
    os1.put (c);
    os2.put (c);
  }

  assert (os1.str () == os2.str ());
  cout << filt.stream ().count () << endl;
}
