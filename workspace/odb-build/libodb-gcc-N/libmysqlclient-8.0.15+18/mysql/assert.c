/* file      : mysql/assert.c -*- C -*-
 * license   : GPLv2 with Universal FOSS Exception; see accompanying LICENSE file
 */

#include <mysql/my_config.h>

#ifdef _MSC_VER
#  include <windows.h> // C_ASSERT()
#endif

#include <time.h>
#include <sys/types.h>

/*
 * Make sure that some assumptions made at the preprocessing stage are true.
 */

/*
 * _Static_assert() is introduced in C11, is supported by GCC and Clang for C99
 * as well, and is not supported by VC at all.
 */
#ifdef _MSC_VER
#  define _Static_assert(C, M) C_ASSERT (C)
#endif

_Static_assert (sizeof (off_t)  == SIZEOF_OFF_T,  "unexpected off_t size");
_Static_assert (sizeof (time_t) == SIZEOF_TIME_T, "unexpected time_t size");
