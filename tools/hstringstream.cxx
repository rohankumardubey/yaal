/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hstringstream.cxx - this file is integral part of `yaal' project.

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
#include "hstringstream.h"

namespace yaal
{

namespace tools
{

HStringStream::HStringStream( void ) : f_oBuffer()
	{
	}

char const* const HStringStream::raw( void ) const
	{
	return ( f_oBuffer.raw() );
	}

int HStringStream::do_write( void const* const a_pvBuffer, int const a_iSize )
	{
	M_PROLOG
	f_oBuffer.insert( f_oBuffer.get_length(), a_iSize, static_cast< char const* const>( a_pvBuffer ) );
	return ( a_iSize );
	M_EPILOG
	}

void HStringStream::do_flush( void ) const
	{
	}

int HStringStream::do_read( void* const a_pvBuffer, int const a_iSize )
	{
	M_PROLOG
	int l_iLength = f_oBuffer.get_length();
	l_iLength = ( l_iLength < a_iSize ? l_iLength : a_iSize );
	strncpy( static_cast<char* const>( a_pvBuffer ), f_oBuffer, l_iLength );
	return ( l_iLength );
	M_EPILOG
	}

char const* const operator << ( yaal::hcore::HStreamInterface const&, HStringStream const& stream )
	{
	return ( stream.raw() );
	}

}

}

