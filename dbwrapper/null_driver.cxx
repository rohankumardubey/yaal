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

extern "C"
{

void db_disconnect( void* );

void* db_connect( char const* /*dataBase_*/,
		char const* /*login_*/, char const* /*password_*/ )
	{
	return ( NULL );
	}

void db_disconnect( void* /*data_*/ )
	{
	return;
	}

int dbrs_errno( void* /*dataB_*/, void* /*dataR_*/ )
	{
	return ( errno );
	}

char const* dbrs_error( void* /*dataB_*/, void* /*dataR_*/ )
	{
	return ( "" );
	}

void* db_query( void* /*data_*/, char const* /*query_*/ )
	{
	return ( NULL );
	}

void rs_unquery( void* /*data_*/ )
	{
	return;
	}

char const* rs_get( void* /*data_*/, int long /*row_*/, int /*column_*/ )
	{
	return ( NULL );
	}

int rs_fields_count( void* /*data_*/ )
	{
	return ( 0 );
	}

int long dbrs_records_count( void* /*dataB_*/, void* /*dataR_*/ )
	{
	return ( 0 );
	}

int long dbrs_id( void* /*dataB_*/, void* /*dataR_*/ )
	{
	return ( 0 );
	}

char const* rs_column_name( void* /*dataR_*/, int /*field_*/ )
	{
	return ( NULL );
	}

}

