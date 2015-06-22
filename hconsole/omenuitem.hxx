/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

  omenuitem.hxx - this file is integral part of `yaal' project.

  i.  You may not make any changes in Copyright information.
  ii. You must attach Copyright information to any part of every copy
      of this software.

Copyright:

 You can use this software free of charge and you can redistribute its binary
 package freely but:
  1. You are not allowed to use any part of sources of this software.
  2. You are not allowed to redistribute any part of sources of this software.
  3. You are not allowed to reverse engineer this software.
  4. If you want to distribute a binary package of this software you cannot
     demand any fees for it. You cannot even demand
     a return of cost of the media or distribution (CD for example).
  5. You cannot involve this software in any commercial activity (for example
     as a free add-on to paid software or newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#ifndef YAAL_HCONSOLE_OMENUITEM_HXX_INCLUDED
#define YAAL_HCONSOLE_OMENUITEM_HXX_INCLUDED 1

#include "hcore/hboundcall.hxx"
#include "hcore/hstring.hxx"
#include "hcore/htime.hxx"

namespace yaal {

namespace hconsole {

class HTUIProcess;

/*! \brief TUI menu node.
 */
struct OMenuItem {
public: /* All is public for simpler usage. */
	typedef yaal::hcore::HBoundCall<> call_t;
	call_t _call;
	yaal::hcore::HString _label;
/* Methods */
	OMenuItem( void );
	OMenuItem( OMenuItem const& );
	OMenuItem& operator = ( OMenuItem const& );
	void reset( void );
	void call( void );
	void swap( OMenuItem& );
	yaal::hcore::HString const& get_string( void ) const;
	int long get_integer( void ) const M_DEBUG_CODE( __attribute__((__noreturn__)) );
	double get_real( void ) const M_DEBUG_CODE( __attribute__((__noreturn__)) );
	yaal::hcore::HTime const& get_time( void ) const M_DEBUG_CODE( __attribute__((__noreturn__)) );
};

inline void swap( yaal::hconsole::OMenuItem& a, yaal::hconsole::OMenuItem& b )
	{ a.swap( b ); }

}

}

#endif /* #ifndef YAAL_HCONSOLE_OMENUITEM_HXX_INCLUDED */

