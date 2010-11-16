/*
---           yaal 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hring.cxx - this file is integral part of yaal project.

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

#include <libintl.h>

#include "hcore/base.hxx"
M_VCSID( "$Id: "__ID__" $" )
#include "hring.hxx"

namespace yaal
{

namespace tools
{

char const* const _errMsgHRing_[ 5 ] =
	{
	_( "ok" ),
/* HRing::ERROR::BAD_SIZE */         _( "bad size requested" ),
/* HRing::ERROR::BAD_INDEX */        _( "index does not fit into array" ),
/* HRing::ERROR::INVALID_ITERATOR */ _( "invalid iterator used for operation" ),
/* HRing::ERROR::RING_IS_FULL */     _( "ring is already full" )
	};

}

}

