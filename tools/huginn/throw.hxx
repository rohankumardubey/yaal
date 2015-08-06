/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  throw.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_TOOLS_HUGINN_THROW_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_THROW_HXX_INCLUDED 1

#include "tools/hhuginn.hxx"
#include "tools/huginn/statement.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HThrow : public HStatement {
public:
	typedef HThrow this_type;
	typedef HStatement base_type;
private:
	HHuginn* _huginn;
	HHuginn::expression_t _expression;
	int _position;
public:
	HThrow( HHuginn*, HHuginn::expression_t const&, int );
protected:
	virtual void do_execute( HThread* ) const override;
private:
	HThrow( HThrow const& ) = delete;
	HThrow& operator = ( HThrow const& ) = delete;
};

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_THROW_HXX_INCLUDED */
