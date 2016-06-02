/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

  hcollector.cxx - this file is integral part of `yaal' project.

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

#include <cstring>
#include <cstdlib>
#include <termios.h>
#include <sys/types.h> /* FD_* macros */
#include <libintl.h>

#undef ECHO

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "hcollector.hxx"
#include "tools.hxx"
#include "hcore/hlog.hxx"

using namespace yaal::hcore;

namespace yaal {

namespace tools {

namespace {

namespace PROTOCOL {

char const SYN[] = "SYN\n";
char const ACK[] = "ACK\n";
char const DTA[] = "DTA"; /* warrning! no endline */
char const FIN[] = "FIN"; /* warrning! no endline, but \0 at end, so sizeof() retruns 4 */
char const ERR[] = "ERR\n";

}

}

char const* const _error_ = _( "collector device not opened" );

HCollector::HCollector( char const* devicePath_ )
					: HSerial( devicePath_ ), _lines( 0 ),
						_line() {
	M_PROLOG
	memset( _readBuf, 0, RECV_BUF_SIZE );
	/*
	 * Actual _readBuf buffer size is equal to RECV_BUF_SIZE + 1.
	 * So we have additional one byte for string terminator (0).
	 */
	_readBuf[ RECV_BUF_SIZE ] = 0;
	set_flags( HSerial::FLAG_TEXT );
	return;
	M_EPILOG
}

bool HCollector::test_char( char const* buffer_, int index_ ) const {
	return (
			buffer_ [ index_ ]
							&& ( ( ( buffer_ [ index_ ] >= '0' )
									&& ( buffer_ [ index_ ] <= '9' ) )
								|| ( ( buffer_ [ index_ ] >= 'A' )
									&& ( buffer_ [ index_ ] <= 'Z' ) )
								|| ( ( ( buffer_ [ index_ ] == 10 )
										|| ( buffer_ [ index_ ] == 13 )
										|| ( buffer_ [ index_ ] == '?' ) )
									&& index_ ) )
							);
}

int HCollector::send_line( char const* line_ ) {
	M_PROLOG
	int long ctr = 0;
	int cRC = 0;
	int error = -1;
	int length = static_cast<int>( ::strlen( line_ ) );
	HString line, localCopy;
	if ( length < 1 )
		return ( 0 );
	localCopy = line_;
	if ( line_[ length - 1 ] == '\n' ) {
		length --;
		localCopy.set_at( length, 0 );
	}
	for ( ctr = 0; ctr < length; ctr ++ )
		cRC += localCopy[ ctr ];
	line.format ( "%s%02x%02x%s\n", PROTOCOL::DTA,
			length & 0x0ff, cRC & 0x0ff, localCopy.raw() );
	::memset( _readBuf, 0, RECV_BUF_SIZE );
	length += static_cast<int>( ::strlen( PROTOCOL::DTA ) );
	length += ( 2 /* for length */ + 2 /* for crc */ + 1 /* for newline */ );
	while ( ::strncmp( _readBuf, PROTOCOL::ACK, ::strlen( PROTOCOL::ACK ) ) ) {
		flush( TCOFLUSH );
		ctr = HRawFile::write( line.raw(), length );
		wait_for_eot();
		::memset( _readBuf, 0, RECV_BUF_SIZE );
		HRawFile::read( _readBuf, RECV_BUF_SIZE );
		flush( TCIFLUSH );
		error ++;
	}
	return ( static_cast<int>( error + length - ctr ) );
	M_EPILOG
}

int HCollector::receive_line( HString& line_ ) {
	M_PROLOG
	int long error = -1;
	int ctr = 0;
	int cRC = 0, pCRC = -1;
	int long length = 0, pLength = -1;
	int ackLenght = static_cast<int>( ::strlen( PROTOCOL::ACK ) );
	int errLenght = static_cast<int>( ::strlen( PROTOCOL::ERR ) );
	/* P prefix means sender transmission side data */
	while ( ( pCRC != cRC ) || ( pLength != length ) ) {
		_line = "";
		_readBuf[ 0 ] = 0;
		while ( strlen( _readBuf ) < static_cast<size_t>( RECV_BUF_SIZE ) ) {
			::memset( _readBuf, 0, RECV_BUF_SIZE );
			HRawFile::read( _readBuf, RECV_BUF_SIZE );
			_line += _readBuf;
		}
		flush( TCIFLUSH );
		line_ = _line;
		line_.shift_left(	static_cast<int long>( ::strlen( PROTOCOL::DTA ) ) + 2 /* for length */ + 2 /* for crc */ );
		length = line_.get_length() - 1;
		for ( ctr = 0; ctr < length; ctr ++ )
			cRC += line_[ ctr ];
		length &= 0x0ff;
		cRC &= 0x0ff;
		::memset( _readBuf, 0, RECV_BUF_SIZE );
		::strncpy( _readBuf, _line.raw() + ::strlen( PROTOCOL::DTA ), 2 );
		pLength = strtol ( _readBuf, nullptr, 0x10 );
		::memset( _readBuf, 0, RECV_BUF_SIZE );
		::strncpy( _readBuf, _line.raw()
				+ ::strlen( PROTOCOL::DTA ) + 2 /* for Plength */, 2 );
		pCRC = lexical_cast<int, char const*>( _readBuf );
		if ( ( pCRC != cRC ) || ( pLength != length ) )
			error += ( errLenght - HRawFile::write( PROTOCOL::ERR, errLenght ) );
		error ++;
	}
	flush( TCOFLUSH );
	error += ( ackLenght - HRawFile::write( PROTOCOL::ACK, ackLenght ) );
	wait_for_eot();
	_lines ++;
	return ( static_cast<int>( error ) );
	M_EPILOG
}

int HCollector::establish_connection ( int timeOut_ ) {
	M_PROLOG
/*
	 We have small problem here.
	 There are two ways that communication can start.
	 Either waiting part runs before initializing part (the easy case),
	 or initializing part runs before waiting part (here comes the trouble).
*/
	int length = static_cast<int>( ::strlen( PROTOCOL::SYN ) ), error = -1;
	if ( _fileDescriptor < 0 )
		M_THROW( _error_, _fileDescriptor );
	::memset( _readBuf, 0, RECV_BUF_SIZE );
	while ( ::strncmp( _readBuf, PROTOCOL::ACK, ::strlen( PROTOCOL::ACK ) ) ) {
		flush( TCOFLUSH );
		if ( HRawFile::write( PROTOCOL::SYN, length ) != length )
			M_THROW( "write", length );
		wait_for_eot();
		if ( tcsendbreak( _fileDescriptor, 0 ) )
			M_THROW( "tcsendbreak", errno );
		memset( _readBuf, 0, RECV_BUF_SIZE );
		timed_read( _readBuf, RECV_BUF_SIZE, 1 );
		flush( TCIFLUSH );
		error ++;
		if ( error > timeOut_ )
			return ( -1 );
	}
	log( LOG_LEVEL::DEBUG ) << "Collector: Connected ! (estab)" << endl;
	return ( error );
	M_EPILOG
}

int HCollector::wait_for_connection ( int timeOut_ ) {
	M_PROLOG
	int error = - 1;
	int length = static_cast<int>( ::strlen( PROTOCOL::ACK ) );
	if ( _fileDescriptor < 0 )
		M_THROW ( _error_, _fileDescriptor );
	::memset( _readBuf, 0, RECV_BUF_SIZE );
	while ( ::strncmp( _readBuf, PROTOCOL::SYN, strlen ( PROTOCOL::SYN ) ) ) {
		if ( timed_read( _readBuf, RECV_BUF_SIZE, timeOut_ ) >= 0 )
			error ++;
		else
			return ( -1 );
	}
	error += static_cast<int>( length - HRawFile::write( PROTOCOL::ACK, length ) );
	log( LOG_LEVEL::DEBUG ) << "Collector: Connected ! (wait)" << endl;
	return ( error );
	M_EPILOG
}

int HCollector::read_collector ( void ( *process_line )( char const*, int ) ) {
	M_PROLOG
	int error = 0;
	_lines = 0;
	error = wait_for_connection( tools::_collectorConnectionTimeOut_ );
	HString line;
	while ( error >= 0 ) {
		error += receive_line( line );
		/* '\n' is stripped from each line so we need to FIN treat special */
		if ( ! ::strncmp( line.raw(), PROTOCOL::FIN, sizeof ( PROTOCOL::FIN ) ) ) {
			break;
		}
		process_line( line.raw(), _lines );
	}
	return ( error );
	M_EPILOG
}

}

}

