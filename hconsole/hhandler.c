/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hhandler.c - this file is integral part of `stdhapi' project.

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

#include "../hcore/hexception.h"
M_CVSID ( "$CVSHeader$" );
#include "hhandler.h"

HHandler::HHandler ( size_t a_iKeyHandlers, size_t a_iCommandHandlers )
				: f_oPreprocessHandlers ( a_iKeyHandlers ),
					f_oPostprocessHandlers ( a_iKeyHandlers ),
					f_oCommandHandlers ( a_iCommandHandlers )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

HHandler::~HHandler ( void )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

int HHandler::register_preprocess_handler ( int a_iCodeCount, int * a_piCodes,
		HANDLER_t HANDLER )
	{
	M_PROLOG
	int l_iCtr = 0;
	if ( a_piCodes )
		for ( l_iCtr = 0; l_iCtr < a_iCodeCount; l_iCtr ++ )
			f_oPreprocessHandlers [ a_piCodes [ l_iCtr ] ] = HANDLER;
	else f_oPreprocessHandlers [ a_iCodeCount ] = HANDLER;
	return ( 0 );
	M_EPILOG
	}

int HHandler::register_postprocess_handler ( int a_iCodeCount, int * a_piCodes,
		HANDLER_t HANDLER )
	{
	M_PROLOG
	int l_iCtr = 0;
	if ( a_piCodes )
		for ( l_iCtr = 0; l_iCtr < a_iCodeCount; l_iCtr ++ )
			f_oPostprocessHandlers [ a_piCodes [ l_iCtr ] ] = HANDLER;
	else f_oPostprocessHandlers [ a_iCodeCount ] = HANDLER;
	return ( 0 );
	M_EPILOG
	}

int HHandler::process_input ( int a_iCode, process_handler_key_map_t & a_oMap )
	{
	M_PROLOG
	HANDLER_t HANDLER = NULL;
	if ( a_oMap.get ( a_iCode, HANDLER ) )
		a_iCode = ( this->*HANDLER ) ( a_iCode, NULL );
	return ( a_iCode );
	M_EPILOG
	}

HString HHandler::process_command ( void )
	{
	M_PROLOG
	HANDLER_t HANDLER = NULL;
	HString l_oCommand;
	if ( f_oCommand )
		{
		l_oCommand = f_oCommand.split ( " \t", 0 );
		if ( f_oCommandHandlers.get ( l_oCommand, HANDLER ) )
			{
			( this->*HANDLER ) ( 0, ( char * ) f_oCommand );
			f_oCommand = "";
			}
		}
	return ( f_oCommand );
	M_EPILOG
	}

