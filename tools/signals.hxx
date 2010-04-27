/*
---          `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	signals.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_TOOLS_SIGHAND_HXX_INCLUDED
#define YAAL_TOOLS_SIGHAND_HXX_INCLUDED

#include "hcore/hpointer.hxx"
#include "hcore/hchunk.hxx"
#include "hcore/hmultimap.hxx"
#include "hcore/hlist.hxx"
#include "hcore/hthread.hxx"
#include "hcore/hsingleton.hxx"

namespace yaal
{

namespace tools
{  

/*! \brief Interface for handlers for HSignalService.
 */
class HSignalHandlerInterface
	{
protected:
	typedef HSignalHandlerInterface self_t;
public:
	typedef yaal::hcore::HPointer<HSignalHandlerInterface> ptr_t;
	virtual ~HSignalHandlerInterface() { }
	typedef int ( HSignalHandlerInterface::* HANDLER_t )( int );
	};

/*! \brief Exception generated in HSignalHandlerInterface implementator.
 */
typedef yaal::hcore::HExceptionT<HSignalHandlerInterface> HSignalHandlerInterfaceException;

/*! \brief Posix signal manager.
 *
 * Instance of HSignalService class is a global object that handles
 * dispatching of operating system level signals.
 */
class HSignalService
	{
protected:
	typedef HSignalService self_t;
public:
	/*! \brief Signal hadler interface.
	 *
	 * HHandlerGeneric is base for all registrable handler types.
	 */
	class HHandlerGeneric
		{
		HSignalHandlerInterface::HANDLER_t HANDLER;
	public:
		typedef yaal::hcore::HPointer<HHandlerGeneric> ptr_t;
		HHandlerGeneric( HSignalHandlerInterface::HANDLER_t handle ) : HANDLER( handle ) { }
		virtual ~HHandlerGeneric( void ) { }
		virtual HSignalHandlerInterface* get_base() = 0;
		int invoke( int );
		};
	/*! \brief Create handler as method in your class.
	 */
	class HHandlerInternal : public HHandlerGeneric
		{
		HSignalHandlerInterface::ptr_t f_oBase;
	public:
		template<typename tType>
		HHandlerInternal( HSignalHandlerInterface::ptr_t a_oBase, tType handle )
			: HHandlerGeneric( static_cast<HSignalHandlerInterface::HANDLER_t>( handle ) ), f_oBase( a_oBase ) { }
		virtual HSignalHandlerInterface* get_base();
		};
	/*! \brief Create handle as free standing function.
	 */
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
	typedef yaal::hcore::HList<HHandlerGeneric::ptr_t> handler_list_t;
	typedef yaal::hcore::HPointer<handler_list_t> handler_list_ptr_t;
	typedef yaal::hcore::HMultiMap<int, HHandlerGeneric::ptr_t> handlers_t;
	static int f_iExitStatus;
	bool f_bLoop;
	yaal::hcore::HChunk f_oLocker;
	yaal::hcore::HThread _thread;
	yaal::hcore::HMutex f_oMutex;
	handlers_t f_oHandlers;
public:
	void register_handler( int, HHandlerGeneric::ptr_t );
private:
	HSignalService( void );
	~HSignalService( void );
	void* run( void );
	void lock_on( int );
	static int life_time( int );
	void schedule_exit( int );
	static void exit( int ) __attribute__(( __noreturn__ ));
	friend class yaal::hcore::HSingleton<HSignalService>;
	friend class yaal::hcore::HDestructor<HSignalService>;
	};

/*! \brief Get access to global signal manager.
 */
typedef yaal::hcore::HSingleton<HSignalService> HSignalServiceFactory;

/*! \brief Exception generated by HSignalService.
 */
typedef yaal::hcore::HExceptionT<HSignalService> HSignalServiceException;

}

}

#endif /* not YAAL_TOOLS_SIGHAND_HXX_INCLUDED */

