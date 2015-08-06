/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  clock.cxx - this file is integral part of `yaal' project.

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

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "clock.hxx"
#include "helper.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

HClock::HClock( HHuginn::HClass* class_ )
	: HObject( class_ )
	, _clock() {
	return;
}

HHuginn::value_t HClock::miliseconds( huginn::HThread*, HHuginn::HObject* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Clock.miliseconds";
	verify_arg_count( name, values_, 0, 0, position_ );
	huginn::HClock* c( static_cast<huginn::HClock*>( object_ ) );
	return ( make_pointer<HHuginn::HInteger>( c->_clock.get_time_elapsed( time::UNIT::MILISECOND ) ) );
	M_EPILOG
}

HHuginn::value_t HClock::reset( huginn::HThread*, HHuginn::HObject* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Clock.reset";
	verify_arg_count( name, values_, 0, 0, position_ );
	huginn::HClock* c( static_cast<huginn::HClock*>( object_ ) );
	c->_clock.reset();
	return ( object_->get_pointer() );
	M_EPILOG
}

HHuginn::class_t HClock::get_class( HHuginn* huginn_ ) {
	M_PROLOG
	HHuginn::class_t c(
		make_pointer<HHuginn::HClass>(
			huginn_,
			HHuginn::HType::register_type( "Clock", huginn_ ),
			nullptr,
			HHuginn::HClass::field_names_t{
				"miliseconds",
				"reset"
			},
			HHuginn::values_t{
				make_pointer<HHuginn::HClass::HMethod>( hcore::call( &HClock::miliseconds, _1, _2, _3, _4 ) ),
				make_pointer<HHuginn::HClass::HMethod>( hcore::call( &HClock::reset, _1, _2, _3, _4 ) )
			}
		)
	);
	return ( c );
	M_EPILOG
}

}

}

}
