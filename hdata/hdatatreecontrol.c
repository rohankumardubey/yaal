/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hdatatreecontrol.c - this file is integral part of `stdhapi' project.

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
#include "hdatatreecontrol.h"

HDataTreeControl::HDataTreeControl ( HRecordSet * a_poRecordSet,
		HWindow * a_poWindow, int a_iRow, int a_iColumn, int a_iHeight,
		int a_iWidth, const char * a_pcTitle, bool a_bDrawLabel,
		int a_iDisabledAttribute, int a_iEnabledAttribute, int a_iFocusedAttribute )
								: HControl ( a_poWindow, a_iRow, a_iColumn, a_iHeight,
										a_iWidth, a_pcTitle, a_bDrawLabel, a_iDisabledAttribute,
										a_iEnabledAttribute, a_iFocusedAttribute ),
								HDataControl ( a_poRecordSet ),
								HTreeControl ( NULL, 0, 0, 0, 0, NULL )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

HDataTreeControl::~HDataTreeControl ( void )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

void HDataTreeControl::populate ( long int /*a_iId*/ )
	{
	M_PROLOG
	f_poRecordSet->open ( );
	while ( ! f_poRecordSet->is_eof ( ) )
		{
		f_poRecordSet->move_next ( );
		}
	f_poRecordSet->close ( );
	return;
	M_EPILOG
	}

