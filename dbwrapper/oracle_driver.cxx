/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	null_driver.cxx - this file is integral part of `yaal' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You cannot use any part of sources of this software.
  2. You cannot redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you cannot demand any fees
     for this software.
     You cannot even demand cost of the carrier (CD for example).
  5. You cannot include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <sys/stat.h>
#include <stdlib.h> /* strtol */

#define __STRICT_ANSI__
#include <oci.h>

#include "hcore/memory.hxx"
#include "hcore/base.hxx"
#include "hcore/hstring.hxx"
#include "hcore/hprogramoptionshandler.hxx"
#include "hcore/hcore.hxx"
#include "hcore/hlog.hxx"

using namespace yaal;
using namespace yaal::hcore;

extern "C"
{

static char const* const _logTag_ = "Oracle: ";

HString _instanceName_;

struct OAllocator
	{
	OAllocator* _next;
	char* _buffer;
	};

typedef struct
	{
	int _status;
	OCIEnv* _environment;
	OCIError* _error;
	OCISvcCtx* _serviceContext;
	} OOracle;

typedef struct
	{
	int* _status;
	OCIError* _error;
	OCIStmt* _statement;
	OAllocator* _allocator;
	} OQuery;

OOracle* _brokenDB_ = NULL;

void db_disconnect( void* );
void db_unquery( void* );

void* db_connect( char const* /* In Oracle user name is name of schema. */,
		char const* login_, char const* password_ )
	{
	OOracle* oracle( NULL );
	if ( _brokenDB_ )
		{
		db_disconnect( _brokenDB_ );
		_brokenDB_ = NULL;
		}
	oracle = memory::calloc<OOracle>( 1 );
	if ( ( oracle->_status = OCIInitialize( OCI_DEFAULT, NULL, NULL,
					NULL, NULL ) ) != OCI_SUCCESS )
		{
		_brokenDB_ = oracle;
		return ( NULL );
		}
	if ( ( oracle->_status = OCIEnvCreate( &oracle->_environment,
				OCI_DEFAULT | OCI_THREADED, NULL, NULL, NULL, NULL, 0,
				NULL ) ) != OCI_SUCCESS )
		{
		_brokenDB_ = oracle;
		return ( NULL );
		}
	if ( ( oracle->_status = OCIHandleAlloc( oracle->_environment,
				reinterpret_cast<void**>( &oracle->_error ),
				OCI_HTYPE_ERROR, 0, NULL ) ) != OCI_SUCCESS )
		{
		_brokenDB_ = oracle;
		return ( NULL );
		}
	if ( ( oracle->_status = OCILogon ( oracle->_environment,
				oracle->_error, &oracle->_serviceContext,
				reinterpret_cast<OraText const*>( login_ ),
				static_cast<ub4>( ::strlen( login_ ) ),
				reinterpret_cast<OraText const*>( password_ ),
				static_cast<ub4>( ::strlen( password_ ) ),
				reinterpret_cast<OraText const*>( _instanceName_.raw() ),
				static_cast<ub4>( _instanceName_.get_length() ) ) ) != OCI_SUCCESS )
		{
		_brokenDB_ = oracle;
		return ( NULL );
		}
	return ( oracle );
	}

void db_disconnect( void* data_ )
	{
	OOracle* oracle( static_cast<OOracle*>( data_ ) );
	if ( ! oracle )
		oracle = _brokenDB_;
	if ( oracle )
		{
		if ( oracle->_serviceContext )
			OCILogoff ( oracle->_serviceContext, oracle->_error );
		oracle->_serviceContext = NULL;
		if ( oracle->_error )
			OCIHandleFree ( oracle->_error, OCI_HTYPE_ERROR );
		oracle->_error = NULL;
		if ( oracle->_environment )
			OCIHandleFree ( oracle->_environment, OCI_HTYPE_ENV );
		oracle->_environment = NULL;
		memory::free( oracle );
		}
	return;
	}

int db_errno( void* data_ )
	{
	int error( 0 );
	OOracle const* oracle( NULL );
	if ( ! data_ )
		data_ = _brokenDB_;
	if ( ! data_ )
		return ( 0 );
	oracle = static_cast<OOracle const*>( data_ );
	if ( ( oracle->_status != OCI_SUCCESS_WITH_INFO )
			&& ( oracle->_status != OCI_ERROR ) )
		return ( oracle->_status );
	OCIErrorGet( oracle->_error, 1, NULL, &error, NULL, 0,
			OCI_HTYPE_ERROR );
	return ( error );
	}

char const* db_error( void* data_ )
	{
	sb4 code( 0 );
	static char textBuffer[ OCI_ERROR_MAXMSG_SIZE ];
	OOracle const* oracle( NULL );
	if ( ! data_ )
		data_ = _brokenDB_;
	if ( ! data_ )
		return ( "fatal" );
	oracle = static_cast<OOracle const*>( data_ );
	switch ( oracle->_status )
		{
		case ( OCI_SUCCESS_WITH_INFO ):
		case ( OCI_ERROR ):
			OCIErrorGet( oracle->_error, 1, NULL, &code,
					reinterpret_cast<OraText*>( textBuffer ),
					OCI_ERROR_MAXMSG_SIZE - 2, OCI_HTYPE_ERROR );
		break;
		case ( OCI_NEED_DATA ):
			return ( "OCI_NEED_DATA" );
		case ( OCI_NO_DATA ):
			return ( "OCI_NO_DATA" );
		case ( OCI_INVALID_HANDLE ):
			return ( "OCI_INVALID_HANDLE" );
		case ( OCI_STILL_EXECUTING ):
			return ( "OCI_STILL_EXECUTING" );
		case ( OCI_CONTINUE ):
			return ( "OCI_CONTINUE" );
		default:
			snprintf( textBuffer, OCI_ERROR_MAXMSG_SIZE - 2,
					"Error - %d", oracle->_status );
		}
	return ( textBuffer );
	}

void* db_query( void* data_, char const* query_ )
	{
	OOracle* oracle( static_cast<OOracle*>( data_ ) );
	OQuery* queryObj( memory::calloc<OQuery>( 1 ) );
	HString queryStr( query_ );
	int iters = 0;
	int length = static_cast<int>( ::strlen( query_ ) );
	char* end = ( const_cast<char*>( query_ ) + length ) - 1;
	
	if ( ( *end ) == ';' )
		( *end ) = 0;
	oracle->_status = OCIStmtPrepare2( oracle->_serviceContext,
			&queryObj->_statement, oracle->_error,
			reinterpret_cast<OraText const*>( query_ ),
			static_cast<int>( ::strlen( query_ ) ), NULL, 0, OCI_NTV_SYNTAX, OCI_DEFAULT );
	queryObj->_status = &oracle->_status;
	queryObj->_error = oracle->_error;
	if ( ( oracle->_status != OCI_SUCCESS )
			&& ( oracle->_status != OCI_SUCCESS_WITH_INFO ) )
		{
		log( LOG_TYPE::ERROR ) << _logTag_ << __FUNCTION__ << ": failed to prepare statement." << endl;
		db_unquery( queryObj );
		queryObj = NULL;
		}
	else
		{
		if ( oracle->_status == OCI_SUCCESS_WITH_INFO )
			log( LOG_TYPE::INFO ) << _logTag_ <<  __FUNCTION__ << ": " << db_error( oracle ) << endl;
		queryStr.upper();
		if ( queryStr.find( "INSERT" ) == 0 )
			iters = 1;
		else if ( queryStr.find( "UPDATE" ) == 0 )
			iters = 1;
		else if ( queryStr.find( "DELETE" ) == 0 )
			iters = 1;
		oracle->_status = OCIStmtExecute( oracle->_serviceContext,
				queryObj->_statement, oracle->_error, iters, 0,
				NULL, NULL,
				OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS | OCI_STMT_SCROLLABLE_READONLY );
		if ( ( oracle->_status != OCI_SUCCESS )
				&& ( oracle->_status != OCI_SUCCESS_WITH_INFO ) )
			{
			log( LOG_TYPE::ERROR ) << _logTag_ << __FUNCTION__ << ": failed to execute statement." << endl;
			db_unquery( queryObj );
			queryObj = NULL;
			}
		else if ( oracle->_status == OCI_SUCCESS_WITH_INFO )
			log( LOG_TYPE::INFO ) << _logTag_ <<  __FUNCTION__ << ": " << db_error( oracle ) << endl;
		}
	return ( queryObj );
	}

void db_unquery( void* data_ )
	{
	OAllocator* allocator( NULL );
	OQuery* query( static_cast<OQuery*>( data_ ) );
	if ( ( ( * query->_status ) == OCI_SUCCESS )
			|| ( ( * query->_status ) == OCI_SUCCESS_WITH_INFO ) )
		( * query->_status ) = OCIStmtRelease( query->_statement,
				query->_error, NULL, 0, OCI_DEFAULT );
	else
		OCIStmtRelease( query->_statement,
				NULL, NULL, 0, OCI_DEFAULT );
	allocator = query->_allocator;
	while ( allocator )
		{
		allocator = query->_allocator->_next;
		memory::free ( query->_allocator );
		query->_allocator = allocator;
		}
	memory::free ( query );
	return;
	}

char const* rs_get( void* data_, int long row_, int column_ )
	{
	int size( 0 );
	char* data( NULL );
	OAllocator* allocator;
	OCIParam* parameter( NULL );
	OCIDefine* result( NULL );
	OQuery* query( static_cast<OQuery*>( data_ ) );
	if ( ( ( *query->_status ) = OCIParamGet( query->_statement,
					OCI_HTYPE_STMT, query->_error,
					reinterpret_cast<void**>( &parameter ), column_ + 1 ) ) == OCI_SUCCESS )
		{
		if ( ( ( *query->_status ) = OCIAttrGet( parameter,
						OCI_DTYPE_PARAM, &size, 0, OCI_ATTR_DATA_SIZE,
						query->_error ) ) == OCI_SUCCESS )
			{
			data = memory::calloc<char>( size + 1 );
			if ( ( ( *query->_status ) = OCIDefineByPos( query->_statement,
							&result, query->_error, column_ + 1, data, size + 1,
							SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT ) ) == OCI_SUCCESS )
				{
				if ( ( ( *query->_status ) = OCIStmtFetch2( query->_statement,
								query->_error, 1, OCI_FETCH_ABSOLUTE, static_cast<ub4>( row_ ),
								OCI_DEFAULT ) ) == OCI_SUCCESS )
					{
					allocator = memory::calloc<OAllocator>( 1 );
					allocator->_buffer = data;
					if ( query->_allocator )
						query->_allocator->_next = allocator;
					else
						query->_allocator = allocator;
					}
				else
					memory::free( data );
				}
			}
		}
	if ( parameter )
		OCIDescriptorFree( parameter, OCI_DTYPE_PARAM );
	return ( NULL );
	}

int rs_fields_count( void* data_ )
	{
	int fields( -1 );
	OQuery* query( static_cast<OQuery*>( data_ ) );
	if ( ( ( *query->_status ) = OCIAttrGet( query->_statement,
					OCI_HTYPE_STMT, & fields, 0, OCI_ATTR_PARAM_COUNT,
					query->_error ) ) != OCI_SUCCESS )
		fields = - 1;
	return ( fields );
	}

int long dbrs_records_count( void*, void* dataR_ )
	{
	int rows( 0 );
	OQuery* query( static_cast<OQuery*>( dataR_ ) );
	( *query->_status ) = OCIStmtFetch2( query->_statement,
			 query->_error, 1,
			 OCI_FETCH_LAST, 0,
			 OCI_DEFAULT );
	if ( ( ( *query->_status ) != OCI_SUCCESS )
			&& ( ( *query->_status ) != OCI_SUCCESS_WITH_INFO ) )
		{
		log( LOG_TYPE::ERROR ) << _logTag_ << __FUNCTION__ << ": failed to fetch last row." << endl;
		return ( -1 );
		}
	if ( ( ( *query->_status ) = OCIAttrGet( query->_statement,
					OCI_HTYPE_STMT, &rows, 0, OCI_ATTR_CURRENT_POSITION,
					query->_error ) ) != OCI_SUCCESS )
		{
		if ( ( ( *query->_status ) = OCIAttrGet( query->_statement,
						OCI_HTYPE_STMT, &rows, 0, OCI_ATTR_ROW_COUNT,
						query->_error ) ) != OCI_SUCCESS )
			rows = -1;
		}
	return ( rows );
	}

int long dbrs_id( void*, void* dataR_ )
	{
	int nameLength( 0 );
	int long id( 0 );
	HString sQL;
	text* name( NULL );
	OQuery* query( static_cast<OQuery*>( dataR_ ) );
	OQuery* autonumber( NULL );
	if ( ( ( *query->_status ) = OCIAttrGet ( query->_statement,
					OCI_HTYPE_STMT, &name,
					reinterpret_cast<ub4*>( &nameLength ),
					OCI_ATTR_NAME, query->_error ) ) == OCI_SUCCESS )
		{
		name[ nameLength ] = 0;
		sQL.format ( "SELECT %s_sequence.currval FROM dual;",
				reinterpret_cast<char*>( name ) );
		autonumber = static_cast<OQuery*>( db_query( autonumber, sQL.raw() ) );
		id = strtol( rs_get( autonumber, 0, 0 ), NULL, 10 );
		db_unquery( autonumber );
		}
	return ( id );
	}

char const* rs_column_name( void* dataR_, int field_ )
	{
	int nameLength( 0 );
	text* name( NULL );
	OCIParam* parameter( NULL );
	OQuery* query = static_cast<OQuery*>( dataR_ );
	if ( ( ( *query->_status ) = OCIParamGet( query->_statement,
					OCI_HTYPE_STMT, query->_error,
					reinterpret_cast<void**>( &parameter ), field_ + 1 ) ) == OCI_SUCCESS )
		{
		if ( ( ( *query->_status ) = OCIAttrGet( parameter,
						OCI_DTYPE_PARAM, &name,
						reinterpret_cast<ub4*>( &nameLength ),
						OCI_ATTR_NAME, query->_error ) ) == OCI_SUCCESS )
			{
			if ( nameLength >= 0 )
				name [ nameLength ] = 0;
			}
		}
	if ( parameter )
		OCIDescriptorFree( parameter, OCI_DTYPE_PARAM );
	return ( reinterpret_cast < char * > ( name ) );
	}

void oracle_init( void ) __attribute__((__constructor__));
void oracle_init( void )
	{
	yaal_options()( "instance_name", program_options_helper::option_value( _instanceName_ ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "name of the Oracle database instance", "name" );
	yaal_options().process_rc_file( "yaal", "oracle", NULL );
	return;
	}

int yaal_oracle_driver_main( int, char** ) __attribute__(( __noreturn__ ));
int yaal_oracle_driver_main( int, char** )
	{
	::exit( 0 );
	}

}

