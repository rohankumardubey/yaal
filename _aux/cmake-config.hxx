#ifndef YAAL_CONFIG_HXX_INCLUDED
#define YAAL_CONFIG_HXX_INCLUDED 1
#define PACKAGE_NAME "yaal"
#define PACKAGE_VERSION "0.0.0"
#cmakedefine01 HAVE_STRCASESTR
#cmakedefine01 HAVE_ASPRINTF
#cmakedefine01 HAVE_STRTOLD
#cmakedefine01 HAVE_POWL
#cmakedefine01 HAVE_DECL_TEMP_FAILURE_RETRY
#cmakedefine01 HAVE_DECL_FLOORL
#cmakedefine01 HAVE_GETHOSTBYNAME_R
#cmakedefine01 HAVE_DECL_GETADDRINFO
#cmakedefine01 HAVE_GNU_GETHOSTBYNAME_R
#cmakedefine01 HAVE_GNU_GETHOSTBYADDR_R
#cmakedefine01 HAVE_BASENAME_IN_CSTRING
#cmakedefine01 HAVE_BASENAME_ARG_CONST
#cmakedefine01 HAVE_DECL_SUN_LEN
#cmakedefine01 HAVE_DECL_B76800
#cmakedefine01 HAVE_DECL_B28800
#cmakedefine01 HAVE_DECL_B14400
#cmakedefine01 HAVE_DECL_B7200
#cmakedefine01 HAVE_DECL_VSWTC
#cmakedefine01 HAVE_DECL_SIGIOT
#cmakedefine01 HAVE_DECL_ERR
#cmakedefine01 HAVE_MEMRCHR
#cmakedefine01 HAVE_GETLINE
#define LIB_PREFIX "@LIB_PREFIX@"
#define LIB_INFIX "@LIB_INFIX@"
#define LIB_EXT "@LIB_EXT@"
#define SYMBOL_PREFIX "@SYMBOL_PREFIX@"
#define NCURSES_ATTR_GET_SECOND_ARG_TYPE @NCURSES_ATTR_GET_SECOND_ARG_TYPE@
#cmakedefine01 HAVE_SQLITE3_H
#cmakedefine01 HAVE_POSTGRESQL_LIBPQ_FE_H
#cmakedefine01 HAVE_LIBPQ_FE_H
#cmakedefine01 HAVE_MYSQL_MYSQL_H
#cmakedefine01 HAVE_OCI_H
#cmakedefine01 HAVE_OPENSSL_SSL_H
#cmakedefine01 HAVE_LIBXML2_LIBXML_XMLVERSION_H
#cmakedefine01 HAVE_EXECINFO_H
#cmakedefine HAVE_SYS_CONSIO_H
#cmakedefine HAVE_GPM_H
#cmakedefine01 HAVE_NCURSES_CURSES_H
#cmakedefine01 HAVE_CURSES_H
#ifndef __GNUC__
#define __attribute__( x ) /**/
#endif /* not __GNUC__ */
#ifdef __MSVCXX__
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#pragma warning( disable : 4068 )
#pragma warning( disable : 4180 )
#pragma warning( disable : 4250 )
#pragma warning( disable : 4351 )
#pragma warning( disable : 4355 )
#pragma warning( disable : 4646 )
#pragma warning( disable : 4996 )
#undef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#define HAVE_PRETTY_FUNCTION 1
#ifdef __YAAL_BUILD__
#undef HAVE_DECL_TEMP_FAILURE_RETRY
#define HAVE_ICONV_INPUT_CONST 1
#undef TEMP_FAILURE_RETRY
#undef HAVE_GETHOSTBYNAME_R
#define HAVE_GETHOSTBYNAME_R 1
#define snprintf _snprintf
#define strcasecmp stricmp
#define __va_copy( x, y ) ( ( x ) = ( y ) )
extern "C" int setenv( char const*, char  const*, int );
#ifdef PTHREAD_H
#define pthread_self() pthread_self().p
#endif /* PTHREAD_H */
#ifdef _CTIME_
#include <pthread.h>
#include <bits/types.h>
extern "C" int clock_gettime( __clockid_t, struct timespec* );
static int const CLOCK_REALTIME = 0;
#endif /* _CTIME */
#ifdef _CSTRING_
#include <pthread.h>
/* Those functions are available from libgw32c library, but declarations
 * are placed in glibc/string.h which horribly conflicts with Visual C++. */
extern "C" void* memrchr( void const*, int, size_t );
extern "C" char* basename( char const* );
extern "C" char* strptime( char const*, char const*, struct tm* );
extern "C" char* strsignal( int );
#endif /* _CSTRING_ */
#ifdef _CSTDIO_
extern "C" int long getline( char**, size_t*, FILE* );
#endif /* _CSTDIO_ */
#ifdef _CSIGNAL_
#include <glibc/signal.h>
#endif /* _CSIGNAL_ */
#ifdef _UNISTD_H
#include <sys/select.h>
#include <glibc/errno.h>
#endif /* _UNISTD_H */
#include "cleanup.hxx"
#endif /* __YAAL_BUILD__ */
#endif /* __MSVCXX__ */
#endif /* not YAAL_CONFIG_HXX_INCLUDED */
