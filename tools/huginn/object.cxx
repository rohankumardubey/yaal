/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  object.cxx - this file is integral part of `yaal' project.

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
#include "keyword.hxx"
#include "thread.hxx"
#include "helper.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace hobject_destructor_helper {

void deleter( HHuginn::HValue* );
void deleter( HHuginn::HValue* ) {
}

template<typename T>
struct allocator {
	template<typename U>
	struct rebind {
		typedef allocator<U> other;
	};
	T* _mem;
	allocator( T* mem_ )
		: _mem( mem_ ) {
	}
	template<typename U>
	allocator( allocator<U> const& other_ )
		: _mem( reinterpret_cast<T*>( const_cast<U*>( other_._mem ) ) ) {
	}
	allocator( allocator const& ) = default;
	allocator& operator = ( allocator const& ) = default;
	T* allocate( int ) {
		return ( _mem );
	}
	void deallocate( T*, int ) {
	}
};

typedef yaal::hcore::HSharedDeleterAllocator<HHuginn::HValue, void(*)( HHuginn::HValue*), allocator<HHuginn::HObject>> holder_t;

static int const BUFFER_SIZE = sizeof ( holder_t );
typedef typename memory::aligned<BUFFER_SIZE, holder_t>::type buffer_t;

}

HHuginn::HObject::HObject( HClass const* class_ )
	: HValue( class_ )
	, _fields( class_->get_defaults() ) {
}

HHuginn::HObject::HObject( HClass const* class_, fields_t const& fields_ )
	: HValue( class_ )
	, _fields( fields_ ) {
}

HHuginn::HObject::~HObject( void ) {
	M_PROLOG
	/* *FIXME* *TODO* Remove !is_builtin() test after HObject, HValue hierarchy fix. */
	HHuginn::HClass const* clss( get_class() );
	HHuginn* huginn( clss->huginn() );
	huginn::HThread* t( huginn && ! is_builtin( clss->name() ) ? huginn->current_thread() : nullptr );
	if ( t && ! t->has_runtime_exception() ) {
		hobject_destructor_helper::buffer_t buffer;
		hobject_destructor_helper::allocator<hobject_destructor_helper::holder_t> allocator( buffer.mem() );
		HHuginn::value_t nonOwning( this, &hobject_destructor_helper::deleter, allocator );
		int destructorIdx( clss->field_index( KEYWORD::DESTRUCTOR_IDENTIFIER ) );
		HClass const* c( clss->super() );
		if ( destructorIdx >= 0 ) {
			try {
				static_cast<HClass::HMethod const*>(
					_fields[destructorIdx].raw()
				)->function()( t, &nonOwning, values_t{}, 0 );
			} catch ( HHuginnRuntimeException const& e ) {
				t->break_execution( HFrame::STATE::RUNTIME_EXCEPTION );
				t->set_exception( e.message(), e.position() );
				c = nullptr;
			}
		}
		while ( c ) {
			destructorIdx = c->field_index( KEYWORD::DESTRUCTOR_IDENTIFIER );
			if ( destructorIdx >= 0 ) {
				try {
					c->function( destructorIdx )( t, &nonOwning, values_t{}, 0 );
				} catch ( HHuginnRuntimeException const& e ) {
					t->break_execution( HFrame::STATE::RUNTIME_EXCEPTION );
					t->set_exception( e.message(), e.position() );
					break;
				}
			}
			c = c->super();
		}
	}
	return;
	M_DESTRUCTOR_EPILOG
}

HHuginn::value_t& HHuginn::HObject::field_ref( int index_ ) {
	M_PROLOG
	return ( _fields[index_] );
	M_EPILOG
}

HHuginn::value_t HHuginn::HObject::do_field( HHuginn::value_t const& object_, int index_ ) const {
	M_PROLOG
	value_t const& f( _fields[index_] );
	bool isMethod( f->type_id() == TYPE::METHOD );
	return (
		isMethod
			? pointer_static_cast<HHuginn::HValue>( make_pointer<HClass::HBoundMethod>( *static_cast<HClass::HMethod const*>( f.raw() ), object_ ) )
			: f
	);
	M_EPILOG
}

bool HHuginn::HObject::is_kind_of( HHuginn::identifier_id_t typeName_ ) const {
	M_PROLOG
	return ( get_class()->is_kind_of( typeName_ ) );
	M_EPILOG
}

HHuginn::value_t HHuginn::HObject::call_method(
	huginn::HThread* thread_,
	HHuginn::value_t const& object_,
	yaal::hcore::HString const& methodName_,
	HHuginn::values_t const& arguments_,
	int position_
) const {
	M_PROLOG
	M_ASSERT( object_.raw() == this );
	HHuginn::value_t res;
	int idx( field_index( thread_->huginn().identifier_id( methodName_ ) ) );
	if ( idx >= 0 ) {
		HHuginn::value_t const& f( field( object_, idx ) );
		if ( f->type_id() == HHuginn::TYPE::METHOD ) {
			HHuginn::HClass::HMethod const* m( static_cast<HHuginn::HClass::HMethod const*>( f.raw() ) );
			res = m->function()( thread_, const_cast<HHuginn::value_t*>( &object_ ), arguments_, position_ );
		} else {
			throw HHuginn::HHuginnRuntimeException(
				"`"_ys
				.append( methodName_ )
				.append( "' in class `" )
				.append( get_class()->name() )
				.append( "' is not a method." ),
				position_
			);
		}
	} else {
		throw HHuginn::HHuginnRuntimeException(
			"Class `"_ys
			.append( get_class()->name() )
			.append( "' does not have `" )
			.append( methodName_ )
			.append( "' method." ),
			position_
		);
	}
	return ( res );
	M_EPILOG
}

HHuginn::value_t HHuginn::HObject::do_clone( HHuginn* huginn_ ) const {
	M_PROLOG
	values_t fields;
	for ( value_t const& v : _fields ) {
		fields.push_back( v->clone( huginn_ ) );
	}
	return ( make_pointer<HObject>( get_class(), fields ) );
	M_EPILOG
}

}

}
