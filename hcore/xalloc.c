/*
---          `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	xalloc.c - this file is integral part of `stdhapi' project.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hexception.h"
M_CVSID ( "$CVSHeader$" );

extern "C"
{

void * xmalloc ( size_t a_ulSize )
	{
	register void * l_pvNewPtr = malloc ( a_ulSize );
	if ( l_pvNewPtr == 0 )
		{
		perror ( "xmalloc" );
		abort ( );
		}
	return ( l_pvNewPtr );
	}

void * xcalloc ( size_t a_ulSize )
	{
	register void * l_pvNewPtr = xmalloc ( a_ulSize );
	memset ( l_pvNewPtr, 0, a_ulSize );
	return ( l_pvNewPtr );
	}

void * xrealloc ( void * a_pvPtr, size_t a_ulSize )
	{
	register void * l_pvNewPtr = realloc ( a_pvPtr, a_ulSize );
	if ( l_pvNewPtr == 0 )
		{
		perror ( "xrealloc" );
		abort ( );
		}
	return ( l_pvNewPtr );
	}

void xfree ( void * & a_rpvPtr )
	{
	if ( a_rpvPtr == 0 )
		{
		perror ( "xfree" );
		abort ( );
		}
	free ( a_rpvPtr );
	a_rpvPtr = 0;
	return;
	}

char * xstrdup ( const char * a_pcStr )
	{
	char * l_pcNew = 0;
	l_pcNew = ( char * ) xcalloc ( strlen ( a_pcStr ) + 1 );
	strcpy ( l_pcNew, a_pcStr );
	return ( l_pcNew );
	}

}

