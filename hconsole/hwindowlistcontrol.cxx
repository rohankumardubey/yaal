/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hwindowlist.cxx - this file is integral part of `yaal' project.

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

#include "hcore/hexception.h"
M_VCSID ( "$Id$" )
#include "hwindowlistcontrol.h"

using namespace yaal::hcore;

namespace yaal
{

namespace hconsole
{

HWindowListControl::HWindowListControl ( HWindow * a_poParent, int a_iRow,
		int a_iColumn, int a_iHeight, int a_iWidth, char const * a_pcLabel,
		HWindowListControl::model_ptr_t a_oWindows )
									:	HControl ( a_poParent, a_iRow, a_iColumn, a_iHeight,
											a_iWidth, a_pcLabel ),
										HSearchableControl ( false ),
										HListControl_t<HWindow::ptr_t> ( a_poParent, a_iRow, a_iColumn, a_iHeight,
												a_iWidth, a_pcLabel, a_oWindows )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

HWindowListControl::~HWindowListControl ( void )
	{
	M_PROLOG
/*	HWindowListControl::remove_tail ( D_FORCE_REMOVE_ELEMENT ); */
	return;
	M_EPILOG
	}

int HWindowListControl::process_input ( int a_iCode )
	{
	M_PROLOG
	a_iCode = HListControl_t<HWindow::ptr_t>::process_input ( a_iCode );
	if ( ( f_oList->size() > 1 ) && ( ( a_iCode == '\r' ) || ( a_iCode == ' ' ) ) )
		{
		f_oList->go ( f_iControlOffset + f_iCursorPosition - 1 );
		a_iCode = KEY < '\t' >::meta;
		}
	return ( a_iCode );
	M_EPILOG
	}

void HWindowListControl::do_refresh ( void )
	{
	M_PROLOG
	HListControl_t<HWindow::ptr_t>::do_refresh ( );
	M_EPILOG
	}

/*
void HWindowListControl::add_tail ( HItem * a_poElement )
	{
	M_PROLOG
	f_oList->push_back ( a_poElement );
	M_EPILOG
	}
*/

namespace list_control_helper
{

/*
template<>
yaal::hcore::HString const GetLongFromCell( HInfo const& a_oInfo )
	{
	return ( HString ( a_oInfo.get<int long>() ) );
	}

template<>
yaal::hcore::HString const GetDoubleFromCell( HInfo const& a_oInfo )
	{
	return ( HString ( a_oInfo.get<double>() ) );
	}
*/

template<>
yaal::hcore::HString const GetStringFromCell( HWindow::ptr_t const& a_oWindow )
	{
	return ( a_oWindow->get_title() );
	}

/*
template<>
char const * GetTimeFromCell( HInfo const & a_oInfo )
	{
	return ( a_oInfo.get<yaal::hcore::HTime const &>() );
	}

int long GetIdFromCell( HItem const & )
	{
	return ( 0 );
	}

bool GetStateFromCell( HItem const & )
	{
	return ( false );
	}

template<>
bool compare_cells( HInfo const & a_oLeft, HInfo const & a_oRight, OSortHelper & a_roSortHelper )
	{
	double l_dDifference = 0;
	a_roSortHelper.progress();
	switch ( a_roSortHelper.f_eType )
		{
		case ( D_LONG_INT ):
			return ( a_oLeft.get<long>() > a_oRight.get<long>() );
		case ( D_DOUBLE ):
			l_dDifference = a_oLeft.get<double>() > a_oRight.get<double>();
		break;
		case ( D_HSTRING ):
			return ( strcasecmp ( a_oLeft.get<HString const &>(),
					 a_oRight.get<HString const &>() ) > 0 );
		case ( D_HTIME ):
			l_dDifference = static_cast<time_t>( a_oLeft.get<HTime const &>() ) > static_cast<time_t>( a_oRight.get<HTime const &>() );
		break;
		default:
			break;
		}
	return ( l_dDifference > 0 ? 1 : ( l_dDifference < 0 ? - 1 : 0 ) );
	}
*/

}

}

}

