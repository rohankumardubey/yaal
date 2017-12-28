cmake_minimum_required( VERSION 3.4 )

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
elseif ( ${CMAKE_HOME_DIRECTORY} MATCHES "/_aux/cmake" )
	get_filename_component( CMAKE_HOME_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../../ ABSOLUTE )
endif()

set( CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/_aux/cmake ${CMAKE_MODULE_PATH} )

include( lib )

if( CMAKE_HOST_WIN32 )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
	set( CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO" )
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO" )
	set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO" )
	if ( NOT ( "${PROJECT_NAME}" STREQUAL "yaal" ) ) # For clients only.
		add_definitions( -D__MSVCXX__ /FIyaal/fix.hxx )
	endif()
endif()

if ( "${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
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
else()
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "" )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "" )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "" )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "" )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "" )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "" )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "" )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "" )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "" )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_COV "" )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_COV "" )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_COV "" )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_PROF "" )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_PROF "" )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_PROF "" )
endif()

if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT ( "$ENV{PREFIX}" STREQUAL "" ) )
	set( CMAKE_INSTALL_PREFIX $ENV{PREFIX} CACHE PATH "Install prefix." FORCE )
endif()

include( CheckCXXSourceCompiles )
include( CheckIncludeFileCXX )
include( CheckFunctionExists )
include( CheckSymbolExists )
include( CheckTypeSize )
include( GNUInstallDirs )

if ( NOT ( "$ENV{SYSCONFDIR}" STREQUAL "" ) )
	set( CMAKE_INSTALL_FULL_SYSCONFDIR  $ENV{SYSCONFDIR} CACHE PATH "Path to global system configuration directory." FORCE )
	set( sysconfdir "$ENV{SYSCONFDIR}" )
else()
	set( sysconfdir "${CMAKE_INSTALL_SYSCONFDIR}" )
endif ()

if ( NOT ( "$ENV{LOCALSTATEDIR}" STREQUAL "" ) )
	set( CMAKE_INSTALL_FULL_LOCALSTATEDIR  $ENV{LOCALSTATEDIR} CACHE PATH "Path to data files which the programs modify while they run." FORCE )
	set( localstatedir "$ENV{LOCALSTATEDIR}" )
else()
	set( localstatedir "${CMAKE_INSTALL_LOCALSTATEDIR}" )
endif ()

if ( NOT ( "$ENV{DATADIR}" STREQUAL "" ) )
	set( CMAKE_INSTALL_FULL_DATADIR  $ENV{DATADIR} CACHE PATH "Path to read only application speciffic data files." FORCE )
	set( datadir "$ENV{DATADIR}" )
else()
	set( datadir "${CMAKE_INSTALL_DATADIR}" )
endif ()

set( prefix ${CMAKE_INSTALL_PREFIX} )
set( exec_prefix "\${prefix}" )
set( libdir "\${prefix}/${CMAKE_INSTALL_LIBDIR}" )
set( includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}" )

set( V ${VERBOSE} )
unset( VERBOSE CACHE )
set( VERBOSE ${V} )

if ( NOT CMAKE_HOST_WIN32 )
	check_cxx_compiler_flag( "-std=c++11" CXX11 )
	if ( NOT CXX11 )
		message( FATAL_ERROR "Requested C++ standard version (${CXX_STANDARD}) is not supported." )
	endif()

	find_program( HAVE_UNAME uname )
	if ( NOT HAVE_UNAME )
		message( FATAL_ERROR "Cannot guest operating system name." )
	endif()

	execute_process( COMMAND uname -s OUTPUT_VARIABLE UNAME_OS_NAME OUTPUT_STRIP_TRAILING_WHITESPACE )
	if ( "x${UNAME_OS_NAME}" STREQUAL "xLinux" )
		find_program( HAVE_LSB_RELEASE lsb_release )
		if ( HAVE_LSB_RELEASE )
			execute_process( COMMAND lsb_release -i COMMAND awk /Distributor/{print\ \$3} OUTPUT_VARIABLE HOST_OS_TYPE OUTPUT_STRIP_TRAILING_WHITESPACE )
		endif()
		set( __HOST_OS_TYPE_LINUX__ 1 )
		if ( "x${HOST_OS_TYPE}" STREQUAL "xDebian" )
			set( __HOST_OS_TYPE_DEBIAN__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xUbuntu" )
			set( __HOST_OS_TYPE_UBUNTU__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xCentOS" )
			set( __HOST_OS_TYPE_CENTOS__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xFedora" )
			set( __HOST_OS_TYPE_FEDORA__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xPLD" )
			set( __HOST_OS_TYPE_PLD__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xSlackware" )
			set( __HOST_OS_TYPE_SLACKWARE__ 1 )
		endif()
	else()
		if ( "x${HOST_OS_TYPE}" STREQUAL "xFreeBSD" )
			set( __HOST_OS_TYPE_FREEBSD__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xSunOS" )
			set( __HOST_OS_TYPE_SOLARIS__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xDarwin" )
			set( __HOST_OS_TYPE_DARWIN__ 1 )
		elseif ( "x${HOST_OS_TYPE}" STREQUAL "xCygwin" )
			set( __HOST_OS_TYPE_CYGWIN__ 1 )
		else()
			message( FATAL_ERROR "Cannot recognize host operating system type!" )
		endif()
	endif()
else()
	set( __HOST_OS_TYPE_WINDOWS__ 1 )
endif()

if ( __HOST_OS_TYPE_LINUX__ OR __HOST_OS_TYPE_CYGWIN__ )
	execute_process( COMMAND free -m COMMAND awk "/^Mem:/{print $2}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
elseif ( __HOST_OS_TYPE_FREEBSD__ )
	execute_process( COMMAND /sbin/sysctl hw.physmem COMMAND awk "{print int( $2 / 1024 / 1024 )}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
elseif ( __HOST_OS_TYPE_SOLARIS__ )
	execute_process( COMMAND prtconf COMMAND awk "/Memory/{print $3}" OUTPUT_VARIABLE PHYS_MEM OUTPUT_STRIP_TRAILING_WHITESPACE )
endif()

add_definitions( -D_GNU_SOURCE -D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS )
set( CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_GNU_SOURCE -D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS )

if ( "x${HOST_OS_TYPE}" STREQUAL "xRaspbian" )
	use_cxx_compiler_flag( -mcpu=cortex-a53 )
	use_cxx_compiler_flag( -mfpu=neon-fp-armv8 )
	use_cxx_compiler_flag( -mfloat-abi=hard )
	use_cxx_compiler_flag( -mlittle-endian )
	use_cxx_compiler_flag( -munaligned-access )
	use_cxx_compiler_flag( -funsafe-math-optimizations )
endif()
use_cxx_compiler_flag( -pthread )
use_cxx_compiler_flag( -fmessage-length=0 )
use_cxx_compiler_flag( -std=c++11 )
use_cxx_compiler_flag( -Wno-invalid-source-encoding )
use_cxx_compiler_flag( -Wno-strict-overflow )
use_cxx_compiler_flag( -pipe )
use_cxx_compiler_flag( -m64 )
option(BUILD_NATIVE "Use all available CPU features." OFF)
if ( BUILD_NATIVE )
	use_cxx_compiler_flag( -march=native )
endif()
use_cxx_compiler_flag( -Wl,--fatal-warnings )
use_cxx_compiler_flag( -Wl,--no-undefined )
use_cxx_compiler_flag( -Wl,--enable-auto-import )
use_cxx_compiler_flag( -Wl,--demangle )
use_cxx_compiler_flag( -Wl,-C )
use_cxx_compiler_flag( -Wextra )
if ( NOT CMAKE_HOST_WIN32 )
	get_directory_property( OPTIONS COMPILE_OPTIONS )
	if ( NOT ( "-Wextra" IN_LIST OPTIONS ) )
		use_cxx_compiler_flag( -W )
	endif()
	use_cxx_compiler_flag( -Wall )
endif()
use_cxx_compiler_flag( -Wcast-align )
use_cxx_compiler_flag( -Wcast-qual )
use_cxx_compiler_flag( -Wconversion  )
use_cxx_compiler_flag( -Wendif-labels )
use_cxx_compiler_flag( -Werror )
use_cxx_compiler_flag( -Wlarger-than-32768 )
use_cxx_compiler_flag( -Wmissing-declarations )
use_cxx_compiler_flag( -Wmissing-format-attribute )
use_cxx_compiler_flag( -Wmissing-noreturn )
use_cxx_compiler_flag( -Wpointer-arith )
use_cxx_compiler_flag( -Wredundant-decls )
use_cxx_compiler_flag( -Wsign-compare )
use_cxx_compiler_flag( -Wsign-conversion )
use_cxx_compiler_flag( -Wundef )
use_cxx_compiler_flag( -Wwrite-strings )
use_cxx_compiler_flag( -Weffc++ )
use_cxx_compiler_flag( -Wold-style-cast )
use_cxx_compiler_flag( -Woverloaded-virtual )
use_cxx_compiler_flag( -Wsign-promo )

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "debug" )
	msg( "Generating debug build." )
	set( LIB_INFIX "-d" )
	use_cxx_compiler_flag( -O0 )
	use_cxx_compiler_flag( -ggdb )
	use_cxx_compiler_flag( -g3 )
	use_cxx_compiler_flag( -ggdb3 )
	use_cxx_compiler_flag( -fno-inline )
	add_definitions( -D__DEBUG__ )
	if ( __HOST_OS_TYPE_SOLARIS__ )
		use_cxx_compiler_flag( -gstabs+ )
	endif()
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "release" )
	msg( "Generating release build." )
	set( LIB_INFIX "" )
	use_cxx_compiler_flag( -O3 )
	use_cxx_compiler_flag( -fexpensive-optimizations )
	use_cxx_compiler_flag( -Wno-error )
	add_definitions( -DNDEBUG )
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "prof" )
	msg( "Generating profiled build." )
	seT( liB_INFIX "-p" )
	use_cxx_compiler_flag( -pg )
	set( CMAKE_SHARED_LINKER_FLAGS -pg )
	set( CMAKE_EXE_LINKER_FLAGS -pg )
	add_definitions( -g -DNDEBUG )
elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "cov" )
	msg( "Generating coverage build." )
	set( LIB_INFIX "-c" )
	set( CMAKE_REQUIRED_LIBRARIES --coverage )
	use_cxx_compiler_flag( --coverage )
	set( CMAKE_SHARED_LINKER_FLAGS --coverage )
	set( CMAKE_EXE_LINKER_FLAGS --coverage )
	add_definitions( -D__DEBUG__ )
endif()

if ( NOT CMAKE_HOST_WIN32 )
	set( CMAKE_REQUIRED_FLAGS "-Wshadow -Werror" )
	check_cxx_source_compiles( "#include <pthread.h>
		int main( int, char** ) { return ( 0 ); }" WSHADOW )
	if ( WSHADOW )
		add_definitions( "-Wshadow" )
	endif( WSHADOW )
	set( CMAKE_REQUIRED_FLAGS "-pedantic-errors -Wno-long-long -Werror" )
	check_cxx_source_compiles( "#include <cstdlib>
		int main( int, char** ) { return ( 0 ); }" PEDANTIC_ERRORS )
	if ( PEDANTIC_ERRORS )
		add_definitions( "-pedantic-errors" )
	endif( PEDANTIC_ERRORS )
	set( CMAKE_REQUIRED_FLAGS )
endif( NOT CMAKE_HOST_WIN32 )

if ( NOT CMAKE_HOST_WIN32 )
	include_directories( "/usr/local/include" )
endif ( NOT CMAKE_HOST_WIN32 )

find_package(Git)
if ( GIT_FOUND )
	execute_process( COMMAND git rev-parse HEAD OUTPUT_VARIABLE PACKAGE_VCS OUTPUT_STRIP_TRAILING_WHITESPACE )
	set( GITID "git log --no-color -n 1 --pretty=format:\"%H %ad %an\"" )
else()
	set( PACKAGE_VCS "" )
	set( GITID "true" )
endif()

set( TARGET_PATH "${CMAKE_HOME_DIRECTORY}/build/${CMAKE_BUILD_TYPE}" )
include_directories( ${TARGET_PATH} ${CMAKE_HOME_DIRECTORY} ${CMAKE_INCLUDE_PATH} )
#set( CMAKE_VERBOSE_MAKEFILE true )
enable_language( C )
file( STRINGS ${CMAKE_HOME_DIRECTORY}/Makefile.mk.in PROJECT_VERSION LIMIT_COUNT 1 REGEX "^VERSION[\\t ]*=[\\t ]*" )
string( REGEX REPLACE "^VERSION[\\t ]*=[\\t ]*" "" PROJECT_VERSION ${PROJECT_VERSION} )
file( STRINGS ${CMAKE_HOME_DIRECTORY}/Makefile.mk.in PROJECT_SUBVERSION LIMIT_COUNT 1 REGEX "^SUBVERSION[\\t ]*=[\\t ]*" )
string( REGEX REPLACE "^SUBVERSION[\\t ]*=[\\t ]*" "" PROJECT_SUBVERSION ${PROJECT_SUBVERSION} )
file( STRINGS ${CMAKE_HOME_DIRECTORY}/Makefile.mk.in PROJECT_EXTRAVERSION LIMIT_COUNT 1 REGEX "^EXTRAVERSION[\\t ]*=[\\t ]*" )
string( REGEX REPLACE "^EXTRAVERSION[\\t ]*=[\\t ]*" "" PROJECT_EXTRAVERSION ${PROJECT_EXTRAVERSION} )

today( TODAY )
set( VERSION "${PROJECT_VERSION}.${PROJECT_SUBVERSION}.${PROJECT_EXTRAVERSION}-${TODAY}" )

link_directories( ${CMAKE_LIBRARY_PATH} )

if ( NOT CMAKE_HOST_WIN32 )
	add_custom_target(
		commit_id ALL
		COMMAND "${CMAKE_HOME_DIRECTORY}/_aux/update-commit-id" "${CMAKE_HOME_DIRECTORY}" "${TARGET_PATH}/commit_id.hxx" "${GITID}"
		COMMENT "Regenerating commit_id.hxx."
	)
else()
	add_custom_target(
		commit_id ALL
		COMMAND update-commit-id "${CMAKE_HOME_DIRECTORY}" "${TARGET_PATH}/commit_id.hxx"
		DEPENDS update-config-id
		COMMENT "Regenerating commit_id.hxx."
	)
endif()

if ( CMAKE_HOST_WIN32 )
	if ( IS_DIRECTORY "${CMAKE_HOME_DIRECTORY}/_deploy/freebsd" )
		set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_HOME_DIRECTORY}/_deploy/freebsd/pkg-descr")
	endif()
	if ( EXISTS "${CMAKE_HOME_DIRECTORY}/LICENSE.md" )
		configure_file( "${CMAKE_HOME_DIRECTORY}/LICENSE.md" "${CMAKE_HOME_DIRECTORY}/build/COPYRIGHT.txt" COPYONLY )
		set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_HOME_DIRECTORY}/build/COPYRIGHT.txt" )
	endif()
	set( CPACK_PACKAGE_VENDOR "CodeStation.org" )
	set( CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME} )
	set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${PROJECT_NAME} )
	set( CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION} )
	set( CPACK_PACKAGE_VERSION_MINOR ${PROJECT_SUBVERSION} )
	set( CPACK_PACKAGE_VERSION_PATCH ${PROJECT_EXTRAVERSION} )
	set( CPACK_GENERATOR "WIX" )
	set( CPACK_ARCHIVE_COMPONENT_INSTALL ON )
	set( CPACK_WIX_LIGHT_EXTRA_FLAGS "-sval" )
	set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "C:/Windows/System32/msvcr120.dll" ) # lib OpenSSL dependency
	include( InstallRequiredSystemLibraries )
	include( CPack )
endif()

