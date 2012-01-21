/*
---           yaal 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hdata.cxx - this file is integral part of yaal project.

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

#include <cstdio>
#include <cstdlib>

#include "hcore/base.hxx"
M_VCSID( "$Id: "__ID__" $" )
M_VCSID( "$Id: "__TID__" $" )
#include "hcore/hcore.hxx"
#include "hconsole/hconsole.hxx"
#include "tools/tools.hxx"
#include "dbwrapper/dbwrapper.hxx"
#include "hdata.hxx"

using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace yaal {

namespace hdata {

void banner( void ) {
	::printf( "\thdata\n" );
	return;
}

}

}

extern "C"
int yaal_hdata_main( int, char** );
extern "C"
int yaal_hdata_main( int, char** ) {
	static char const dynamicLinkerPath[]
#ifdef HAVE_INTERP_ALLOWED
		__attribute__(( __section__(".interp") ))
#endif /* #ifdef HAVE_INTERP_ALLOWED */
		= __DYNAMIC_LINKER__;
	if ( dynamicLinkerPath[ 0 ] ) {
		yaal::hcore::banner();
		yaal::tools::banner();
		yaal::dbwrapper::banner();
		yaal::hconsole::banner();
		yaal::hdata::banner();
		::exit( 0 );
	}
	return ( 0 );
}

