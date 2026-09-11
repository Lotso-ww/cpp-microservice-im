/* file      : libcrypto/downstream/crypto/platform.h -*- C -*-
 * license   : OpenSSL and SSLeay Licenses; see accompanying LICENSE file
 */

/* This header is used for inclusion of upstream's auto-generated
 * platform-specific header.
 *
 * If there is the auto-generated header foo.h, then the workflow is normally
 * as follows:
 *
 * - Create foo/.
 *
 * - Symlink platform.h into foo/.
 *
 * - Copy upstream's auto-generated for different platforms foo.h into foo/,
 *   using the `<architecture>-<os>[-<compiler>].h` naming convention (see
 *   below for exact names).
 *
 * - Create foo.h including foo/platform.h.
 *
 * See bn_conf.h for the real example.
 */

#ifndef _WIN32
#  if defined(__x86_64__)                                 /* POSIX x86_64 */
#    if defined(__linux__)
#      include "x86_64-linux.h"
#    elif defined(__FreeBSD__)
#      include "x86_64-freebsd.h"
#    elif defined(__APPLE__)
#      include "x86_64-macos.h"
#    else
#      error x86_64 architecture is not supported for this OS
#    endif
#  elif defined(__arm64__) || defined(__aarch64__)        /* POSIX arm64 */
#    if defined(__APPLE__)
#      include "arm64-macos.h"
#    elif defined(__linux__)
#      include "arm64-linux.h"
/*
 * arm64 architecture is not currently supported for FreeBSD.
 */
/*
#    elif defined(__FreeBSD__)
#      include "arm64-freebsd.h"
*/
#    else
#      error arm64 architecture is not supported for this OS
#    endif
#  elif defined(__i386__)                                 /* POSIX i386 */
/*
 * i686 architecture is not currently supported for POSIX.
 */
#    error i686 architecture is not supported for this OS
/*
#    if defined(__linux__)
#      include "i686-linux.h"
#    elif defined(__FreeBSD__)
#      include "i686-freebsd.h"
#    elif defined(__APPLE__)
#      include "i686-macos.h"
#    else
#      error i686 architecture is not supported for this OS
#    endif
*/
#  else
#    error unknown architecture
#  endif
#else
#  if defined(_WIN64)                                     /* Windows x86_64 */
#    if defined(_MSC_VER)
#      include "x86_64-win32-msvc.h"
#    elif defined(__MINGW32__)
#      include "x86_64-win32-mingw32.h"
#    else
#      error x86_64 architecture is not supported for this compiler
#    endif
#  else                                                   /* Windows i386 */
#    if defined(_MSC_VER)
#      include "i686-win32-msvc.h"
#    elif defined(__MINGW32__)
#      include "i686-win32-mingw32.h"
#    else
#      error i686 architecture is not supported for this compiler
#    endif
#  endif
#endif
