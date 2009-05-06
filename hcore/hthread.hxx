/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hthread.hxx - this file is integral part of `yaal' project.

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
/*! \file hcore/hthread.hxx
 * \brief Declaration of threading related classes.
 *
 * HThread, HThreadT<>, HMutex, HLock, HSemaphore, HCondition.
 */

#ifndef YAAL_HCORE_HTHREAD_HXX_INCLUDED
#define YAAL_HCORE_HTHREAD_HXX_INCLUDED

#include "hcore/hchunk.hxx"
#include "hcore/hrawdestructor.hxx"

namespace yaal
{

namespace hcore
{

class HCondition;
/*! \brief Implementation of multi-threaded synchronizing prymitive - Mutex.
 */
class HMutex
	{
	typedef HMutex self_t;
public:
	/*! \brief Mutex types.
	 */
	struct TYPE
		{
		/*! \brief Mutex types.
		 */
		typedef enum
			{
			DEFAULT = 0,      /*!< Default mutex type (non-recursive). */
			RECURSIVE = 1,    /*!< Recursive mutex type. */
			NON_RECURSIVE = 2 /*!< Implicit non-recursive mutex type. */
			} mutex_type_t;
		};
private:
	/*{*/
	TYPE::mutex_type_t f_eType;
	HChunk f_oAttributes;
	HChunk f_oMutex;
	HRawDestructor f_oAttrDS;
	/*}*/
public:
	/*{*/
	HMutex( TYPE::mutex_type_t const /* recursive | fair */ = TYPE::DEFAULT );
	virtual ~HMutex( void );
	void lock( void );
	bool try_lock( void );
	void unlock( void );
	/*}*/
private:
	/*{*/
	HMutex( HMutex const& );
	HMutex& operator = ( HMutex const& );
	friend class HCondition;
	/*}*/
	};

typedef HExceptionT<HMutex> HMutexException;

/*! \brief Multi-threaded synchronizing prymitive - Semaphore.
 */
class HSemaphore
	{
	typedef HSemaphore self_t;
	HChunk f_oSemaphore;
public:
	HSemaphore( void );
	virtual ~HSemaphore( void );
	void wait( void );
	void signal( void );
private:
	HSemaphore( HSemaphore const& );
	HSemaphore& operator = ( HSemaphore const& );
	};

typedef HExceptionT<HSemaphore> HSemaphoreException;

/*! \brief Core multi-threading primitive - Thread.
 */
class HThread
	{
	typedef HThread self_t;
	typedef enum
		{
		DEAD,
		SPAWNING,
		ALIVE,
		ZOMBIE
		} status_t;
	status_t f_eStatus;
	HChunk f_oAttributes;
	HChunk f_oThread;
	mutable HMutex f_oMutex;
	HSemaphore f_oSemaphore;
	HRawDestructor f_oAttrDS;
public:
	HThread( void );
	virtual ~HThread( void );
	int spawn( void );
	int long finish( void );
	void schedule_finish( void );
 	bool is_alive( void ) const;
	static int long get_id( void );
private:
	virtual int run( void );
	void* control( void );
	static void* SPAWN( void* );
	HThread( HThread const& );
	HThread& operator = ( HThread const& );
	};

typedef HExceptionT<HThread> HThreadException;

/*! \brief Interface to basic multi-threading primitive - Thread.
 */
template<typename tType>
class HThreadT : public HThread
	{
	tType& call;
public:
	HThreadT( tType& callee ) : call( callee ) {}
private:
	virtual int run( void )
		{ return ( call( const_cast<HThreadT const* const>( this ) ) ); }
	};

/*! \brief HLock Implementats automatic multi-threaded synchronizing prymitive.
 *
 * Scope based automation of locking and unlocking of Mutexes.
 */
class HLock
	{
	HMutex& f_roMutex;
public:
	explicit HLock( HMutex& );
	virtual ~HLock( void );
private:
	HLock( HLock const& );
	HLock& operator = ( HLock const& );
	};

/*! \brief Multi-threaded synchronizing prymitive - Conditional Variable.
 */
class HCondition
	{
	typedef HCondition self_t;
	HChunk f_oAttributes;
	HChunk f_oCondition;
	HMutex& f_roMutex;
public:
	typedef enum
		{
		OK,
		TIMEOUT,
		INTERRUPT
		} status_t;
	HCondition( HMutex& );
	virtual ~HCondition( void );
	status_t wait( int long unsigned const&, int long unsigned const& );
	void signal( void );
private:
	HCondition( HCondition const& );
	HCondition& operator = ( HCondition const& );
	};

typedef HExceptionT<HCondition> HConditionException;

}

}

#endif /* not YAAL_HCORE_HTHREAD_HXX_INCLUDED */

