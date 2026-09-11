/* file      : mysql/config.h.in -*- C -*-
 * license   : GPLv2 with FOSS License Exception; see accompanying COPYING file
 */

#ifndef MYSQL_CONFIG_H
#define MYSQL_CONFIG_H

#define MYSQL_VERSION_MAJOR  8
#define MYSQL_VERSION_MINOR  0
#define MACHINE_TYPE         "x86_64"
#define SYSTEM_TYPE          "linux-gnu"
#define DEFAULT_BASEDIR      "/usr"
#define DEFAULT_CHARSET_HOME DEFAULT_BASEDIR
#define PLUGINDIR            "/usr/lib/mysql/plugin"
#define SHAREDIR             "/usr/share/mysql"
#define DEFAULT_SYSCONFDIR   DEFAULT_BASEDIR"/etc"
#define MYSQL_DATADIR        DEFAULT_BASEDIR"/data"

#endif /* MYSQL_CONFIG_H */
