/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hmultimap.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_HCORE_HMULTIMAP_HXX_INCLUDED
#define YAAL_HCORE_HMULTIMAP_HXX_INCLUDED 1

#include "hcore/hpointer.hxx"
#include "hcore/hmap.hxx"
#include "hcore/hlist.hxx"
#include "hcore/hmulticontainer.hxx"
#include "hcore/memory.hxx"

namespace yaal {

namespace hcore {

/*! \brief Binary tree based map with relation one-to-many on keys to values.
 *
 * HMultiMap<> is a template representing self balancing binary search tree
 * data structure that holds pairs of keys and lists of values.
 *
 * \tparam key_type_t - type of key held in map.
 * \tparam value_type_t - type of value held in map.
 * \tparam compare_t - key ordering definition.
 */
template<typename key_type_t, typename value_type_t,
	typename compare_t = less<key_type_t>,
	typename allocator_t = allocator::system<HPair<key_type_t, value_type_t> >,
	template<typename, typename>class storage_policy_t = HMultiContainerStorage::HTransparent>
class HMultiMap {
public:
	typedef key_type_t key_type;
	typedef value_type_t data_type;
	typedef compare_t compare_type;
	typedef storage_policy_t<key_type const, data_type> storage_t;
	typedef HList<typename storage_t::stored_type> value_list_t;
	typedef HPointer<value_list_t> value_list_ptr_t;
	typedef HPair<key_type const, data_type> value_type;
	template<typename const_qual_t>
	class HIterator;
	typedef HIterator<value_type> iterator;
	typedef HIterator<value_type const> const_iterator;
	typedef HReverseIterator<iterator> reverse_iterator;
	typedef HReverseIterator<const_iterator> const_reverse_iterator;
private:
	typedef HMap<key_type, value_list_ptr_t, compare_type, allocator_t> multimap_engine_t;
	multimap_engine_t _engine;
public:
	typedef typename multimap_engine_t::allocator_type allocator_type;
	typedef HMultiMap<key_type_t, value_type_t, compare_t, allocator_t, storage_policy_t> this_type;
	HMultiMap( void )
		: _engine()
		{}
	explicit HMultiMap( compare_type const& compare_ )
		: _engine( compare_ )
		{}
	explicit HMultiMap( allocator_type const& allocator_ )
		: _engine( allocator_ )
		{}
	template<typename iterator_t>
	HMultiMap( iterator_t first, iterator_t last, compare_type const& compare_ = compare_type(), allocator_type const& allocator_ = allocator_type() )
		: _engine( compare_, allocator_ ) {
		M_PROLOG
		insert( first, last );
		return;
		M_EPILOG
	}
	HMultiMap( HMultiMap const& multimap_ )
		: _engine( multimap_._engine.compare(), multimap_._engine.get_allocator() ) {
		M_PROLOG
		_engine.copy_from( multimap_._engine );
		return;
		M_EPILOG
	}
	HMultiMap& operator = ( HMultiMap const& multimap_ ) {
		M_PROLOG
		if ( &multimap_ != this ) {
			HMultiMap tmp( multimap_ );
			swap( multimap_ );
		}
		return ( *this );
		M_EPILOG
	}
	allocator_type const& get_allocator( void ) const {
		return ( _engine.get_allocator() );
	}
	int long size( void ) const
		{ return ( get_size() ); }
	int long get_size( void ) const {
		M_PROLOG
		int long sizeAcc( 0 );
		for ( typename multimap_engine_t::const_iterator it( _engine.begin() ), endIt( _engine.end() ); it != endIt; ++ it )
			sizeAcc += it->second->get_size();
		return ( sizeAcc );
		M_EPILOG
	}
	bool empty( void ) const
		{ return ( is_empty() );	}
	bool is_empty( void ) const
		{ return ( _engine.is_empty() );	}
	iterator push_back( key_type const& key, data_type const& value ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = ensure_key( key );
		major->second->push_back( storage_t::value( key, value ) );
		typename value_list_t::reverse_iterator minor = major->second->rbegin();
		return ( iterator( this, major, minor.base() ) );
		M_EPILOG
	}
	iterator insert( value_type const& e ) {
		typename multimap_engine_t::iterator major = ensure_key( e.first );
		major->second->push_back( storage_t::value( e ) );
		typename value_list_t::iterator minor = major->second->rbegin().base();
		return ( iterator( this, major, minor ) );
	}
	template<typename iterator_t>
	void insert( iterator_t first, iterator_t last ) {
		M_PROLOG
		for ( ; first != last; ++ first )
			insert( *first );
		return;
		M_EPILOG
	}
	template<typename iter_t>
	void push_back( iter_t i, iter_t endIt ) {
		M_PROLOG
		for ( ; i != endIt; ++ i )
			push_back( *i );
		return;
		M_EPILOG
	}
	iterator push_front( key_type const& key, data_type const& value ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = ensure_key( key );
		major->second->push_front( storage_t::value( key, value ) );
		typename value_list_t::iterator minor = major->second->begin();
		return ( iterator( this, major, minor ) );
		M_EPILOG
	}
	template<typename iter_t>
	void push_front( iter_t i, iter_t endIt ) {
		M_PROLOG
		for ( ; i != endIt; ++ i )
			push_front( *i );
		return;
		M_EPILOG
	}
	/*! \brief Erase all elements of given \e key.
	 *
	 * \param key_ - erase elements with this key.
	 * \return number of elements actualy erased.
	 */
	int long erase( key_type const& key_ ) {
		M_PROLOG
		iterator it = find( key_ );
		int long erased( 0 );
		if ( it != end() ) {
			erased = it._major->second->get_size();
			_engine.erase( it._major );
		}
		return ( erased );
		M_EPILOG
	}
	iterator erase( iterator& it ) {
		M_PROLOG
		iterator newIt( it );
		++ newIt;
		value_list_ptr_t list = it._major->second;
		list->erase( it._minor );
		if ( ! list->size() )
			_engine.erase( it._major );
		return ( newIt );
		M_EPILOG
	}
	void erase( iterator first_, iterator const& last_ ) {
		M_PROLOG
		while ( first_ != last_ )
			first_ = erase( first_ );
		return;
		M_EPILOG
	}
	const_iterator find( key_type const& key ) const {
		M_PROLOG
		typename multimap_engine_t::const_iterator major = _engine.find( key );
		typename value_list_t::const_iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( const_iterator( this, major, minor ) );
		M_EPILOG
	}
	iterator find( key_type const& key ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = _engine.find( key );
		typename value_list_t::iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( iterator( this, major, minor ) );
		M_EPILOG
	}
	const_iterator lower_bound( key_type const& key_ ) const {
		M_PROLOG
		typename multimap_engine_t::const_iterator major =  _engine.lower_bound( key_ );
		typename value_list_t::const_iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( const_iterator( this, major, minor ) );
		M_EPILOG }
	iterator lower_bound( key_type const& key_ ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = _engine.lower_bound( key_ );
		typename value_list_t::iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( iterator( this, major, minor ) );
		M_EPILOG
	}
	const_iterator upper_bound( key_type const& key_ ) const {
		M_PROLOG
		typename multimap_engine_t::const_iterator major = _engine.upper_bound( key_ );
		typename value_list_t::const_iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( const_iterator( this, major, minor ) );
		M_EPILOG
	}
	iterator upper_bound( key_type const& key_ ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = _engine.upper_bound( key_ );
		typename value_list_t::iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( iterator( this, major, minor ) );
		M_EPILOG
	}
	const_iterator begin( void ) const {
		M_PROLOG
		typename multimap_engine_t::const_iterator major = _engine.begin();
		typename value_list_t::const_iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( const_iterator( this, major, minor ) );
		M_EPILOG
	}
	const_iterator cbegin( void ) const {
		return ( begin() );
	}
	iterator begin( void ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = _engine.begin();
		typename value_list_t::iterator minor;
		if ( major != _engine.end() )
			minor = major->second->begin();
		return ( iterator( this, major, minor ) );
		M_EPILOG
	}
	const_iterator end( void ) const
		{ return ( const_iterator( this, _engine.end(), typename value_list_t::const_iterator() ) ); }
	const_iterator cend( void ) const
		{ return ( end() ); }
	iterator end( void )
		{ return ( iterator( this, _engine.end(), typename value_list_t::iterator() ) ); }
	const_reverse_iterator rbegin( void ) const {
		M_PROLOG
		return ( end() );
		M_EPILOG
	}
	const_reverse_iterator crbegin( void ) const {
		M_PROLOG
		return ( end() );
		M_EPILOG
	}
	reverse_iterator rbegin( void ) {
		M_PROLOG
		return ( end() );
		M_EPILOG
	}
	const_reverse_iterator rend( void ) const
		{ return ( begin() ); }
	const_reverse_iterator crend( void ) const
		{ return ( begin() ); }
	reverse_iterator rend( void )
		{ return ( begin() ); }
	void clear( void )
		{ _engine.clear(); }
	int long count( key_type const& key ) const {
		M_PROLOG
		typename multimap_engine_t::const_iterator major = _engine.find( key );
		int long cnt = 0;
		if ( major != _engine.end() )
			cnt = major->second->size();
		return ( cnt );
		M_EPILOG
	}
	void swap( HMultiMap& multimap_ ) {
		if ( &multimap_ != this ) {
			using yaal::swap;
			swap( _engine, multimap_._engine );
		}
		return;
	}
	bool operator == ( HMultiMap const& map_ ) const
		{ M_PROLOG return ( ( &map_ == this ) || safe_equal( begin(), end(), map_.begin(), map_.end() ) ); M_EPILOG }
	bool operator < ( HMultiMap const& map_ ) const
		{ M_PROLOG return ( ( &map_ != this ) && lexicographical_compare( begin(), end(), map_.begin(), map_.end() ) ); M_EPILOG }
private:
	typename multimap_engine_t::iterator ensure_key( key_type const& key ) {
		M_PROLOG
		typename multimap_engine_t::iterator major = _engine.find( key );
		if ( major == _engine.end() ) {
			value_list_ptr_t list = value_list_ptr_t( make_pointer<value_list_t>() );
			major = _engine.insert( make_pair( key, list ) ).first;
		}
		return ( major );
		M_EPILOG
	}
};

/*! \brief Forward iterator for HMultiMap<>.
 */
template<typename key_type_t, typename value_type_t, typename compare_t, typename allocator_t, template<typename, typename>class storage_policy_t>
template<typename const_qual_t>
class HMultiMap<key_type_t, value_type_t, compare_t, allocator_t, storage_policy_t>::HIterator : public iterator_interface<typename HMultiMap<key_type, data_type, compare_type, allocator_t, storage_policy_t>::storage_t::template const_aware_type<const_qual_t>::accessor_t, iterator_category::forward> {
	typedef key_type_t key_type;
	typedef value_type_t data_type;
	typedef HMultiMap<key_type, data_type, compare_type, allocator_t, storage_policy_t> multi_map_t;
	typedef typename trait::ternary<trait::same_type<const_qual_t, const_qual_t const>::value,
					typename multi_map_t::multimap_engine_t::const_iterator,
					typename multi_map_t::multimap_engine_t::iterator>::type key_iterator_t;
	typedef typename trait::ternary<trait::same_type<const_qual_t, const_qual_t const>::value,
					typename value_list_t::const_iterator,
					typename value_list_t::iterator>::type value_iterator_t;
	multi_map_t const* _owner;
	key_iterator_t _major;
	value_iterator_t _minor;
public:
	typedef iterator_interface<typename multi_map_t::storage_t::template const_aware_type<const_qual_t>::accessor_t, iterator_category::forward> base_type;
	HIterator( void ) : base_type(), _owner( NULL ), _major(), _minor() {}
	HIterator( HIterator const& it_ ) : base_type(), _owner( it_._owner ), _major( it_._major ), _minor( it_._minor ) {}
	template<typename other_const_qual_t>
	HIterator( HIterator<other_const_qual_t> const& it_ ) : base_type(), _owner( it_._owner ), _major( it_._major ), _minor( it_._minor ) {
		STATIC_ASSERT(( trait::same_type<const_qual_t, other_const_qual_t>::value || trait::same_type<const_qual_t, other_const_qual_t const>::value ));
	}
	HIterator& operator = ( HIterator const& it_ ) {
		if ( &it_ != this ) {
			_owner = it_._owner;
			_major = it_._major;
			_minor = it_._minor;
		}
		return ( *this );
	}
	HIterator& operator ++ ( void ) {
		M_PROLOG
		++ _minor;
		if ( _minor == _major->second->end() ) {
			++ _major;
			if ( _major != _owner->_engine.end() )
				_minor = _major->second->begin();
		}
		return ( *this );
		M_EPILOG
	}
	HIterator const operator ++ ( int ) {
		HIterator it( *this );
		operator++();
		return ( it );
	}
	HIterator& operator -- ( void ) {
		M_PROLOG
		if ( _minor != value_iterator_t() )
			-- _minor;
		else {
			if ( _major != _owner->_engine.rend().base() )
				_minor = _major->second->rbegin().base();
		}
		if ( _major == _owner->_engine.end() ) {
			-- _major;
			_minor = _major->second->rbegin().base();
		}
		if ( _minor == _major->second->rend().base() ) {
			-- _major;
			if ( _major != _owner->_engine.rend().base() )
				_minor = _major->second->rbegin().base();
			else
				_minor = value_iterator_t();
		}
		return ( *this );
		M_EPILOG
	}
	HIterator const operator -- ( int ) {
		HIterator it( *this );
		operator--();
		return ( it );
	}
	typename multi_map_t::storage_t::template const_aware_type<const_qual_t>::accessor_t operator* ( void ) const
		{	return ( multi_map_t::storage_t::template const_aware_type<const_qual_t>::accessor( _major->first, *_minor ) );	}
	typename multi_map_t::storage_t::template const_aware_type<const_qual_t>::accessor_ptr_t operator->( void ) const
		{	return ( &multi_map_t::storage_t::template const_aware_type<const_qual_t>::accessor( _major->first, *_minor ) );	}
	bool operator == ( HIterator const& it ) const
		{ return ( ( _major == it._major ) && ( _minor == it._minor ) ); }
	bool operator != ( HIterator const& it ) const
		{ return ( ! ( ( _major == it._major ) && ( _minor == it._minor ) ) ); }
private:
	friend class HMultiMap<key_type, data_type, compare_type, allocator_t, storage_policy_t>;
	explicit HIterator( multi_map_t const* const owner_,
			key_iterator_t const& major,
			value_iterator_t const& minor ) : base_type(), _owner( owner_ ), _major( major ), _minor( minor ) {};
};

}

template<typename key_type_t, typename value_type_t, typename compare_t, typename allocator_t, template<typename, typename>class storage_policy_t>
inline void swap( yaal::hcore::HMultiMap<key_type_t, value_type_t, compare_t, allocator_t, storage_policy_t>& a,
		yaal::hcore::HMultiMap<key_type_t, value_type_t, compare_t, allocator_t, storage_policy_t>& b )
	{ a.swap( b ); }

}

#endif /* #ifndef YAAL_HCORE_HMULTIMAP_HXX_INCLUDED */

