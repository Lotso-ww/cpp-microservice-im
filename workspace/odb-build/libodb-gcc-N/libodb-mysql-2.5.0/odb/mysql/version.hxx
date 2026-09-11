// file      : odb/mysql/version.hxx.in
// license   : GNU GPL v2; see accompanying LICENSE file

#ifndef LIBODB_MYSQL_VERSION // Note: using the version macro itself.

#include <odb/mysql/details/config.hxx> // LIBODB_MYSQL_MARIADB

// New numeric version format is AAAAABBBBBCCCCCDDDE where:
//
// AAAAA - major version number
// BBBBB - minor version number
// CCCCC - bugfix version number
// DDD   - alpha / beta (DDD + 500) version number
// E     - final (0) / snapshot (1)
//
// When DDDE is not 0, 1 is subtracted from AAAAABBBBBCCCCC. For example:
//
// Version      AAAAABBBBBCCCCCDDDE
//
// 0.1.0        0000000001000000000
// 0.1.2        0000000001000020000
// 1.2.3        0000100002000030000
// 2.2.0-a.1    0000200001999990010
// 3.0.0-b.2    0000299999999995020
// 2.2.0-a.1.z  0000200001999990011
//
#define LIBODB_MYSQL_VERSION_FULL  200005000000000ULL
#define LIBODB_MYSQL_VERSION_STR   "2.5.0"
#define LIBODB_MYSQL_VERSION_ID    "2.5.0"

#define LIBODB_MYSQL_VERSION_MAJOR 2
#define LIBODB_MYSQL_VERSION_MINOR 5
#define LIBODB_MYSQL_VERSION_PATCH 0

#define LIBODB_MYSQL_PRE_RELEASE   false

#define LIBODB_MYSQL_SNAPSHOT      0ULL
#define LIBODB_MYSQL_SNAPSHOT_ID   ""

#ifdef LIBODB_MYSQL_MARIADB
#  include <mysql/mariadb_version.h>

// We support MariaDB starting from 10.2.2 when the library started to be
// named as as mariadb, rather than mysqlclient. Before that we just don't
// distinguish it from the MySQL client library.
//
#  if !defined(MYSQL_VERSION_ID) || MYSQL_VERSION_ID < 100202
#    error unexpected MariaDB version detected
#  endif
#else
#  include <mysql/mysql_version.h>

// Check that we have a compatible MySQL version (5.0.3 or later).
//
#  if !defined(MYSQL_VERSION_ID) || MYSQL_VERSION_ID < 50003
#    error incompatible MySQL version detected
#  endif
#endif

#include <odb/version.hxx>

#ifdef LIBODB_VERSION_FULL
#  if !(LIBODB_VERSION_FULL == 200005000000000ULL)
#    error incompatible libodb version, libodb == 2.5.0 is required
#  endif
#endif


// Old/deprecated numeric version format is AABBCCDD where:
//
// AA - major version number
// BB - minor version number
// CC - bugfix version number
// DD - alpha / beta (DD + 50) version number
//
// When DD is not 00, 1 is subtracted from AABBCC. For example:
//
// Version     AABBCCDD
// 2.0.0       02000000
// 2.1.0       02010000
// 2.1.1       02010100
// 2.2.0.a1    02019901
// 3.0.0.b2    02999952
//
#define LIBODB_MYSQL_VERSION 2050000

#endif // LIBODB_MYSQL_VERSION
