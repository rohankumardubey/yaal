/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hlist.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_HCORE_HLIST_H
#define __YAAL_HCORE_HLIST_H

#line 31 "hlist.h"

#define D_VCSID_HLIST_H "$Id$"

#include "hcore/hexception.h"

namespace yaal
{

namespace hcore
{

extern char const * const g_ppcErrMsgHList [ ];

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++                          ++++++++++++++++++++++++*/
/*+++++++++++++++++++++++ general list declaration ++++++++++++++++++++++++*/
/*+++++++++++++++++++++++                          ++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

struct OListBits
	{
	struct ERROR
		{
		typedef enum
			{
			E_OK = 0,
			E_EMPTYELEMENT,
			E_BADINDEX,
			E_BADFLAG,
			E_EMPTY,
			E_BADOFFSET,
			E_BADNUMBER,
			E_BADORDER
			} error_t;
		};
	typedef enum
		{
		D_TREAT_AS_CLOSED = 1,
		D_TREAT_AS_OPENED = 2
		} treatment_t;
	typedef enum
		{
		D_OK = 0,
		D_ERROR = 1,
		D_FINAL_REACHED = 2,
		D_NOT_FOUND = 4
		} status_t;
	typedef enum
		{
		D_UNSORTED,
		D_ASCENDING,
		D_DESCENDING
		} sort_order_t;
	virtual ~OListBits ( void ) { } 
	};

template<typename tType> 
class HList : public OListBits
	{
	class HElement;
	int f_iSize;					/* how many elements this list contains */
	HElement * f_poHook;			/* "begining" of the list ( "first" element ) */
/* for internal use only */
	sort_order_t f_eOrder;		/* last-to-current sort order */
	int f_iIndex;							/* this two fiels will allow boost operator [ ],
															 int holds last */
	HElement * f_poIndex;			/* index and HElement * holds pointer to this
															 last element */
public:
	template<OListBits::treatment_t const treatment = OListBits::D_TREAT_AS_OPENED>
	class HIterator;
	typedef HIterator<> iterator;
	typedef HIterator<OListBits::D_TREAT_AS_CLOSED> cyclic_iterator;
	HList( int = 0 );                 /* Creates list, with specified size */
	virtual ~HList( void );
	HList( HList const& );
	HList& operator = ( HList const& );
	cyclic_iterator hook( void ) const;
	iterator begin( void ) const;
	iterator end( void ) const;
	iterator rend( void ) const;
	iterator rbegin( void ) const;
	void clear( void );
	int size( void ) const;
	/*!
	 * Adds new element at specified position.
	 */
	template<OListBits::treatment_t treatment>
	HIterator<treatment> insert( HIterator<treatment> const&, tType* = NULL );
	tType& add_head( tType const* = NULL );    /* adds new element at beggining of the list */
	tType& add_tail( tType const* = NULL );	/* adds new element at end of the list */
	void push_back( tType const& );
	void pop_back( void );
	void push_front( tType const& );
	/*!
	 * Add element in the way that keeps order.
	 */
	template<typename T>
	tType& add_orderly( tType const&, T const&, sort_order_t = D_ASCENDING );
	status_t remove_head( tType** = NULL );
	status_t remove_tail( tType** = NULL );
	template<OListBits::treatment_t treatment>
	HIterator<treatment> erase( HIterator<treatment> const& );
	/* sets cursor at specified index or number */
	iterator n_th( int );
	tType& operator[] ( int );
	tType const& operator[] ( int ) const;
	tType& head( void );
	tType const& head( void ) const;
	tType& tail( void );
	tType const& tail( void ) const;
	void exchange( int, int );
	void sort_by_contents( sort_order_t = D_ASCENDING );
	bool empty( void );
	bool is_empty( void );
	template<typename T>
	void sort( T const&, sort_order_t = D_ASCENDING );
protected:
	HElement* element_by_index ( int );
	void exchange( HElement*, HElement* );
	void sub_swap( HElement*, HElement*, HElement* );
	friend class HList<tType>::iterator;
	friend class HList<tType>::cyclic_iterator;
	};

template<typename tType>
class HList<tType>::HElement
	{
	HElement* f_poPrevious;
	HElement* f_poNext;
	tType f_tObject; /* The Object itself. */
	explicit HElement ( HElement* );
	virtual ~HElement ();
	HElement ( HElement const & );
	HElement& operator = ( HElement const& );
	friend class HList<tType>;
	friend class HList<tType>::iterator;
	friend class HList<tType>::cyclic_iterator;
	};

template<typename tType>
template<OListBits::treatment_t const treatment>
class HList<tType>::HIterator
	{
protected:
	/*{*/
	HList<tType> const* f_poOwner;
	HElement* f_poCurrent;
	/*}*/
public:
	/*{*/
	HIterator( void );
	template<OListBits::treatment_t family>
	HIterator( HIterator<family> const& );
	HIterator& operator ++ ( void )
		{
		M_PROLOG
		if ( f_poCurrent )
			{
			f_poCurrent = f_poCurrent->f_poNext;
			if ( ( treatment == OListBits::D_TREAT_AS_OPENED ) && ( f_poCurrent == f_poOwner->f_poHook ) )
				f_poCurrent = NULL;
			}
		return ( *this );
		M_EPILOG
		}
	HIterator const operator ++ ( int )
		{
		M_PROLOG
		HIterator l_oIterator ( *this );
		++ ( *this );
		return ( l_oIterator );
		M_EPILOG
		}
	HIterator& operator -- ( void )
		{
		M_PROLOG
		if ( f_poCurrent )
			{
			f_poCurrent = f_poCurrent->f_poPrevious;
			if ( ( treatment == OListBits::D_TREAT_AS_OPENED ) && ( f_poCurrent == f_poOwner->f_poHook->f_poPrevious ) )
				f_poCurrent = NULL;
			}
		return ( *this );
		M_EPILOG
		}
	HIterator const operator -- ( int )
		{
		M_PROLOG
		HIterator l_oIterator( * this );
		-- ( *this );
		return ( l_oIterator );
		M_EPILOG
		}
	template<OListBits::treatment_t family>
	HIterator& operator = ( HIterator<family> const& );
	bool operator == ( HIterator const & ) const;
	bool operator != ( HIterator const & ) const;
	tType& operator* ( void );
	tType const& operator* ( void ) const;
	tType* operator->( void );
	tType const* operator->( void ) const;
	bool is_valid( void ) const;
	/*}*/
protected:
	/*{*/
	friend class HList<tType>;
	HIterator( HList<tType> const* const, HElement* const );
	/*}*/
	};

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++                             ++++++++++++++++++++++*/
/*++++++++++++++++++++++ general list implementation ++++++++++++++++++++++*/
/*++++++++++++++++++++++                             ++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

template<typename tType>
HList<tType>::HElement::HElement( HElement* a_poElement )
	: f_poPrevious ( NULL ), f_poNext ( NULL ), f_tObject()
	{
	if ( a_poElement == 0 )
		{
		f_poPrevious = this;
		f_poNext = this;
		}
	else
		{
		f_poPrevious = a_poElement->f_poPrevious;
		f_poNext = a_poElement;
		a_poElement->f_poPrevious = this;
		f_poPrevious->f_poNext = this;
		}
	}

template<typename tType>
HList<tType>::HElement::~HElement( void )
	{
	f_poPrevious->f_poNext = f_poNext;
	f_poNext->f_poPrevious = f_poPrevious;
	return;
	}

//========================== Iterator ========================================

template<typename tType>
template<OListBits::treatment_t const treatment>
HList<tType>::HIterator<treatment>::HIterator( void ) : f_poOwner( NULL ), f_poCurrent( NULL )
	{
	return;
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
template<OListBits::treatment_t family>
HList<tType>::HIterator<treatment>::HIterator( HIterator<family> const& a_roIterator ) : f_poOwner( NULL ), f_poCurrent( NULL )
	{
	M_PROLOG
	operator = ( a_roIterator );
	return;
	M_EPILOG
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
HList<tType>::HIterator<treatment>::HIterator ( HList<tType> const* const a_poOwner,
		HElement* const a_poElement ) : f_poOwner( a_poOwner ),
	f_poCurrent( a_poElement )
	{
	return;
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
template<OListBits::treatment_t family>
typename HList<tType>::template HIterator<treatment>& HList<tType>::HIterator<treatment>::operator= ( HIterator<family> const& a_roIterator )
	{
	M_PROLOG
	if ( reinterpret_cast<HIterator<treatment> const*>( &a_roIterator ) != this )
		{
		f_poOwner = a_roIterator.f_poOwner;
		f_poCurrent = a_roIterator.f_poCurrent;
		}
	return ( *this );
	M_EPILOG
	}

/*
template<typename tType>
typename HList<tType>::HIterator & HList< tType >::HIterator::operator ++ ( void )

template<typename tType>
typename HList<tType>::HIterator const HList< tType >::HIterator::operator ++ ( int )

template<typename tType>
typename HList<tType>::HIterator & HList< tType >::HIterator::operator -- ( void )

template<typename tType>
typename HList<tType>::HIterator const HList< tType >::HIterator::operator -- ( int )
*/

template<typename tType>
template<OListBits::treatment_t const treatment>
bool HList<tType>::HIterator<treatment>::operator == ( HIterator const& a_roIterator ) const
	{
	M_PROLOG
	M_ASSERT( ( ! ( f_poOwner && a_roIterator.f_poOwner ) ) || ( f_poOwner == a_roIterator.f_poOwner ) );
	return ( f_poCurrent == a_roIterator.f_poCurrent );
	M_EPILOG
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
bool HList<tType>::HIterator<treatment>::operator!= ( HIterator const& a_roIterator ) const
	{
	M_PROLOG
	M_ASSERT( ( ! ( f_poOwner && a_roIterator.f_poOwner ) ) || ( f_poOwner == a_roIterator.f_poOwner ) );
	return ( f_poCurrent != a_roIterator.f_poCurrent );
	M_EPILOG
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
tType& HList<tType>::HIterator<treatment>::operator* ( void )
	{
	return ( f_poCurrent->f_tObject );
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
tType const & HList<tType>::HIterator<treatment>::operator* ( void ) const
	{
	return ( f_poCurrent->f_tObject );
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
tType * HList<tType>::HIterator<treatment>::operator -> ( void )
	{
	return ( & f_poCurrent->f_tObject );
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
bool HList<tType>::HIterator<treatment>::is_valid( void ) const
	{
	return ( f_poOwner && f_poOwner->f_poHook && f_poCurrent );
	}

//============================================================================

template<typename tType>
HList<tType>::HList ( int a_iSize )
	: OListBits(), f_iSize ( 0 ),
	f_poHook ( NULL ), f_eOrder ( D_UNSORTED ),
	f_iIndex ( 0 ), f_poIndex ( NULL )
	{
	M_PROLOG
	while ( a_iSize -- )
		HList::add_tail();
	return ;
	M_EPILOG
	}

template<typename tType>
HList<tType>::~HList ( void )
	{
	M_PROLOG
	HList::clear ();
	return ;
	M_EPILOG
	}

template<typename tType>
HList<tType>::HList ( HList<tType> const & a_roList )
	: OListBits(), f_iSize ( 0 ),
	f_poHook ( NULL ), f_eOrder ( D_UNSORTED ),
	f_iIndex ( 0 ), f_poIndex ( NULL )
	{
	M_PROLOG
	( * this ) = a_roList;
	return;
	M_EPILOG
	}

template<typename tType>
typename HList<tType>::iterator HList<tType>::begin( void ) const
	{
	return ( iterator( this, f_poHook ) );
	}

template<typename tType>
typename HList<tType>::iterator HList<tType>::end( void ) const
	{
	return ( iterator( this, NULL ) );
	}

template<typename tType>
typename HList<tType>::iterator HList<tType>::rend( void ) const
	{
	return ( iterator( this, NULL ) );
	}

template<typename tType>
typename HList<tType>::iterator HList<tType>::rbegin( void ) const
	{
	return ( iterator( this, f_poHook->f_poPrevious ) );
	}

template<typename tType>
typename HList<tType>::cyclic_iterator HList<tType>::hook( void ) const
	{
	return ( cyclic_iterator( this, f_poHook ) );
	}

template<typename tType>
HList<tType>& HList<tType>::operator = ( HList<tType> const& a_roList )
	{
	M_PROLOG
	int l_iCtr = 0;
	int l_iCount = 0;
	if ( this != & a_roList )
		{
		l_iCount = f_iSize < a_roList.f_iSize ? f_iSize : a_roList.f_iSize;
		iterator thisIt = begin();
		iterator otherIt = a_roList.begin();
		if ( l_iCount )
			{
			for ( l_iCtr = 0; l_iCtr < l_iCount; l_iCtr ++ )	
				{
				*thisIt = *otherIt;
				if ( otherIt.f_poCurrent == a_roList.f_poHook )
					f_poHook = thisIt.f_poCurrent;
				if ( otherIt.f_poCurrent == a_roList.f_poIndex )
					f_poIndex = thisIt.f_poCurrent;
				++ thisIt; ++ otherIt;
				}
			}
		if ( f_iSize > a_roList.f_iSize )
			{
			l_iCount = f_iSize - a_roList.f_iSize;
			while ( l_iCount -- )
				remove_tail();
			}
		else if ( f_iSize < a_roList.f_iSize )
			{
			for ( ; l_iCtr < a_roList.f_iSize; l_iCtr ++ )	
				{
				add_tail( &*otherIt );
				if ( otherIt.f_poCurrent == a_roList.f_poHook )
					f_poHook = f_poHook->f_poPrevious;
				if ( otherIt.f_poCurrent == a_roList.f_poIndex )
					f_poIndex = f_poHook->f_poPrevious;
				++ otherIt;
				}
			}
		f_iIndex = a_roList.f_iIndex;
		f_eOrder = a_roList.f_eOrder;
		f_iSize = a_roList.f_iSize;
		}
	return ( *this );
	M_EPILOG
	}

template<typename tType>
void HList<tType>::clear( void )
	{
	M_PROLOG
	while ( f_iSize -- )
		delete f_poHook->f_poNext;
	f_iIndex = 0;
	f_poIndex = NULL;
	f_poHook = NULL;
	f_iSize = 0;
	return ;
	M_EPILOG
	}

template<typename tType>
bool HList<tType>::is_empty( void )
	{
	return ( ! f_iSize );
	}

template<typename tType>
bool HList<tType>::empty( void )
	{
	return ( ! f_iSize );
	}

template<typename tType>
int HList<tType>::size( void ) const
	{
	M_PROLOG
	return ( f_iSize );
	M_EPILOG
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
typename HList<tType>::template HIterator<treatment> HList<tType>::insert( HIterator<treatment> const& a_oPositon,
		tType* a_ptObject )
	{
	M_PROLOG
	HElement* l_poElement = new HElement( a_oPositon.f_poCurrent );
	if ( f_iSize == 0 )
		f_poHook = l_poElement;
	f_iSize ++;
	f_iIndex = 0;
	f_poIndex = NULL;
	if ( a_ptObject )
		l_poElement->f_tObject = *a_ptObject;
	return ( iterator( this, l_poElement ) );
	M_EPILOG
	}

template<typename tType>
tType& HList<tType>::add_head( tType const* a_ptObject )
	{
	M_PROLOG
	f_poHook = new HElement ( f_poHook );
	f_iSize ++;
	if ( f_poIndex )
		f_poIndex = f_poIndex->f_poPrevious;
	if ( a_ptObject )
		f_poHook->f_tObject = *a_ptObject;
	return ( f_poHook->f_tObject );
	M_EPILOG
	}

template<typename tType>
tType& HList<tType>::add_tail( tType const* a_ptObject )
	{
	M_PROLOG
	HElement* l_poElement = new HElement( f_poHook );
	if ( f_iSize == 0 )
		f_poHook = l_poElement;
	f_iSize ++;
	if ( a_ptObject )
		l_poElement->f_tObject = *a_ptObject;
	return ( l_poElement->f_tObject );
	M_EPILOG
	}

template<typename tType>
void HList<tType>::push_back( tType const& a_rtObject )
	{
	M_PROLOG
	add_tail( &a_rtObject );
	return;
	M_EPILOG
	}

template<typename tType>
void HList<tType>::pop_back( void )
	{
	M_PROLOG
	remove_tail();
	return;
	M_EPILOG
	}

template<typename tType>
void HList<tType>::push_front( tType const& a_rtObject )
	{
	M_PROLOG
	add_head( &a_rtObject );
	return;
	M_EPILOG
	}

template<typename tType>
template<typename T>
tType& HList<tType>::add_orderly( tType const& a_rtObject,
		T const& less, sort_order_t a_eOrder )
	{
	M_PROLOG
	bool l_bBefore = false;
	int l_iIndex = 0, l_iOldIndex = -1, l_iLower = 0, l_iUpper = f_iSize;
	HElement * l_poElement = new HElement ( NULL );
	l_poElement->f_tObject = a_rtObject;
	if ( ( f_eOrder != D_UNSORTED ) && ( f_eOrder != a_eOrder ) )
		M_THROW ( g_ppcErrMsgHList [ ERROR::E_BADORDER ], a_eOrder );
	f_eOrder = a_eOrder;
	while ( f_iSize && ( l_iOldIndex != l_iIndex ) )
		{
		l_iOldIndex = l_iIndex;
		l_iIndex = ( l_iLower + l_iUpper ) / 2;
		element_by_index( l_iIndex );
		if ( less ( f_poIndex->f_tObject, l_poElement->f_tObject ) )
			l_iLower = l_iIndex;
		else
			l_iUpper = l_iIndex;
		}
	if ( f_poIndex )
		{
		if ( less ( f_poIndex->f_tObject, l_poElement->f_tObject ) )
			f_poIndex = f_poIndex->f_poNext;
		else
			l_bBefore = true;
		}
	if ( ! f_iSize )
		f_poHook = f_poIndex = l_poElement;
	else
		{
		l_poElement->f_poNext = f_poIndex;
		l_poElement->f_poPrevious = f_poIndex->f_poPrevious;
		f_poIndex->f_poPrevious->f_poNext = l_poElement;
		f_poIndex->f_poPrevious = l_poElement;
		}
	f_iSize ++;
	f_poIndex = NULL;
	f_iIndex = 0;
	if ( l_bBefore && ! l_iIndex )
		f_poHook = f_poHook->f_poPrevious;
	return ( l_poElement->f_tObject );
	M_EPILOG
	}

template<typename tType>
template<OListBits::treatment_t const treatment>
typename HList<tType>::template HIterator<treatment> HList<tType>::erase( HIterator<treatment> const& a_roIterator )
	{
	M_PROLOG
	HIterator<treatment> it = a_roIterator;
	++ it;
	if ( ! f_iSize )
		M_THROW( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	/* 1 2 3 4 5 6 7
	 *     ^
	 * 1 2 4 5 6 7
	 *     ^
	 * 1 2 3 4 5
	 * ^
	 * 2 3 4 5
	 * ^
	 * 1 2 3 4 5
	 *         ^
	 * 1 2 3 4  (o)
	 *       ^
	 * 1 2 3 4  (c) 
	 * ^
	 */
	if ( a_roIterator.f_poCurrent == f_poHook )
		f_poHook = f_poHook->f_poNext;
	delete a_roIterator.f_poCurrent;
	f_iSize --;
	if ( f_iSize == 0 )
		f_poHook = NULL;
	f_iIndex = 0;
	f_poIndex = NULL;
	return ( it );
	M_EPILOG
	}

template<typename tType>
OListBits::status_t HList<tType>::remove_head( tType** a_pptObject )
	{
	M_PROLOG
	status_t l_eError = D_OK;
	HElement * l_poElement = NULL;
	if ( f_iSize > 0 )
		{
		l_poElement = f_poHook;
		if ( a_pptObject )
			( *a_pptObject ) = &l_poElement->f_tObject;
		f_poHook = f_poHook->f_poNext;
		}
	else
		M_THROW( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	if ( f_poIndex )
		f_poIndex = f_poIndex->f_poNext;
	delete l_poElement;
	f_iSize--;
	if ( f_iSize == 0 )
		{
		f_poHook = NULL;
		f_poIndex = NULL;
		f_iIndex = 0;
		}
	return ( l_eError );
	M_EPILOG
	}

template<typename tType>
OListBits::status_t HList<tType>::remove_tail( tType** a_pptObject )
	{
	M_PROLOG
	status_t l_eError = D_OK;
	HElement * l_poElement = NULL;
	if ( f_iSize > 0 )
		{
		l_poElement = f_poHook->f_poPrevious;
		if ( a_pptObject )
			( *a_pptObject ) = &l_poElement->f_tObject;
		if ( l_poElement == f_poIndex )
			{
			f_poIndex = l_poElement->f_poPrevious;
			f_iIndex --;
			}
		delete l_poElement;
		-- f_iSize;
		if ( f_iSize == 0 )
			{
			f_poHook = NULL;
			f_poIndex = NULL;
			f_iIndex = 0;
			}
		}
	else
		M_THROW( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	return ( l_eError );
	M_EPILOG
	}

template<typename tType>
tType& HList<tType>::operator[] ( int a_iIndex )
	{
	M_PROLOG
	return ( element_by_index( a_iIndex )->f_tObject );
	M_EPILOG
	}

template<typename tType>
typename HList<tType>::HElement* HList<tType>::element_by_index( int a_iIndex )
	{
	M_PROLOG
	if ( f_iSize == 0 )
		M_THROW( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	if ( a_iIndex < 0 )
		a_iIndex += f_iSize;
	if ( ( a_iIndex >= f_iSize ) || ( a_iIndex < 0 ) )
		M_THROW( g_ppcErrMsgHList[ ERROR::E_BADINDEX ], a_iIndex );
	if ( ! f_poIndex )
		f_poIndex = f_poHook;
/*
we are looking for the smallest distance between a_iIndex and 0, f_iIndex, f_iSize.
we have two cases, a_iIndex is between 0 and f_iIndex:
	0 ..... a_iIndex ..... f_iIndex ..... f_iSize
meaning a_iIndex < f_iIndex
and a_iIndex is between f_iIndex and f_iSize
	0 ..... f_iIndex ..... a_iIndex ..... f_iSize
meaning a_iIndex > f_iIndex
lets take closer look at first case  ( a_iIndex < f_iIndex )
we have to check if a_iIndex is lowwer or geater than f_iIndex/2
*/
	if ( a_iIndex < f_iIndex )
		{
		if ( a_iIndex < ( f_iIndex / 2 ) )
			for ( f_iIndex = 0, f_poIndex = f_poHook; f_iIndex < a_iIndex; f_iIndex ++ )
				f_poIndex = f_poIndex->f_poNext;
		else
			for ( ; f_iIndex > a_iIndex; f_iIndex -- )
				f_poIndex = f_poIndex->f_poPrevious;
		}
	else
		{
		if ( a_iIndex < ( ( f_iSize + f_iIndex ) / 2 ) )
			for ( ; f_iIndex < a_iIndex; f_iIndex ++ )
				f_poIndex = f_poIndex->f_poNext;
		else
			for ( f_iIndex = f_iSize - 1, f_poIndex = f_poHook->f_poPrevious;
					f_iIndex > a_iIndex; f_iIndex -- )
				f_poIndex = f_poIndex->f_poPrevious;
		}
	return ( f_poIndex );
	M_EPILOG
	}

template<typename tType>
typename HList<tType>::iterator HList<tType>::n_th( int a_iIndex )
	{
	M_PROLOG
	return ( iterator( this, element_by_index( a_iIndex ) ) );
	M_EPILOG
	}

template<typename tType>
void HList<tType>::exchange( HElement* a_poLeft, HElement* a_poRight )
	{
	M_PROLOG
	HElement* l_poNext = NULL;
	HElement* l_poPrevious = NULL;
	if ( a_poLeft == a_poRight )
		return;
	if ( a_poLeft == f_poHook )
		f_poHook = a_poRight;
	else if ( a_poRight == f_poHook )
		f_poHook = a_poLeft;
	if ( a_poLeft == f_poIndex )
		f_poIndex = a_poRight;
	else if ( a_poRight == f_poIndex )
		f_poIndex = a_poLeft;
/*
 *                         ( p L n )
 *          ( p R n ) <------+   +------> ( p R n )
 *  ( L ) <---+   +---> ( L )     ( L ) <---+   +---> ( L )
 */
	l_poNext = a_poRight->f_poNext;
	l_poPrevious = a_poRight->f_poPrevious;
	a_poLeft->f_poNext->f_poPrevious = a_poRight;
	a_poLeft->f_poPrevious->f_poNext = a_poRight;
	l_poNext->f_poPrevious = a_poLeft;
	l_poPrevious->f_poNext = a_poLeft;

	l_poNext = a_poLeft->f_poNext;
	l_poPrevious = a_poLeft->f_poPrevious;
	a_poLeft->f_poNext = a_poRight->f_poNext;
	a_poLeft->f_poPrevious = a_poRight->f_poPrevious;
	a_poRight->f_poNext = l_poNext;
	a_poRight->f_poPrevious = l_poPrevious;
	return;
	M_EPILOG
	}

template<typename tType>
void HList<tType>::exchange( int a_iLeft, int a_iRight )
	{
	M_PROLOG
	HElement* l_poLeft = NULL;
	HElement* l_poRight = NULL;
	if ( a_iLeft == a_iRight )
		return;
	l_poLeft = element_by_index( a_iLeft );
	l_poRight = element_by_index( a_iRight );
	exchange( l_poLeft, l_poRight );
	return;
	M_EPILOG
	}

template<typename tType>
tType& HList<tType>::head( void )
	{
	M_PROLOG
	if ( f_poHook == 0 )
		M_THROW ( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	return ( f_poHook->f_tObject );
	M_EPILOG
	}

template<typename tType>
tType const& HList<tType>::head( void ) const
	{
	M_PROLOG
	if ( f_poHook == 0 )
		M_THROW ( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	return ( f_poHook->f_tObject );
	M_EPILOG
	}

template<typename tType>
tType& HList<tType>::tail( void )
	{
	M_PROLOG
	if ( f_poHook == 0 )
		M_THROW ( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	return ( f_poHook->f_poPrevious->f_tObject );
	M_EPILOG
	}

template<typename tType>
tType const& HList<tType>::tail( void ) const
	{
	M_PROLOG
	if ( f_poHook == 0 )
		M_THROW ( g_ppcErrMsgHList[ ERROR::E_EMPTY ], errno );
	return ( f_poHook->f_poPrevious->f_tObject );
	M_EPILOG
	}

template<typename tType>
template<typename T>
void HList<tType>::sort( T const& less, sort_order_t a_eOrder )
	{
	M_PROLOG
	int l_iCtr = f_iSize;
	int l_iCtrLoc = 0;
	HElement* l_poBaseLower = f_poHook;
	HElement* l_poBaseUpper = f_poHook->f_poPrevious;
	HElement* l_poExtreamLower = NULL;
	HElement* l_poExtreamUpper = NULL;
	HElement* l_poPointer = NULL;
	f_eOrder = a_eOrder;
	if ( ( f_eOrder != D_ASCENDING ) && ( f_eOrder != D_DESCENDING ) )
		M_THROW ( g_ppcErrMsgHList [ ERROR::E_BADORDER ], f_eOrder );
	while ( l_iCtr >= 0 )
		{
		l_iCtrLoc = l_iCtr;
		l_poExtreamLower = l_poBaseLower;
		l_poExtreamUpper = l_poBaseUpper;
		l_poPointer = l_poBaseLower;
		while ( l_iCtrLoc -- )
			{
			if ( ( l_poPointer != l_poExtreamLower )
					&& less ( l_poPointer->f_tObject, l_poExtreamLower->f_tObject ) )
				l_poExtreamLower = l_poPointer;
			if ( ( l_poPointer != l_poExtreamUpper )
					&& less ( l_poExtreamUpper->f_tObject, l_poPointer->f_tObject ) )
				l_poExtreamUpper = l_poPointer;
			l_poPointer = l_poPointer->f_poNext;
			}
		if ( l_poExtreamLower != l_poBaseLower )
			{
			exchange ( l_poBaseLower, l_poExtreamLower );
			if ( l_poExtreamLower == l_poBaseUpper )
				l_poBaseUpper = l_poBaseLower;
			l_poBaseLower = l_poExtreamLower;
			}
		if ( l_poExtreamUpper != l_poBaseUpper )
			{
			exchange ( l_poBaseUpper, l_poExtreamUpper );
			l_poBaseUpper = l_poExtreamUpper;
			}
		l_poBaseLower = l_poBaseLower->f_poNext;
		l_poBaseUpper = l_poBaseUpper->f_poPrevious;
		l_iCtr -= 2;
		}
	return;
	M_EPILOG
	}

template<typename tType>
void HList<tType>::sub_swap ( HElement * a_poLeft, HElement * a_poCenter,
		HElement* a_poRight )
	{
	M_PROLOG
	HElement * l_poCenterPrevious = NULL, * l_poRightNext = NULL;
	HElement * l_poLeftPrevious = NULL;
	if ( a_poLeft == a_poCenter )
		return;
	if ( a_poLeft == f_poHook )
		f_poHook = a_poCenter;
	else if ( a_poCenter == f_poHook )
		f_poHook = a_poLeft;
	if ( a_poLeft->f_poPrevious == a_poRight )
		return;
/*
 *   wwwwq lxxxxxx cyyyyyyr pwwww -> wwwwq cyyyyyyr lxxxxxx pwwww
 */
	
	l_poCenterPrevious = a_poCenter->f_poPrevious;
	l_poRightNext = a_poRight->f_poNext;
	l_poLeftPrevious = a_poLeft->f_poPrevious;
	
	l_poLeftPrevious->f_poNext = a_poCenter;
	l_poCenterPrevious->f_poNext = l_poRightNext;
	l_poRightNext->f_poPrevious = l_poCenterPrevious;
	a_poLeft->f_poPrevious = a_poRight;
	a_poRight->f_poNext = a_poLeft;
	a_poCenter->f_poPrevious = l_poLeftPrevious;
	
	return;
	M_EPILOG
	}

template<typename tType>
void HList<tType>::sort_by_contents ( sort_order_t a_eOrder )
	{
	M_PROLOG
	f_eOrder = a_eOrder;
	sort ( yaal::less<tType> );
	return ;
	M_EPILOG
	}

}

}

#endif /* not __YAAL_HCORE_HLIST_H */

