/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hitem.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_HCONSOLE_HITEM_HXX_INCLUDED
#define YAAL_HCONSOLE_HITEM_HXX_INCLUDED 1

#include "hcore/harray.hxx"
#include "hcore/hinfo.hxx"

namespace yaal {

namespace hconsole {

/*! \brief Basic data unit stored in HListControl and HTreeControl.
 */
template <typename tType = yaal::hcore::HInfo>
class HItem_t : public hcore::HArray<tType> {
public:
	int long	m_lId;
	bool	m_bChecked;
	HItem_t ( int = 0 );
	virtual ~HItem_t ( void );
	HItem_t ( HItem_t const &, int );
	HItem_t & operator = ( HItem_t const & );
};

template <typename tType>
HItem_t<tType>::HItem_t ( int size_ ) : hcore::HArray<tType> ( size_ ),
															 m_lId ( 0 ), m_bChecked ( false ) {
	return;
}

template <typename tType>
HItem_t<tType>::~HItem_t ( void ) {
	return;
}

template <typename tType>
HItem_t<tType>::HItem_t ( HItem_t const & item_, int ) : hcore::HArray<tType> ( item_ ),
	m_lId ( item_.m_lId ), m_bChecked ( item_.m_bChecked ) {
	return;
}

template <typename tType>
HItem_t<tType> & HItem_t<tType>::operator = ( HItem_t<tType> const & item_ ) {
	M_PROLOG
	if ( this != & item_ ) {
		(*this).hcore::HArray<tType>::operator = ( item_ );
		m_lId = item_.m_lId;
		m_bChecked = item_.m_bChecked;
	}
	return ( *this );
	M_EPILOG
}

typedef HItem_t<> HItem;

}

}

#endif /* #ifndef YAAL_HCONSOLE_HITEM_HXX_INCLUDED */

