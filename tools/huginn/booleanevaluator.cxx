/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "booleanevaluator.hxx"
#include "runtime.hxx"
#include "thread.hxx"
#include "expression.hxx"
#include "helper.hxx"
#include "objectfactory.hxx"
#include "tools/hhuginn.hxx"

namespace yaal {

namespace tools {

namespace huginn {

HBooleanEvaluator::HBooleanEvaluator( HRuntime* runtime_, expressions_t const& expressions_, OPERATOR operator_ )
	: HValue( runtime_->object_factory()->boolean_class() ),
	_expressions( expressions_ ),
	_operator( operator_ ) {
	return;
}

bool HBooleanEvaluator::execute( huginn::HThread* thread_ ) {
	M_PROLOG
	bool all( true );
	HFrame* f( thread_->current_frame() );
	for ( HHuginn::expression_t const& e : _expressions ) {
		e->execute( thread_ );
		if ( ! thread_->can_continue() ) {
			break;
		}
		HHuginn::value_t result( f->result() );
		if ( result->type_id() != HHuginn::TYPE::BOOLEAN ) {
			throw HHuginn::HHuginnRuntimeException(
				hcore::to_string( _errMsgHHuginn_[ERR_CODE::OPS_NOT_BOOL] ).append( type_name( result->type_id() ) ),
				e->file_id(),
				e->position()
			);
		}
		bool v( static_cast<HHuginn::HBoolean*>( result.raw() )->value() );
		if ( ( ( _operator == OPERATOR::BOOLEAN_AND ) && ! v ) || ( ( _operator == OPERATOR::BOOLEAN_OR ) && v ) ) {
			all = false;
			break;
		}
	}
	bool ret( _operator == OPERATOR::BOOLEAN_AND ? all : ! all );
	return ( ret );
	M_EPILOG
}

}

}

}

