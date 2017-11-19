/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  time.hxx - this file is integral part of `yaal' project.

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

/* YAAL_PRIVATE_IMPLEMENTATION_DETAIL */

#ifndef YAAL_TOOLS_HUGINN_TIME_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_TIME_HXX_INCLUDED 1

#include "tools/hhuginn.hxx"
#include "hcore/htime.hxx"

namespace yaal {

namespace tools {

namespace huginn {

typedef yaal::hcore::HTime& ( yaal::hcore::HTime::*time_mod_t )( int );
typedef yaal::hcore::HTime& ( yaal::hcore::HTime::*time_set_t )( int, int, int );
typedef int ( yaal::hcore::HTime::*time_get_t )( void ) const;

class HTime : public HHuginn::HValue {
	yaal::hcore::HTime _time;
public:
	HTime( HHuginn::HClass const*, yaal::hcore::HTime const& );
	static HHuginn::value_t mod( char const*, time_mod_t, huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t get( char const*, time_get_t, huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t set( char const*, time_set_t, huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t set_datetime( huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t get_month( huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t subtract( huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t from_string( huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::value_t to_string( huginn::HThread*, HHuginn::value_t*, HHuginn::values_t&, int );
	static HHuginn::class_t get_class( HRuntime* );
private:
	virtual HHuginn::value_t do_clone( huginn::HThread*, int ) const override;
};

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_TIME_HXX_INCLUDED */

