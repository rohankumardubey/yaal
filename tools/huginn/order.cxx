/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  order.cxx - this file is integral part of `yaal' project.

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

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "tools/hhuginn.hxx"
#include "runtime.hxx"
#include "iterator.hxx"
#include "compiler.hxx"
#include "value_builtin.hxx"
#include "helper.hxx"
#include "thread.hxx"
#include "objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

class HOrderIterator : public HIteratorInterface {
	HHuginn::HOrder::values_t* _order;
	HHuginn::HOrder::values_t::iterator _it;
public:
	HOrderIterator( HHuginn::HOrder::values_t* order_ )
		: _order( order_ ), _it( order_->begin() ) {
		return;
	}
protected:
	virtual HHuginn::value_t do_value( HThread*, int ) override {
		return ( *_it );
	}
	virtual bool do_is_valid( void ) override {
		return ( _it != _order->end() );
	}
	virtual void do_next( HThread*, int ) override {
		++ _it;
	}
private:
	HOrderIterator( HOrderIterator const& ) = delete;
	HOrderIterator& operator = ( HOrderIterator const& ) = delete;
};

namespace order {

inline HHuginn::value_t add( huginn::HThread*, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "order.add", values_, 1, 1, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	static_cast<HHuginn::HOrder*>( object_->raw() )->insert( values_[0], position_ );
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t has_key( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "order.has_key", values_, 1, 1, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	bool hasKey( static_cast<HHuginn::HOrder*>( object_->raw() )->has_key( values_[0], position_ ) );
	return ( thread_->object_factory().create_boolean( hasKey ) );
	M_EPILOG
}

inline HHuginn::value_t erase( huginn::HThread*, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "order.erase", values_, 1, 1, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	static_cast<HHuginn::HOrder*>( object_->raw() )->erase( values_[0], position_ );
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t clear( huginn::HThread*, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "order.clear", values_, 0, 0, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	static_cast<HHuginn::HOrder*>( object_->raw() )->value().clear();
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t update( huginn::HThread*, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "order.update";
	verify_arg_count( name, values_, 1, 1, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	verify_arg_type( name, values_, 0, HHuginn::TYPE::ORDER, true, position_ );
	HHuginn::HOrder& l( *static_cast<HHuginn::HOrder*>( object_->raw() ) );
	HHuginn::HOrder const& r( *static_cast<HHuginn::HOrder const*>( values_[0].raw() ) );
	if ( r.key_type()->type_id() != HHuginn::TYPE::NONE ) {
		l.verify_key_type( r.key_type(), position_ );
	}
	HHuginn::HOrder::values_t& lv( l.value() );
	HHuginn::HOrder::values_t const& rv( r.value() );
	for ( HHuginn::HOrder::values_t::const_iterator it( rv.begin() ), end( rv.end() ); it != end; ++ it ) {
		lv.insert( *it );
	}
	return ( *object_ );
	M_EPILOG
}

inline HHuginn::value_t equals( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "order.equals";
	verify_arg_count( name, values_, 1, 1, position_ );
	M_ASSERT( (*object_)->type_id() == HHuginn::TYPE::ORDER );
	verify_arg_type( name, values_, 0, HHuginn::TYPE::ORDER, true, position_ );
	HHuginn::HOrder::values_t const& l( static_cast<HHuginn::HOrder*>( object_->raw() )->value() );
	HHuginn::HOrder::values_t const& r( static_cast<HHuginn::HOrder const*>( values_[0].raw() )->value() );
	bool equal( l.get_size() == r.get_size() );
	for ( HHuginn::HOrder::values_t::const_iterator lit( l.begin() ), rit( r.begin() ), end( l.end() ); equal && ( lit != end ); ++ lit, ++ rit ) {
		equal = value_builtin::equals( thread_, *lit, *rit, position_ );
	}
	return ( thread_->object_factory().create_boolean( equal ) );
	M_EPILOG
}

HHuginn::class_t get_class( HRuntime* );
HHuginn::class_t get_class( HRuntime* runtime_ ) {
	M_PROLOG
	HHuginn::class_t c(
		make_pointer<HHuginn::HClass>(
			runtime_,
			type_id( HHuginn::TYPE::ORDER ),
			runtime_->identifier_id( type_name( HHuginn::TYPE::ORDER ) ),
			nullptr,
			HHuginn::field_definitions_t{
				{ "add",     make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::add, _1, _2, _3, _4 ) ),     "( *elem* ) - add given element *elem* to an `order`" },
				{ "has_key", make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::has_key, _1, _2, _3, _4 ) ), "( *elem* ) - tell if given element *elem* is in the `order`" },
				{ "erase",   make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::erase, _1, _2, _3, _4 ) ),   "( *elem* ) - remove given element *elem* from the `order`" },
				{ "clear",   make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::clear, _1, _2, _3, _4 ) ),   "erase `order`'s content, `order` becomes empty" },
				{ "update",  make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::update, _1, _2, _3, _4 ) ),  "( *other* ) - update content of this `order` with values from *other* `order`" },
				{ "equals",  make_pointer<HHuginn::HClass::HMethod>( hcore::call( &order::equals, _1, _2, _3, _4 ) ),  "( *other* ) - test if *other* `order` has the same content" }
			},
			"The `order` is a collection of sorted values of uniform types. It supports operations of addition, search and element removal."
		)
	);
	return ( c );
	M_EPILOG
}

}

}

HHuginn::HOrder::HOrder( HHuginn::HClass const* class_ )
	: HIterable( class_ )
	, _data( &value_builtin::less_low )
	, _keyType( &huginn::_noneClass_ ) {
	return;
}

HHuginn::HOrder::HOrder( HHuginn::HClass const* class_, values_t&& data_, HHuginn::HClass const* keyType_ )
	: HIterable( class_ )
	, _data( yaal::move( data_ ) )
	, _keyType( keyType_ ) {
	return;
}

int long HHuginn::HOrder::do_size( void ) const {
	return ( _data.get_size() );
}

void HHuginn::HOrder::verify_key_type( HHuginn::HHuginn::HClass const* keyType_, int position_ ) const {
	if ( ( _keyType->type_id() != TYPE::NONE ) && ( keyType_ != _keyType ) ) {
		throw HHuginnRuntimeException( "Non-uniform key types, got "_ys.append( a_type_name( keyType_ ) ).append( " instead of " ).append( a_type_name( _keyType ) ).append( "." ), position_ );
	}
	if ( ! OCompiler::is_comparable( keyType_->type_id() ) ) {
		throw HHuginnRuntimeException( "Key type `"_ys.append( keyType_->name() ).append( "' is not a comparable." ), position_ );
	}
	return;
}

bool HHuginn::HOrder::has_key( HHuginn::value_t const& value_, int position_ ) const {
	M_PROLOG
	verify_key_type( value_->get_class(), position_ );
	return ( _data.find( value_ ) != _data.end() );
	M_EPILOG
}

void HHuginn::HOrder::erase( HHuginn::value_t const& value_, int position_ ) {
	M_PROLOG
	verify_key_type( value_->get_class(), position_ );
	_data.erase( value_ );
	return;
	M_EPILOG
}

void HHuginn::HOrder::insert( HHuginn::value_t const& value_, int position_ ) {
	M_PROLOG
	verify_key_type( value_->get_class(), position_ );
	_data.insert( value_ );
	_keyType = value_->get_class();
	return;
	M_EPILOG
}

HHuginn::HClass const* HHuginn::HOrder::key_type( void ) const {
	return ( _keyType );
}

HHuginn::HIterable::HIterator HHuginn::HOrder::do_iterator( huginn::HThread*, int ) {
	HIterator::iterator_implementation_t impl( new ( memory::yaal ) huginn::HOrderIterator( &_data ) );
	return ( HIterator( yaal::move( impl ) ) );
}

HHuginn::value_t HHuginn::HOrder::do_clone( huginn::HThread* thread_, int position_ ) const {
	values_t data( &value_builtin::less_low );
	for ( values_t::value_type const& v : _data ) {
		data.insert( data.end(), v->clone( thread_, position_ ) );
	}
	return ( thread_->runtime().object_factory()->create_order( yaal::move( data ), _keyType ) );
}

}

}

