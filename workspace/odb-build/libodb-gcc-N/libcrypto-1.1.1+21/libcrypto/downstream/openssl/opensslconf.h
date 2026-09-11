/* file      : libcrypto/downstream/openssl/opensslconf.h -*- C -*-
 * license   : OpenSSL and SSLeay Licenses; see accompanying LICENSE file
 */

#ifndef LIBCRYPTO_DOWNSTREAM_OPENSSL_OPENSSLCONF_H
#define LIBCRYPTO_DOWNSTREAM_OPENSSL_OPENSSLCONF_H

/* Define OPENSSL_PIC and {L|B}_ENDIAN macros (that upstream defines on the
 * command line via -D) if we are building libcrypto (see ../../buildfile for
 * details).
 */
#ifdef LIBCRYPTO_BUILD
#  define OPENSSL_PIC

/* Endianess.
 */
#  ifdef __FreeBSD__
#    include <sys/endian.h> /* BYTE_ORDER */
#  else
#    if defined(_WIN32)
#      ifndef BYTE_ORDER
#        define BIG_ENDIAN    4321
#        define LITTLE_ENDIAN 1234
#        define BYTE_ORDER    LITTLE_ENDIAN
#      endif
#    else
#      include <sys/param.h>  /* BYTE_ORDER/__BYTE_ORDER */
#      ifndef BYTE_ORDER
#        ifdef __BYTE_ORDER
#          define BYTE_ORDER    __BYTE_ORDER
#          define BIG_ENDIAN    __BIG_ENDIAN
#          define LITTLE_ENDIAN __LITTLE_ENDIAN
#        else
#          error no BYTE_ORDER/__BYTE_ORDER define
#        endif
#      endif
#    endif
#  endif

#  if BYTE_ORDER == BIG_ENDIAN
#    define B_ENDIAN
#  else
#    define L_ENDIAN
#  endif
#endif

/* Include upstream's auto-generated platform-specific opensslconf.h.
 */
#include <openssl/opensslconf/platform.h>

#endif /* LIBCRYPTO_DOWNSTREAM_OPENSSL_OPENSSLCONF_H */
