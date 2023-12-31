#ifndef YAAL_MSVCXX_SYNCHRONIZEDQUEUE_HXX_INCLUDED
#define YAAL_MSVCXX_SYNCHRONIZEDQUEUE_HXX_INCLUDED 1

#include <queue>

#include "crit.hxx"
#include "hcore/hthread.hxx"

namespace msvcxx {

template<typename T>
class SynchronizedQueue {
	std::queue<T> _data;
	CMutex _mutex;
	yaal::hcore::HSemaphore _semaphore;
public:
	SynchronizedQueue( void )
		: _data()
		, _mutex()
		, _semaphore() {
	}
	bool pop( T& elem ) {
		_semaphore.wait();
		CLock l( _mutex );
		bool found = false;
		if ( ! _data.empty() ) {
			found = true;
			elem = _data.front();
			_data.pop();
		}
		return ( ! found );
	}
	void push( T const& elem ) {
		CLock l( _mutex );
		_data.push( elem );
		_semaphore.signal();
		return;
	}
};

}

#endif /* #ifndef YAAL_MSVCXX_SYNCHRONIZEDQUEUE_HXX_INCLUDED */
