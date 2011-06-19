/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	mouse.cxx - this file is integral part of `yaal' project.

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

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <libintl.h>

#include "config.hxx"

#ifdef HAVE_SYS_CONSIO_H
#	include <sys/consio.h>
#	include <csignal>
#	include <fcntl.h>
#elif defined ( HAVE_GPM_H )
#	include <gpm.h>
#endif /* HAVE_GPM_H */

#include "hcore/base.hxx"
M_VCSID( "$Id: "__ID__" $" )
M_VCSID( "$Id: "__TID__" $" )
#include "mouse.hxx"
#include "console.hxx"
#include "hcore/hstring.hxx"
#include "hcore/hlog.hxx"
#include "hcore/system.hxx"

#ifdef HAVE_CURSES_H
#	include <curses.h>
#elif defined ( HAVE_NCURSES_CURSES_H )
#	include <ncurses/curses.h>
#else /* HAVE_NCURSES_CURSES_H */
#	error "No ncurses header available."
#endif /* not HAVE_NCURSES_CURSES_H */

using namespace yaal::hcore;

namespace yaal
{

namespace hconsole
{

namespace mouse
{

fun_console_mouse_open_t mouse_open = NULL;
fun_console_mouse_get_t mouse_get = NULL;
fun_console_mouse_close_t mouse_close = NULL;

int hunt_tty ( int offset_ )
	{
	M_PROLOG
	/* this hack allows to guess current controling virtual terminal screen */
	int vC = 0;
	char const* ttyName = NULL;
	char const* ptr = NULL;
	ttyName = ttyname( STDIN_FILENO );
	if ( ttyName && ! ::strncmp( ttyName, "/dev/ttyv", 8 + offset_ ) )
		vC = lexical_cast<int>( ttyName + 8 + offset_ );
	else
		{
		ttyName = ::getenv( "STY" );
		if ( ttyName )
			{
			if ( ( ptr = ::strstr( ttyName, ".tty" ) ) )
				vC = lexical_cast<int>( ptr + 4 + offset_ );
			else if ( ( ptr = ::strstr( ttyName, ".pts" ) ) )
				vC = lexical_cast<int>( ptr + 4 + offset_ );
			}
		else
			M_THROW( "cannot find controling virtual console", errno );
		}
	return ( vC );
	M_EPILOG
	}

#ifdef HAVE_SYS_CONSIO_H

int _mouse_ = 0;

int console_mouse_open( void )
	{
	M_PROLOG
	int vC = 0;
	char tty[] = "/dev/ttyv0";
	HString error;
	mouse_info mouse;
	mouse.operation = MOUSE_MODE;
	mouse.u.mode.mode = 0;
	mouse.u.mode.signal = SIGUSR1;
	vC = hunt_tty( 1 );
	tty[ 9 ] = '0' + vC;
	_mouse_ = ::open( tty, O_RDWR );
	if ( _mouse_ < 0 )
		{
		error.format( _( "cannot open mouse, %s" ), error_message( errno ) );
		M_THROW( error, vC );
		}
	if ( ::ioctl( _mouse_, CONS_MOUSECTL, &mouse ) < 0 )
		{
		error.format( _( "cannot setup mouse mode, %s" ), error_message( errno ) );
		TEMP_FAILURE_RETRY( hcore::system::close( _mouse_ ) );
		M_THROW( error, errno );
		}

	log( LOG_TYPE::INFO ) << "i have opened device: `" << tty << '\'' << endl;

	return ( 0 );
	M_EPILOG
	}

int console_mouse_get( OMouse& mouse_ )
	{
	M_PROLOG
	mouse_info mouse;
	mouse.operation = MOUSE_GETINFO;
	if ( ::ioctl( _mouse_, CONS_MOUSECTL, &mouse ) < 0 )
		M_THROW( "cannot get mouse data", errno );
	else
		{
		mouse_._buttons = mouse.u.data.buttons;
		mouse_._row = mouse.u.data.y / 16;
		mouse_._column = mouse.u.data.x / 8;
		}
	return ( 0 );
	M_EPILOG
	}

int console_mouse_close( void )
	{
	M_PROLOG
	if ( ! _mouse_ )
		M_THROW( "mouse not opened", errno );
	TEMP_FAILURE_RETRY( hcore::system::close( _mouse_ ) );
	_mouse_ = 0;
	return ( 0 );
	M_EPILOG
	}

#elif defined ( HAVE_GPM_H )

int console_mouse_open( void )
	{
	M_PROLOG
	int vC = 0;
	HString error;
	Gpm_Connect gpm;
	gpm.minMod = 0;
	gpm.maxMod = 0;
	gpm.pid = hcore::system::getpid();
	gpm.vc = vC = hunt_tty( 0 );
	gpm.eventMask = static_cast<int>( GPM_UP );
	gpm.defaultMask = static_cast<int short unsigned>( ~gpm.eventMask );
	gpm_zerobased = true;
	if ( Gpm_Open( &gpm, vC ) == -1 )
		{
		error.format( "Can't open mouse connection: %s", error_message( errno ) );
		M_THROW( error, vC );
		}
	log( LOG_TYPE::INFO ) << "i have opened device: `" << vC << '\'' << endl;
	return ( gpm_fd );
	M_EPILOG
	}

int console_mouse_get( OMouse& mouse_ )
	{
	M_PROLOG
	Gpm_Event event;
	if ( Gpm_GetEvent( &event ) != 1 )
		M_THROW( _( "cannot retrieve event") , errno );
	mouse_._buttons = event.buttons;
	mouse_._row = event.y;
	mouse_._column = event.x;
	return ( 0 );
	M_EPILOG
	}

int console_mouse_close( void )
	{
	M_PROLOG
	while ( Gpm_Close() )
		;
	return ( 0 );
	M_EPILOG
	}

#else /* HAVE_GPM_H */

int console_mouse_open( void )
	{
	M_PROLOG
	int a( 1 );
	if ( a )
		M_THROW( _( "console mouse support not compiled" ), errno );
	return ( 0 );
	M_EPILOG
	}

int console_mouse_get( OMouse& )
	{
	M_PROLOG
	int a( 1 );
	if ( a )
		M_THROW( _( "console mouse support not compiled" ), errno );
	return ( 0 );
	M_EPILOG
	}

int console_mouse_close( void )
	{
	M_PROLOG
/*	I cannot throw exception here bacause exception was probably
 *	thrown by console_mouse_open in enter_curses and now this
 *	function is probably called from leave_curses from exception
 *	catch block, so new exception would be recursive and would
 *	overload the stack because of infinite number of exceptions */
	return ( 0 );
	M_EPILOG
	}

#endif /* ! HAVE_GPM_H */

int x_mouse_open( void )
	{
	M_PROLOG
	mmask_t mouseMask, desiredMouseMask;
	desiredMouseMask = BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED;
	mouseMask = mousemask( desiredMouseMask, NULL );
	if ( ! mouseMask )
		M_THROW( "mousemask() returned 0", errno );
	else if ( ( mouseMask & desiredMouseMask ) < desiredMouseMask )
		{
		HString error;
		error.format( "could not set up apropriate mask: 1 = %d, 2 = %d, 3 = %d",
				mouseMask & BUTTON1_CLICKED, mouseMask & BUTTON2_CLICKED,
				mouseMask & BUTTON3_CLICKED );
		throw ( HMouseException( error ) );
		}
	return ( 0 );
	M_EPILOG
	}

int x_mouse_get( OMouse& mouse_ )
	{
	M_PROLOG
	MEVENT mouse;
	if ( getmouse( &mouse ) != OK )
		M_THROW( "cannot get mouse data", errno );
	else
		{
		mouse_._buttons = static_cast<int>( mouse.bstate );
		mouse_._row = mouse.y;
		mouse_._column = mouse.x;
		}
	return ( 0 );
	M_EPILOG
	}

int x_mouse_close( void )
	{
	return ( 0 );
	}

}

}

}

