/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hdataeditcontrol.c - this file is integral part of `stdhapi' project.

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
#include "hdataeditcontrol.h"

HDataEditControl::HDataEditControl( HDataWindow * a_poParent,
		int a_iRow, int a_iColumn, int a_iWidth, int a_iHeight,
		const char * a_pcLabel, int a_iBufferSize, const char * a_pcValue,
		const char * a_pcMask, bool a_bReplace, bool a_bRightAligned,
		bool a_bMultiLine, bool a_bPassword, int a_iMaxHistoryLevel,
		bool a_bDrawLabel, int a_iDisabledAttribute,
		int a_iEnabledAttribute, int a_iFocusedAttribute )
								:	HControl ( a_poParent, a_iRow, a_iColumn, a_iHeight,
										a_iWidth, a_pcLabel, a_bDrawLabel, a_iDisabledAttribute,
										a_iEnabledAttribute, a_iFocusedAttribute ),
									HEditControl( a_poParent,
										a_iRow, a_iColumn, a_iWidth, a_iHeight,
										a_pcLabel, a_iBufferSize, a_pcValue,
										a_pcMask, a_bReplace, a_bRightAligned,
										a_bMultiLine, a_bPassword, a_iMaxHistoryLevel,
										a_bDrawLabel, a_iDisabledAttribute,
										a_iEnabledAttribute, a_iFocusedAttribute )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

int HDataEditControl::process_input ( int a_iCode )
	{
	M_PROLOG
	a_iCode = HEditControl::process_input ( a_iCode );
	return ( a_iCode );
	M_EPILOG
	}

