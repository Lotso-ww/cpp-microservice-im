/*
 * file      : mysql/binlog_config.h
 * license   : GPLv2 with Universal FOSS Exception; see accompanying LICENSE file
 */

#ifndef MYSQL_BINLOG_CONFIG_H
#define MYSQL_BINLOG_CONFIG_H

/*
 * For the semantics of the following macros refer to
 * binlog_config.h.cmake.orig and libbinlogevents/configure.cmake.
 *
 * Note that we will explicitly undefine macros that are present in the
 * libmysqlclient source code but should not be defined. While this is not
 * technically required, it simplifies the change tracking (see README-DEV).
 * As a bonus we also make sure that they are not get eventually defined by
 * some system headers.
 */

#include <mysql/my_config.h> // BYTE_ORDER, BIG_ENDIAN, SIZEOF_*

/*
 * Macros defined by upstream's binlog_config.h (see binlog_config.h.cmake for
 * details).
 */
#if BYTE_ORDER == BIG_ENDIAN
# define IS_BIG_ENDIAN 1
#endif

/*
 * If le{64,32,16}toh() are defined in endian.h. That's why the macros are not
 * defined for FreeBSD.
 */
#ifdef __linux__
#  define HAVE_LE64TOH 1
#  define HAVE_LE32TOH 1
#  define HAVE_LE16TOH 1
#endif

#ifndef _WIN32
#  define HAVE_STRNDUP 1
#endif

#if defined(HAVE_LE64TOH) && defined(HAVE_LE32TOH) && defined(HAVE_LE16TOH)
#  define HAVE_ENDIAN_CONVERSION_MACROS 1
#endif

#define HAVE_LONG_LONG 1
#define HAVE_LONG      1
#define HAVE_INT       1

#endif /* MYSQL_BINLOG_CONFIG_H */
