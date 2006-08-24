/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hserial.c - this file is integral part of `yaal' project.

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

#include <cstring>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <libintl.h>

#include "config.h"

#include "hcore/hexception.h"
M_VCSID ( "$Id$" )
#include "hserial.h"
#include "tools.h"

#include "hcore/hlog.h"

using namespace yaal::hcore;

namespace yaal
{

namespace tools
{

namespace
	{
	char const * const n_pcEAlreadyOpened = _ ( "serial port already openend" );
	char const * const n_pcENotOpened = _ ( "serial port not opened" );
	}

HSerial::flags_t HSerial::D_FLAGS_TEXT = HSerial::D_FLAGS_DEFAULT | HSerial::D_FLAGS_CANONICAL | HSerial::D_FLAGS_CR2NL;

HSerial::HSerial ( char const * const a_pcDevice )
				: HRawFile ( ), f_eSpeed ( D_SPEED_DEFAULT ),
	f_eFlags ( D_FLAGS_DEFAULT ), f_oDevicePath ( ),
	f_oTIO ( sizeof ( termios ), true ),
	f_oBackUpTIO ( sizeof ( termios ), true )
	{
	M_PROLOG
	if ( a_pcDevice )
		f_oDevicePath = a_pcDevice;
	else
		f_oDevicePath = tools::n_pcSerialDevice;
	compile ( );
	return;
	M_EPILOG
	}

HSerial::~HSerial ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor >= 0 )
		HSerial::close ( );
	M_ASSERT ( f_iFileDescriptor < 0 );
	return;
	M_EPILOG
	}

bool HSerial::open ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor >= 0 )
		M_THROW ( n_pcEAlreadyOpened, errno );
	compile( );
	/* O_NONBLOCK allow open device even if nothing seats on other side */
	f_iFileDescriptor = ::open ( f_oDevicePath, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if ( f_iFileDescriptor < 0 )
		M_THROW ( strerror ( errno ), errno );
	if ( ! isatty ( f_iFileDescriptor ) )
		M_THROW ( "not a tty", f_iFileDescriptor );
	tcgetattr ( f_iFileDescriptor, reinterpret_cast < termios * > ( f_oBackUpTIO.raw ( ) ) );
	fcntl ( f_iFileDescriptor, F_SETFD, 0 );
	fcntl ( f_iFileDescriptor, F_SETFL, 0 );
	tcflush ( f_iFileDescriptor, TCIOFLUSH );
	tcsetattr ( f_iFileDescriptor, TCSANOW,
			reinterpret_cast < termios const * const > ( static_cast < char const * const > ( f_oTIO ) ) );
	return ( false );
	M_EPILOG
	}

int HSerial::close ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor >= 0 )
		tcsetattr ( f_iFileDescriptor, TCSANOW,
				reinterpret_cast < termios const * const > ( static_cast < char const * const > ( f_oBackUpTIO ) ) );
	return ( HRawFile::close ( ) );
	M_EPILOG
	}

void HSerial::compile ( void )
	{
	M_PROLOG
	termios & l_sTIO = * reinterpret_cast < termios * > ( f_oTIO.raw ( ) );
	memset ( & l_sTIO, 0, sizeof ( termios ) );
	memset ( f_oBackUpTIO.raw ( ), 0, sizeof ( termios ) );
/*
 *   initialize all control characters
 *   default values can be found in /usr/include/termios.h, and are given
 *   in the comments, but we don't need them here
 */
	l_sTIO.c_cc [ VINTR ]    = 0;    /* Ctrl-c */
	l_sTIO.c_cc [ VQUIT ]    = 0;    /* Ctrl-\ */
	l_sTIO.c_cc [ VERASE ]   = 0;    /* del */
	l_sTIO.c_cc [ VKILL ]    = 0;    /* @ */
	l_sTIO.c_cc [ VEOF ]     = 4;    /* Ctrl-d */
	l_sTIO.c_cc [ VTIME ]    = 0;    /* inter-character timer unused */
	l_sTIO.c_cc [ VMIN ]     = 1;    /* blocking read until 1 character arrives */
#if HAVE_DECL_VSWTC
	l_sTIO.c_cc [ VSWTC ]    = 0;    /* '\0' */
#endif /* HAVE_DECL_VSWTC */
	l_sTIO.c_cc [ VSTART ]   = 0;    /* Ctrl-q */
	l_sTIO.c_cc [ VSTOP ]    = 0;    /* Ctrl-s */
	l_sTIO.c_cc [ VSUSP ]    = 0;    /* Ctrl-z */
	l_sTIO.c_cc [ VEOL ]     = 0;    /* '\0' */
	l_sTIO.c_cc [ VREPRINT ] = 0;    /* Ctrl-r */
	l_sTIO.c_cc [ VDISCARD ] = 0;    /* Ctrl-u */
	l_sTIO.c_cc [ VWERASE ]  = 0;    /* Ctrl-w */
	l_sTIO.c_cc [ VLNEXT ]   = 0;    /* Ctrl-v */
	l_sTIO.c_cc [ VEOL2 ]    = 0;    /* '\0' */
	compile_flags ( );
	compile_speed ( );
	return;
	M_EPILOG
	}

void HSerial::set_speed ( speed_t a_eSpeed )
	{
	M_PROLOG
	f_eSpeed = a_eSpeed;
	return;
	M_EPILOG
	}

void HSerial::compile_speed ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor >= 0 )
		M_THROW ( n_pcEAlreadyOpened, errno );
	termios & l_sTIO = * reinterpret_cast < termios * > ( f_oTIO.raw ( ) );
	int l_iBaudRate = 0;
	if ( f_eSpeed == D_SPEED_DEFAULT )
		f_eSpeed = tools::n_eBaudRate;
	switch ( f_eSpeed )
		{
		case ( D_SPEED_B230400 ): l_iBaudRate = B230400; break;
		case ( D_SPEED_B115200 ): l_iBaudRate = B115200; break;
#if ( HAVE_DECL_B76800 )
		case ( D_SPEED_B76800 ): l_iBaudRate = B76800; break;
#endif /* HAVE_DECL_B76800 */
		case ( D_SPEED_B57600 ): l_iBaudRate = B57600; break;
		case ( D_SPEED_B38400 ): l_iBaudRate = B38400; break;
#if ( HAVE_DECL_B28800 )
		case ( D_SPEED_B28800 ): l_iBaudRate = B28800; break;
#endif /* HAVE_DECL_B28800 */
		case ( D_SPEED_B19200 ): l_iBaudRate = B19200; break;
#if ( HAVE_DECL_B14400 )
		case ( D_SPEED_B14400 ): l_iBaudRate = B14400; break;
#endif /* HAVE_DECL_B14400 */
		case ( D_SPEED_B9600 ): l_iBaudRate = B9600; break;
#if ( HAVE_DECL_B7200 )
		case ( D_SPEED_B7200 ): l_iBaudRate = B7200; break;
#endif /* HAVE_DECL_B7200 */
		case ( D_SPEED_B4800 ): l_iBaudRate = B4800; break;
		case ( D_SPEED_B2400 ): l_iBaudRate = B2400; break;
		case ( D_SPEED_DEFAULT ): break;
		default :
			{
			M_THROW ( _ ( "unknown speed" ), f_eSpeed );
			break;
			}
		}
	cfsetispeed ( & l_sTIO, l_iBaudRate );
	cfsetospeed ( & l_sTIO, l_iBaudRate );
	return;
	M_EPILOG
	}

void HSerial::set_flags ( flags_t a_eFlags )
	{
	M_PROLOG
	f_eFlags = a_eFlags;
	return;
	M_EPILOG
	}

void HSerial::compile_flags ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor >= 0 )
		M_THROW ( n_pcEAlreadyOpened, errno );
	termios & l_sTIO = * reinterpret_cast < termios * > ( f_oTIO.raw ( ) );
	int l_iCtr = 0;
	if ( f_eFlags & D_FLAGS_DEFAULT )
		f_eFlags |= tools::n_eSerialFlags;
	/* consistency tests */
	if ( ( f_eFlags & D_FLAGS_STOP_BITS_1 ) && ( f_eFlags & D_FLAGS_STOP_BITS_2 ) )
		M_THROW ( _ ( "stop bits setup inconsistent" ), f_eFlags );
	if ( ( f_eFlags & D_FLAGS_FLOW_CONTROL_HARDWARE ) && ( f_eFlags & D_FLAGS_FLOW_CONTROL_SOFTWARE ) )
		M_THROW ( _ ( "flow control inconsistent" ), f_eFlags );
	if ( f_eFlags & D_FLAGS_BITS_PER_BYTE_8 )
		l_iCtr ++, l_sTIO.c_cflag = CS8;
	if ( f_eFlags & D_FLAGS_BITS_PER_BYTE_7 )
		l_iCtr ++, l_sTIO.c_cflag = CS7;
	if ( f_eFlags & D_FLAGS_BITS_PER_BYTE_6 )
		l_iCtr ++, l_sTIO.c_cflag = CS6;
	if ( f_eFlags & D_FLAGS_BITS_PER_BYTE_5 )
		l_iCtr ++, l_sTIO.c_cflag = CS5;
	if ( l_iCtr != 1 )
		M_THROW ( _ ( "bits per byte inconsistent" ), f_eFlags );

	/* compiling settings */
	/* setting c_cflag */
/*
 *   BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
 *   CRTSCTS : output hardware flow control (only used if the cable has
 *             all necessary lines. See sect. 7 of Serial-HOWTO)
 *             CS8     : 8n1 (8bit, no parity, 1 stopbit)
 *             CLOCAL  : local connection, no modem contol
 *             CREAD   : enable receiving characters
 */
/*
 *   statement above is *FALSE*, I can not use cfsetispeed and cfsetospeed,
 *   i *MUST* use it. On newer systes c_cflag and BAUDRATE simply does not work.
 *   Newwer interface for setting speed (baudrate)
 */
	l_sTIO.c_cflag |= CSIZE | CREAD /* | CLOCAL */;
	if ( f_eFlags & D_FLAGS_FLOW_CONTROL_HARDWARE )
		l_sTIO.c_cflag |= CRTSCTS;

	/* setting c_iflag */
/*
 *   IGNPAR  : ignore bytes with parity errors
 *   ICRNL   : map CR to NL (otherwise a CR input on the other computer
 *             will not terminate input)
 *             otherwise make device raw (no other input processing)
 *   IXANY   : (not in POSIX.1; XSI) Enable any character to restart output.
 *   IGNBRK  : Ignore BREAK condition on input.
 *   INPCK   : Enable input parity checking.
 */
	l_sTIO.c_iflag = IGNPAR | IGNBRK | IXANY;
	if ( f_eFlags & D_FLAGS_CR2NL )
		l_sTIO.c_iflag |= ICRNL;
	if ( f_eFlags & D_FLAGS_FLOW_CONTROL_SOFTWARE )
		l_sTIO.c_iflag |= IXON | IXOFF;
	if ( f_eFlags & D_FLAGS_PARITY_CHECK )
		l_sTIO.c_iflag |= INPCK;

	/* setting c_oflag */
/*
 *  Raw output.
 *  CSTOPB  : Set two stop bits, rather than one.
 *  PARENB  : Enable parity generation on output and parity checking for input.
 *  PARODD  : Parity for input and output is odd.
 */
	l_sTIO.c_oflag = 0;
	if ( f_eFlags & D_FLAGS_STOP_BITS_2 )
		l_sTIO.c_oflag |= CSTOPB;
	if ( f_eFlags & D_FLAGS_PARITY_CHECK )
		l_sTIO.c_oflag |= PARENB;
	if ( f_eFlags & D_FLAGS_PARITY_ODD )
		l_sTIO.c_oflag |= PARODD;

/*
 *   ICANON  : enable canonical input disable all echo functionality,
 *             and don't send signals to calling program
 *             more over: return from read() only after EOL
 *   IEXTEN  : Enable implementation-defined input processing.
 *             This flag, as well as ICANON must be enabled for the special
 *             characters EOL2,  LNEXT,  REPRINT,  WERASE to be interpreted,
 *             and for the IUCLC flag to be effective.
 *   ECHO    : Echo input characters.
 */
	l_sTIO.c_lflag = IEXTEN;
	if ( f_eFlags & D_FLAGS_CANONICAL )
		l_sTIO.c_lflag |= ICANON;
	if ( f_eFlags & D_FLAGS_ECHO )
		l_sTIO.c_lflag |= ECHO;
	return;
	M_EPILOG
	}

void HSerial::flush ( int a_iType )
	{
	M_PROLOG
	HString l_oErrMsg;
	if ( f_iFileDescriptor < 0 )
		M_THROW ( n_pcENotOpened, errno );
	if ( tcflush ( f_iFileDescriptor, a_iType ) )
		{
		switch ( a_iType )
			{
			case ( TCIFLUSH ):
				M_THROW ( "tcflush ( TCIFLUSH )", errno );
			case ( TCOFLUSH ):
				M_THROW ( "tcflush ( TCOFLUSH )", errno );
			case ( TCIOFLUSH ):
				M_THROW ( "tcflush ( TCIOFLUSH )", errno );
			default :
				{
				l_oErrMsg.format ( "tcflush ( %d )", a_iType );
				M_THROW ( l_oErrMsg, errno );
				}
			}
		}
	return;
	M_EPILOG
	}

void HSerial::wait_for_eot ( void )
	{
	M_PROLOG
	if ( f_iFileDescriptor < 0 )
		M_THROW ( n_pcENotOpened, errno );
	if ( tcdrain ( f_iFileDescriptor ) )
		M_THROW ( "tcdrain", errno );
	return;
	M_EPILOG
	}

int HSerial::timed_read ( void * const a_pcBuffer, int const a_iSize,
		int const a_iTimeOutSec, int const a_iTimeOutUsec )
	{
	M_PROLOG
	if ( f_iFileDescriptor < 0 )
		M_THROW ( n_pcENotOpened, errno );
	int l_iError = 0;
	timeval l_xWait;
	fd_set l_xFdSet;
	l_xWait.tv_sec = a_iTimeOutSec;
	l_xWait.tv_usec = a_iTimeOutUsec;
	FD_ZERO ( & l_xFdSet );
	FD_SET ( f_iFileDescriptor, & l_xFdSet );
	l_iError = select ( FD_SETSIZE, & l_xFdSet, NULL, NULL, & l_xWait );
	if ( ( l_iError > 0 ) && FD_ISSET ( f_iFileDescriptor, & l_xFdSet ) )
		return ( HRawFile::read ( a_pcBuffer, a_iSize ) );
	return ( - 1 );
	M_EPILOG
	}

}

}

