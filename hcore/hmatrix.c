/*
---           `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hmatrix.c - this file is integral part of `stdhapi' project.

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

#include "hexception.h"
M_CVSID ( "$CVSHeader$" );
#include "hmatrix.h"
#include "hvector.h"

HMatrix::HMatrix( int a_iRows, int a_iColumns )
	{
	M_PROLOG
	int l_iCtr;
	if ( a_iRows < 1 )
		throw new HException ( __WHERE__, "Rows < 1", a_iRows );
	else f_iRows = a_iRows;
	if ( a_iColumns < 1 )
		throw new HException ( __WHERE__, "Columns < 1", a_iColumns );
	else f_iColumns = a_iColumns;
	if ( !( f_ppoBody = new HVector * [ f_iRows ] ) ) 
		throw new HException ( __WHERE__, "new returned", ( int ) f_ppoBody );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		{
		if ( !( f_ppoBody[ l_iCtr ] = new HVector( f_iColumns ) ) )
			throw new HException ( __WHERE__, "new returned 0", l_iCtr );
		}
	return ;
	M_EPILOG
	}
	
HMatrix::HMatrix( const HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	f_iRows = a_roMatrix.f_iRows;
	f_iColumns = a_roMatrix.f_iColumns;
	if ( !( f_ppoBody = new HVector * [ f_iRows ] ) )
			throw new HException ( __WHERE__, "new returned", ( int ) f_ppoBody );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		{
		if ( !( f_ppoBody[ l_iCtr ] = new HVector( f_iColumns ) ) )
			throw HException ( __WHERE__, "new returned 0", l_iCtr );
		*f_ppoBody[ l_iCtr ] = *a_roMatrix.f_ppoBody[ l_iCtr ];
		}
	return ;
	M_EPILOG
	}
	
HMatrix::~HMatrix( void )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) delete f_ppoBody[ l_iCtr ];
	delete [] f_ppoBody;
	return ;
	M_EPILOG
	}
	
int HMatrix::set( double ** d )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) f_ppoBody [ l_iCtr ]->set ( d [ l_iCtr ] );
	return ( 0 );
	M_EPILOG
	}
	
int HMatrix::set( HVector * a_poVector )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) * f_ppoBody [ l_iCtr ] = ( * a_poVector ) [ l_iCtr ];
	return ( 0 );
	M_EPILOG
	}
	
int HMatrix::row( void )
	{
	M_PROLOG
	return ( f_iRows );
	M_EPILOG
	}
	
int HMatrix::col( void )
	{
	M_PROLOG
	return ( f_iColumns );
	M_EPILOG
	}
	
int HMatrix::change_space( int a_iRows, int a_iColumns )
	{
	M_PROLOG
/* #warning FIXME */
	int l_iCtr, j, bakrow, bakcol, r, c;
	HVector ** l_ppoTmp;
	bakrow = f_iRows;
	bakcol = f_iColumns;
	l_ppoTmp = f_ppoBody;
	r = ( f_iRows < bakrow ? f_iRows : bakrow );
	c = ( f_iColumns < bakcol ? f_iColumns : bakcol );
	if ( a_iRows < 1 )
		throw new HException ( __WHERE__, "Rows < 1", a_iRows );
	else f_iRows = a_iRows;
	if ( a_iColumns < 1 )
		throw new HException ( __WHERE__, "Columns < 1", a_iColumns );
	else f_iColumns = a_iColumns;
	if ( !( f_ppoBody = new HVector * [ f_iRows ] ) )
		throw new HException ( __WHERE__, "new returned", ( int ) f_ppoBody );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		{
		if ( !( f_ppoBody [ l_iCtr ] = new HVector ( f_iColumns ) ) )
			throw new HException ( __WHERE__, "new returned 0", l_iCtr );
		}
	for ( l_iCtr = 0; l_iCtr < r; l_iCtr++ )
		for ( j = 0; j < c; j++ )
			( * f_ppoBody[ l_iCtr ] ) [ j ] = ( * l_ppoTmp[ l_iCtr ] ) [ j ];
	delete [] l_ppoTmp;
	return ( 0 );
	M_EPILOG
	}
	
HVector & HMatrix::operator [ ] ( int a_iIndex )
	{
	M_PROLOG
	if ( a_iIndex < 0 )
		throw new HException ( __WHERE__, "negative index value", a_iIndex );
	if ( a_iIndex >= f_iRows )
		throw new HException ( __WHERE__, "index excides vector size", a_iIndex );
	return ( *f_ppoBody [ a_iIndex ] );
	M_EPILOG
	}
	
double HMatrix::det ( void )
	{
	M_PROLOG
	int l_iCtr, k, q = 0;
	double d;
	if ( f_iRows != f_iColumns )throw new HException ( __WHERE__, "matrix is not square", f_iRows - f_iColumns );
	HMatrix l_oMatrix( f_iRows, f_iColumns );
	HVector l_oVector( f_iColumns );
	l_oMatrix = *this;
	for ( k = 0; k < f_iRows; k++ )
		{
		if ( l_oMatrix[ k ][ k ] != 0 ) continue;
		for ( l_iCtr = 0; l_iCtr < k; l_iCtr++ )
			if ( ( l_oMatrix[ k ][ l_iCtr ] != 0 ) && ( l_oMatrix[ l_iCtr ][ k ] != 0 ) )
				{
				l_oVector = l_oMatrix[ l_iCtr ];
				l_oMatrix[ l_iCtr ] = l_oMatrix[ k ];
				l_oMatrix[ k ] = l_oVector;
				q++;
				break;
				}
		if ( l_iCtr == k )
			for ( l_iCtr = k; l_iCtr < f_iRows; l_iCtr++ )
				if ( l_oMatrix[ k ][ l_iCtr ] != 0 )
					{
					l_oVector = l_oMatrix[ l_iCtr ];
					l_oMatrix[ l_iCtr ] = l_oMatrix[ k ];
					l_oMatrix[ k ] = l_oVector;
					q++;
					break;
					}
		}
	for ( k = 0; k < f_iRows; k++ ) if ( l_oMatrix[ k ][ k ] == 0 ) return ( 0 );
	for ( k = 0; k < f_iRows; k++ )
		{
		if ( l_oMatrix[ k ][ k ] == 0 ) return ( 0 );
		for ( l_iCtr = k + 1; l_iCtr < f_iRows; l_iCtr++ )
			{
			d = -l_oMatrix[ l_iCtr ][ k ] / l_oMatrix[ k ][ k ];
			HVector v = ( d * l_oMatrix[ k ] );
			l_oMatrix[ l_iCtr ] += v;
			}
		}
	d = 1;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) d *= l_oMatrix[ l_iCtr ][ l_iCtr ];
	if ( q % 2 ) d = -d;
	return ( d );
	M_EPILOG
	}
	
double HMatrix::M( int r, int c )
	{
	M_PROLOG
	int l_iCtr, j, ii = 0, jj = 0;
	if ( ( f_iRows == 1 ) || ( f_iColumns == 1 ) || ( f_iRows != f_iColumns ) ) return ( 0 );
	HMatrix l_oMatrix( f_iRows - 1, f_iColumns - 1 );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		{
		if ( l_iCtr == r ) continue;
		for ( j = 0; j < f_iColumns; j++ )
			{
			if ( j == c ) continue;
			l_oMatrix[ ii ][ jj ] = ( *f_ppoBody[ l_iCtr ] ) [ j ];
			jj++;
			}
		ii++;
		}
	return ( l_oMatrix.det() );
	M_EPILOG
	}
	
HMatrix HMatrix::T( void )
	{
	M_PROLOG
	int l_iCtr, j;
	HMatrix l_oMatrix( f_iColumns, f_iRows );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) for ( j = 0; j < f_iColumns; j++ ) 
		l_oMatrix [ j ][ l_iCtr ] = ( *f_ppoBody[ l_iCtr ] ) [ j ];
	return ( l_oMatrix );
	M_EPILOG
	}
	
HMatrix HMatrix::_1( void )
	{
	M_PROLOG
	double d;
	int l_iCtr, j;
	if ( ( d = det() ) != 0 )
		{
		HMatrix l_oMatrix( f_iRows, f_iColumns );
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) for ( j = 0; j < f_iColumns; j++ )
			if ( ( l_iCtr + j ) % 2 ) l_oMatrix [ l_iCtr ][ j ] = -M( l_iCtr, j );
			else l_oMatrix [ l_iCtr ][ j ] = M( l_iCtr, j );
		return ( l_oMatrix.T() / d );
		}
	return ( T() );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator = ( const HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	if ( ( & a_roMatrix != this ) && ( f_iRows == a_roMatrix.f_iRows ) 
			&& ( f_iColumns == a_roMatrix.f_iColumns ) ) 
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) 
			*f_ppoBody[ l_iCtr ] = *a_roMatrix.f_ppoBody[ l_iCtr ];
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator = ( double d )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) *f_ppoBody[ l_iCtr ] = d;
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator <= ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr, r;
	if ( & a_roMatrix != this )
		{
		r = ( f_iRows < a_roMatrix.f_iRows ? f_iRows : a_roMatrix.f_iRows );
		for ( l_iCtr = 0; l_iCtr < r; l_iCtr++ ) * f_ppoBody [ l_iCtr ] <= a_roMatrix [ l_iCtr ];
		}
	return ( *this );
	M_EPILOG
	}
	
HMatrix HMatrix::operator + ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	if ( ( f_iRows == a_roMatrix.f_iRows )
			&& ( f_iColumns == a_roMatrix.f_iColumns ) )
		{
		HMatrix l_oMatrix( f_iRows, f_iColumns );
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
			l_oMatrix [ l_iCtr ] = ( * f_ppoBody[ l_iCtr ] + a_roMatrix [ l_iCtr ] );
		return ( l_oMatrix );
		}
	return ( *this );
	M_EPILOG
	}
	
HMatrix HMatrix::operator - ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	if ( ( f_iRows == a_roMatrix.f_iRows )
			&& ( f_iColumns == a_roMatrix.f_iColumns ) )
		{
		HMatrix l_oMatrix ( f_iRows, f_iColumns );
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
			{
			l_oMatrix [ l_iCtr ] = ( *f_ppoBody [ l_iCtr ] - a_roMatrix [ l_iCtr ] );
			}
		return ( l_oMatrix );
		}
	return ( *this );
	M_EPILOG
	}
	
HMatrix HMatrix::operator * ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	double d;
	int l_iCtr, j, k;
	if ( f_iColumns == a_roMatrix.f_iRows )
		{
		HMatrix l_oMatrix ( f_iRows, a_roMatrix.f_iColumns );
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) 
			for ( j = 0; j < a_roMatrix.f_iColumns; j++ )
				{
				d = 0;
				for ( k = 0; k < f_iColumns; k++ ) 
					d += ( ( *( f_ppoBody [ l_iCtr ] ) ) [ k ] * a_roMatrix [ k ][ j ] );
				l_oMatrix [ l_iCtr ][ j ] = d;
				}
		return ( l_oMatrix );
		}
	return ( *this );
	M_EPILOG
	}
	
HMatrix HMatrix::operator * ( double d )
	{
	M_PROLOG
	int l_iCtr;
	HMatrix l_oMatrix ( f_iRows, f_iColumns );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		l_oMatrix [ l_iCtr ] = ( * f_ppoBody [ l_iCtr ] * d );
	return ( l_oMatrix );
	M_EPILOG
	}
	
HMatrix HMatrix::operator / ( double d )
	{
	M_PROLOG
	int l_iCtr;
	HMatrix l_oMatrix ( f_iRows, f_iColumns );
	if ( d ) for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
			l_oMatrix [ l_iCtr ] = ( * f_ppoBody [ l_iCtr ] / d );
	return ( l_oMatrix );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator += ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	if ( ( f_iRows == a_roMatrix.f_iRows ) 
			&& ( f_iColumns == a_roMatrix.f_iColumns ) ) 
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) * f_ppoBody [ l_iCtr ] += a_roMatrix[ l_iCtr ];
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator -= ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	if ( ( f_iRows == a_roMatrix.f_iRows ) 
			&& ( f_iColumns == a_roMatrix.f_iColumns ) ) 
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) * f_ppoBody [ l_iCtr ] -= a_roMatrix [ l_iCtr ];
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator *= ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	double d;
	int l_iCtr, j, k;
	if ( f_iColumns == a_roMatrix.f_iRows == a_roMatrix.f_iColumns )
		{
		HMatrix l_oMatrix ( f_iRows, a_roMatrix.f_iColumns );
		for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
			for ( j = 0; j < a_roMatrix.f_iColumns; j++ )
				{
				d = 0;
				for ( k = 0; k < f_iColumns; k++ ) 
					d += ( * f_ppoBody [ l_iCtr ] ) [ k ] * a_roMatrix [ k ][ j ];
				l_oMatrix [ l_iCtr ][ j ] = d;
				}
		*this = l_oMatrix;
		}
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator *= ( double d )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) ( * f_ppoBody [ l_iCtr ] *= d );
	return ( *this );
	M_EPILOG
	}
	
HMatrix & HMatrix::operator /= ( double d )
	{
	M_PROLOG
	int l_iCtr;
	if ( d ) for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) ( * f_ppoBody [ l_iCtr ] /= d );
	return ( *this );
	M_EPILOG
	}
	
HMatrix HMatrix::operator ~ ( void )
	{
	M_PROLOG
	int l_iCtr;
	HMatrix l_oMatrix ( f_iRows, f_iColumns );
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ )
		{
		l_oMatrix [ l_iCtr ] = ~ * f_ppoBody [ l_iCtr ];
		}
	return ( l_oMatrix );
	M_EPILOG
	}
	
double HMatrix::operator ! ( void )
	{
	M_PROLOG
	return ( det() );
	M_EPILOG
	}
	
int HMatrix::operator == ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	for ( l_iCtr = 0; l_iCtr < f_iRows; l_iCtr++ ) 
		if ( *f_ppoBody [ l_iCtr ] != a_roMatrix [ l_iCtr ] ) return ( 0 );
	return ( 1 );
	M_EPILOG
	}
	
int HMatrix::operator != ( HMatrix & a_roMatrix )
	{
	M_PROLOG
	return ( 1 -( *this == a_roMatrix ) );
	M_EPILOG
	}
	
HVector operator * ( HVector & a_roVector, HMatrix & a_roMatrix )
	{
	M_PROLOG
	double d;
	int j, k;
	if ( a_roVector.dim() == a_roMatrix.f_iRows )
		{
		HVector l_oVector( a_roMatrix.f_iColumns );
		for ( j = 0; j < a_roMatrix.f_iColumns; j++ )
			{
			d = 0;
			for ( k = 0; k < a_roMatrix.f_iColumns; k++ ) 
				d += a_roVector[ k ] * a_roMatrix [ k ][ j ];
			l_oVector[ j ] = d;
			}
		return ( l_oVector );
		}
	return ( a_roVector );
	M_EPILOG
	}
	
HVector operator*=( HVector & a_roVector, HMatrix & a_roMatrix )
	{
	M_PROLOG
	if ( a_roVector.dim() == a_roMatrix.f_iRows == a_roMatrix.f_iColumns )
		{
		HVector l_oVector = a_roVector * a_roMatrix;
		a_roVector = l_oVector;
		}
	return ( a_roVector );
	M_EPILOG
	}
	
HMatrix operator*( double d, HMatrix & a_roMatrix )
	{
	M_PROLOG
	int l_iCtr;
	HMatrix l_oMatrix( a_roMatrix.f_iRows, a_roMatrix.f_iColumns );
	for ( l_iCtr = 0; l_iCtr < a_roMatrix.row(); l_iCtr++ )
		{
		l_oMatrix [ l_iCtr ] = ( a_roMatrix [ l_iCtr ] * d );
		}
	return ( l_oMatrix );
	M_EPILOG
	}
	
HVector T( HMatrix & a_roMatrix, HVector & a_roVector )
	{
	M_PROLOG
	HMatrix l_oMatrix = a_roMatrix.T();
	return ( a_roVector * l_oMatrix );
	M_EPILOG
	}
	
