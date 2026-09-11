/* file      : tests/basic/driver.c
 * license   : OpenSSL and SSLeay Licenses; see accompanying LICENSE file
 */

#include <stdio.h>
#include <assert.h>

#include <openssl/crypto.h>

int
main ()
{
  assert (OPENSSL_init_crypto (0 /* opts */, NULL /* settings */) == 1);

  printf ("version: %s\n", OpenSSL_version (OPENSSL_VERSION));
  printf ("%s\n", OpenSSL_version (OPENSSL_BUILT_ON));
  printf ("%s\n", OpenSSL_version (OPENSSL_PLATFORM));
  printf ("%s\n", OpenSSL_version (OPENSSL_CFLAGS));
  printf ("%s\n", OpenSSL_version (OPENSSL_DIR));
  printf ("%s\n", OpenSSL_version (OPENSSL_ENGINES_DIR));

  return 0;
}
