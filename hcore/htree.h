/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	htree.h - this file is integral part of `stdhapi' project.

	i.  You may not make any changes in Copyright information.
	ii. You must atttach Copyright information to any part of every copy
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

#ifndef __HCORE_HTREE_H
#define __HCORE_HTREE_H

#line 31

#include "hexception.h"
#include "hlist.h"

namespace stdhapi
{

namespace hcore
{

#define D_CVSID_HTREE_H "$CVSHeader$"

#define D_FILL_NEW_MANUAL	0
#define D_FILL_NEW_AUTO		1

template < typename tttType >
class HTree;

template < typename ttType >
class HBranchList : public HList < ttType >
	{
protected:
	/*{*/
	/*}*/
public:
	/*{*/
	virtual ~HBranchList ( void );
	/*}*/
protected:
	/*{*/
	virtual int long empty ( typename HList < ttType > ::HElement * );
	/*}*/
	};

template < typename tttType >
class HTree
	{
protected:
	class HNode
		{
	protected:
		/*{*/
		tttType f_tLeaf;																/* object itself */
		int f_iNumber;																	/* serial number */
		int f_iHits;			/* how many times element's object was accessed */
		int f_iLevel;																		/* self explanary */
		HBranchList < HNode * > f_oBranch;	/* list of next level nodes */
		HNode * f_poTrunk;									/* self explanary */
		HTree * f_poTree;										/* tree that owns node */
		/*}*/
	public:
		/*{*/
		inline tttType & get_object ( void )	/* this is special get used for  */
			{																		/* all derived classes (f.e. in  */
			return ( f_tLeaf );									/* compare methods)              */
			}																		/* this get does not modify hits */
		/*}*/
	protected:
		/*{*/
		HNode ( HNode * = NULL ); /* ( parent ) */
		virtual ~HNode ( void );
		void put ( tttType );
		tttType get ( void );
		HNode * previous ( int = D_TREAT_AS_OPENED );
		HNode * next ( void );
		/*}*/
	private:
		/*{*/
		HNode ( HNode const & );
		HNode & operator = ( HNode const & );
		/*}*/
		friend class HTree < tttType >;
		friend class HBranchList < HNode * >;
		};
private:
protected:
	/*{*/
	HNode * f_poRoot;			/* self explanary */
	HNode * f_poSelected;	/* local temporary pointer, "cursor" */
	int f_iHighestNumber;							/* serial number of last added element */
	/*}*/
public:
	/*{*/
	HTree ( void );
	virtual ~HTree ( );
	virtual void flush ( void );
	tttType & remove_node ( void );
	tttType cut_branch ( int );
	tttType cut_leaf ( int );
	tttType & down ( void );
	tttType & up ( void );
	void graft ( int, HTree < tttType > * );
	/*}*/
private:
	/*{*/
	HTree ( HTree const & );
	HTree & operator = ( HTree const & );
	/*}*/
	friend class HTree < tttType > :: HNode;
	};

/*
 *	*VERY IMPORTANT NOTE*:
 *
 *	Virtual class members called from base class constructor or base class
 *	destructor context does *NOT* act as _virtual_ .
 *
 */

template < typename ttType >
HBranchList < ttType > ::~HBranchList ( void )
	{
	M_PROLOG
	HList < ttType > ::flush ( );
	return;
	M_EPILOG
	}

template < typename ttType >
int long HBranchList < ttType > ::empty ( typename HList < ttType > ::HElement * a_poElement )
	{
	M_PROLOG
	ttType l_poLocalRoot = a_poElement->get_object ( );
	delete l_poLocalRoot;
	return ( HList < ttType > ::empty ( a_poElement ) );
	M_EPILOG
	}

template < typename tttType >
HTree < tttType > ::HNode::HNode( HNode * a_poNode ) : f_tLeaf ( ),
	f_iNumber ( 0 ), f_iHits ( 0 ), f_iLevel ( 0 ), f_oBranch ( ),
	f_poTrunk ( a_poNode ), f_poTree ( NULL )
	{
	M_PROLOG
	if ( a_poNode )
		{
		f_poTree = a_poNode->f_poTree;
		f_iLevel = a_poNode->f_iLevel + 1;
		if ( f_poTree )
			f_iNumber = f_poTree->f_iHighestNumber ++;
		}
	return ;
	M_EPILOG
	}

template < typename tttType >
HTree < tttType > ::HNode::~HNode ( void )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

template < typename tttType >
void HTree < tttType > ::HNode::put( tttType a_tttArgument )
	{
	M_PROLOG
	f_tLeaf = a_tttArgument;
	f_iHits ++;
	return ;
	M_EPILOG
	}
	
template < typename tttType >
tttType HTree < tttType > ::HNode::get( void )
	{
	M_PROLOG
	if ( ! f_iHits ) 
		M_THROW ( "no leaf to get from.", g_iErrNo );
	f_iHits ++;
	return ( f_tLeaf );
	M_EPILOG
	}

template < typename tttType >
typename HTree <tttType> ::HNode * HTree < tttType > ::HNode::previous ( int a_iFlag )
	{
	M_PROLOG
	HNode * l_poNode = NULL;
	if ( f_poTrunk )
		{
		l_poNode = f_poTrunk->f_oBranch.present ( );
		while ( l_poNode != this )
			{
			f_poTrunk->f_oBranch.to_head ( );
			l_poNode = f_poTrunk->f_oBranch.present ( );
			}
		f_poTrunk->f_oBranch.to_head ( 1, a_iFlag );
		l_poNode = f_poTrunk->f_oBranch.present ( );
		}
	return ( l_poNode );
	M_EPILOG
	}

template < typename tttType >
typename HTree < tttType > ::HNode * HTree < tttType > ::HNode::next ( void )
	{
	M_PROLOG
	HNode * * l_ppoNode = NULL;
	if ( f_poTrunk )
		{
		for ( l_ppoNode = & f_poTrunk->f_oBranch.go ( 0 );
				l_ppoNode && ( ( * l_ppoNode ) != this );
				l_ppoNode = f_poTrunk->f_oBranch.to_tail ( 1,	D_TREAT_AS_OPENED ) )
			;
		if ( l_ppoNode )
			l_ppoNode = f_poTrunk->f_oBranch.to_tail ( 1,	D_TREAT_AS_OPENED );
		}
	return ( l_ppoNode ? * l_ppoNode : NULL );
	M_EPILOG
	}

template < typename tttType >
HTree < tttType >::HTree ( void )
	: f_poRoot ( NULL ), f_poSelected ( NULL ), f_iHighestNumber ( 0 )
	{
	M_PROLOG
	return;
	M_EPILOG
	}

template < typename tttType >
HTree < tttType > ::~HTree( void )
	{
	M_PROLOG
	HTree::flush ( );
	return ;
	M_EPILOG
	}

template < typename tttType >
void HTree < tttType > ::flush ( void )
	{
	M_PROLOG
	if ( f_poRoot )
		delete f_poRoot;
	f_poRoot = NULL;
	M_EPILOG
	}

}

}

#endif /* not __HCORE_HTREE_H */
