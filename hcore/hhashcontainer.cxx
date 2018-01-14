/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "hhashcontainer.hxx"
#include "algorithm.hxx"

namespace yaal {

namespace hcore {

int long const x_tag_g_pulPrimes[ 32 ] = {
	1,          3,        7,         13,        31,        61,
	127,        251,      509,       1021,      2039,      4093,
	8191,       16381,    32749,     65521,     131071,    262139,
	524287,     1048573,  2097143,   4194301,   8388593,   16777213,
	33554393,   67108859, 134217689, 268435399, 536870909, 1073741789,
	2147483647, 0
}, * const _primes_ = x_tag_g_pulPrimes;

template<>
int long hash<HString>::operator () ( HString const& string_ ) const {
	int long h( 0 );
	for ( code_point_t c : string_ ) {
		h *= 101;
		h += static_cast<int>( c.get() );
	}
	return ( h );
}

template<>
int long hash<code_point_t>::operator () ( code_point_t const& val_ ) const {
	return ( static_cast<int long>( val_.get() ) );
}

template<>
int long hash<double long>::operator () ( double long const& val_ ) const {
	return ( static_cast<int long>( reinterpret_cast<int long long const&>( static_cast<double const&>( val_ ) ) ) );
}

template<typename T>
int long hash<T>::operator () ( T const& key_ ) const {
	return ( static_cast<int long>( key_ ) );
}

template struct hash<bool>;
template struct hash<char>;
template struct hash<char signed>;
template struct hash<char unsigned>;
template struct hash<int short>;
template struct hash<int short unsigned>;
template struct hash<int>;
template struct hash<int unsigned>;
template struct hash<int long>;
template struct hash<int long unsigned>;
template struct hash<int long long>;
template struct hash<int long long unsigned>;

}

}

