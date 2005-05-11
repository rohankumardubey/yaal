/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	dbwrapper.c - this file is integral part of `stdhapi' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You can not use any part of sources of this software.
  2. You can not redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you can not demand any fees
     for this software.
     You can not even demand cost of the carrier (CD for example).
  5. You can not include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

#include "hcore/hexception.h"
M_CVSID ( "$CVSHeader$" );
#include "hcore/xalloc.h"
#include "hcore/hlog.h"
#include "hcore/hstring.h"
#include "hcore/rc_file.h"
#include "hcore/hcore.h"
#include "hconsole/hconsole.h"
#include "tools/tools.h"
#include "dbwrapper.h"
#include "db_driver_loader.h"

using namespace stdhapi::hcore;
using namespace stdhapi::hconsole;
using namespace stdhapi::tools;

namespace stdhapi
{

namespace dbwrapper
{

void dbwrapper_error ( void );
void dbwrapper_exit ( void ) __attribute__  ((noreturn));

extern void * n_pvDlHandle;

int		n_iDataBaseDriver = 0;

OVariable n_psVariables [ ] =
	{
		{ 0, NULL, NULL }
	};

bool set_dbwrapper_variables ( HString & a_roOption, HString & a_roValue )
	{
	if ( ! strcasecmp ( a_roOption, "set_env" ) )
		set_env ( a_roValue );
	else if ( ! strcasecmp ( a_roOption, "log_mask" ) )
		{
		if ( ! strcasecmp ( a_roValue, "LOG_SQL" ) )
			n_lLogMask |= D_LOG_SQL;
		else
			return ( true );
		}
	else if ( ! strcasecmp ( a_roOption, "data_base_driver" ) )
		{
		if ( ! ( strcasecmp ( a_roValue, "none" )
					&& strcasecmp ( a_roValue, "null" ) ) )
			dbwrapper::n_iDataBaseDriver = D_DB_DRIVER_NONE;
		else if ( ! strcmp ( a_roValue, "MySQL" ) )
			dbwrapper::n_iDataBaseDriver = D_DB_DRIVER_MYSQL;
		else if ( ! strcmp ( a_roValue, "PostgreSQL" ) )
			dbwrapper::n_iDataBaseDriver = D_DB_DRIVER_POSTGRESQL;
		else if ( ! strcmp ( a_roValue, "SQLite" ) )
			dbwrapper::n_iDataBaseDriver = D_DB_DRIVER_SQLITE;
		else if ( ! strcmp ( a_roValue, "SQLite3" ) )
			dbwrapper::n_iDataBaseDriver = D_DB_DRIVER_SQLITE3;
		else
			{
			log ( D_LOG_ERROR ) << "Error: `" << a_roValue;
			log << "' is unknown driver." << endl;
			exit ( 1 );
			}
		}
	else
		return ( true );
	return ( false );
	}

extern "C"
void dbwrapper_init ( void ); __attribute__ ( ( constructor ) )
void dbwrapper_init ( void )
	{
	dbwrapper::db_connect = dbwrapper::autoloader_db_connect;
	dbwrapper::db_disconnect = dbwrapper::autoloader_db_disconnect;
	dbwrapper::db_errno = dbwrapper::autoloader_db_errno;
	dbwrapper::db_error = dbwrapper::autoloader_db_error;
	dbwrapper::db_query = dbwrapper::autoloader_db_query;
	dbwrapper::db_unquery = dbwrapper::autoloader_db_unquery;
	dbwrapper::rs_get = dbwrapper::autoloader_rs_get;
	dbwrapper::rs_fields_count = dbwrapper::autoloader_rs_fields_count;
	dbwrapper::dbrs_records_count = dbwrapper::autoloader_dbrs_records_count;
	dbwrapper::dbrs_id = dbwrapper::autoloader_dbrs_id;
	dbwrapper::rs_column_name = dbwrapper::autoloader_rs_column_name;
	rc_file::process_rc_file ( "stdhapi", "dbwrapper", n_psVariables,
			set_dbwrapper_variables );
	return;
	}

extern char const * g_pcDone;

void dbwrapper_fini ( void ); __attribute__ ( ( destructor ) )
void dbwrapper_fini ( void )
	{
	if ( dbwrapper::n_pvDlHandle )
		{
		fprintf ( stderr, "Unloading dynamic database driver ... " );
#ifndef __HOST_OS_TYPE_FREEBSD__
		if ( dlclose ( dbwrapper::n_pvDlHandle ) )
			{
			dbwrapper_error ( );
			dbwrapper_exit ( );
			}
		else
#endif /* not __HOST_OS_TYPE_FREEBSD__ */
			fprintf ( stderr, g_pcDone );
		}
	dbwrapper::n_pvDlHandle = NULL;
	return;
	}

static char const g_pcDynamicLinkerPath [ ] __attribute__(( __unused__,
			__section__(".interp") )) = __DYNAMIC_LINKER__;

void stdhapi_dbwrapper_banner ( void )
	{
	fprintf ( stderr, "\tdbwrapper\n" );
	return;
	}

extern "C"
void stdhapi_dbwrapper_main ( void ) __attribute__(( __noreturn__ ));
void stdhapi_dbwrapper_main ( void )
	{
	if ( g_pcDynamicLinkerPath [ 0 ] )
		{
		stdhapi_hcore_banner ( );
		stdhapi_hconsole_banner ( );
		stdhapi_tools_banner ( );
		stdhapi_dbwrapper_banner ( );
		}
	exit ( 0 );
	}

}

}

/* older versions of g++ fail to handle __attribute__((constructor))
   if no static object exists */

#if __GNUC__ < 3 || \
	 ( __GNUC__ == 3 && __GNUC_MINOR__ < 3 )

namespace
	{
HString g_oDummyDBWRAPPER;
	}

#endif

