/*
---          `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	signals.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_TOOLS_SIGHAND_H
#define __YAAL_TOOLS_SIGHAND_H

#include "hcore/hpointer.h"
#include "hcore/hchunk.h"
#include "hcore/hmap.h"
#include "hcore/hlist.h"
#include "hcore/hthread.h"
#include "hcore/hsingleton.h"

namespace yaal
{

namespace tools
{  

class HSignalHandlerInterface
	{
public:
	typedef yaal::hcore::HPointer<HSignalHandlerInterface> ptr_t;
	virtual ~HSignalHandlerInterface() { }
	typedef int ( HSignalHandlerInterface::* HANDLER_t )( int );
	};

class HSignalService
	{
public:
	class HHandlerGeneric
		{
		HSignalHandlerInterface::HANDLER_t HANDLER;
	public:
		typedef yaal::hcore::HPointer<HHandlerGeneric, yaal::hcore::HPointerScalar, yaal::hcore::HPointerRelaxed> ptr_t;
		HHandlerGeneric( HSignalHandlerInterface::HANDLER_t handle ) : HANDLER( handle ) { }
		virtual ~HHandlerGeneric( void ) { }
		virtual HSignalHandlerInterface* get_base() = 0;
		int invoke( int );
		};
	class HHandlerInternal : public HHandlerGeneric
		{
		HSignalHandlerInterface::ptr_t f_oBase;
	public:
		template<typename tType>
		HHandlerInternal( HSignalHandlerInterface::ptr_t a_oBase, tType handle )
			: HHandlerGeneric( static_cast<HSignalHandlerInterface::HANDLER_t>( handle ) ), f_oBase( a_oBase ) { }
		virtual HSignalHandlerInterface* get_base();
		};
	class HHandlerExternal : public HHandlerGeneric
		{
		HSignalHandlerInterface* f_poBase;
	public:
		template<typename tType>
		HHandlerExternal( HSignalHandlerInterface* a_poBase, tType handle )
			: HHandlerGeneric( static_cast<HSignalHandlerInterface::HANDLER_t>( handle ) ), f_poBase( a_poBase ) { }
		HHandlerExternal( HHandlerExternal const& );
		HHandlerExternal& operator = ( HHandlerExternal const& );
		virtual HSignalHandlerInterface* get_base();
		};
private:
	typedef yaal::hcore::HThreadT<HSignalService> signal_service_t;
	typedef yaal::hcore::HList<HHandlerGeneric::ptr_t> handler_list_t;
	typedef yaal::hcore::HPointer<handler_list_t, yaal::hcore::HPointerScalar, yaal::hcore::HPointerRelaxed> handler_list_ptr_t;
	typedef yaal::hcore::HMap<int, handler_list_ptr_t> handlers_t;
	static int f_iExitStatus;
	bool f_bLoop;
	yaal::hcore::HChunk f_oLocker;
	signal_service_t f_oWorker;
	yaal::hcore::HMutex f_oMutex;
	handlers_t f_oHandlers;
public:
	void register_handler( int, HHandlerGeneric::ptr_t );
private:
	HSignalService( void );
	~HSignalService( void );
	int operator()( yaal::hcore::HThread const* );
	void lock_on( int );
	static int life_time( int );
	void schedule_exit( int );
	static void exit( int ) __attribute__(( __noreturn__ ));
	friend class yaal::hcore::HSingleton<HSignalService>;
	friend class yaal::hcore::HDestructor<HSignalService>;
	friend class yaal::hcore::HThreadT<HSignalService>;
	};

typedef yaal::hcore::HSingleton<HSignalService> HSignalServiceFactory;

}

}

#endif /* not __YAAL_TOOLS_SIGHAND_H */

