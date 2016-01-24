/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  expression.cxx - this file is integral part of `yaal' project.

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
#include "expression.hxx"
#include "frame.hxx"
#include "thread.hxx"
#include "value_builtin.hxx"
#include "booleanevaluator.hxx"
#include "helper.hxx"
#include "objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;

namespace yaal {

namespace tools {

namespace huginn {

HExpression::HExpression( int position_ )
	: HStatement(),
	_executionSteps(),
	_position( position_ ) {
	return;
}

int HExpression::position( void ) const {
	M_PROLOG
	return ( _position );
	M_EPILOG
}

void HExpression::set_position( int position_ ) {
	M_PROLOG
	_position = position_;
	return;
	M_EPILOG
}

int HExpression::add_execution_step( execution_step_t const& executionStep_ ) {
	M_PROLOG
	_executionSteps.push_back( executionStep_ );
	return ( static_cast<int>( _executionSteps.get_size() - 1 ) );
	M_EPILOG
}

void HExpression::replace_execution_step( int index_, execution_step_t const& executionStep_ ) {
	M_PROLOG
	_executionSteps[index_] = executionStep_;
	return;
	M_EPILOG
}

void HExpression::pop_execution_step( void ) {
	M_PROLOG
	_executionSteps.pop_back();
	return;
	M_EPILOG
}

void HExpression::merge( HExpression& expression_ ) {
	M_PROLOG
	move( expression_._executionSteps.begin(), expression_._executionSteps.end(), back_insert_iterator( _executionSteps ) );
	expression_._executionSteps.clear();
	return;
	M_EPILOG
}

bool HExpression::is_empty( void ) const {
	M_PROLOG
	return ( _executionSteps.is_empty() );
	M_EPILOG
}

void HExpression::oper( OPERATOR operator_, HFrame* frame_, int position_ ) {
	M_PROLOG
	frame_->operations().emplace( operator_, position_ );
	return;
	M_EPILOG
}

void HExpression::close_parenthesis( HFrame* frame_, int position_ ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( ! frame_->values().is_empty() );
	OPERATOR o( frame_->operations().top()._operator );
	M_ASSERT( ( o == OPERATOR::ABSOLUTE ) || ( o == OPERATOR::PARENTHESIS ) );
	frame_->operations().pop();
	if ( o == OPERATOR::ABSOLUTE ) {
		HHuginn::value_t v( yaal::move( frame_->values().top() ) );
		frame_->values().pop();
		frame_->values().push( value_builtin::abs( frame_->thread(), v, position_ ) );
	}
	return;
	M_EPILOG
}

void HExpression::get_reference( HHuginn::identifier_id_t identifierId_, HFrame* frame_, int position_ ) {
	M_PROLOG
	HHuginn::value_t v( frame_->get_reference( identifierId_, position_ ) );
	frame_->values().push( v );
	return;
	M_EPILOG
}

void HExpression::get_field( ACCESS access_, HHuginn::identifier_id_t identifierId_, huginn::HFrame* frame_, int position_ ) {
	M_PROLOG
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::MEMBER_ACCESS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v->get_class()->is_complex() ) {
		int fi( v->field_index( identifierId_ ) );
		if ( fi < 0 ) {
			throw HHuginn::HHuginnRuntimeException(
				"`"_ys
				.append( v->get_class()->name() )
				.append( "' does not have `" )
				.append( frame_->thread()->huginn().identifier_name( identifierId_ ) )
				.append( "' member." ),
				p
			);
		}
		if ( access_ == ACCESS::VALUE ) {
			frame_->values().push( v->field( v, fi ) );
		} else if ( ! v.unique() ) {
			HHuginn::HObject* o( dynamic_cast<HHuginn::HObject*>( v.raw() ) );
			if ( o != nullptr ) {
				frame_->values().push( make_pointer<HHuginn::HReference>( o->field_ref( fi ) ) );
			} else {
				throw HHuginn::HHuginnRuntimeException( "Assignment to read-only location.", position_ );
			}
		} else {
			throw HHuginn::HHuginnRuntimeException( "Assignment to temporary.", position_ );
		}
	} else {
		HHuginn::HObjectReference* oref( dynamic_cast<HHuginn::HObjectReference*>( v.raw() ) );
		if ( oref != nullptr ) { /* Handle `super' keyword. */
			int fi( oref->field_index( identifierId_ ) );
			if ( fi < 0 ) {
				throw HHuginn::HHuginnRuntimeException(
					"`"_ys
					.append( oref->get_class()->name() )
					.append( "' does not have `" )
					.append( frame_->thread()->huginn().identifier_name( identifierId_ ) )
					.append( "' member." ),
					p
				);
			}
			if ( access_ == ACCESS::VALUE ) {
				frame_->values().push( oref->field( fi ) );
			} else {
				throw HHuginn::HHuginnRuntimeException( "Changing upcasted reference.", p );
			}
		} else {
			throw HHuginn::HHuginnRuntimeException( "`"_ys.append( v->get_class()->name() ).append( "' is not a compound object." ), p );
		}
	}
	return;
	M_EPILOG
}

void HExpression::make_variable( HHuginn::identifier_id_t identifierId_, HFrame* frame_, int position_ ) {
	M_PROLOG
	frame_->values().push( frame_->make_variable( identifierId_, position_ ) );
	return;
	M_EPILOG
}

void HExpression::set_variable( HFrame* frame_, int ) {
	M_PROLOG
	operations_t& operations( frame_->operations() );
	while ( ! operations.is_empty() ) {
		operations_t::value_type& operation( operations.top() );
		if ( ( operation._operator < OPERATOR::ASSIGN ) || ( operation._operator > OPERATOR::POWER_ASSIGN ) ) {
			break;
		}
		int p( operation._position );
		operations.pop();
		HHuginn::value_t src( yaal::move( frame_->values().top() ) );
		frame_->values().pop();
		HHuginn::value_t dst( yaal::move( frame_->values().top() ) );
		frame_->values().pop();
		if ( dst->type_id() != HHuginn::TYPE::REFERENCE ) {
			M_ASSERT( dst->type_id() == HHuginn::TYPE::CHARACTER );
			throw HHuginn::HHuginnRuntimeException( "String does not support item assignment.", p );
		}
		HHuginn::value_t& ref( static_cast<HHuginn::HReference*>( dst.raw() )->value() );
		if ( operation._operator == OPERATOR::ASSIGN ) {
			ref = src;
		} else {
			if ( ref->type_id() != src->type_id() ) {
				operands_type_mismatch( op_to_str( operation._operator ), ref->type_id(), src->type_id(), p );
			}
			switch ( operation._operator ) {
				case ( OPERATOR::PLUS_ASSIGN ):     { value_builtin::add( ref, src, p ); } break;
				case ( OPERATOR::MINUS_ASSIGN ):    { value_builtin::sub( ref, src, p ); } break;
				case ( OPERATOR::MULTIPLY_ASSIGN ): { value_builtin::mul( ref, src, p ); } break;
				case ( OPERATOR::DIVIDE_ASSIGN ):   { value_builtin::div( frame_->thread(), ref, src, p ); } break;
				case ( OPERATOR::MODULO_ASSIGN ):   { value_builtin::mod( frame_->thread(), ref, src, p ); } break;
				case ( OPERATOR::POWER_ASSIGN ):    { value_builtin::pow( ref, src, p ); } break;
				default: {
					M_ASSERT( ! "bad code path"[0] );
				} break;
			}
		}
		frame_->values().push( ref );
	}
	return;
	M_EPILOG
}

void HExpression::function_call( HFrame* frame_, int position_ ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( ! frame_->values().is_empty() );
	HHuginn::values_t values;
	while ( frame_->operations().top()._operator == OPERATOR::FUNCTION_ARGUMENT ) {
		frame_->operations().pop();
		M_ASSERT( ! frame_->operations().is_empty() );
		values.push_back( yaal::move( frame_->values().top() ) );
		frame_->values().pop();
		M_ASSERT( ! frame_->values().is_empty() );
	}
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::FUNCTION_CALL );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::HClass const* c( frame_->values().top()->get_class() );
	HHuginn::type_id_t t( c->type_id() );
	if ( ( t != HHuginn::TYPE::FUNCTION_REFERENCE ) && ( t != HHuginn::TYPE::METHOD ) ) {
		throw HHuginn::HHuginnRuntimeException( "Reference `"_ys.append( c->name() ).append( "' is not a function." ), position_ );
	}
	HHuginn::value_t f( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	reverse( values.begin(), values.end() );
	if ( t == HHuginn::TYPE::FUNCTION_REFERENCE ) {
		frame_->values().push( static_cast<HHuginn::HFunctionReference*>( f.raw() )->function()( frame_->thread(), nullptr, values, p ) );
	} else {
		HHuginn::HClass::HBoundMethod* m( dynamic_cast<HHuginn::HClass::HBoundMethod*>( f.raw() ) );
		M_ASSERT( m );
		frame_->values().push( m->function()( frame_->thread(), m->object(), values, p ) );
	}
	return;
	M_EPILOG
}

void HExpression::make_dict( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	struct ValuePosition {
		HHuginn::value_t _value;
		int _position;
		ValuePosition( HHuginn::value_t const& val_, int pos_ )
			: _value( val_ ), _position( pos_ ) {
		}
	};
	typedef yaal::hcore::HArray<ValuePosition> positioned_values_t;
	positioned_values_t values;
	while ( frame_->operations().top()._operator == OPERATOR::FUNCTION_ARGUMENT ) {
		M_ASSERT( ! frame_->operations().is_empty() );
		values.emplace_back( yaal::move( frame_->values().top() ), frame_->operations().top()._position );
		frame_->operations().pop();
		frame_->values().pop();
		M_ASSERT( ! frame_->operations().is_empty() );
	}
	M_ASSERT( ( values.get_size() % 2 ) == 0 );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::FUNCTION_CALL );
	frame_->operations().pop();
	reverse( values.begin(), values.end() );
	HHuginn::value_t dict( frame_->thread()->object_factory().create_dict() );
	HHuginn::HDict* m( static_cast<HHuginn::HDict*>( dict.raw() ) );
	for ( int i( 0 ), S( static_cast<int>( values.get_size() ) ); i < S; i += 2 ) {
		m->insert( values[i]._value, values[i + 1]._value, values[i]._position );
	}
	frame_->values().push( dict );
	return;
	M_EPILOG
}

void HExpression::plus( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::PLUS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::PLUS ), v1->type_id(), v2->type_id(), p );
	}
	value_builtin::add( v1, v2, p );
	frame_->values().push( yaal::move( v1 ) );
	return;
	M_EPILOG
}

void HExpression::minus( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::MINUS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::MINUS ), v1->type_id(), v2->type_id(), p );
	}
	value_builtin::sub( v1, v2, p );
	frame_->values().push( yaal::move( v1 ) );
	return;
	M_EPILOG
}

void HExpression::mul( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::MULTIPLY );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::MULTIPLY ), v1->type_id(), v2->type_id(), p );
	}
	value_builtin::mul( v1, v2, p );
	frame_->values().push( yaal::move( v1 ) );
	return;
	M_EPILOG
}

void HExpression::div( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::DIVIDE );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::DIVIDE ), v1->type_id(), v2->type_id(), p );
	}
	value_builtin::div( frame_->thread(), v1, v2, p );
	frame_->values().push( yaal::move( v1 ) );
	return;
	M_EPILOG
}

void HExpression::mod( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::MODULO );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::MODULO ), v1->type_id(), v2->type_id(), p );
	}
	value_builtin::mod( frame_->thread(), v1, v2, p );
	frame_->values().push( yaal::move( v1 ) );
	return;
	M_EPILOG
}

void HExpression::negate( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::NEGATE );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	M_ASSERT( ! frame_->values().is_empty() );
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	frame_->values().push( value_builtin::neg( frame_->thread(), v, p ) );
	return;
	M_EPILOG
}

void HExpression::power( HFrame* frame_, int ) {
	M_PROLOG
	while ( ! frame_->operations().is_empty() && ( frame_->operations().top()._operator == OPERATOR::POWER ) ) {
		int p( frame_->operations().top()._position );
		frame_->operations().pop();
		HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
		frame_->values().pop();
		HHuginn::value_t v1( frame_->values().top()->clone( &frame_->thread()->huginn() ) );
		frame_->values().pop();
		if ( v1->type_id() != v2->type_id() ) {
			operands_type_mismatch( op_to_str( OPERATOR::POWER ), v1->type_id(), v2->type_id(), p );
		}
		value_builtin::pow( v1, v2, p );
		frame_->values().push( yaal::move( v1 ) );
	}
	return;
	M_EPILOG
}

void HExpression::subscript( ACCESS access_, HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::FUNCTION_ARGUMENT );
	frame_->operations().pop();
	HHuginn::value_t index( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::SUBSCRIPT );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t base( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	frame_->values().push( value_builtin::subscript( frame_->thread(), access_, base, index, p ) );
	return;
	M_EPILOG
}

void HExpression::range( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	HHuginn::value_t from;
	HHuginn::value_t to;
	HHuginn::value_t step;
	HHuginn::value_t* v( &step );
	OPERATOR op( frame_->operations().top()._operator );
	M_ASSERT( ( op == OPERATOR::FUNCTION_ARGUMENT ) || ( op == OPERATOR::SUBSCRIPT_ARGUMENT ) );
	int range( 0 );
	while ( ( op == OPERATOR::FUNCTION_ARGUMENT ) || ( op == OPERATOR::SUBSCRIPT_ARGUMENT ) ) {
		if ( op == OPERATOR::FUNCTION_ARGUMENT ) {
			*v = yaal::move( frame_->values().top() );
			frame_->values().pop();
		} else {
			if ( range == 0 ) {
				v = &to;
			} else {
				v = &from;
			}
			++ range;
		}
		frame_->operations().pop();
		op = frame_->operations().top()._operator;
	}
	M_ASSERT( range <= 2 );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::SUBSCRIPT );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	bool select( ( !! from ) || ( !! to ) || ( !! step ) );
	HHuginn::value_t base( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( ! select && ( range > 0 ) ) {
		HHuginn::type_id_t t( base->type_id() );
		if ( ( t != HHuginn::TYPE::LIST ) && ( t != HHuginn::TYPE::DEQUE ) && ( t != HHuginn::TYPE::STRING ) ) {
			throw HHuginn::HHuginnRuntimeException( "Range operator not supported on `"_ys.append( base->get_class()->name() ).append( "'." ), p );
		}
		frame_->values().push( yaal::move( base ) );
	} else {
		if ( range == 1 ) {
			from = yaal::move( to );
			to = yaal::move( step );
		}
		frame_->values().push( value_builtin::range( frame_->thread(), base, from, to, step, p ) );
	}
	return;
	M_EPILOG
}

void HExpression::equals( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::EQUALS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::type_id_t t1( v1->type_id() );
	HHuginn::type_id_t t2( v2->type_id() );
	if ( ( t1 != t2 ) && ( t1 != HHuginn::TYPE::NONE ) && ( t2 != HHuginn::TYPE::NONE ) ) {
		operands_type_mismatch( op_to_str( OPERATOR::EQUALS ), t1, t2, p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( value_builtin::equals( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::not_equals( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::NOT_EQUALS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::type_id_t t1( v1->type_id() );
	HHuginn::type_id_t t2( v2->type_id() );
	if ( ( t1 != t2 ) && ( t1 != HHuginn::TYPE::NONE ) && ( t2 != HHuginn::TYPE::NONE ) ) {
		operands_type_mismatch( op_to_str( OPERATOR::NOT_EQUALS ), t1, t2, p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( ! value_builtin::equals( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::less( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::LESS );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::LESS ), v1->type_id(), v2->type_id(), p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( value_builtin::less( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::greater( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::GREATER );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::GREATER ), v1->type_id(), v2->type_id(), p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( value_builtin::greater( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::less_or_equal( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::LESS_OR_EQUAL );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::LESS_OR_EQUAL ), v1->type_id(), v2->type_id(), p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( value_builtin::less_or_equal( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::greater_or_equal( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::GREATER_OR_EQUAL );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v1->type_id() != v2->type_id() ) {
		operands_type_mismatch( op_to_str( OPERATOR::GREATER_OR_EQUAL ), v1->type_id(), v2->type_id(), p );
	}
	frame_->values().push( frame_->thread()->object_factory().create_boolean( value_builtin::greater_or_equal( v1, v2, p ) ) );
	return;
	M_EPILOG
}

void HExpression::boolean_and( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::BOOLEAN_AND );
	frame_->operations().pop();
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	M_ASSERT( dynamic_cast<HBooleanEvaluator*>( v.raw() ) );
	frame_->values().push( frame_->thread()->object_factory().create_boolean( static_cast<HBooleanEvaluator*>( v.raw() )->execute( frame_->thread() ) ) );
	return;
	M_EPILOG
}

void HExpression::boolean_or( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::BOOLEAN_OR );
	frame_->operations().pop();
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	M_ASSERT( dynamic_cast<HBooleanEvaluator*>( v.raw() ) );
	frame_->values().push( frame_->thread()->object_factory().create_boolean( static_cast<HBooleanEvaluator*>( v.raw() )->execute( frame_->thread() ) ) );
	return;
	M_EPILOG
}

void HExpression::boolean_xor( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::BOOLEAN_XOR );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v2( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	HHuginn::value_t v1( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( ( v1->type_id() != HHuginn::TYPE::BOOLEAN ) || ( v2->type_id() != HHuginn::TYPE::BOOLEAN ) ) {
		throw HHuginn::HHuginnRuntimeException( _errMsgHHuginn_[ERR_CODE::OPS_NOT_BOOL], p );
	}
	frame_->values().push( value_builtin::boolean_xor( frame_->thread(), v1, v2, p ) );
	return;
	M_EPILOG
}

void HExpression::boolean_not( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::BOOLEAN_NOT );
	int p( frame_->operations().top()._position );
	frame_->operations().pop();
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	if ( v->type_id() != HHuginn::TYPE::BOOLEAN ) {
		throw HHuginn::HHuginnRuntimeException( _errMsgHHuginn_[ERR_CODE::OP_NOT_BOOL], p );
	}
	frame_->values().push( value_builtin::boolean_not( frame_->thread(), v, p ) );
	return;
	M_EPILOG
}

void HExpression::ternary( HFrame* frame_, int ) {
	M_PROLOG
	M_ASSERT( ! frame_->operations().is_empty() );
	M_ASSERT( frame_->operations().top()._operator == OPERATOR::TERNARY );
	frame_->operations().pop();
	HHuginn::value_t v( yaal::move( frame_->values().top() ) );
	frame_->values().pop();
	M_ASSERT( dynamic_cast<HHuginn::HTernaryEvaluator*>( v.raw() ) );
	frame_->values().push( static_cast<HHuginn::HTernaryEvaluator*>( v.raw() )->execute( frame_->thread() ) );
	return;
	M_EPILOG
}

void HExpression::store_direct( HHuginn::value_t const& value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( value_ );
	return;
	M_EPILOG
}

void HExpression::store_real( double long value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( frame_->thread()->object_factory().create_real( value_ ) );
	return;
	M_EPILOG
}

void HExpression::store_integer( int long long value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( frame_->thread()->object_factory().create_integer( value_ ) );
	return;
	M_EPILOG
}

void HExpression::store_string( yaal::hcore::HString const& value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( frame_->thread()->huginn().object_factory()->create_string( value_ ) );
	return;
	M_EPILOG
}

void HExpression::store_number( yaal::hcore::HString const& value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( frame_->thread()->object_factory().create_number( value_ ) );
	return;
	M_EPILOG
}

void HExpression::store_character( char value_, HFrame* frame_, int ) {
	M_PROLOG
	frame_->values().push( frame_->thread()->object_factory().create_character( value_ ) );
	return;
	M_EPILOG
}

void HExpression::do_execute( huginn::HThread* thread_ ) const {
	M_PROLOG
	HFrame* f( thread_->current_frame() );
	for ( execution_step_t const& e : _executionSteps ) {
		e( f );
		if ( ! thread_->can_continue() ) {
			break;
		}
	}
	if ( f->state() != HFrame::STATE::EXCEPTION ) {
		f->set_result( yaal::move( f->values().top() ) );
	}
	f->values().pop();
	return;
	M_EPILOG
}

}

}

}

