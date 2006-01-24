/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hdatalistcontrol.c - this file is integral part of `stdhapi' project.

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

#include "config.h"

#include "hcore/hexception.h"
M_CVSID ( "$CVSHeader$" )
#include "hdatalistcontrol.h"
#include "hdatawindow.h"

using namespace stdhapi::hcore;
using namespace stdhapi::hconsole;
using namespace stdhapi::dbwrapper;

namespace stdhapi
{

namespace hdata
{

HDataListControl::HDataListControl ( HRecordSet * a_poRecordSet,
		HDataWindow * a_poWindow, int a_iRow, int a_iColumn, int a_iHeight,
		int a_iWidth, char const * a_pcTitle,
		bool a_bCheckable, bool a_bSortable, bool a_bSearchable,
		bool a_bDrawHeader, bool a_bDrawLabel, int a_iDisabledAttribute,
		int a_iEnabledAttribute, int a_iFocusedAttribute )
								: HControl ( a_poWindow, a_iRow, a_iColumn, a_iHeight,
										a_iWidth, a_pcTitle, a_bDrawLabel, a_iDisabledAttribute,
										a_iEnabledAttribute, a_iFocusedAttribute ),
								HSearchableControl ( a_bSearchable ),
								HListControl ( NULL, 0, 0, 0, 0, NULL,
										a_bCheckable, a_bSortable, false, a_bDrawHeader ),
								HDataControl ( a_poRecordSet )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

HDataListControl::~HDataListControl ( void )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

void HDataListControl::load ( int long /*a_iId*/ )
	{
	M_PROLOG
	int l_iCount = 0, l_iCtr = 0, l_iQuantity = f_iQuantity;
	int l_iCursorPosition = f_iCursorPosition;
	int l_iControlOffset = f_iControlOffset;
	HElement * l_poSelected = f_poSelected;
	HElement * l_poFirstVisibleRow = f_poFirstVisibleRow;
	HItem l_oItem ( f_oHeader.quantity ( ) );
	HDataWindow * l_poParent = dynamic_cast < HDataWindow * > ( f_poParent );
	M_ASSERT ( l_poParent );
	l_poParent->set_sync_store ( & l_oItem );
	if ( f_poRecordSet->is_open ( ) )
		l_iCount = f_poRecordSet->requery ( );
	else
		l_iCount = f_poRecordSet->open ( );
	l_poParent->status_bar ( )->init_progress ( static_cast < double > ( l_iCount ), "Collecting ..." );
	if ( f_iQuantity )
		go ( 0 );
	while ( ! f_poRecordSet->is_eof ( ) )
		{
		l_poParent->status_bar ( )->update_progress ( );
		if ( l_iCtr ++ < l_iQuantity )
			{	
			f_poSelected->get_object ( ) = l_oItem;
			to_tail ( );
			}
		else
			add_tail ( & l_oItem );
		f_poRecordSet->move_next ( );
		}
	if ( l_poSelected )
		f_poSelected = l_poSelected;
	else
		f_poSelected = f_poHook;
	if ( l_poFirstVisibleRow )
		f_poFirstVisibleRow = l_poFirstVisibleRow;
	else
		f_poFirstVisibleRow = f_poHook;
	f_iCursorPosition = l_iCursorPosition;
	f_iControlOffset = l_iControlOffset;
	while ( l_iCtr ++ < l_iQuantity )remove_tail ( D_EMPTY_IF_NOT_EMPTIED );
	l_poParent->set_sync_store ( );
	return;
	M_EPILOG
	}

int long HDataListControl::get_current_id ( void )
	{
	M_PROLOG
	return ( static_cast < HItem * > ( & f_poSelected->get_object ( ) )->m_lId );
	M_EPILOG
	}

void HDataListControl::add_new ( void )
	{
	M_PROLOG
	add_tail ( );
	process_input ( KEY_CODES::D_HOME );
	process_input ( KEY_CODES::D_END );
	return;
	M_EPILOG
	}

void HDataListControl::cancel_new ( void )
	{
	M_PROLOG
	remove_tail ( D_EMPTY_IF_NOT_EMPTIED );
	if ( f_iQuantity )
		{
		process_input ( KEY_CODES::D_HOME );
		process_input ( KEY_CODES::D_END );
		}
	return;
	M_EPILOG
	}

}

}

