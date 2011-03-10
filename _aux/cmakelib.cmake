cmake_minimum_required( VERSION 2.8 )
string( TOUPPER ${PROJECT_NAME} PROJECT_NAME_UC )

# setting CMAKE_CONFIGURATION_TYPES breaks default CMAKE_INSTALL_CONFIG_NAME.
# set( CMAKE_CONFIGURATION_TYPES debug release )

if ( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE debug CACHE STRING "Choose the type of build, options are: debug, release, cov, prof." FORCE )
endif ( NOT CMAKE_BUILD_TYPE )
if ( ( NOT ( "${CMAKE_BUILD_TYPE}" STREQUAL "debug" ) )
	AND ( NOT ( "${CMAKE_BUILD_TYPE}" STREQUAL "release" ) )
	AND ( NOT ( "${CMAKE_BUILD_TYPE}" STREQUAL "cov" ) )
	AND ( NOT ( "${CMAKE_BUILD_TYPE}" STREQUAL "prof" ) ) )
	message( FATAL_ERROR "Only `debug' and `release' tergets are supported." )
endif()
if ( ${CMAKE_HOME_DIRECTORY} MATCHES "/_aux" )
	get_filename_component( CMAKE_HOME_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../ ABSOLUTE )
else()
	if ( CMAKE_HOST_WIN32 )
		add_definitions( -D__MSVCXX__ /FIyaal/fix.hxx )
	endif()
endif()

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY build/${CMAKE_BUILD_TYPE} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY build/${CMAKE_BUILD_TYPE} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY build/${CMAKE_BUILD_TYPE} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG build/debug )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG build/debug )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE build/release )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE build/release )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_COV build/cov )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_COV build/cov )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_COV build/cov )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_PROF build/prof )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_PROF build/prof )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_PROF build/prof )

if ( NOT ( "$ENV{PREFIX}" STREQUAL "" ) )
	set( CMAKE_INSTALL_PREFIX $ENV{PREFIX} CACHE PATH "Install prefix." FORCE )
endif ( NOT ( "$ENV{PREFIX}" STREQUAL "" ) )

set( CMAKE_MODULE_PATH ../yaal/_aux )

include( CheckCXXSourceCompiles )
include( CheckCXXCompilerFlag )
include( CheckIncludeFileCXX )
include( CheckFunctionExists )
include( CheckSymbolExists )

macro( use_cxx_compiler_flag flag )
	string( REGEX REPLACE "^-" "" name "${flag}" )
	string( REGEX REPLACE "^-" "" name "${name}" )
	string( REGEX REPLACE "\\+\\+" "XX" name ${name} )
	check_cxx_compiler_flag( ${flag} ${name} )
#	message( "FLAG = ${flag}, NAME = ${name}, VALUE = ${${name}}" )
	if ( ${${name}} )
		add_definitions( ${flag} )
	endif( ${${name}} )
endmacro( use_cxx_compiler_flag )

set( V ${VERBOSE} )
unset( VERBOSE CACHE )
set( VERBOSE ${V} )

macro( msg text )
	if ( ${VERBOSE} )
		message( STATUS ${text} )
	endif( ${VERBOSE} )
endmacro( msg )

if ( EXISTS /etc/debconf.conf )
	set( __HOST_OS_TYPE_LINUX__ 1 )
	set( __HOST_OS_TYPE_DEBIAN__ 1 )
elseif ( EXISTS /etc/00-header )
	set( __HOST_OS_TYPE_LINUX__ 1 )
	set( __HOST_OS_TYPE_UBUNTU__ 1 )
elseif ( EXISTS /etc/yum.repos.d/CentOS-Base.repo )
	set( __HOST_OS_TYPE_LINUX__ 1 )
	set( __HOST_OS_TYPE_CENTOS__ 1 )
elseif ( EXISTS /etc/poldek/poldek.conf )
	set( __HOST_OS_TYPE_LINUX__ 1 )
	set( __HOST_OS_TYPE_PLD__ 1 )
elseif ( EXISTS /etc/random-seed )
	set( __HOST_OS_TYPE_LINUX__ 1 )
	set( __HOST_OS_TYPE_SLACKWARE__ 1 )
elseif ( EXISTS /etc/rc )
	set( __HOST_OS_TYPE_FREEBSD__ 1 )
elseif ( EXISTS /etc/dfs/dfstab )
	set( __HOST_OS_TYPE_SOLARIS__ 1 )
elseif ( EXISTS c:/windows )
	set( __HOST_OS_TYPE_WINDOWS__ 1 )
elseif ( EXISTS /cygdrive )
	set( __HOST_OS_TYPE_CYGWIN__ 1 )
endif()

if ( __HOST_OS_TYPE_LINUX__ OR __HOST_OS_TYPE_CYGWIN__ )
	execute_process( COMMAND free -m COMMAND awk "/^Mem:/{print $2}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
elseif ( __HOST_OS_TYPE_FREEBSD__ )
	execute_process( COMMAND /sbin/sysctl hw.physmem COMMAND awk "{print int( $2 / 1024 / 1024 )}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
elseif ( __HOST_OS_TYPE_SOLARIS__ )
	execute_process( COMMAND prtconf COMMAND awk "/Memory/{print $3}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
endif()

use_cxx_compiler_flag( -pthread )
use_cxx_compiler_flag( -fmessage-length=0 )
use_cxx_compiler_flag( -std=gnu++98 )
use_cxx_compiler_flag( -pipe )
use_cxx_compiler_flag( -m64 )
use_cxx_compiler_flag( -Wl,--fatal-warnings )
use_cxx_compiler_flag( -Wl,--no-undefined )
use_cxx_compiler_flag( -Wl,--enable-auto-import )
use_cxx_compiler_flag( -Wextra )
if ( NOT CMAKE_HOST_WIN32 )
	use_cxx_compiler_flag( -W )
	use_cxx_compiler_flag( -Wall )
endif()
use_cxx_compiler_flag( -Wcast-align )
use_cxx_compiler_flag( -Wcast-qual )
use_cxx_compiler_flag( -Wconversion  )
use_cxx_compiler_flag( -Wendif-labels )
use_cxx_compiler_flag( -Werror )
use_cxx_compiler_flag( -Winline )
use_cxx_compiler_flag( -Wlarger-than-16384 )
use_cxx_compiler_flag( -Wlong-long )
use_cxx_compiler_flag( -Wmissing-noreturn )
use_cxx_compiler_flag( -Wpointer-arith )
use_cxx_compiler_flag( -Wredundant-decls )
use_cxx_compiler_flag( -Wsign-compare )
use_cxx_compiler_flag( -Wundef )
use_cxx_compiler_flag( -Wwrite-strings )
use_cxx_compiler_flag( -Weffc++ )
use_cxx_compiler_flag( -Wsign-promo )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "debug" )
	msg( "Generating debug build." )
	set( LIB_INFIX "-d" )
	use_cxx_compiler_flag( -O0 )
	use_cxx_compiler_flag( -g3 )
	use_cxx_compiler_flag( -ggdb3 )
	use_cxx_compiler_flag( -fno-inline )
	add_definitions( -D__DEBUG__ )
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "release" )
	msg( "Generating release build." )
	set( LIB_INFIX "" )
	use_cxx_compiler_flag( -O3 )
	use_cxx_compiler_flag( -fexpensive-optimizations )
	use_cxx_compiler_flag( -Wno-error )
	add_definitions( -DNDEBUG )
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "prof" )
	msg( "Generating profiled build." )
	set( LIB_INFIX "-p" )
	use_cxx_compiler_flag( -pg )
	set( CMAKE_SHARED_LINKER_FLAGS -pg )
	set( CMAKE_EXE_LINKER_FLAGS -pg )
	add_definitions( -D__DEBUG__ )
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "cov" )
	msg( "Generating coverage build." )
	set( LIB_INFIX "-c" )
	set( CMAKE_REQUIRED_LIBRARIES --coverage )
	use_cxx_compiler_flag( --coverage )
	set( CMAKE_SHARED_LINKER_FLAGS --coverage )
	set( CMAKE_EXE_LINKER_FLAGS --coverage )
	add_definitions( -D__DEBUG__ )
endif()

add_definitions( -D__ID__="" -D__TID__="" )
set( TARGET_PATH "${CMAKE_HOME_DIRECTORY}/build/${CMAKE_BUILD_TYPE}" )
include_directories( ${TARGET_PATH} ${CMAKE_HOME_DIRECTORY} ${CMAKE_INCLUDE_PATH} )
#set( CMAKE_VERBOSE_MAKEFILE true )
enable_language( C )

link_directories( ${CMAKE_LIBRARY_PATH} )

