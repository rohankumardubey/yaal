/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	htime.cxx - this file is integral part of `yaal' project.

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

#include "config.hxx"

#include "base.hxx"
M_VCSID( "$Id: "__ID__" $" )
M_VCSID( "$Id: "__TID__" $" )
#include "htime.hxx"

namespace yaal
{

namespace hcore
{

char const* const _defaultTimeFormat_ = "%a, %d %b %Y %H:%M:%S %z";

HTime::HTime( void ) : _format ( _defaultTimeFormat_ ),
	_cache( 1, HChunk::STRATEGY::GEOMETRIC ), _value(), _broken()
	{
	M_PROLOG
	set_now();
	return;
	M_EPILOG
	}

HTime::HTime( char const* const strTime_ )
	: _format ( _defaultTimeFormat_ ),
	_cache( 1, HChunk::STRATEGY::GEOMETRIC ), _value(), _broken()
	{
	M_PROLOG
	char const* err( ::strptime( strTime_, _format.raw(), &_broken ) );
	if ( ! err )
		err = ::strptime( strTime_, "%F %T", &_broken );
	M_ENSURE( err );
	_value = ::mktime( &_broken );
	return;
	M_EPILOG
	}

HTime::HTime( HTime const& time_ ) : _format( _defaultTimeFormat_ ),
	_cache( 1, HChunk::STRATEGY::GEOMETRIC ), _value(), _broken()
	{
	M_PROLOG
	operator = ( time_ );
	return;
	M_EPILOG
	}

HTime::HTime( time_t const& time_ ) : _format( _defaultTimeFormat_ ),
	_cache( 1, HChunk::STRATEGY::GEOMETRIC ), _value( time_ ), _broken()
	{
	M_PROLOG
	M_ENSURE( localtime_r( &_value, &_broken ) );
	return;
	M_EPILOG
	}

HTime::HTime( int const year_, int const month_, int const day_,
							 int const hour_, int const minute_, int const second_ )
	: _format( _defaultTimeFormat_ ), _cache( 1, HChunk::STRATEGY::GEOMETRIC ), _value(),
	_broken()
	{
	M_PROLOG
	set_datetime( year_, month_, day_, hour_, minute_, second_ );
	return;
	M_EPILOG
	}

HTime::~HTime( void )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

void HTime::set( time_t const& time_ )
	{
	M_PROLOG
	_value = time_;
	M_ENSURE( localtime_r( &_value, &_broken ) );
	return;
	M_EPILOG
	}

void HTime::set_now ( void )
	{
	M_PROLOG
	_value = time( NULL );
	M_ENSURE( localtime_r( &_value, &_broken ) );
	return;
	M_EPILOG
	}

void HTime::format( char const * const format_ )
	{
	M_PROLOG
	_format = format_;
	return;
	M_EPILOG
	}

void HTime::set_time( int const hour_, int const minute_,
											 int const second_ )
	{
	M_PROLOG
	if ( ( hour_ < 0 ) || ( hour_ > 23 ) )
		M_THROW( "bad hour", hour_ );
	_broken.tm_hour = hour_;
	if ( ( minute_ < 0 ) || ( minute_ > 59 ) )
		M_THROW( "bad minute", minute_ );
	_broken.tm_min = minute_;
	if ( ( second_ < 0 ) || ( second_ > 59 ) )
		M_THROW( "bad second", second_ );
	_broken.tm_sec = second_;
	_value = mktime( &_broken );
	return;
	M_EPILOG
	}

void HTime::set_date( int const year_, int const month_,
											 int const day_ )
	{
	M_PROLOG
	_broken.tm_year = year_ - 1900;
	if ( ( month_ < 1 ) || ( month_ > 12 ) )
		M_THROW( "bad month in year", month_ );
	_broken.tm_mon = month_ - 1;
	if ( ( day_ < 1 ) || ( day_ > 31 ) )
		M_THROW( "bad day of month", day_ );
	_broken.tm_mday = day_;
	_value = mktime( &_broken );
	return;
	M_EPILOG
	}

void HTime::set_datetime( int const year_, int const month_,
											 int const day_, int const hour_,
											 int const minute_, int const second_ )
	{
	M_PROLOG
	set_date( year_, month_, day_ );
	set_time( hour_, minute_, second_ );
	return;
	M_EPILOG
	}

int HTime::get_year( void ) const
	{
	M_PROLOG
	return ( _broken.tm_year + 1900 );
	M_EPILOG
	}

int HTime::get_month( void ) const
	{
	M_PROLOG
	return ( _broken.tm_mon + 1 );
	M_EPILOG
	}

int HTime::get_day( void ) const
	{
	M_PROLOG
	return ( _broken.tm_mday );
	M_EPILOG
	}

int HTime::get_hour( void ) const
	{
	M_PROLOG
	return ( _broken.tm_hour );
	M_EPILOG
	}

int HTime::get_minute( void ) const
	{
	M_PROLOG
	return ( _broken.tm_min );
	M_EPILOG
	}

int HTime::get_second( void ) const
	{
	M_PROLOG
	return ( _broken.tm_sec );
	M_EPILOG
	}

HTime& HTime::operator = ( HTime const& time_ )
	{
	M_PROLOG
	if ( this != & time_ )
		{
		_format = time_._format;
		_value = time_._value;
		_cache.raw()[ 0 ] = 0;
		memcpy ( &_broken, &time_._broken, sizeof ( tm ) );
		}
	return ( * this );
	M_EPILOG
	}

HTime HTime::operator - ( HTime const& time_ ) const
	{
	M_PROLOG
	HTime time;
	time._format = _format;
	time._value = static_cast<time_t>( difftime( _value, time_._value ) );
	M_ENSURE( gmtime_r( &time._value, &time._broken ) );
	return ( time );
	M_EPILOG
	}

HTime & HTime::operator -= ( HTime const& time_ )
	{
	M_PROLOG
	( *this ) = ( *this ) - time_;
	return ( *this );
	M_EPILOG
	}

bool HTime::operator == ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value == time_ );
	M_EPILOG
	}

bool HTime::operator != ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value != time_ );
	M_EPILOG
	}

bool HTime::operator <= ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value <= time_ );
	M_EPILOG
	}

bool HTime::operator >= ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value >= time_ );
	M_EPILOG
	}

bool HTime::operator < ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value < time_ );
	M_EPILOG
	}

bool HTime::operator > ( time_t const& time_ ) const
	{
	M_PROLOG
	return ( _value > time_ );
	M_EPILOG
	}

HTime::operator char const* ( void ) const
	{
	M_PROLOG
	int long size = 0;
#ifdef HAVE_SMART_STRFTIME
	size = ::strftime( NULL, 1024, _format.raw(), &_broken ) + 1;
	if ( size < 2 )
		M_THROW( "bad format", errno );
	_cache.realloc( size );
	M_ENSURE( static_cast<int>( ::strftime( _cache.raw(),
					size, _format.raw(), &_broken ) ) < size );
#else /* HAVE_SMART_STRFTIME */
	_cache.realloc( 64 ); /* FIXME that is pretty dumb hack */
	size = ::strftime( _cache.raw(), 63, _format.raw(), &_broken ) + 1;
	if ( size < 2 )
		M_THROW( "bad format", errno );
#endif /* not HAVE_SMART_STRFTIME */
	return ( _cache.raw() );
	M_EPILOG
	}

HTime::operator time_t ( void ) const
	{
	return ( _value );
	}

}

}

