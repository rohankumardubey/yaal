/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hcore/htls.hxx - this file is integral part of `yaal' project.

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
/*! \file hcore/htls.hxx
 * \brief Declaration/implementation of HTLS<> class.
 */

#ifndef YAAL_HCORE_HTLS_HXX_INCLUDED
#define YAAL_HCORE_HTLS_HXX_INCLUDED 1

#include "hcore/compat.hxx"
#include "hcore/memory.hxx"
#include "hcore/hdeque.hxx"
#include "hcore/hresource.hxx"
#include "hcore/hpointer.hxx"
#include "hcore/hboundcall.hxx"
#include "hcore/hthread.hxx"

namespace yaal
{

namespace hcore
{

/*! \brief HTLS<> adds automation around handling Thread Local Storage objects.
 */
template<typename tType>
class HTLS
	{
public:
	typedef HTLS<tType> this_type;
	typedef yaal::hcore::HPointer<tType> ptr_t;
	typedef yaal::hcore::HDeque<ptr_t> instances_t;
	typedef yaal::hcore::HBoundCall<> constructor_t;
	typedef yaal::hcore::HResource<yaal::hcore::HLock> external_lock;
	typedef typename instances_t::iterator iterator;
	typedef typename instances_t::const_iterator const_iterator;
private:
	static CXX_EXTENSION_TLS tType* _tls;
	yaal::hcore::HMutex _mutex;
	instances_t _instances;
	constructor_t _constructor;
public:
	HTLS( void )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( void )>( &this_type::construct ), this ) ) {}
	void construct( void )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0>
	HTLS( T0 const& a0_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const& )>( &this_type::construct ), this, a0_ ) ) {}
	template<typename T0>
	void construct( T0 const& a0_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0, typename T1>
	HTLS( T0 const& a0_, T1 const& a1_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const&, T1 const& )>( &this_type::construct ), this, a0_, a1_ ) ) {}
	template<typename T0, typename T1>
	void construct( T0 const& a0_, T1 const& a1_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_, a1_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0, typename T1, typename T2>
	HTLS( T0 const& a0_, T1 const& a1_, T2 const& a2_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const&, T1 const&, T2 const& )>( &this_type::construct ), this, a0_, a1_, a2_ ) ) {}
	template<typename T0, typename T1, typename T2>
	void construct( T0 const& a0_, T1 const& a1_, T2 const& a2_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_, a1_, a2_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0, typename T1, typename T2, typename T3>
	HTLS( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const&, T1 const&, T2 const&, T3 const& )>( &this_type::construct ),
					this, a0_, a1_, a2_, a3_ ) ) {}
	template<typename T0, typename T1, typename T2, typename T3>
	void construct( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_, a1_, a2_, a3_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0, typename T1, typename T2, typename T3, typename T4>
	HTLS( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_, T4 const& a4_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const&, T1 const&, T2 const&, T3 const&, T4 const& )>( &this_type::construct ),
					this, a0_, a1_, a2_, a3_, a4_ ) ) {}
	template<typename T0, typename T1, typename T2, typename T3, typename T4>
	void construct( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_, T4 const& a4_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_, a1_, a2_, a3_, a4_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	HTLS( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_, T4 const& a4_, T5 const& a5_ )
		: _mutex(), _instances(),
		_constructor( call( static_cast<void ( this_type::* )( T0 const&, T1 const&, T2 const&, T3 const&, T4 const&, T5 const& )>( &this_type::construct ),
					this, a0_, a1_, a2_, a3_, a4_ ) ) {}
	template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	void construct( T0 const& a0_, T1 const& a1_, T2 const& a2_, T3 const& a3_, T4 const& a4_, T5 const& a5_ )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		M_ASSERT( ! _tls );
		ptr_t p( _tls = new ( memory::yaal ) tType( a0_, a1_, a2_, a3_, a4_, a5_ ) );
		_instances.push_back( p );
		M_EPILOG
		}
	~HTLS( void )
		{
		M_PROLOG
		yaal::hcore::HLock l( _mutex );
		_instances.clear();
		_tls = NULL;
		M_DESTRUCTOR_EPILOG
		}
	external_lock acquire( void )
		{
		M_PROLOG
		external_lock l( new ( memory::yaal ) yaal::hcore::HLock( _mutex ) );
		return ( l );
		M_EPILOG
		}
	tType* operator->( void )
		{
		M_PROLOG
		if ( ! _tls )
			_constructor();
		return ( _tls );
		M_EPILOG
		}
	const_iterator begin( void ) const
		{
		return ( _instances.begin() );
		}
	iterator begin( void )
		{
		return ( _instances.begin() );
		}
	const_iterator end( void ) const
		{
		return ( _instances.end() );
		}
	iterator end( void )
		{
		return ( _instances.end() );
		}
	};
template<typename tType>
CXX_EXTENSION_TLS tType* HTLS<tType>::_tls = NULL;

}

}

#endif /* #ifndef YAAL_HCORE_HTLS_HXX_INCLUDED */

