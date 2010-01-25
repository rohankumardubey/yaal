/*
---           yaal 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	base64.cxx - this file is integral part of yaal project.

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

#include <cctype>

#include "hcore/base.hxx"
M_VCSID( "$Id: "__ID__" $" )
#include "base64.hxx"
#include "hcore/hformat.hxx"

using namespace yaal::hcore;

namespace yaal
{

namespace tools
{

char const n_pcBase64EncodeTable[][65] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" } ;
u8_t n_pcBase64DecodeTable[2][ 256 ];

class HBase64DecodeTableInitializer
	{
public:
	HBase64DecodeTableInitializer( void )
		{
		::memset( n_pcBase64DecodeTable[0], 0, sizeof ( n_pcBase64DecodeTable[0] ) );
		::memset( n_pcBase64DecodeTable[1], 0, sizeof ( n_pcBase64DecodeTable[1] ) );
		for ( size_t m = 0; m < 2; ++ m )
			{
			for ( size_t i = 0; i < sizeof ( n_pcBase64EncodeTable[m] ); ++ i )
				n_pcBase64DecodeTable[m][ static_cast<int>( n_pcBase64EncodeTable[m][ i ] ) ] = static_cast<u8_t>( i );
			}
		}
	} base64DecodeTableInit;

void do_buf_3_to_4( HString& out, u32_t const& in, int mode, int long pad = 0 )
	{
	char buf[ 5 ] = { 0, 0, 0, 0, 0 };
	buf[ 0 ] = n_pcBase64EncodeTable[mode][ ( in >> 18 ) & 63 ];
	buf[ 1 ] = n_pcBase64EncodeTable[mode][ ( in >> 12 ) & 63 ];
	buf[ 2 ] = n_pcBase64EncodeTable[mode][ ( in >> 6 ) & 63 ];
	buf[ 3 ] = n_pcBase64EncodeTable[mode][ in & 63 ];
	if ( pad )
		::memset( ( ( buf + sizeof ( buf ) ) - pad ) - 1, '=', pad );
	out += buf;
	return;
	}

void do_buf_4_to_3( char* out, u32_t const& in, int size = 3 )
	{
	out[ 0 ] = static_cast<char>( ( in >> 16 ) & 255 );
	if ( size > 1 )
		out[ 1 ] = static_cast<char>( ( in >> 8 ) & 255 );
	if ( size > 2 )
		out[ 2 ] = static_cast<char>( in & 255 );
	return;
	}

yaal::hcore::HString base64_raw_encode( char const* ptr, int long length, bool standardCompliantMode )
	{
	M_PROLOG
	int shifts[] = { 16, 8, 0 };
	static int const PREALLOCATE = 128;
	HString output( PREALLOCATE, true );
	u32_t coder = 0;
	for ( int long i = 0; i < length; ++ i )
		{
		int shift = shifts[ i % 3 ];
		coder |= ( static_cast<u8_t>( ptr[ i ] ) << shift );
		if ( ! shift )
			{
			do_buf_3_to_4( output, coder, standardCompliantMode ? 1 : 0 );
			coder = 0;
			}
		}
	if ( length % 3 )
		do_buf_3_to_4( output, coder, standardCompliantMode ? 1 : 0, 3 - ( length % 3 ) );
	return ( output );
	M_EPILOG
	}

yaal::hcore::HString base64::encode( yaal::hcore::HString const& message, bool standardCompliantMode )
	{
	M_PROLOG
	return ( base64_raw_encode( message.raw(), message.get_length(), standardCompliantMode ) );
	M_EPILOG
	}

bool is_base64_character( char ch_, bool standardCompliantMode_ )
	{
	return ( ( ( ch_ >= 'A' ) && ( ch_ <= 'Z' ) )
				|| ( ( ch_ >= 'a' ) && ( ch_ <= 'z' ) )
				|| ( ( ch_ >= '0' ) && ( ch_ <= '9' ) )
				|| ( ! standardCompliantMode_ && ( ch_ == '-' ) ) || ( standardCompliantMode_ && ( ch_ == '+' ) )
				|| ( ! standardCompliantMode_ && ( ch_ == '_' ) ) || ( standardCompliantMode_ && ( ch_ == '/' ) )
				|| ( ch_ == '=' ) );
	}

int long base64_raw_decode( yaal::hcore::HString const& message, char* output, int long bufSize, bool standardCompliantMode )
	{
	M_PROLOG
	int shifts[] = { 18, 12, 6, 0 };
	u32_t coder = 0;
	char const* ptr = message.raw();
	int long length = message.get_length();
	int long size( 0 );
	int long i( 0 );
	M_ENSURE( ( ( ( ( length * 3 ) / 4 ) + 1 ) <= bufSize ) || ( ! length ) );
	for ( ; ( i < length ) && ( ptr[ i ] != '=' ); ++ i )
		{
		char ch = ptr[ i ];
		M_ENSURE( is_base64_character( ch, standardCompliantMode ) );
		int shift = shifts[ i % 4 ];
		coder |= ( n_pcBase64DecodeTable[standardCompliantMode ? 1 : 0][ static_cast<u8_t>( ptr[ i ] ) ] << shift );
		if ( ! shift )
			{
			do_buf_4_to_3( output + size, coder );
			size += 3;
			coder = 0;
			}
		}
	if ( i % 4 )
		{
		do_buf_4_to_3( output + size, coder, static_cast<int>( ( bufSize - size - 1 ) ) );
		size += ( length - i );
		}
	return ( size );
	M_EPILOG
	}

yaal::hcore::HString base64::decode( yaal::hcore::HString const& message, bool standardCompliantMode )
	{
	M_PROLOG
	HPool<char> buf( message.get_length() );
	base64_raw_decode( message, buf.raw(), buf.get_size(), standardCompliantMode );
	return ( buf.raw() );
	M_EPILOG
	}

void base64::encode( yaal::hcore::HStreamInterface& in, yaal::hcore::HStreamInterface& out, bool standardCompliantMode, int wrap_ )
	{
	M_PROLOG
	M_ENSURE( wrap_ >= 0 );
	int const BASE64LINELEN = 57;
	char buf[BASE64LINELEN];
	int long size( 0 );
	HString line( 80, true );
	int long offset( 0 );
	bool needEndl( false );
	while ( ( size = in.read( buf, sizeof ( buf ) ) ) > 0 )
		{
		line = base64_raw_encode( buf, size, standardCompliantMode );
		int long nRead( line.get_length() );
		if ( wrap_ )
			{
			char const* ptr = line.raw();
			while ( ( offset + nRead ) >= wrap_ )
				{
				int long nWrite( wrap_ - offset );
				out.write( ptr, nWrite );
				ptr += nWrite;
				nRead -= nWrite;
				offset = 0;
				out << endl;
				needEndl = false;
				}
			if ( nRead > 0 )
				{
				out.write( ptr, nRead );
				offset = nRead;
				needEndl = true;
				}
			}
		else
			out << line << flush;
		}
	if ( wrap_ && needEndl )
		out << endl;
	return;
	M_EPILOG
	}

void base64::decode( yaal::hcore::HStreamInterface& in, yaal::hcore::HStreamInterface& out, bool standardCompliantMode )
	{
	M_PROLOG
	int const BASE64LINELEN = 76;
	int const BUF_LEN = 80;
	char buf[BUF_LEN];
	int long size( 0 );
	HString line( BUF_LEN, true );
	HString decodebuf( BASE64LINELEN + 1, true );
	int long pos( 0 );
	while ( in.read_until_n( line, BUF_LEN ) )
		{
		char const* const ptr = line.raw();
		int const SIZE = static_cast<int>( line.get_length() );
		for ( int i = 0; i < SIZE; ++ i, ++ pos )
			{
			char ch = ptr[ i ];
			M_ENSURE_EX( is_base64_character( ch, standardCompliantMode ) || isalpha( ch ), ( HFormat( "char: %c, at position: %ld" ) % ch % pos ).string() );
			if ( is_base64_character( ch, standardCompliantMode	) )
				decodebuf += ch;
			if ( decodebuf.get_length() >= BASE64LINELEN )
				{
				M_ASSERT( decodebuf.get_length() == BASE64LINELEN );
				size = base64_raw_decode( decodebuf, buf, sizeof ( buf ), standardCompliantMode );
				out.write( buf, size );
				decodebuf.clear();
				}
			}
		}
	return;
	M_EPILOG
	}

}

}

