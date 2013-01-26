/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hnumber.cxx - this file is integral part of `yaal' project.

  i.  You may not make any changes in Copyright information.
  ii. You must attach Copyright information to any part of every copy
      of this software.

Copyright:

 You can use this software free of charge and you can redistribute its binary
 package freely but:
  1. You are not allowed to use any part of sources of this software.
  2. You are not allowed to redistribute any part of sources of this software.
  3. You are not allowed to reverse engineer this software.
  4. If you want to distribute a binary package of this software you cannot
     demand any fees for it. You cannot even demand
     a return of cost of the media or distribution (CD for example).
  5. You cannot involve this software in any commercial activity (for example
     as a free add-on to paid software or newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#include <cstring>
#include <cstdio>
#include <cmath>

#include "base.hxx"
M_VCSID( "$Id: "__ID__" $" )
M_VCSID( "$Id: "__TID__" $" )
#include "algorithm.hxx"
#include "hnumber.hxx"
#include "hfile.hxx"

/*
 * Naming conventions:
 *
 *   addend + addend = sum
 *   (or alternative form)
 *   augend + addend = sum
 *
 *   minuend - subtrahend = difference
 *
 *   multiplicand * factor = product
 *   (or alternative form)
 *   multiplicand * multiplier = product
 *
 *   dividend / divisor = quotient
 *
 *   dividend % divisor = remainder
 */

namespace yaal {

namespace hcore {

namespace {
int const DECIMAL_DIGITS_IN_LEAF_CONST = 1;
int const SPECIAL_CHARS = 3; /* minus, dot, nil */
char const VALID_CHARACTERS[] = "-.0123456789";
int const JUST_DIGITS = 2;
int const HARDCODED_MINIMUM_PRECISION = 16;
int const A_MINUS = 0;
int const A_DOT = 1;
int const A_ZERO = 2;
int const NUMBER_START = 3;
int const KARATSUBA_THRESHOLD = 20; /* FIXME: 20 is fine */
i32_t const DECIMAL_SHIFT[] = {
	1l,
	10l,
	100l,
	1000l,
	10000l,
	100000l,
	1000000l,
	10000000l,
	100000000l,
	1000000000l
};
i32_t const LEAF = meta::power<10, DECIMAL_DIGITS_IN_LEAF_CONST>::value;
i32_t const LEAF_SQ = static_cast<i32_t>( ::sqrt( LEAF ) );
char ZFORMAT[] = "%00u";

char unused = ZFORMAT[2] = static_cast<char>( '0' + DECIMAL_DIGITS_IN_LEAF_CONST );

inline i32_t leafcmp( i32_t const* left_, i32_t const* right_, int long len_ ) {
	i32_t cmp( 0 );
	for ( int long i( 0 ); ! cmp && ( i < len_ ); ++ i )
		cmp = left_[ i ] - right_[ i ];
	return ( cmp );
}

}

int const HNumber::DECIMAL_DIGITS_IN_LEAF = DECIMAL_DIGITS_IN_LEAF_CONST;
int HNumber::DEFAULT_PRECISION = 100;

HNumber::HNumber( void )
	: _precision( DEFAULT_PRECISION > HARDCODED_MINIMUM_PRECISION
			? DEFAULT_PRECISION : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	return;
	M_EPILOG
}

HNumber::~HNumber( void ) {
	M_PROLOG
	return;
	M_EPILOG
}

HNumber::HNumber( double long number_ )
	: _precision( DEFAULT_PRECISION > HARDCODED_MINIMUM_PRECISION
			? DEFAULT_PRECISION : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_double( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( double long number_, int precision_ )
	: _precision( precision_ > HARDCODED_MINIMUM_PRECISION
			? precision_ : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_double( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( char const* const number_ )
	: _precision( DEFAULT_PRECISION > HARDCODED_MINIMUM_PRECISION
			? DEFAULT_PRECISION : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_string( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( char const* const number_, int precision_ )
	: _precision( precision_ > HARDCODED_MINIMUM_PRECISION
			? precision_ : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_string( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( HString const& number_ )
	: _precision( DEFAULT_PRECISION > HARDCODED_MINIMUM_PRECISION
			? DEFAULT_PRECISION : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_string( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( HString const& number_, int precision_ )
	: _precision( precision_ > HARDCODED_MINIMUM_PRECISION
			? precision_ : HARDCODED_MINIMUM_PRECISION ),
	_leafCount( 0 ), _integralPartSize( 0 ),
	_canonical(), _cache(), _negative( false ) {
	M_PROLOG
	from_string( number_ );
	return;
	M_EPILOG
}

HNumber::HNumber( HNumber const& source )
	: _precision( source._precision ),
	_leafCount( source._leafCount ),
	_integralPartSize( source._integralPartSize ),
	_canonical(), _cache(),
	_negative( source._negative ) {
	M_PROLOG
	if ( source._leafCount ) {
		_canonical.realloc( chunk_size<i32_t>( source._leafCount ), HChunk::STRATEGY::EXACT );
		::memcpy( _canonical.raw(), source._canonical.raw(), chunk_size<i32_t>( source._leafCount ) );
		if ( source._cache.get_size() > 0 ) {
			_cache.realloc( source._cache.get_size(), HChunk::STRATEGY::EXACT );
			::memcpy( _cache.raw(), source._cache.raw(), source._cache.get_size() );
		}
	}
	return;
	M_EPILOG
}

HNumber& HNumber::operator = ( HNumber const& source ) {
	M_PROLOG
	if ( &source != this ) {
		HNumber tmp( source );
		swap( tmp );
	}
	return ( *this );
	M_EPILOG
}

void HNumber::swap( HNumber& other ) {
	M_PROLOG
	if ( &other != this ) {
		using yaal::swap;
		swap( _precision, other._precision );
		swap( _leafCount, other._leafCount );
		swap( _integralPartSize, other._integralPartSize );
		swap( _canonical, other._canonical );
		swap( _cache, other._cache );
		swap( _negative, other._negative );
	}
	return;
	M_EPILOG
}

void HNumber::from_double( double long number_ ) {
	M_PROLOG
	HString source( number_ );
	from_string( source );
	return;
	M_EPILOG
}

void HNumber::from_string( HString const& number_ ) {
	M_PROLOG
	/* ! - represent known but invalid character, ? - represent unknown character */
	int long start( number_.find_one_of( VALID_CHARACTERS ) );
	M_ENSURE( start != HString::npos ); /* exclude "!!!!" */
	char const* const src = number_.raw();
	_negative = ( src[ start ] == VALID_CHARACTERS[ A_MINUS ] ); /* "!!!-???" */
	if ( _negative )
		++ start;
	int long len( number_.get_length() );
	M_ENSURE( start < len ); /* exclude "!!-" */
	M_ENSURE( number_.find_one_of( VALID_CHARACTERS + A_DOT, start ) == start ); /* exclude "--" and "-!!" */
	int long idx( number_.find_other_than( "0", start ) ); /* skip leading 0s */
	int long end( start + 1 );
	_integralPartSize = 0;
	_leafCount = 0;
	if ( idx != HString::npos ) do /* "!!![-][.1-9]???" or "000." */ {
		int long firstValid( start );
		start = idx;
		int long dot( number_.find( VALID_CHARACTERS[ A_DOT ], start ) );
		idx = number_.find_other_than( VALID_CHARACTERS + A_DOT, start );
		if ( ( idx != HString::npos ) && ( idx < dot ) ) /* "!!232.!!!" */
			dot = HString::npos;
		int long digit( number_.find_one_of( VALID_CHARACTERS + A_ZERO, start ) );
		if ( ( digit == HString::npos ) && ( firstValid < start ) )
			break;
		M_ENSURE( digit != HString::npos ); /* must have digit */
		M_ENSURE( ( digit - start ) <= 1 ); /* exclude "-..!!" and "..!!" */
		end = number_.find_other_than( VALID_CHARACTERS + ( dot >= 0 ? A_ZERO : A_DOT ), dot >= 0 ? dot + 1 : start );
		( end != HString::npos ) || ( end = len );
		if ( dot != HString::npos ) {
			idx = number_.reverse_find_other_than( "0", len - end );
			end = ( idx != HString::npos ) ? len - idx : start + 1;
		}
		M_ASSERT( ( dot == HString::npos ) || ( ( end - dot ) > 0 ) );
		_integralPartSize = ( dot != HString::npos ? ( ( dot - start ) + ( DECIMAL_DIGITS_IN_LEAF_CONST - 1 ) ) / DECIMAL_DIGITS_IN_LEAF_CONST : ( ( end - start ) + ( DECIMAL_DIGITS_IN_LEAF_CONST - 1 ) ) / DECIMAL_DIGITS_IN_LEAF_CONST );
		int long fractionalPart( dot != HString::npos ? ( ( end - ( dot + 1 ) ) + ( DECIMAL_DIGITS_IN_LEAF_CONST - 1 ) ) / DECIMAL_DIGITS_IN_LEAF_CONST : 0 );
		_leafCount = _integralPartSize + fractionalPart;
		if ( _leafCount > 0 )
			_canonical.realloc( chunk_size<i32_t>( _leafCount ) );
		i32_t* dst( _canonical.get<i32_t>() );
		i32_t leaf( 0 );
		int digitInLeaf( 0 );
		if ( dot != HString::npos ) /* scan fractional part */ {
			idx = _integralPartSize;
			for ( int long i( dot + 1 ); i < end; ++ i, ++ digitInLeaf ) {
				M_ASSERT( src[ i ] >= VALID_CHARACTERS[ A_ZERO ] );
				if ( digitInLeaf == DECIMAL_DIGITS_IN_LEAF_CONST ) {
					dst[idx ++] = leaf;
					digitInLeaf = 0;
					leaf = 0;
				}
				leaf += ( ( src[ i ] - VALID_CHARACTERS[ A_ZERO ] ) * DECIMAL_SHIFT[ ( DECIMAL_DIGITS_IN_LEAF_CONST - digitInLeaf ) - 1 ] );
			}
			if ( idx < _leafCount )
				dst[idx] = leaf;
		}
		if ( _integralPartSize > 0 ) {
			idx = _integralPartSize - 1;
			leaf = 0;
			digitInLeaf = 0;
			for ( int long i( ( dot != HString::npos ? dot : end ) - 1 ); i >= start; -- i, ++ digitInLeaf ) {
				M_ASSERT( src[ i ] >= VALID_CHARACTERS[ A_ZERO ] );
				if ( digitInLeaf == DECIMAL_DIGITS_IN_LEAF_CONST ) {
					M_ASSERT( idx >= 0 );
					dst[idx --] = leaf;
					digitInLeaf = 0;
					leaf = 0;
				}
				leaf += ( ( src[ i ] - VALID_CHARACTERS[ A_ZERO ] ) * DECIMAL_SHIFT[ digitInLeaf ] );
			}
			if ( idx >= 0 )
				dst[idx] = leaf;
		}
		if ( dot == HString::npos )
			_integralPartSize = _leafCount;
		if ( fractional_length() >= _precision )
			_precision = fractional_length() + 1;
	} while ( 0 );
	if ( _leafCount == 0 )
		_negative = false;
	return;
	M_EPILOG
}

HString HNumber::to_string( void ) const {
	M_PROLOG
	i32_t const* const src( _canonical.get<i32_t>() );
	_cache.realloc( _leafCount * DECIMAL_DIGITS_IN_LEAF_CONST + 3 ); /* + 1 for '.', + 1 for '-' and + 1 for terminating NIL */
	char* ptr( _cache.get<char>() );
	if ( _negative )
		*ptr ++ = VALID_CHARACTERS[ A_MINUS ];
	int leaf( 0 );
	for ( ; leaf < _integralPartSize; ++ leaf )
		ptr += snprintf( ptr, DECIMAL_DIGITS_IN_LEAF_CONST + 1, leaf ? ZFORMAT : "%u", src[ leaf ] ); /* + 1 for terminating NIL */
	if ( _leafCount > _integralPartSize )
		*ptr ++ = VALID_CHARACTERS[ A_DOT ];
	for ( ; leaf < _leafCount; ++ leaf )
		ptr += snprintf( ptr, DECIMAL_DIGITS_IN_LEAF_CONST + 1, ZFORMAT, src[ leaf ] );
	if ( ! _leafCount )
		*ptr ++ = '0';
	else if ( _leafCount > _integralPartSize ) {
		while ( *( ptr - 1 ) == '0' )
			-- ptr;
	}
	*ptr = 0;
	char const* s( _cache.get<char>() );
	HString str( s, ptr - s );
	return ( str );
	M_EPILOG
}

double long HNumber::to_double( void ) const {
	M_PROLOG
	return ( lexical_cast<double>( to_string() ) );
	M_EPILOG
}

int long HNumber::get_precision( void ) const {
	return ( _precision );
}

void HNumber::set_precision( int long precision_ ) {
	M_PROLOG
	if ( ( precision_ >= HARDCODED_MINIMUM_PRECISION )
			&& ( ( precision_ <= _precision ) || ( fractional_length() < _precision ) ) )
		_precision = precision_;
	if ( ( _integralPartSize + _precision ) < _leafCount )
		_leafCount = _integralPartSize + _precision;
	return;
	M_EPILOG
}

int long HNumber::fractional_length( void ) const {
	return ( _leafCount - _integralPartSize );
}

bool HNumber::is_exact( void ) const {
	M_ASSERT( ( _leafCount - _integralPartSize ) <= _precision );
	return ( ( _leafCount - _integralPartSize ) < _precision );
}

int long HNumber::absolute_lower( HNumber const& other ) const {
	i32_t const* p1( _canonical.get<i32_t>() );
	i32_t const* p2( other._canonical.get<i32_t>() );
	int long cmp( 1 );
	if ( _integralPartSize < other._integralPartSize )
		cmp = -1;
	else if ( _integralPartSize == other._integralPartSize ) {
		int long len = min( _leafCount, other._leafCount );
		cmp = leafcmp( p1, p2, len );
		if ( ! cmp )
			cmp = _leafCount - other._leafCount;
	}
	return ( cmp );
}

bool HNumber::operator == ( HNumber const& other ) const {
	return ( ( _negative == other._negative )
			&& ( _leafCount == other._leafCount )
			&& ( _integralPartSize == other._integralPartSize )
			&& ! ::memcmp( _canonical.raw(), other._canonical.raw(), chunk_size<i32_t>( _leafCount ) ) );
}

bool HNumber::operator != ( HNumber const& other ) const {
	return ( ! operator == ( other ) );
}

bool HNumber::operator < ( HNumber const& other ) const {
	bool lower = false;
	if ( _negative && ! other._negative )
		lower = true;
	else if ( ( _negative && other._negative ) || ( ! ( _negative || other._negative ) ) ) {
		int long cmp = absolute_lower( other );
		lower = cmp < 0;
		if ( cmp && _negative && other._negative )
			lower = ! lower;
	}
	return ( lower );
}

bool HNumber::operator > ( HNumber const& other ) const {
	return ( other < *this );
}

bool HNumber::operator <= ( HNumber const& other ) const {
	return ( ( operator == ( other ) ) || ( operator < ( other ) ) );
}

bool HNumber::operator >= ( HNumber const& other ) const {
	return ( other <= *this );
}

bool HNumber::mutate_addition( i32_t* res, int long ressize,
		i32_t const* const ep[], int long* lm, int long* rm, bool sub, bool swp ) const {
	i32_t e[ 2 ];
	int long lmx[] = { lm ? ( swp ? lm[ 1 ] : lm[ 0 ] ) : 0, lm ? ( swp ? lm[ 0 ] : lm[ 1 ] ) : 0 };
	int long rmx[] = { rm ? ( swp ? rm[ 1 ] : rm[ 0 ] ) : 0, rm ? ( swp ? rm[ 0 ] : rm[ 1 ] ) : 0 };
	i32_t const* const epx[] = {
		swp ? ep[ 1 ] - lmx[ 0 ] - 1 : ep[ 0 ] - lmx[ 0 ] - 1,
		swp ? ep[ 0 ] - lmx[ 1 ] - 1 : ep[ 1 ] - lmx[ 1 ] - 1
	};
	int long idx( ressize - 1 ); /* index of first processed leaf */
	int side( ( rmx[ 0 ] > rmx[ 1 ] ) ? 1 : 0 );
	int long off( rmx[ 1 - side ] );
	i32_t const* src( epx[ side ] );
	e[ 1 - side ] = 0;
	i32_t carrier( 0 );
	while ( off -- && ( idx > 0 ) ) {
		e[ side ] = src[ idx ];
		if ( sub ) {
			i32_t x( e[ 0 ] - carrier );
			if ( x < e[ 1 ] ) {
				x += LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x - e[ 1 ];
		} else {
		 	i32_t x( e[ 0 ] + e[ 1 ] + carrier );
			if ( x >= LEAF ) {
				x -= LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x;
		}
		-- idx;
	}
	side = ( lmx[ 0 ] > lmx[ 1 ] ) ? 1 : 0;
	off = lmx[ 1 - side ];
	while ( idx > off ) {
		if ( sub ) {
			i32_t x( epx[ 0 ][ idx ] - carrier );
			if ( x < epx[ 1 ][ idx ] ) {
				x += LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x - epx[ 1 ][ idx ];
		} else {
			i32_t x( epx[ 0 ][ idx ] + epx[ 1 ][ idx ] + carrier );
			if ( x >= LEAF ) {
				x -= LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x;
		}
		-- idx;
	}
	src = epx[ side ];
	e[ 1 - side ] = 0;
	while ( idx > 0 ) {
		e[ side ] = src[ idx ];
		if ( sub ) {
			i32_t x( e[ 0 ] - carrier );
			if ( x < e[ 1 ] ) {
				x += LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x - e[ 1 ];
		} else {
			i32_t x( e[ 0 ] + e[ 1 ] + carrier );
			if ( x >= LEAF ) {
				x -= LEAF;
				carrier = 1;
			} else
				carrier = 0;
			res[ idx ] = x;
		}
		-- idx;
	}
	if ( carrier && ! sub )
		++ res[ 0 ];
	return ( carrier ? true : false );
}

HNumber HNumber::operator + ( HNumber const& addend_ ) const {
	M_PROLOG
	HNumber n( *this );
	n += addend_;
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator += ( HNumber const& addend_ ) {
	M_PROLOG
	int long ips = max( _integralPartSize, addend_._integralPartSize );
	int long dps = max( fractional_length(), addend_.fractional_length() );
	if ( fractional_length() < addend_.fractional_length() )
		_precision = is_exact() ? addend_._precision : _precision;
	else
		_precision = addend_.is_exact() ? _precision : addend_._precision;
	( dps <= _precision ) || ( dps = _precision );
	int long ressize = ips + dps + 1; /* + 1 for possible carrier */
	_cache.realloc( chunk_size<i32_t>( ressize ) );
	::memset( _cache.raw(), 0, _cache.get_size() );
	i32_t* res( _cache.get<i32_t>() );
	i32_t const* ep1( _canonical.get<i32_t>() );
	i32_t const* ep2( addend_._canonical.get<i32_t>() );
	int long lm[] = { ips - _integralPartSize, ips - addend_._integralPartSize };
	int long rm[] = { dps - fractional_length(), dps - addend_.fractional_length() };
	( rm[ 0 ] >= 0 ) || ( rm[ 0 ] = 0 );
	( rm[ 1 ] >= 0 ) || ( rm[ 1 ] = 0 );
	i32_t const* ep[] = { ep1, ep2 };
	bool sub = ( ( _negative && ! addend_._negative ) || ( ! _negative && addend_._negative ) );
	bool swp = sub && ( absolute_lower( addend_ ) < 0 );
	mutate_addition( res, ressize, ep, lm, rm, sub, swp );
	_integralPartSize = ips;
	if ( ressize > 0 ) {
		_negative = sub ? ( _negative ? ! swp : swp ) : ( _negative && addend_._negative );
		++ _integralPartSize;
	}
	_leafCount = ressize;
	_canonical.swap( _cache );
	normalize();
	if ( _leafCount == 0 )
		_negative = false;
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator - ( HNumber const& subtrahend_ ) const {
	M_PROLOG
	HNumber n( *this );
	n -= subtrahend_;
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator -= ( HNumber const& subtrahend_ ) {
	M_PROLOG
	_negative = ! _negative;
	if ( ! _leafCount )
		_negative = false;
	operator += ( subtrahend_ );
	_negative = ! _negative;
	if ( ! _leafCount )
		_negative = false;
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator * ( HNumber const& factor_ ) const {
	M_PROLOG
	HNumber n( *this );
	n *= factor_;
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator *= ( HNumber const& factor_ ) {
	M_PROLOG
	HNumber const& n( factor_._leafCount < _leafCount ? factor_ : *this );
	HNumber const& factor( factor_._leafCount < _leafCount ? *this : factor_ );
	if ( n._leafCount && factor._leafCount ) {
		::memset( _cache.raw(), 0, _cache.get_size() );
		karatsuba( _cache,
				n._canonical.get<i32_t>(), n._leafCount,
				factor._canonical.get<i32_t>(), factor._leafCount );
		_leafCount += factor_._leafCount;
		_integralPartSize += factor_._integralPartSize;
		_canonical.swap( _cache );
		normalize();
		_negative = ! ( ( n._negative && factor._negative ) || ! ( n._negative || factor._negative ) );
	} else {
		_leafCount = _integralPartSize = 0;
		_negative = false;
	}
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator / ( HNumber const& divisor_ ) const {
	M_PROLOG
	HNumber n( *this );
	n /= divisor_;
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator /= ( HNumber const& divisor_ ) {
	M_PROLOG
	M_ENSURE( divisor_._leafCount > 0 );
	if ( _leafCount ) {
		/*
		 * We use "long division" pen and paper algorithm.
		 *
		 * Eamples of pecial cases that have to be handled.
		 * (each digit is one leaf)
		 *
		 * 1 / 3
		 * 1 / 13
		 * 10 / 3
		 * 10 / 13
		 * 10 / 13131313
		 * 0.1 / 3
		 * 0.1 / 13
		 * 0.1 / 131313
		 * 130000 / 13
		 * 0.0001 / 3
		 * 131313 / 13
		 * 131313.131313 / 13
		 */
		/*
		 * Find proper part of divisor, i.e.:
		 * .00123 -> 123
		 * 123000 -> 123
		 */
		i32_t const* divisor( divisor_._canonical.get<i32_t>() );
		int long divisorLeafCount( divisor_._leafCount );
		int long lshift( 0 );
		while ( ! divisor[ lshift ] )
			++ lshift;
		divisor += lshift;
		divisorLeafCount -= lshift;
		int long rshift( 0 );
		while ( ! divisor[ divisorLeafCount - 1 ] ) {
			-- divisorLeafCount;
			++ rshift;
		}
		M_ASSERT( ( divisorLeafCount > 0 ) && ! ( lshift && rshift ) );
		/*
		 * Pre-allocate memory for result.
		 */
		_cache.realloc( chunk_size<i32_t>( _leafCount ) );
		/*
		 * We need memory buffer for dividend sample,
		 * and another one for multiplier sample.
		 * Assuming divisor has n leafs, a multiplier sample's buffer must
		 * be able to accomodate n + 1 leafs ( + 1 for possible carrier).
		 */
		HChunk buffer( chunk_size<i32_t>( divisorLeafCount * 2 + 1 ) );
		i32_t* multiplierSample( buffer.get<i32_t>() );
		i32_t* dividendSample( buffer.get<i32_t>() + divisorLeafCount + 1 );
		i32_t* dividend( _canonical.get<i32_t>() );
		int long precision( ( _precision + DECIMAL_DIGITS_IN_LEAF_CONST - 1 ) / DECIMAL_DIGITS_IN_LEAF_CONST );
		int long dividendLeafNo( min( _leafCount, divisorLeafCount ) ); /* Index of the next leaf to process */
		::memcpy( dividendSample, dividend, chunk_size<i32_t>( dividendLeafNo ) );
		/*
		 * Number of leafs in quotient after leaf point.
		 * Variable used to stop calculations based on maximum precision.
		 */
		int long fractionalLeafs( 0 );
		int long quotientLeafNo( 0 ); /* Index of currently guessed quotient leaf. */
		do {
			/*
			 * Using binary search algorithm we have to guess next leaf value.
			 * Assuming that "correctLeaf" is proper next leaf value to be guessed,
			 * following relation holds:
			 * leafHi > correctLeaf >= leafLow
			 */
			i32_t leafHi( LEAF ); /* binary search algo upper bound  */
			i32_t leaf( LEAF_SQ ); /* guessed leaf */
			i32_t leafLow( 0 ); /* binary search algo lower bound */

			/*
			 * Fix possible underflow in dividendSample.
			 */
			int long shift( 0 );
			while ( ! dividendSample[ shift ] )
				++ shift;
			int long divSampleLen( divisorLeafCount - shift );
			if ( shift > 0 ) {
				_cache.realloc( chunk_size<i32_t>( quotientLeafNo + shift ) );
				::memset( _cache.get<i32_t>() + quotientLeafNo, 0, chunk_size<i32_t>( shift ) );
				quotientLeafNo += shift;
				::memmove( dividendSample, dividendSample + shift, chunk_size<i32_t>( divSampleLen ) );
			}
			/*
			 * Compensate for missing leafs.
			 */
			if ( ( shift > 0 ) && ( dividendLeafNo < _leafCount ) ) {
				int long compensationLen( min( _leafCount - dividendLeafNo, divisorLeafCount - divSampleLen ) );
				::memcpy( dividendSample + divSampleLen, dividend + dividendLeafNo, chunk_size<i32_t>( compensationLen ) );
				divSampleLen += compensationLen;
			}

			/*
			 * Fill rest of dividendSample with 0.
			 */
			if ( divSampleLen < divisorLeafCount ) {
				::memset( dividendSample + divSampleLen, 0, chunk_size<i32_t>( divisorLeafCount - divSampleLen ) );
				divSampleLen = divisorLeafCount;
			}

			/*
			 * If dividendSample is still not enough to produce next quotient leaf
			 * then fetch next leaf from divident to dividendSample.
			 */
			if ( leafcmp( dividendSample, divisor, divSampleLen ) < 0 ) /* We establish that divisor_ <= multiplierSample. */ {
				dividendSample[divSampleLen] = dividendLeafNo < _leafCount ? dividend[dividendLeafNo] : 0;
				++ divSampleLen;
				++ dividendLeafNo;
				if ( quotientLeafNo > 0 ) {
					_cache.realloc( chunk_size<i32_t>( quotientLeafNo + 1 ) );
					_cache.get<i32_t>()[ quotientLeafNo ++ ] = 0;
				}
			}
			leafLow = 1;
			/* Binary search loop. */
			while ( leafHi > ( leaf + 1 ) ) {
				/*
				 * Multiply current guess about current leaf by divisor.
				 */
				int long multiplierLeafCount( karatsuba( buffer, &leaf, 1, divisor, divisorLeafCount ) );
				/*
				 * If product has more leafs than divisor than it is bigger.
				 */
				i32_t cmp( multiplierLeafCount > divisorLeafCount ? 1 : 0 );
				if ( ! cmp )
					cmp = leafcmp( multiplierSample + 1, dividendSample, divSampleLen );
				if ( cmp > 0 )
					leafHi = leaf;
				else if ( cmp < 0 )
					leafLow = leaf;
				else
					break;
				leaf = ( leafHi + leafLow ) / 2;
			}
			/*
			 * A helper for mutate_addition.
			 * The substraction is done `in place'.
			 */
			i32_t const* ep[] = { dividendSample, divisor };
			mutate_addition( dividendSample + 1, divisorLeafCount + 1 + 1, ep, NULL, NULL, true, false );
			_cache.realloc( chunk_size<i32_t>( quotientLeafNo + 1 ) );
			_cache.get<i32_t>()[ quotientLeafNo ++ ] = leaf;

		} while ( fractionalLeafs < precision );
		_canonical.swap( _cache );
		_integralPartSize -= divisor_._integralPartSize;
		_leafCount = quotientLeafNo;
		_negative = ! ( ( _negative && divisor_._negative ) || ! ( _negative || divisor_._negative ) );
	}
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator - ( void ) const {
	M_PROLOG
	HNumber n( *this );
	if ( _leafCount )
		n._negative = ! n._negative;
	return ( n );
	M_EPILOG
}

HNumber HNumber::operator ^ ( int long unsigned exp ) const {
	M_PROLOG
	HNumber n( *this );
	n ^= exp;
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator ^= ( int long unsigned exp ) {
	M_PROLOG
	int long unsigned p( exp >> 1 );
	HNumber n( *this );
	if ( p > 2 )
		operator ^= ( p );
	else
		while ( -- p )
			operator *= ( n );
	operator *= ( *this );
	if ( exp % 2 )
		operator *= ( n );
	return ( *this );
	M_EPILOG
}

HNumber& HNumber::operator ++ ( void ) {
	M_PROLOG
	operator += ( 1 );
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator ++ ( int ) {
	M_PROLOG
	HNumber n( *this );
	operator ++ ();
	return ( n );
	M_EPILOG
}

HNumber& HNumber::operator -- ( void ) {
	M_PROLOG
	operator -= ( 1 );
	return ( *this );
	M_EPILOG
}

HNumber HNumber::operator -- ( int ) {
	M_PROLOG
	HNumber n( *this );
	operator -- ();
	return ( n );
	M_EPILOG
}

void HNumber::normalize( void ) {
	M_PROLOG
	i32_t* res( _canonical.get<i32_t>() );
	int shift( 0 );
	while ( ( shift < _integralPartSize ) && ( res[ shift ] == 0 ) )
		++ shift;
	if ( shift ) {
		_integralPartSize -= shift;
		_leafCount -= shift;
		::memmove( res, res + shift, chunk_size<i32_t>( _leafCount ) );
	}
	while ( ( fractional_length() > 0 ) && ( res[ _leafCount - 1 ] == 0 ) )
		-- _leafCount;
	if ( _leafCount == ( _integralPartSize + _precision ) )
		++ _precision;
	return;
	M_EPILOG
}

int long HNumber::karatsuba( HChunk& result, i32_t const* fx, int long fxs, i32_t const* fy, int long fys ) {
	int long shift( 0 );
	while ( ( shift < fxs ) && ! fx[ shift ] )
		++ shift;
	int long fxrl( fxs - shift );
	int long totalShift( shift );
	shift = 0;
	while ( ( shift < fys ) && ! fy[ shift ] )
		++ shift;
	int long fyrl( fys - shift );
	totalShift += shift;
	int long leafCount( 0 );
	if ( ( ( fxrl > KARATSUBA_THRESHOLD ) || ( fyrl > KARATSUBA_THRESHOLD ) ) && ( fxrl > 0 ) && ( fyrl > 0 ) ) {
		fx += ( fxs - fxrl );
		fy += ( fys - fyrl );
		fxs = fxrl;
		fys = fyrl;
		result.realloc( chunk_size<i32_t>( leafCount = fxs + fys + totalShift ) );

		int long fs( max( fxs, fys ) );
		int long fl( min( fxs, fys ) );
		int long const m( ( fs / 2 ) + ( fs & 1 ) ); /* Size of upper/lower half of number */
		HChunk r2m; /* intermediate result ( fx1 * fx2 ( * B ^ 2m ) ) + 1 for carrier */
		int long const r2ms( karatsuba( r2m, fx, fxs - m, fy, fys - m ) );
		HChunk r; /* intermediate result ( fx2 * fy2 ) + 1 for carrier */
		int long const rs( karatsuba( r, fx + ( fxs > m ? fxs - m : 0 ), min( fxs, m ), fy + ( fys > m ? fys - m : 0 ), min( fys, m ) ) );
		int long const size( fxs + fys + 1 ); 
		HChunk buffer( chunk_size<i32_t>( size + ( m + 1 ) * 2 ) ); /* + 1 for carrier */
		i32_t* hx( buffer.get<i32_t>() + size );
		i32_t* hy( buffer.get<i32_t>() + size + m + 1 );
		/* preparation of hx and hy */
		/* hx = fx / B^m + fx % B^m */
		int long lm[] = { 2 * m - fxs, 0 };
		i32_t const* ep[] = { fx, fx + fxs - m };
		if ( fxs > m )
			mutate_addition( hx, m + 1, ep, lm, NULL, false, false );
		else
			::memcpy( hx + m + 1 - fxs, fx, chunk_size<i32_t>( fxs ) );
		/* hy */
		lm[ 0 ] = 2 * m - fys;
		ep[ 0 ] = fy;
		ep[ 1 ] = fy + fys - m;
		if ( fys > m )
			mutate_addition( hy, m + 1, ep, lm, NULL, false, false );
		else
			::memcpy( hy + m + 1 - fys, fy, chunk_size<i32_t>( fys ) );
		/* find Z */
		HChunk Z;
		int long Zs( karatsuba( Z, hx, m + 1, hy, m + 1 ) );
		/* combine all results */
		
		i32_t* const res( buffer.get<i32_t>() );

		/* res = Z*B^m + r */
		i32_t const* p( Z.get<i32_t>() );
		shift = 0;
		while ( ( shift < Zs ) && ! p[ shift ] )
			++ shift;
		Zs -= shift;
		p += shift;
		ep[ 0 ] = r.get<i32_t>();
		ep[ 1 ] = p;
		lm[ 0 ] = Zs - rs + m;
		lm[ 1 ] = 0;
		if ( ep[ 0 ] ) {
			::memcpy( res + size - rs, r.raw(), chunk_size<i32_t>( rs ) );
			mutate_addition( res + size - m - Zs - 1, Zs + 1, ep, lm, NULL, false, false );
		} else
			::memcpy( res + size - m - Zs, p, chunk_size<i32_t>( Zs ) );

		/* res += r2m*B^2m */
		Zs = r2ms;
		shift = 0;
		p = r2m.get<i32_t>();
		lm[ 0 ] = 0;
		if ( p ) {
			while ( ( shift < Zs ) && ! p[ shift ] )
				++ shift;
			Zs -= shift;
			p += shift;
			ep[ 0 ] = res + 1 + shift;
			ep[ 1 ] = p;
			lm[ 1 ] = 0;
			mutate_addition( res + shift, Zs + 1, ep, lm, NULL, false, false );
		}

		/* res -= r2m*B^m, res -= r*B^m */

		int ncar( ( fl > m ? 1 : 0 ) );
		int car( 1 - ncar );

		ep[ 0 ] = res + ncar;
		ep[ 1 ] = r2m.get<i32_t>();
		if ( ep[ 1 ] ) {
			lm[ 1 ] = size - r2ms - m - ncar;
			mutate_addition( res - car, size - m + car, ep, lm, NULL, true, false );
		}
		ep[ 1 ] = r.get<i32_t>();
		if ( ep[ 1 ] ) {
			lm[ 1 ] = size - rs - m - ncar;
			mutate_addition( res - car, size - m + car, ep, lm, NULL, true, false );
		}
		::memcpy( result.get<i32_t>() + totalShift, res + 1, chunk_size<i32_t>( leafCount - totalShift ) );
	} else if ( ( fxrl > 0 ) && ( fyrl > 0 ) ) {
/* variables for mutate_addition() */
		result.realloc( chunk_size<i32_t>( leafCount = fxs + fys ) );
		HChunk element( chunk_size<i32_t>( fys + 1 ) );
		i32_t* e( element.get<i32_t>() );
		i32_t* res( ( result.get<i32_t>() + fxs ) - 2 ); /* - 1 for carrier */
		i32_t const* ep[] = { res, e };
/* the end of variables for mutate_addition() */
		while ( -- fxs >= 0 ) {
			if ( fx[ fxs ] ) {
				i32_t carrier( 0 );
				int long inner( fys ); /* index of last leaf in second factor */
				while ( -- inner >= 0 ) {
					int long pos( inner + 1 );
					i64_t x( static_cast<i64_t>( fx[ fxs ] ) * fy[ inner ] + carrier );
					if ( x >= LEAF ) {
						carrier = static_cast<i32_t>( x / LEAF );
						x %= LEAF;
					} else
						carrier = 0;
					e[ pos ] = static_cast<i32_t>( x );
				}
				e[ 0 ] = carrier;
				ep[ 0 ] = res + 1;
				mutate_addition( res, fys + 1 + 1, ep, NULL, NULL, false, false );
				-- res;
			} else
				-- res;
		}
	}
	return ( leafCount );
}

}

}

