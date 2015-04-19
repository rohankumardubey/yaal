/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	compat.cxx - this file is integral part of `yaal' project.

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

#include <cstdlib>

#include "compat.hxx"

#if ! defined( HAVE_POWL ) || ( HAVE_POWL == 0 ) || ! defined( HAVE_DECL_FLOORL ) || ( HAVE_DECL_FLOORL == 0 )
#include <cmath>
#endif /* not HAVE_POWL *//* not HAVE_DECL_FLOORL */

#if ! defined( HAVE_MEMRCHR ) || ( HAVE_MEMRCHR == 0 ) || ! defined( HAVE_STRNLEN ) || ( HAVE_STRNLEN == 0 )
#include <cstring>
#endif /* #if ! defined( HAVE_MEMRCHR ) || ( HAVE_MEMRCHR == 0 ) || ! defined( HAVE_STRNLEN ) || ( HAVE_STRNLEN == 0 ) */

#if ! defined( HAVE_ASPRINTF ) || ( HAVE_ASPRINTF == 0 )
#include <cstdio>
#include <cstdarg>
#endif /* not HAVE_ASPRINTF */

#if ( defined( HAVE_GETHOSTBYNAME_R ) && ( ! defined( HAVE_GNU_GETHOSTBYNAME_R ) ) ) || ( defined( HAVE_GETHOSTBYADDR_R ) && ( ! defined( HAVE_GNU_GETHOSTBYADDR_R ) ) )
#include <cerrno>
#endif /* ( HAVE_GETHOSTBYNAME_R && not HAVE_GNU_GETHOSTBYNAME_R ) || ( HAVE_GETHOSTBYADDR_R && not HAVE_GNU_GETHOSTBYADDR_R ) */

#if ! defined( HAVE_STRCASESTR ) || ( HAVE_STRCASESTR == 0 )
#include "hstring.hxx"
#endif /* not HAVE_STRCASESTR */

#include "base.hxx"

M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )

#if ! defined( HAVE_MEMRCHR ) || ( HAVE_MEMRCHR == 0 )
void* memrchr( void const* ptr, int what, int long unsigned from ) {
	char const* str( static_cast<char const*>( ptr ) );
	bool found( false );
	while ( from -- > 0 ) {
		if ( str[ from ] == what ) {
			found = true;
			break;
		}
	}
	return ( found ? const_cast<char*>( str + from ) : nullptr );
}
#endif /* not HAVE_MEMRCHR */

#if ! defined( HAVE_STRNLEN ) || ( HAVE_STRNLEN == 0 )
int long strnlen( char const* str_, int long maxLen_ ) {
	char const* end( static_cast<char const*>( ::memchr( str_, 0, static_cast<size_t>( maxLen_ ) ) ) );
	return ( end ? end - str_ : maxLen_ );
}
#endif /* #if ! defined( HAVE_STRNLEN ) || ( HAVE_STRNLEN == 0 ) */

#if ! defined( HAVE_STRCASESTR ) || ( HAVE_STRCASESTR == 0 )
#include <cstring>
char* strcasestr( char const* haystack, char const* needle ) {
	int long idx(
		::yaal::hcore::string_helper::kmpcasesearch(
			haystack,
			static_cast<int long>( ::std::strlen( haystack ) ),
			needle,
			static_cast<int long>( ::std::strlen( needle ) )
		)
	);
	return ( idx >= 0 ? const_cast<char*>( haystack ) + idx : 0 );
}
#endif /* not HAVE_STRCASESTR */

#if defined( HAVE_GETHOSTBYNAME_R )
#if ! defined( HAVE_GNU_GETHOSTBYNAME_R )
#include <cstdlib>
#include <netdb.h>
int gethostbyname_r( char const* a0, struct hostent* a1, char* a2, long unsigned a3, struct hostent** a4, int* a5 ) {
	hostent* h = ::gethostbyname_r( a0, a1, a2, static_cast<int>( a3 ), a5 );
	if ( a4 ) {
		*a4 = h;
	}
	return ( h ? 0 : errno );
}
#endif /* not HAVE_GNU_GETHOSTBYNAME_R */
#endif /* HAVE_GETHOSTBYNAME_R */

#if defined( HAVE_GETHOSTBYADDR_R )
#if ! defined( HAVE_GNU_GETHOSTBYADDR_R )
#include <cstdlib>
#include <netdb.h>
int gethostbyaddr_r( void const* a0, int a1, int a2, struct hostent* a3, char* a4, long unsigned a5, struct hostent** a6, int* a7 ) {
	hostent* h = ::gethostbyaddr_r( static_cast<char const*>( a0 ), a1, a2, a3, a4, static_cast<int>( a5 ), a7 );
	if ( a6 )
		*a6 = h;
	return ( h ? 0 : errno );
}
#endif /* not HAVE_GNU_GETHOSTBYADDR_R */
#endif /* HAVE_GETHOSTBYADDR_R */

#if ! defined( HAVE_POWL ) || ( HAVE_POWL == 0 )
double long powl( double long a, double long b ) {
	return ( ::std::pow( a, b ) );
}
#endif /* not HAVE_POWL */

#if ! defined( HAVE_DECL_FLOORL ) || ( HAVE_DECL_FLOORL == 0 )
double long floorl( double long x ) {
	return ( ::std::floor( x ) );
}
#endif /* not HAVE_DECL_FLOORL */

#if ! defined( HAVE_DECL_SQRTL ) || ( HAVE_DECL_SQRTL == 0 )
double long sqrtl( double long x ) {
	return ( ::std::sqrt( x ) );
}
#endif /* not HAVE_DECL_SQRTL */

#if ! defined( HAVE_DECL_STRTOLD ) || ( HAVE_DECL_STRTOLD == 0 )
double long strtold( char const* str, char** tail ) {
	return ( strtod( str, tail ) );
}
#endif /* not HAVE_DECL_STRTOLD */

#if ! defined( HAVE_CLOCK_GETTIME )
#include <sys/time.h>
int clock_gettime( clockid_t, struct timespec* time_ ) {
	timeval now;
	int err( gettimeofday( &now, NULL ) );
	if ( !err ) {
		time_->tv_sec  = now.tv_sec;
		time_->tv_nsec = now.tv_usec * 1000;
	}
	return ( err );
}
#endif /* #if ! defined( HAVE_CLOCK_GETTIME ) */

#if ! defined( HAVE_TIMER_CREATE )
#include <signal.h>
#include <unistd.h>
#include "hcore/hthread.hxx"
#include "hcore/si.hxx"
class HDarwinTimer {
	yaal::hcore::HThread _thread;
	yaal::hcore::HMutex _mutex;
	yaal::hcore::HCondition _cond;
	int _sigNo;
	itimerspec _timerSpec;
	bool _reset;
	bool _loop;
public:
	HDarwinTimer( int sigNo_ )
		: _thread()
		, _mutex()
		, _cond( _mutex )
		, _sigNo( sigNo_ )
		, _timerSpec()
		, _reset( false )
		, _loop( false ) {
	}
	virtual ~HDarwinTimer( void ) {
		yaal::hcore::HLock l( _mutex );
		_loop = false;
		_reset = true;
		_cond.signal();
		l.unlock();
		_thread.finish();
	}
	void set_time( itimerspec const* timerSpec_ ) {
		yaal::hcore::HLock l( _mutex );
		_timerSpec = *timerSpec_;
		if ( ! _loop ) {
			_thread.spawn( yaal::hcore::call( &HDarwinTimer::wait, this ) );
		} else {
			_reset = true;
			_cond.signal();
		}
	}
	void wait( void ) {
		yaal::hcore::HLock l( _mutex );
		_loop = true;
		while ( _loop ) {
			_cond.wait_for(
				duration(
					_timerSpec.it_value.tv_sec * yaal::hcore::si::NANO_IN_WHOLE +  _timerSpec.it_value.tv_nsec,
					yaal::hcore::time::UNIT::NANOSECOND
				)
			);
			if ( ! _reset ) {
				::kill( getpid(), _sigNo );
			}
			_reset = false;
		}
		_loop = false;
	}
};
int timer_create( clockid_t, struct sigevent* event_, timer_t* timer_ ) {
	int err( event_->sigev_notify == SIGEV_SIGNAL ? 0 : -1 );
	if ( !err ) {
		*timer_ = new HDarwinTimer( event_->sigev_signo );
	}
	return ( err );
}
int timer_settime( timer_t timer_, int, itimerspec const* timerSpec_, itimerspec* ) {
	timer_->set_time( timerSpec_ );
	return ( 0 );
}
int timer_delete( timer_t timer_ ) {
	delete timer_;
	return ( 0 );
}
#endif /* #if ! defined( HAVE_TIMER_CREATE ) */

