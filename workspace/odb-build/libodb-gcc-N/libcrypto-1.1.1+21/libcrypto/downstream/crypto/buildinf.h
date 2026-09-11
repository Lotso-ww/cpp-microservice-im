/* file      : libcrypto/downstream/crypto/buildinf.h -*- C -*-
 * license   : OpenSSL and SSLeay Licenses; see accompanying LICENSE file
 */

/* Replace upstream's auto-generated buildinf.h with our own auto-generated
 * version of it. Note that upstream source files include it as follows:
 *
 * #include "buildinf.h"
 *
 * We proxy the inclusion for the headers auto-generating machinery to work
 * properly.
 */
#include <libcrypto/buildinf-body.h>
