/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hrecordset.c - this file is integral part of `stdhapi' project.

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

#include <stdlib.h>

#include "hrecordset.h"

#include "../hcore/hexception.h"
#include "db_driver_loader.h"

#ifndef NULL
#define NULL 0
#endif /* not NULL */

char g_pcEMode [ ] = "record set is not in appropriate mode for operation";
#define E_MODE g_pcEMode

HRecordSet::HRecordSet ( HDataBase * a_poDataBase )
	{
	M_PROLOG
	f_iFieldCount = 0;
	f_iMode = D_CLOSED;
	f_iSetQuantity = 0;
	f_iCursorPosition = 0;
	f_pvCoreData = NULL;
	f_poDataBase = a_poDataBase;
	m_id = 0;
	return;
	M_EPILOG
	}

HRecordSet::~HRecordSet ( void )
	{
	M_PROLOG
	if ( f_pvCoreData )
		dbwrapper::db_unquery ( f_pvCoreData );
	f_pvCoreData = NULL;
	f_poDataBase = NULL;
	return;
	M_EPILOG
	}

HObject * HRecordSet::clone ( void ) const
	{
	M_PROLOG
	HRecordSet * l_poObject = NULL;
	l_poObject = new HRecordSet ( * this );
	if ( ! l_poObject )
		throw new HException ( __WHERE__, "new returned", ( int ) l_poObject );
	return ( l_poObject );
	M_EPILOG
	}

void HRecordSet::sync ( void )
	{
	M_PROLOG 
	if ( f_iMode == D_CLOSED )throw new HException ( __WHERE__, E_MODE, f_iMode );
	sync ( 0, m_id );
	return;
	M_EPILOG
	}

long int HRecordSet::open ( const char * a_pcQuery )
	{
	M_PROLOG
	int l_iCtr = 0;
	if ( f_iMode != D_CLOSED )throw new HException ( __WHERE__, E_MODE, f_iMode );
	if ( a_pcQuery )f_oSQL = a_pcQuery;
	else
		{
		f_oSQL = "SELECT * FROM " + f_oTable;
		if ( ! m_oFilter.is_empty ( ) )
			f_oSQL += ( " WHERE " + m_oFilter );
		if ( ! m_oSort.is_empty ( ) )
			f_oSQL += ( " ORDER BY " + m_oSort );
		f_oSQL += ';';
		}
	f_iSetQuantity = f_poDataBase->query ( f_oSQL );
	f_pvCoreData = f_poDataBase->get_result ( );
	f_iMode = D_NORMAL;
	f_oColumnNames.flush ( );
	f_oValues.flush ( );
	for ( l_iCtr = 0; l_iCtr < f_iFieldCount; l_iCtr ++ )
		{
		f_oColumnNames.add_tail ( ) = dbwrapper::rs_column_name ( f_pvCoreData,
				l_iCtr );
		f_oValues.add_tail ( );
		}
	sync ( );
	return ( f_iSetQuantity );
	M_EPILOG
	}

void HRecordSet::close ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	if ( f_pvCoreData )
		dbwrapper::db_unquery ( f_pvCoreData );
	f_pvCoreData = NULL;
	f_iMode = D_CLOSED;
	M_EPILOG
	}

long int HRecordSet::requery ( const char * a_pcQuery )
	{
	M_PROLOG
	if ( a_pcQuery )f_oSQL = a_pcQuery;
	close ( );
	return ( open ( f_oSQL ) );
	M_EPILOG
	}

bool HRecordSet::is_open ( void )
	{
	return ( f_iMode != D_CLOSED );
	}

bool HRecordSet::is_eof ( void )
	{
	M_PROLOG
	return ( f_iCursorPosition >= f_iSetQuantity );
	M_EPILOG
	}

bool HRecordSet::is_bof ( void )
	{
	M_PROLOG
	return ( f_iCursorPosition < 0 );
	M_EPILOG
	}

void HRecordSet::move_next ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iCursorPosition ++;
	if ( f_iCursorPosition > f_iSetQuantity )
		throw new HException ( __WHERE__, "end of set reached", f_iCursorPosition );
	if ( f_iCursorPosition < f_iSetQuantity )sync ( );
	return;
	M_EPILOG
	}

void HRecordSet::move_previous ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iCursorPosition --;
	if ( f_iCursorPosition < -1 )
		throw new HException ( __WHERE__, "beginning of set reached",
				f_iCursorPosition );
	if ( f_iCursorPosition >= 0 )sync ( );
	return;
	M_EPILOG
	}

void HRecordSet::move_first ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iCursorPosition = 0;
	sync ( );
	return;
	M_EPILOG
	}

void HRecordSet::move_last ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iCursorPosition = f_iSetQuantity - 1;
	sync ( );
	return;
	M_EPILOG
	}

HString HRecordSet::get ( int a_iField )
	{
	M_PROLOG
	HString l_oTmp = dbwrapper::rs_get ( f_pvCoreData, f_iCursorPosition,
			a_iField );
	return ( l_oTmp );
	M_EPILOG
	}

void HRecordSet::add_new ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iMode = D_ADDING;
	return;
	M_EPILOG
	}

void HRecordSet::edit ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_iMode = D_EDITING;
	return;
	M_EPILOG
	}

long int HRecordSet::update ( void )
	{
	M_PROLOG
	int l_iCtr = 0;
	long int l_iRetVal = 0;
	if ( ( f_iMode != D_ADDING ) && ( f_iMode != D_EDITING ) )
		throw new HException ( __WHERE__, E_MODE, f_iMode );
	sync ( );
	if ( f_iMode == D_ADDING )
		{
		f_oBuffer = "INSERT " + f_oTable + " ( ";
		for ( l_iCtr = 1; l_iCtr < f_iFieldCount; l_iCtr ++ )
			{
			if ( l_iCtr > 1 )f_oBuffer += ", ";
			f_oBuffer += f_oColumnNames [ l_iCtr ];
			}
		f_oBuffer += " ) VALUES ( ";
		for ( l_iCtr = 1; l_iCtr < f_iFieldCount; l_iCtr ++ )
			{
			if ( l_iCtr > 1 )f_oBuffer += ", ";
			f_oBuffer += '\'' + f_oValues [ l_iCtr ] + '\'';
			}
		f_oBuffer += " )";
		}
	else
		{
		f_oBuffer = "UPDATE " + f_oTable + " SET ";
		for ( l_iCtr = 1; l_iCtr < f_iFieldCount; l_iCtr ++ )
			{
			if ( l_iCtr > 1 )f_oBuffer += ", ";
			f_oBuffer += f_oColumnNames [ l_iCtr ] + " = '" + f_oValues [ l_iCtr ] + '\'';
			}
		f_oBuffer += " WHERE id = ";
		f_oBuffer += m_id;
		f_oBuffer += ';';
		}
	f_poDataBase->query ( f_oSQL );
	if ( f_iMode == D_ADDING )
		l_iRetVal = f_poDataBase->insert_id ( );
	f_poDataBase->free_result ( );
	f_iMode = D_NORMAL;
	requery ( );
	return ( l_iRetVal );
	M_EPILOG
	}

void HRecordSet::remove ( void )
	{
	M_PROLOG
	if ( f_iMode != D_NORMAL )throw new HException ( __WHERE__, E_MODE, f_iMode );
	f_oBuffer.format ( "DELETE FROM %s WHERE id = %ld;",
			( char * ) f_oTable, m_id );
	f_poDataBase->query ( f_oSQL );
	f_poDataBase->free_result ( );
	requery ( );
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, char & a_rcChar )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		if ( ! l_oTmp.is_empty ( ) )a_rcChar = l_oTmp [ 0 ];
		}
	else f_oValues [ a_iField ] = a_rcChar;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, short & a_rhShort )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		if ( ! l_oTmp.is_empty ( ) )a_rhShort = atoi ( l_oTmp );
		}
	else f_oValues [ a_iField ] = a_rhShort;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, int & a_riInt )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		if ( ! l_oTmp.is_empty ( ) )a_riInt = atoi ( l_oTmp );
		}
	else f_oValues [ a_iField ] = a_riInt;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, long int & a_rlLongInt )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		if ( ! l_oTmp.is_empty ( ) )a_rlLongInt = atol ( l_oTmp );
		}
	else f_oValues [ a_iField ] = a_rlLongInt;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, double & a_rdDouble )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		if ( ! l_oTmp.is_empty ( ) )a_rdDouble = atof ( l_oTmp );
		}
	else f_oValues [ a_iField ] = a_rdDouble;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, HString & a_roString )
	{
	M_PROLOG
	if ( f_iMode == D_NORMAL )a_roString = get ( a_iField );
	else f_oValues [ a_iField ] = a_roString;
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, HTime & a_roTime )
	{
	M_PROLOG
	HTime l_oTime;
	if ( f_iMode == D_NORMAL )a_roTime = get ( a_iField );
	else
		{
		l_oTime = a_roTime;
		l_oTime.format ( "%F %T" );
		f_oValues [ a_iField ] = ( HString & ) l_oTime;
		}
	return;
	M_EPILOG
	}

void HRecordSet::sync ( int a_iField, HInfo & a_roInfo )
	{
	M_PROLOG
	HString l_oTmp;
	if ( f_iMode == D_NORMAL )
		{
		l_oTmp = get ( a_iField );
		a_roInfo = atoi ( l_oTmp );
		a_roInfo = atol ( l_oTmp );
		a_roInfo = atof ( l_oTmp );
		a_roInfo = l_oTmp;
		}
	else f_oValues [ a_iField ] = ( HString & ) a_roInfo;
	return;
	M_EPILOG
	}

