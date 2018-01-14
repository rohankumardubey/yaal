/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "introexpression.hxx"
#include "thread.hxx"

using namespace yaal;
using namespace yaal::hcore;

namespace yaal {

namespace tools {

namespace huginn {

HIntroExpression::HIntroExpression( HIntrospectorInterface* introspector_, int fileId_, int position_ )
	: HExpression( fileId_, position_ )
	, _introspector( introspector_ ) {
	return;
}

void HIntroExpression::do_execute( huginn::HThread* thread_ ) const {
	M_PROLOG
	HExpression::do_execute( thread_ );
	if ( thread_->runtime().current_thread() ) {
		thread_->current_frame()->set_position( position() );
		_introspector->introspect( thread_->runtime() );
	}
	return;
	M_EPILOG
}

void HIntroExpression::get_variable_direct_note(
	OExecutionStep const& execionStep_,
	huginn::HFrame* frame_
) {
	M_PROLOG
	frame_->values().emplace( frame_->get_variable( execionStep_._access, execionStep_._statementId, execionStep_._index ) );
	if ( execionStep_._access == HFrame::ACCESS::REFERENCE ) {
		frame_->note_variable( execionStep_._identifierId, execionStep_._statementId, execionStep_._index );
	}
	return;
	M_EPILOG
}

}

}

}

