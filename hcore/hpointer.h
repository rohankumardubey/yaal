/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hpointer.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_HCORE_HPOINTER_H
#define __YAAL_HCORE_HPOINTER_H

#line 31 "hpointer.h"

#define D_VCSID_YAAL_HCORE_HPOINTER_H "$Id$"

#include "hcore/hexception.h"

namespace yaal
{

namespace hcore
{

/* Due to extream vitality of this class,
 * none of the methods are guarded. */

template<typename tType>
struct HPointerScalar
	{
	static void delete_pointee ( tType* );
	};

template<typename tType>
struct HPointerArray
	{
	static void delete_pointee ( tType* );
	static tType& object_at ( tType*, int );
	};

template<typename tType>
struct HPointerStrict
	{
	};

template<typename tType>
struct HPointerRelaxed
	{
	static tType* raw ( tType* );
	};

template<typename tType, template<typename>class pointer_type_t = HPointerScalar,
				 template<typename>class access_type_t = HPointerStrict >
class HPointer
	{
	class HShared
		{
		tType* f_ptPointer;
		int f_iReferenceCounter;
		explicit HShared ( tType* const );
		virtual ~HShared ( void ) throw ( );
		bool release ( void ) throw ( );
		HShared ( HShared const& );
		HShared& operator = ( HShared const& );
		friend class HPointer;
		};
	HShared* f_poShared;
public:
	HPointer ( void );
	explicit HPointer ( tType * const );
	virtual ~HPointer ( void );
	HPointer ( HPointer const& );
	template <typename hier_t>
	HPointer ( HPointer<hier_t, pointer_type_t, access_type_t> const& );
	HPointer & operator= ( HPointer const& );
	template<typename hiert_t>
	HPointer & operator= ( HPointer<hiert_t,pointer_type_t,access_type_t> const& );
	tType& operator* ( void ) const;
	tType& operator[] ( int ) const;
	template<typename hiert_t>
	bool operator== ( HPointer<hiert_t, pointer_type_t, access_type_t> const& ) const;
	template<typename hiert_t>
	bool operator== ( hiert_t const* const ) const;
	template<typename hiert_t>
	bool operator!= ( HPointer<hiert_t, pointer_type_t, access_type_t> const& ) const;
	template<typename hiert_t>
	bool operator!= ( hiert_t const* const ) const;
	tType* operator-> ( void ) const;
	tType* raw ( void ) const;
	bool operator! ( void ) const;
	};

template<typename tType>
void HPointerScalar<tType >::delete_pointee ( tType* a_ptPointer )
	{
	delete a_ptPointer;
	}

template<typename tType>
void HPointerArray<tType >::delete_pointee ( tType* a_ptPointer )
	{
	delete [] a_ptPointer;
	}

template<typename tType>
tType& HPointerArray<tType >::object_at ( tType* a_ptPointer, int a_iIndex )
	{
	return ( a_ptPointer [ a_iIndex ] );
	}

template<typename tType >
tType* HPointerRelaxed<tType >::raw ( tType* a_ptPointer )
	{
	return ( a_ptPointer );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t >
HPointer<tType, pointer_type_t, access_type_t >::HPointer ( void )
	: f_poShared ( NULL )
	{
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
HPointer<tType, pointer_type_t, access_type_t>::HShared::HShared ( tType* const a_ptPointer )
	: f_ptPointer ( a_ptPointer ), f_iReferenceCounter ( 1 )
	{
	M_ASSERT ( f_ptPointer );
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class  access_type_t>
HPointer<tType, pointer_type_t, access_type_t>::HShared::~HShared ( void ) throw ( )
	{
	M_ASSERT ( f_iReferenceCounter == 0 );
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
bool HPointer<tType, pointer_type_t, access_type_t>::HShared::release ( void ) throw ( )
	{
	M_ASSERT ( f_iReferenceCounter > 0 );
	f_iReferenceCounter --;
	if ( ! f_iReferenceCounter )
		{
		pointer_type_t<tType>::delete_pointee ( f_ptPointer );
		f_ptPointer = NULL;
		}
	return ( ! f_iReferenceCounter );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
HPointer<tType, pointer_type_t, access_type_t>::HPointer ( tType* const a_ptPointer )
	: f_poShared ( new HShared ( a_ptPointer ) )
	{
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
HPointer<tType, pointer_type_t, access_type_t>::~HPointer ( void )
	{
	if ( f_poShared && f_poShared->release ( ) )
		delete f_poShared;
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
HPointer<tType, pointer_type_t, access_type_t>::HPointer ( HPointer<tType, pointer_type_t, access_type_t> const& a_roPointer )
	: f_poShared ( NULL )
	{
	operator = ( a_roPointer );
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
HPointer<tType, pointer_type_t, access_type_t>::HPointer ( HPointer<hier_t, pointer_type_t, access_type_t> const& a_roPointer )
	: f_poShared ( NULL )
	{
	operator = ( a_roPointer );
	return;
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
HPointer<tType, pointer_type_t, access_type_t>& HPointer<tType, pointer_type_t, access_type_t>::operator = ( HPointer<tType, pointer_type_t, access_type_t> const& a_roPointer )
	{
	if ( ( &a_roPointer != this ) && ( a_roPointer.f_poShared != f_poShared ) )
		{
		a_roPointer.f_poShared->f_iReferenceCounter ++;
		if ( f_poShared )
			{
			M_ASSERT ( f_poShared->f_ptPointer != a_roPointer.f_poShared->f_ptPointer );
			if ( f_poShared->release ( ) )
				delete f_poShared;
			}
		f_poShared = a_roPointer.f_poShared;
		}
	return ( *this );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
HPointer<tType, pointer_type_t, access_type_t>& HPointer<tType, pointer_type_t, access_type_t>::operator = ( HPointer<hier_t, pointer_type_t, access_type_t> const& a_roPointer )
	{
	HPointer const* alien = reinterpret_cast<HPointer const *>( &a_roPointer );
	if ( ( alien != this ) && ( alien->f_poShared != f_poShared ) )
		{
		alien->f_poShared->f_iReferenceCounter ++;
		if ( f_poShared )
			{
			M_ASSERT ( f_poShared->f_ptPointer != alien->f_poShared->f_ptPointer );
			if ( f_poShared->release ( ) )
				delete f_poShared;
			}
		f_poShared = alien->f_poShared;
		/* Magic - it's a kind of magic
		 * It's a kind of magic
		 * Magic magic magic magic
		 * Ha ha ha it's magic
		 * It's a kind of magic
		 *           -- Roger Taylor */
		f_poShared->f_ptPointer = dynamic_cast<tType*>( reinterpret_cast<hier_t*>( f_poShared->f_ptPointer ) );
		}
	return ( *this );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
tType & HPointer<tType, pointer_type_t, access_type_t>::operator* ( void ) const
	{
	M_ASSERT( f_poShared );
	if ( ! f_poShared->f_ptPointer )
		throw 0;
	return ( * f_poShared->f_ptPointer );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
tType & HPointer<tType, pointer_type_t, access_type_t>::operator[] ( int a_iIndex ) const
	{
	M_ASSERT ( a_iIndex >= 0 );
	return ( pointer_type_t<tType>::object_at ( f_poShared->f_ptPointer, a_iIndex ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
bool HPointer<tType, pointer_type_t, access_type_t>::operator== ( HPointer<hier_t, pointer_type_t, access_type_t>const & a_roPointer ) const
	{
	HPointer const* alien = reinterpret_cast<HPointer const *>( &a_roPointer );
	return ( ( ! ( f_poShared || alien->f_poShared ) ) || ( f_poShared && alien->f_poShared
				&& ( f_poShared->f_ptPointer == alien->f_poShared->f_ptPointer ) ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
bool HPointer<tType, pointer_type_t, access_type_t>::operator== ( hier_t const* const a_ptPointer ) const
	{
	return ( ( ! ( f_poShared || a_ptPointer ) ) || ( f_poShared->f_ptPointer == a_ptPointer ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
bool HPointer<tType, pointer_type_t, access_type_t>::operator!= ( HPointer<hier_t, pointer_type_t, access_type_t>const & a_roPointer ) const
	{
	HPointer const* alien = reinterpret_cast<HPointer const *>( &a_roPointer );
	return ( ( f_poShared && ! alien->f_poShared )
			|| ( ! f_poShared && alien->f_poShared )
			|| ( f_poShared && alien->f_poShared
				&& ( f_poShared->f_ptPointer != alien->f_poShared->f_ptPointer ) ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
template<typename hier_t>
bool HPointer<tType, pointer_type_t, access_type_t>::operator!= ( hier_t const* const a_ptPointer ) const
	{
	return ( ( f_poShared && ( a_ptPointer != f_poShared->f_ptPointer ) ) || ( ! f_poShared && a_ptPointer ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t, typename hier_t>
bool operator== ( hier_t const* const a_ptPointer, HPointer<tType, pointer_type_t, access_type_t> const& a_oPointer )
	{
	return ( a_oPointer == a_ptPointer );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t, typename hier_t>
bool operator!= ( hier_t const* const a_ptPointer, HPointer<tType, pointer_type_t, access_type_t> const& a_oPointer )
	{
	return ( a_oPointer != a_ptPointer );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
tType* HPointer<tType, pointer_type_t, access_type_t>::operator-> ( void ) const
	{
	M_ASSERT( f_poShared );
	if ( ! f_poShared->f_ptPointer )
		throw 0;
	return ( access_type_t<tType>::raw ( f_poShared->f_ptPointer ) );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
tType* HPointer<tType, pointer_type_t, access_type_t>::raw ( void ) const
	{
	return ( f_poShared ? access_type_t<tType>::raw ( f_poShared->f_ptPointer ) : NULL );
	}

template<typename tType, template<typename>class pointer_type_t,
				 template<typename>class access_type_t>
bool HPointer<tType, pointer_type_t, access_type_t>::operator ! ( void ) const
	{
	return ( ! ( f_poShared && f_poShared->f_ptPointer ) );
	}

}

}

#endif /* not __YAAL_HCORE_HPOINTER_H */

