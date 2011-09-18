/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hasynccaller.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_TOOLS_HASYNCCALLER_HXX_INCLUDED
#define YAAL_TOOLS_HASYNCCALLER_HXX_INCLUDED 1

#include "hcore/hboundcall.hxx"
#include "hcore/hthread.hxx"
#include "hcore/hmultimap.hxx"
#include "hcore/hsingleton.hxx"

namespace yaal {

namespace tools {

/*! \brief Interface for asynchronous call providers.
 */
class HAbstractAsyncCaller {
public:
	typedef HAbstractAsyncCaller this_type;
protected:
	typedef int long priority_t;
	typedef yaal::hcore::HBoundCall<> call_t;
	typedef yaal::hcore::HMultiMap<priority_t, call_t> queue_t;
	queue_t _queue;
	yaal::hcore::HThread _thread;
	yaal::hcore::HMutex _mutex;
	bool _loop;
public:
	void register_call( priority_t, call_t );
	void flush( void* );
protected:
	HAbstractAsyncCaller( void );
	virtual ~HAbstractAsyncCaller( void ) {}
	void start( void );
	void stop( void );
	virtual void do_signal( void ) = 0;
	virtual void* do_work( void ) = 0;
private:
	void* run( void );
};

typedef yaal::hcore::HExceptionT<HAbstractAsyncCaller> HAbstractAsyncCallerException;

/*! \brief Invoke function or method asynchronously.
 */
class HAsyncCaller : public HAbstractAsyncCaller, public yaal::hcore::HSingleton<HAsyncCaller> {
public:
	typedef HAbstractAsyncCaller base_type;
	typedef HAsyncCaller this_type;
private:
	yaal::hcore::HSemaphore _semaphore;
	HAsyncCaller( void );
	virtual ~HAsyncCaller( void );
	virtual void* do_work( void );
	virtual void do_signal( void );
	static int life_time( int );
	friend class yaal::hcore::HSingleton<HAsyncCaller>;
	friend class yaal::hcore::HDestructor<HAsyncCaller>;
};

typedef yaal::hcore::HExceptionT<HAsyncCaller, HAbstractAsyncCallerException> HAsyncCallerException;

}

}

#endif /* #ifndef YAAL_TOOLS_HASYNCCALLER_HXX_INCLUDED */

