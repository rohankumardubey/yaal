/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "set.hxx"
#include "runtime.hxx"
#include "builtin.hxx"
#include "iterator.hxx"
#include "compiler.hxx"
#include "instruction.hxx"
#include "helper.hxx"
#include "thread.hxx"
#include "objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

namespace set {

class HSetIterator : public HSkippingIterator {
	huginn::HSet::values_t* _set;
	huginn::HSet::values_t::iterator _it;
	bool _valid;
public:
	HSetIterator( huginn::HSet* owner_ )
		: HSkippingIterator( owner_ )
		, _set( &owner_->value() )
		, _it( _set->begin() )
		, _valid( true ) {
		return;
	}
protected:
	virtual HHuginn::value_t do_value( HThread*, int ) override {
		return ( *_it );
	}
	virtual bool do_is_valid( huginn::HThread*, int ) override {
		return ( _valid && ( _it != _set->end() ) );
	}
	virtual void do_next( HThread*, int ) override {
		if ( _skip == 0 ) {
			++ _it;
		} else {
			-- _skip;
		}
	}
	virtual void do_invalidate( void ) override {
		_valid = false;
	}
	virtual void do_skip( HThread*, int ) override {
		++ _it;
		++ _skip;
	}
	virtual void const* do_id( void ) const override {
		return ( _it.node_id() );
	}
private:
	HSetIterator( HSetIterator const& ) = delete;
	HSetIterator& operator = ( HSetIterator const& ) = delete;
};

class HSetReverseIterator : public HSkippingIterator {
	huginn::HSet::values_t* _set;
	huginn::HSet::values_t::reverse_iterator _it;
	bool _valid;
public:
	HSetReverseIterator( huginn::HSet* owner_ )
		: HSkippingIterator( owner_ )
		, _set( &owner_->value() )
		, _it( _set->rbegin() )
		, _valid( true ) {
		return;
	}
protected:
	virtual HHuginn::value_t do_value( HThread*, int ) override {
		return ( *_it );
	}
	virtual bool do_is_valid( HThread*, int ) override {
		return ( _valid && ( _it != _set->rend() ) );
	}
	virtual void do_next( HThread*, int ) override {
		if ( _skip == 0 ) {
			++ _it;
		} else {
			-- _skip;
		}
	}
	virtual void do_invalidate( void ) override {
		_valid = false;
	}
	virtual void do_skip( HThread*, int ) override {
		++ _it;
		++ _skip;
	}
	virtual void const* do_id( void ) const override {
		return ( _it.raw().node_id() );
	}
private:
	HSetReverseIterator( HSetReverseIterator const& ) = delete;
	HSetReverseIterator& operator = ( HSetReverseIterator const& ) = delete;
};

class HReversedSet : public huginn::HIterable {
	HHuginn::value_t _set;
public:
	HReversedSet( huginn::HClass const* class_, HHuginn::value_t const& set_ )
		: HIterable( class_ )
		, _set( set_ ) {
		M_ASSERT( _set->type_id() == HHuginn::TYPE::SET );
	}
	static HHuginn::class_t get_class( HRuntime* runtime_, HObjectFactory* objectfactory_, huginn::HClass const* origin_ ) {
		M_PROLOG
		HHuginn::class_t c(
			make_pointer<huginn::HClass>(
				runtime_,
				objectfactory_,
				"ReversedSetView",
				"The `ReversedSetView` class represents *lazy* *iterable* reversed view of a `set`.",
				origin_
			)
		);
		return c;
		M_EPILOG
	}
protected:
	virtual int long do_size( huginn::HThread* thread_, int position_ ) const override {
		return ( safe_int::cast<int long>( static_cast<huginn::HSet const*>( _set.raw() )->size( thread_, position_ ) ) );
	}
private:
	virtual iterator_t do_iterator( HThread*, int ) override {
		return ( make_pointer<HSetReverseIterator>( static_cast<huginn::HSet*>( _set.raw() ) ) );
	}
private:
	virtual HHuginn::value_t do_clone( huginn::HThread* thread_, HHuginn::value_t*, int ) const override {
		return ( thread_->object_factory().create<HReversedSet>( HIterable::get_class(), _set ) );
	}
};

inline HHuginn::value_t insert( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "set.insert", values_, 1, 1, thread_, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::SET );
	static_cast<huginn::HSet*>( object_->raw() )->insert( thread_, values_[0], position_ );
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t has_key( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "set.has_key", values_, 1, 1, thread_, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::SET );
	bool hasKey( static_cast<huginn::HSet*>( object_->raw() )->has_key( thread_, values_[0], position_ ) );
	return ( thread_->runtime().boolean_value( hasKey ) );
	M_EPILOG
}

inline HHuginn::value_t erase( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "set.erase", values_, 1, 1, thread_, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::SET );
	static_cast<huginn::HSet*>( object_->raw() )->erase( thread_, values_[0], position_ );
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t clear( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "set.clear", values_, 0, 0, thread_, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::SET );
	static_cast<huginn::HSet*>( object_->raw() )->clear();
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t update( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::SET );
	verify_signature( "set.update", values_, { HHuginn::TYPE::SET }, thread_, position_ );
	(*object_)->operator_add( thread_, *object_, values_[0], position_ );
	return ( *object_ );
	M_EPILOG
}

class HSetClass : public huginn::HClass {
public:
	typedef HSetClass this_type;
	typedef huginn::HClass base_type;
private:
	HHuginn::class_t _reversedSetClass;
public:
	HSetClass(
		HRuntime* runtime_,
		HObjectFactory* objectFactory_
	) : huginn::HClass(
			runtime_,
			objectFactory_,
			huginn::type_id( HHuginn::TYPE::SET ),
			runtime_->identifier_id( type_name( HHuginn::TYPE::SET ) ),
			"The `set` is a collection of unique elements of varying types. It supports operation of element insertion, removal and search.",
			&builtin::set
		)
		, _reversedSetClass() {
		HHuginn::field_definitions_t fd{
			{ "insert",  objectFactory_->create_method( &set::insert ),  "( *elem* ) - insert given element *elem* into a `set`" },
			{ "has_key", objectFactory_->create_method( &set::has_key ), "( *elem* ) - tell if given element *elem* is present in the `set`" },
			{ "erase",   objectFactory_->create_method( &set::erase ),   "( *elem* ) - remove given element *elem* from the `set`" },
			{ "clear",   objectFactory_->create_method( &set::clear ),   "erase `set`'s content, `set` becomes empty" },
			{ "update",  objectFactory_->create_method( &set::update ),  "( *other* ) - update content of this `set` with values from *other* `set`" }
		};
		redefine( nullptr, fd );
		_reversedSetClass = add_class_as_member( this, HReversedSet::get_class( runtime_, objectFactory_, this ), HClass::MEMBER_TYPE::STATIC );
		return;
	}
	huginn::HClass const* reversed_set_class( void ) const {
		return ( _reversedSetClass.raw() );
	}
protected:
	void do_finalize_registration( huginn::HRuntime* runtime_ ) {
		runtime_->huginn()->register_class( _reversedSetClass );
	}
};

HHuginn::class_t get_class( HRuntime*, HObjectFactory* );
HHuginn::class_t get_class( HRuntime* runtime_, HObjectFactory* objectFactory_ ) {
	M_PROLOG
	return ( make_pointer<HSetClass>( runtime_, objectFactory_ ) );
	M_EPILOG
}

HHuginn::value_t reversed_view( huginn::HThread* thread_, HHuginn::value_t const& value_ ) {
	M_PROLOG
	M_ASSERT( value_->type_id() == HHuginn::TYPE::SET );
	HSetClass const* sc( static_cast<HSetClass const*>( value_->get_class() ) );
	return ( thread_->object_factory().create<HReversedSet>( sc->reversed_set_class(), value_ ) );
	M_EPILOG
}

}

HSet::HSet( HClass const* class_ )
	: HInvalidatingIterable( class_ )
	, _helper()
	, _data( _helper, _helper ) {
	return;
}

int long HSet::do_size( huginn::HThread*, int ) const {
	return ( _data.get_size() );
}

bool HSet::has_key( huginn::HThread* thread_, HHuginn::value_t const& key_, int position_ ) const {
	M_PROLOG
	HAnchorGuard<HSet> ag( *this, thread_, position_ );
	return ( _data.find( key_ ) != _data.end() );
	M_EPILOG
}

void HSet::erase( huginn::HThread* thread_, HHuginn::value_t const& key_, int position_ ) {
	M_PROLOG
	HAnchorGuard<HSet> ag( *this, thread_, position_ );
	values_t::iterator it( _data.find( key_ ) );
	if ( it != _data.end() ) {
		skip( thread_, it.node_id(), position_ );
		_data.erase( it );
		if ( ! _observers || _observers->is_empty() ) {
			_data.compact();
		}
	}
	return;
	M_EPILOG
}

void HSet::insert( huginn::HThread* thread_, HHuginn::value_t const& value_, int position_ ) {
	M_PROLOG
	HAnchorGuard<HSet> ag( *this, thread_, position_ );
	_data.insert( value_ );
	return;
	M_EPILOG
}

void HSet::clear( void ) {
	M_PROLOG
	invalidate();
	_data.clear();
	if ( ! _observers || _observers->is_empty() ) {
		_data.compact();
	}
	return;
	M_EPILOG
}

HIterable::iterator_t HSet::do_iterator( huginn::HThread*, int ) {
	return ( make_pointer<set::HSetIterator>( this ) );
}

HHuginn::value_t HSet::do_clone( huginn::HThread* thread_, HHuginn::value_t*, int position_ ) const {
	M_PROLOG
	HHuginn::value_t res( thread_->runtime().object_factory()->create_set() );
	HSet* set( static_cast<HSet*>( res.raw() ) );
	values_t& data( set->value() );
	HAnchorGuard<HSet> ag( *set, thread_, position_ );
	for ( values_t::value_type const& v : _data ) {
		data.insert( v->clone( thread_, const_cast<HHuginn::value_t*>( &v ), position_ ) );
	}
	return res;
	M_EPILOG
}

bool HSet::do_operator_equals( huginn::HThread* thread_, HHuginn::value_t const&, HHuginn::value_t const& other_, int position_ ) const {
	huginn::HSet const& other( *static_cast<huginn::HSet const*>( other_.raw() ) );
	huginn::HSet::values_t const& otherData( other.value() );
	bool equal( _data.get_size() == otherData.get_size() );
	HAnchorGuard<huginn::HSet> ag( other, thread_, position_ );
	for ( huginn::HSet::values_t::const_iterator it( _data.begin() ), end( _data.end() ); equal && ( it != end ); ++ it ) {
		huginn::HSet::values_t::const_iterator otherIt( otherData.find( *it ) );
		equal = otherIt != otherData.end();
	}
	return equal;
}

void HSet::do_operator_add( HThread* thread_, HHuginn::value_t&, HHuginn::value_t const& other_, int position_ ) {
	huginn::HSet::values_t const& otherData( static_cast<huginn::HSet const*>( other_.raw() )->value() );
	HAnchorGuard<huginn::HSet> ag( *this, thread_, position_ );
	for ( huginn::HSet::values_t::const_iterator it( otherData.begin() ), end( otherData.end() ); it != end; ++ it ) {
		_data.insert( *it );
	}
	return;
}

void HSet::do_operator_subtract( HThread* thread_, HHuginn::value_t&, HHuginn::value_t const& other_, int position_ ) {
	huginn::HSet const& otherValue( *static_cast<huginn::HSet const*>( other_.raw() ) );
	huginn::HSet::values_t const& otherData( otherValue.value() );
	HAnchorGuard<huginn::HSet> agSelf( *this, thread_, position_ );
	HAnchorGuard<huginn::HSet> agOther( otherValue, thread_, position_ );
	if ( otherData.get_size() < _data.get_size() ) {
		for ( huginn::HSet::values_t::const_iterator it( otherData.begin() ), end( otherData.end() ); it != end; ++ it ) {
			_data.erase( *it );
		}
	} else {
		for ( huginn::HSet::values_t::const_iterator it( _data.begin() ); it != _data.end(); ) {
			if ( otherData.find( *it ) != otherData.end() ) {
				it = _data.erase( it );
			} else {
				++ it;
			}
		}
	}
	return;
}

void HSet::do_operator_multiply( HThread* thread_, HHuginn::value_t&, HHuginn::value_t const& other_, int position_ ) {
	huginn::HSet const& otherValue( *static_cast<huginn::HSet const*>( other_.raw() ) );
	huginn::HSet::values_t const& otherData( otherValue.value() );
	HAnchorGuard<huginn::HSet> agOther( otherValue, thread_, position_ );
	HAnchorGuard<huginn::HSet> agSelf( *this, thread_, position_ );
	for ( huginn::HSet::values_t::const_iterator it( _data.begin() ); it != _data.end(); ) {
		if ( otherData.find( *it ) != otherData.end() ) {
			++ it;
		} else {
			it = _data.erase( it );
		}
	}
	return;
}

bool HSet::do_operator_contains( HThread* thread_, HHuginn::value_t const&, HHuginn::value_t const& other_, int position_ ) const {
	return ( has_key( thread_, other_, position_ ) );
}

hash_value_t HSet::do_operator_hash( HThread* thread_, HHuginn::value_t const&, int position_ ) const {
	hash_value_t hashValue( static_cast<hash_value_t>( HHuginn::TYPE::SET ) );
	for ( HHuginn::value_t const& v : _data ) {
		hashValue *= 3;
		hashValue += v->operator_hash( thread_, v, position_ );
	}
	return hashValue;
}

yaal::hcore::HString HSet::do_code( huginn::HThread* thread_, HHuginn::value_t const&, HCycleTracker& cycleTracker_, int position_ ) const {
	if ( _data.is_empty() ) {
		return ( "set()" );
	}
	hcore::HString str( "{" );
	bool next( false );
	for ( HHuginn::value_t const& v : _data ) {
		if ( next ) {
			str.append( ", " );
		}
		next = true;
		str.append( v->code( thread_, v, cycleTracker_, position_ ) );
	}
	str.append( "}" );
	return str;
}

yaal::hcore::HString HSet::do_to_string( huginn::HThread* thread_, HHuginn::value_t const&, HCycleTracker& cycleTracker_, int position_ ) const {
	if ( _data.is_empty() ) {
		return ( "set()" );
	}
	hcore::HString str( "{" );
	bool next( false );
	for ( HHuginn::value_t const& v : _data ) {
		if ( next ) {
			str.append( ", " );
		}
		next = true;
		str.append( v->to_string( thread_, v, cycleTracker_, position_ ) );
	}
	str.append( "}" );
	return str;
}

}

}

}

