/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  objectfactory.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_TOOLS_HUGINN_OBJECTFACTORY_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_OBJECTFACTORY_HXX_INCLUDED 1

#include "tools/hhuginn.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HObjectFactory final {
	HHuginn::class_t _string;
	HHuginn::class_t _list;
	HHuginn::class_t _deque;
	HHuginn::class_t _dict;
	HHuginn::class_t _order;
	HHuginn::class_t _lookup;
	HHuginn::class_t _set;
	HHuginn::class_t _exception;
	HHuginn::class_t _conversionException;
	HHuginn::class_t _arithmeticException;
public:
	HObjectFactory( void );
	void register_exception_classes( HHuginn* );
	HHuginn::value_t create_string( yaal::hcore::HString const& = yaal::hcore::HString() ) const;
	HHuginn::value_t create_list( HHuginn::values_t const& = HHuginn::values_t() ) const;
	HHuginn::value_t create_deque( HHuginn::HDeque::values_t const& = HHuginn::HDeque::values_t() ) const;
	HHuginn::value_t create_dict( void ) const;
	HHuginn::value_t create_dict( HHuginn::HDict::values_t const&, HHuginn::type_t ) const;
	HHuginn::value_t create_order( void ) const;
	HHuginn::value_t create_order( HHuginn::HOrder::values_t const&, HHuginn::type_t ) const;
	HHuginn::value_t create_lookup( void ) const;
	HHuginn::value_t create_lookup( HHuginn::HLookup::values_t const& ) const;
	HHuginn::value_t create_set( void ) const;
	HHuginn::value_t create_set( HHuginn::HSet::values_t const& ) const;
	HHuginn::HClass const* exception_class( void ) const;
	HHuginn::HClass const* conversion_exception_class( void ) const;
	HHuginn::HClass const* arithmetic_exception_class( void ) const;
};

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_OBJECTFACTORY_HXX_INCLUDED */
