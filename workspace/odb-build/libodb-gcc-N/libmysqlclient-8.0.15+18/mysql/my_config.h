/* file      : mysql/my_config.h -*- C -*-
 * license   : GPLv2 with Universal FOSS Exception; see accompanying LICENSE file
 */

#ifndef MYSQL_MY_CONFIG_H
#define MYSQL_MY_CONFIG_H

/*
 * For the semantics of the following macros refer to config.h.cmake.orig and
 * upstream's configure.cmake.
 *
 * Note that we will explicitly undefine macros that are present in the
 * libmysqlclient source code but should not be defined. While this is not
 * technically required, it simplifies the change tracking (see README-DEV).
 * As a bonus we also make sure that they are not get eventually defined by
 * some system headers.
 */

/*
 * Includes auto-generated version.h. Defines the following configuration
 * macros:

#define PROTOCOL_VERSION

  * Keep all the macros listed (in this exact form) for the change tracking
  * (see README-DEV).
  */
#include <mysql/mysql_version.h> // PROTOCOL_VERSION, MYSQL_SERVER_VERSION

/*
 * Auto-generated. Defines macros that depend on the version and build2
 * configuration variables.
 */
#include <mysql/config.h>

#define PACKAGE_VERSION MYSQL_SERVER_VERSION
#define VERSION         MYSQL_SERVER_VERSION

/*
 * Is ON by default.
 */
#define ENABLED_PROFILING 1

/*
 * Endianess.
 */
#ifdef __FreeBSD__
#  include <sys/endian.h> /* BYTE_ORDER */
#else
#  if defined(_WIN32)
#    ifndef BYTE_ORDER
#      define BIG_ENDIAN    4321
#      define LITTLE_ENDIAN 1234
#      define BYTE_ORDER    LITTLE_ENDIAN
#    endif
#  else
#    include <sys/param.h>  /* BYTE_ORDER/__BYTE_ORDER */
#    ifndef BYTE_ORDER
#      ifdef __BYTE_ORDER
#        define BYTE_ORDER    __BYTE_ORDER
#        define BIG_ENDIAN    __BIG_ENDIAN
#        define LITTLE_ENDIAN __LITTLE_ENDIAN
#      else
#        error no BYTE_ORDER/__BYTE_ORDER define
#      endif
#    endif
#  endif
#endif

#if BYTE_ORDER == BIG_ENDIAN
# define WORDS_BIGENDIAN 1
#endif

/*
 * Types and type sizes.
 */
#ifndef _WIN32
#  define SIZEOF_VOIDP     __SIZEOF_POINTER__
#  define SIZEOF_CHARP     __SIZEOF_POINTER__
#  define SIZEOF_INT       __SIZEOF_INT__
#  define SIZEOF_LONG      __SIZEOF_LONG__
#  define SIZEOF_LONG_LONG __SIZEOF_LONG_LONG__

/*
 * Is always 8 bytes, thanks to the -D_FILE_OFFSET_BITS=64 preprocessor option
 * passed (see buildfile). We also check this at the compile time using
 * _Static_assert() (see mysql/libmysql/assert.c).
 */
#  define SIZEOF_OFF_T     8

/*
 * There is no way to exactly tell the time_t type size at the preprocessing
 * time, so we define it as the most probable one. We check this assumption
 * at the compile time using _Static_assert() (see mysql/libmysql/assert.c).
 */
#  define SIZEOF_TIME_T __SIZEOF_LONG__
#else
#  define SIZEOF_INT       4
#  define SIZEOF_LONG      4
#  define SIZEOF_LONG_LONG 8
#   define SIZEOF_OFF_T    4
#  ifdef _WIN64
#    define SIZEOF_VOIDP   8
#    define SIZEOF_TIME_T  8
#  else
#    define SIZEOF_VOIDP   4
#    define SIZEOF_TIME_T  8
#  endif
#  define SIZEOF_CHARP     SIZEOF_VOIDP
#endif

/*
 * Hard to even find any records of these types.
 */
#undef HAVE_ULONG

/*
 * GNU libc added strlcpy() and strlcat() in version 2.38 (in anticipation
 * of their addition to POSIX).
 */
#if defined(__FreeBSD__) || \
    defined(__APPLE__)   || \
    (defined(__GLIBC__)       && \
     defined(__GLIBC_MINOR__) && \
     (__GLIBC__  > 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 38))
#  define HAVE_STRLCAT 1
#endif

/*
 * Specific for Linux.
 */
#ifdef __linux__
#  define HAVE_MMAP64               1
#  define HAVE_POSIX_TIMERS         1
#  define HAVE_INTEGER_PTHREAD_SELF 1

/*
 * malloc_info()
 */
#  define HAVE_MALLOC_H    1
#  define HAVE_MALLOC_INFO 1

/*
 * syscall(SYS_gettid)
 */
#  define HAVE_SYS_GETTID 1

/*
 * The fdatasync() system call appeared in FreeBSD more recently (11.1), so we
 * disable it's usage on FreeBSD.
 */
#  define HAVE_DECL_FDATASYNC 1
#  define HAVE_FDATASYNC      1

/*
 * Irrelevant for the client.
 */
#  undef HAVE_LINUX_LARGE_PAGES
#endif

/*
 * Specific for FreeBSD.
 */
#ifdef __FreeBSD__
#  define HAVE_PTHREAD_GETTHREADID_NP 1
#endif

/*
 * Specific for Mac OS.
 */
#ifdef __APPLE__
#  define HAVE_PTHREAD_THREADID_NP 1
#endif

/*
 * Specific for FreeBSD and Linux.
 */
#if defined(__FreeBSD__) || defined(__linux__)
#  define HAVE_CLOCK_GETTIME   1
#  define HAVE_CUSERID         1
#  define HAVE_POSIX_FALLOCATE 1
#endif

/*
 * Specific for FreeBSD and Mac OS.
 */
#if defined(__FreeBSD__) || defined(__APPLE__)
#  define FIONREAD_IN_SYS_FILIO      1
#  define HAVE_KQUEUE                1
#  define HAVE_KQUEUE_TIMERS         1
#  define HAVE_SOCKADDR_IN_SIN_LEN   1
#  define HAVE_SOCKADDR_IN6_SIN6_LEN 1
#endif

/*
 * Specific for POSIX.
 */
#ifndef _WIN32
#  define HAVE_DLFCN_H              1
#  define HAVE_LANGINFO_H           1
#  define HAVE_POLL_H               1
#  define HAVE_POLL                 1
#  define HAVE_PWD_H                1
#  define HAVE_SYS_IOCTL_H          1
#  define HAVE_SYS_MMAN_H           1
#  define HAVE_SYS_RESOURCE_H       1
#  define HAVE_SYS_SELECT_H         1
#  define HAVE_SYS_SOCKET_H         1
#  define HAVE_TERMIOS_H            1
#  define HAVE_UNISTD_H             1
#  define HAVE_SYS_UN_H             1
#  define HAVE_BACKTRACE            1
#  define HAVE_FTRUNCATE            1
#  define HAVE_FSEEKO               1
#  define HAVE_FSYNC                1
#  define HAVE_GETPASS              1
#  define HAVE_GETPWNAM             1
#  define HAVE_GETPWUID             1
#  define HAVE_GETRLIMIT            1
#  define HAVE_GETRUSAGE            1
#  define HAVE_STPCPY               1
#  define HAVE_STPNCPY              1
#  define HAVE_NL_LANGINFO          1
#  define HAVE_SYS_TIME_H           1
#  define HAVE_SYS_TIMES_H          1
#  define HAVE_TIMES                1
#  define HAVE_GETTIMEOFDAY         1
#  define FIONREAD_IN_SYS_IOCTL     1
#  define TIME_WITH_SYS_TIME        1
#  define HAVE_ARPA_INET_H          1
#  define HAVE_NETINET_IN_H         1
#  define HAVE_FCHMOD               1
#  define HAVE_MLOCKALL             1
#  define HAVE_SYS_WAIT_H           1
#  define DEFAULT_TMPDIR            P_tmpdir

/*
 * Not POSIX but are present on Linux and BSDs.
 */
#  define HAVE_EXECINFO_H 1

/*
 * Support for tagging symbols with __attribute__((visibility("hidden")))
 */
#  define HAVE_VISIBILITY_HIDDEN 1

/*
 * Compiler builtins.
 */
#  define HAVE_BUILTIN_UNREACHABLE 1
#  define HAVE_BUILTIN_EXPECT      1
#  define HAVE_BUILTIN_STPCPY      1

/*
 * Libedit.
 */
#  define USE_LIBEDIT_INTERFACE      1
#  define USE_NEW_EDITLINE_INTERFACE 1
#  define HAVE_HIST_ENTRY            1
#  undef  HAVE_TERM_H

#  define GWINSZ_IN_SYS_IOCTL 1
#  define HAVE_INDEX          1
#  define HAVE_INITGROUPS     1

/*
 * Specific for Windows.
 */
#else
#  define WITH_PERFSCHEMA_STORAGE_ENGINE 1
#  define HAVE_TELL                      1
#  define NO_FCNTL_NONBLOCK              1
#  define DEFAULT_TMPDIR                 ""
#  define FN_NO_CASE_SENSE               1
#endif

#ifdef _WIN32
#  define SO_EXT ".dll"
#elif defined(__APPLE__)
#  define SO_EXT ".dylib"
#else
#  define SO_EXT ".so"
#endif

/*
 * Common for all supported OSes/compilers.
 */
#define MAX_INDEXES                  64U
#define HAVE_SYS_TYPES_H             1
#define STACK_DIRECTION              -1
#define CPU_LEVEL1_DCACHE_LINESIZE   64
#define DEFAULT_SECURE_FILE_PRIV_DIR "NULL"

/*
 * Default character set and collation.
 */
#define MYSQL_DEFAULT_CHARSET_NAME   "latin1"
#define MYSQL_DEFAULT_COLLATION_NAME "latin1_swedish_ci"

#undef DISABLE_PSI_THREAD
#undef DISABLE_PSI_MUTEX
#undef DISABLE_PSI_RWLOCK
#undef DISABLE_PSI_COND
#undef DISABLE_PSI_FILE
#undef DISABLE_PSI_TABLE
#undef DISABLE_PSI_SOCKET
#undef DISABLE_PSI_STAGE
#undef DISABLE_PSI_STATEMENT
#undef DISABLE_PSI_SP
#undef DISABLE_PSI_PS
#undef DISABLE_PSI_IDLE
#undef DISABLE_PSI_STATEMENT_DIGEST
#undef DISABLE_PSI_METADATA
#undef DISABLE_PSI_MEMORY
#undef DISABLE_PSI_TRANSACTION
#undef DISABLE_PSI_ERROR

/*
 * Obsolete headers and functions.
 */
#undef HAVE_TERMIO_H

/*
 * Specific for unsupported OSes (Solaris, SunOS, HP-UX...).
 */
#undef HAVE_PRINTSTACK
#undef HAVE_GETHRTIME
#undef HAVE_GETPASSPHRASE
#undef HAVE_SOLARIS_ATOMIC
#undef LINUX_ALPINE

/*
 * The upstream package undefines it for all supported compilers.
 */
#undef HAVE_ABI_CXA_DEMANGLE

/*
 * We don't assume any instrumentation.
 */
#undef HAVE_VALGRIND
#undef HAVE_ASAN
#undef HAVE_TSAN
#undef HAVE_UBSAN
#undef HAVE_LSAN_DO_RECOVERABLE_LEAK_CHECK

/*
 * Usage of alloca() is discouraged on Linux and BSDs, so let's not use it.
 */
#undef HAVE_ALLOCA_H

/*
 * Some exotic host_access(), struct request_info, etc.
 */
#undef HAVE_LIBWRAP
#undef HAVE_LIBWRAP_PROTOTYPES

#undef ENABLED_LOCAL_INFILE
#undef EXTRA_DEBUG

#endif /* MYSQL_MY_CONFIG_H */
