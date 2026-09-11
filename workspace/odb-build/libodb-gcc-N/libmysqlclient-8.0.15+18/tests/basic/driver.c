/* file      : tests/basic/driver.c
 * license   : GPLv2 with Universal FOSS Exception; see accompanying LICENSE file
 */

#include <stdio.h>

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>

int
main ()
{
  if (mysql_library_init (0, NULL, NULL))
  {
    fprintf (stderr, "failed to initialize\n");
    return 1;
  }

  printf ("libmysqlclient version: %s\n", mysql_get_client_info ());
  mysql_library_end ();
  return 0;
}
