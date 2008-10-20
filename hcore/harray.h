/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	harray.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_HCORE_HARRAY_H
#define __YAAL_HCORE_HARRAY_H

#define D_VCSID_YAAL_HCORE_HARRAY_H "$Id$"

#include <new>

#include "hcore/base.h"

namespace yaal
{

namespace hcore
{

extern char const* const n_ppcErrMsgHArray[];

template<typename tType>
class HArray
	{
	typedef HArray<tType> self_t;
public:
	struct ERROR
		{
		typedef enum
			{
			E_OK = 0,
			E_BADSIZE,
			E_NOMEM,
			E_BADINDEX
			} error_t;
		};
protected:
	int long f_lSize;
	int long f_lCapacity;
	tType* f_ptArray;
public:
	template<typename const_qual_t>
	class HIterator;
	typedef HIterator<tType> iterator;
	typedef HIterator<tType const> const_iterator;
	HArray( int long const& = 0 );
	HArray( int long const&, tType const& );
	virtual ~HArray( void );
	HArray( HArray const& );
	HArray& operator = ( HArray const& );
	void resize( int long const& );
	void resize( int long const&, tType const& );
	void reserve( int long const& );
	tType& operator [] ( int long const& );
	tType const& operator [] ( int long const& ) const;
	void push_back( tType const& );
	int long get_size( void ) const;
	int long size( void ) const;
	int long get_capacity( void ) const;
	int long capacity( void ) const;
	void clear( void );
	bool operator ! ( void ) const;
	iterator begin( void );
	iterator find( int long const& );
	iterator end( void );
	const_iterator begin( void ) const;
	const_iterator find( int long const& ) const;
	const_iterator end( void ) const;
	void swap( HArray& );
private:
	tType& get( int long const& ) const;
	};

}

template<typename tType>
inline void swap( yaal::hcore::HArray<tType>& a, yaal::hcore::HArray<tType>& b )
	{ a.swap( b ); }

namespace hcore
{

template<typename tType>
template<typename const_qual_t>
class HArray<tType>::HIterator
	{
	typedef HArray<tType> array_t;
	array_t const* f_poOwner;
	int long f_lIndex;
public:
	HIterator( void ) : f_poOwner( NULL ), f_lIndex( 0 ) {}
	HIterator( HIterator const& a_oIt ) : f_poOwner( a_oIt.f_poOwner ), f_lIndex( a_oIt.f_lIndex ) {}
	HIterator& operator= ( HIterator const& a_oIt )
		{
		if ( &a_oIt != this )
			{
			f_poOwner = a_oIt.f_poOwner;
			f_lIndex = a_oIt.f_lIndex;
			}
		return ( *this );
		}
	HIterator& operator ++ ( void )
		{
		++ f_lIndex;
		return ( *this );
		}
	HIterator const operator ++ ( int )
		{
		HIterator it( f_lIndex );
		++ f_lIndex;
		return ( it );
		}
	HIterator& operator -- ( void )
		{
		-- f_lIndex;
		return ( *this );
		}
	HIterator const operator -- ( int )
		{
		HIterator it( f_lIndex );
		-- f_lIndex;
		return ( it );
		}
	const_qual_t& operator* ( void )
		{ return ( f_poOwner->f_ptArray[ f_lIndex ] ); }
	const_qual_t& operator* ( void ) const
		{ return ( f_poOwner->f_ptArray[ f_lIndex ] ); }
	const_qual_t* operator-> ( void )
		{ return ( &f_poOwner->f_ptArray[ f_lIndex ] ); }
	const_qual_t* operator-> ( void ) const
		{ return ( &f_poOwner->f_ptArray[ f_lIndex ] ); }
	template<typename other_const_qual_t>
	bool operator == ( HIterator<other_const_qual_t> const& it ) const
		{ return ( f_lIndex == it.f_lIndex ); }
	template<typename other_const_qual_t>
	bool operator != ( HIterator<other_const_qual_t> const& it ) const
		{ return ( f_lIndex != it.f_lIndex ); }
private:
	friend class HArray<tType>;
	explicit HIterator( array_t const* a_poOwner, int long const& idx )
		: f_poOwner( a_poOwner ), f_lIndex( idx ) {};
	};

template<typename tType>
HArray<tType>::HArray( int long const& a_lSize ) : f_lSize( 0 ), f_lCapacity( 0 ), f_ptArray( NULL )
	{
	M_PROLOG
	if ( a_lSize )
		reserve( a_lSize );
	return;
	M_EPILOG
	}

template<typename tType>
HArray<tType>::HArray( int long const& a_lSize, tType const& a_tFillWith )
	: f_lSize( 0 ), f_lCapacity( 0 ), f_ptArray( NULL )
	{
	M_PROLOG
	if ( a_lSize )
		{
		reserve( a_lSize );
		fill( begin(), end(), a_tFillWith );
		}
	return;
	M_EPILOG
	}

template<typename tType>
HArray<tType>::~HArray( void )
	{
	M_PROLOG
	clear();
	return;
	M_EPILOG
	}

template<typename tType>
void HArray<tType>::clear( void )
	{
	M_PROLOG
	if ( f_ptArray )
		delete [] f_ptArray;
	f_lSize = 0;
	f_lCapacity = 0;
	f_ptArray = NULL;
	return;
	M_EPILOG
	}

template<typename tType>
HArray<tType>::HArray( HArray const& a_roArray ) : f_lSize( 0 ), f_lCapacity( 0 ), f_ptArray( NULL )
	{
	M_PROLOG
	if ( a_roArray.f_lSize )
		{
		reserve( a_roArray.f_lSize );
		copy( a_roArray.begin(), a_roArray.end(), begin() );
		}
	return;
	M_EPILOG
	}

template<typename tType>
HArray<tType>& HArray<tType>::operator = ( HArray const& a_roArray )
	{
	M_PROLOG
	if ( &a_roArray != this )
		{
		if ( a_roArray.f_lSize > f_lCapacity )
			{
			HArray<tType> tmpArray( a_roArray );
			using yaal::swap;
			swap( tmpArray, *this );
			}
		else
			{
			f_lSize = a_roArray.f_lSize;
			copy( a_roArray.begin(), a_roArray.end(), begin() );
			}
		}
	return ( *this );
	M_EPILOG
	}

template<typename tType>
void HArray<tType>::resize( int long const& a_lSize )
	{
	M_PROLOG
	reserve( a_lSize );
	f_lSize = a_lSize;
	return;
	M_EPILOG
	}

template<typename tType>
void HArray<tType>::resize( int long const& a_lSize, tType const& t )
	{
	M_PROLOG
	reserve( a_lSize );
	if ( a_lSize > f_lSize )
		yaal::fill( f_ptArray + f_lSize, f_ptArray + a_lSize, t );
	f_lSize = a_lSize;
	return;
	M_EPILOG
	}

template<typename tType>
void HArray<tType>::reserve( int long const& a_lSize )
	{
	M_PROLOG
	if ( a_lSize < 1 )
		M_THROW( n_ppcErrMsgHArray[ ERROR::E_BADSIZE ], a_lSize );
	if ( a_lSize > f_lCapacity )
		{
		f_lCapacity = 1;
		while ( f_lCapacity < a_lSize )
			f_lCapacity <<= 1;
		HArray<tType> tmpArray( 0 );
		using yaal::swap;
		swap( tmpArray, *this );
		f_ptArray = new ( std::nothrow ) tType[ a_lSize ];
		if ( ! f_ptArray )
			M_THROW( n_ppcErrMsgHArray[ ERROR::E_NOMEM ], a_lSize );
		swap_ranges( tmpArray.begin(), tmpArray.end(), begin() );
		}
	return;
	M_EPILOG
	}

template<typename tType>
tType& HArray<tType>::get( int long const& a_lIndex ) const
	{
	M_PROLOG
	int long idx = ( a_lIndex < 0 ) ? a_lIndex + f_lSize : a_lIndex;
	if ( ( idx >= f_lSize ) || ( idx < 0 ) )
		M_THROW( n_ppcErrMsgHArray[ ERROR::E_BADINDEX ], idx );
	return ( f_ptArray[ idx ] );
	M_EPILOG
	}

template<typename tType>
tType& HArray<tType>::operator[] ( int long const& a_lIndex )
	{
	M_PROLOG
	return ( get( a_lIndex ) );
	M_EPILOG
	}

template<typename tType>
tType const& HArray<tType>::operator[] ( int long const& a_lIndex ) const
	{
	M_PROLOG
	return ( get( a_lIndex ) );
	M_EPILOG
	}

template<typename tType>
bool HArray<tType>::operator ! ( void ) const
	{
	M_PROLOG
	return ( ! f_ptArray );
	M_EPILOG
	}

template<typename tType>
int long HArray<tType>::get_size( void ) const
	{
	M_PROLOG
	return ( f_lSize );
	M_EPILOG
	}

template<typename tType>
int long HArray<tType>::size( void ) const
	{
	M_PROLOG
	return ( f_lSize );
	M_EPILOG
	}

template<typename tType>
int long HArray<tType>::capacity( void ) const
	{
	return ( get_capacity() );
	}

template<typename tType>
int long HArray<tType>::get_capacity( void ) const
	{
	return ( f_lCapacity );
	}

template<typename tType>
typename HArray<tType>::iterator HArray<tType>::begin( void )
	{
	return ( iterator( this, 0 ) );
	}

template<typename tType>
typename HArray<tType>::iterator HArray<tType>::find( int long const& idx )
	{
	return ( iterator( this, min( idx, f_lSize ) ) );
	}

template<typename tType>
typename HArray<tType>::iterator HArray<tType>::end( void )
	{
	return ( iterator( this, f_lSize ) );
	}

template<typename tType>
typename HArray<tType>::const_iterator HArray<tType>::begin( void ) const
	{
	return ( const_iterator( this, 0 ) );
	}

template<typename tType>
typename HArray<tType>::const_iterator HArray<tType>::find( int long const& idx ) const
	{
	return ( const_iterator( this, min( idx, f_lSize ) ) );
	}

template<typename tType>
typename HArray<tType>::const_iterator HArray<tType>::end( void ) const
	{
	return ( const_iterator( this, f_lSize ) );
	}

template<typename tType>
void HArray<tType>::swap( HArray& other )
	{
	using yaal::swap;
	swap( f_ptArray, other.f_ptArray );
	swap( f_lSize, other.f_lSize );
	swap( f_lCapacity, other.f_lCapacity );
	return;
	}

template<typename tType>
void HArray<tType>::push_back( tType const& a_tPod )
	{
	M_PROLOG
	int long l_iOldTop = f_lSize;
	reserve( f_lSize + 1 );
	f_ptArray[ l_iOldTop ] = a_tPod;
	return;
	M_EPILOG
	}

}

}

#endif /* __YAAL_HCORE_HARRAY_H */

