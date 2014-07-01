/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	executingparser.cxx - this file is integral part of `yaal' project.

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

#include <cctype>

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "executingparser.hxx"

#include "hcore/hstack.hxx"
#include "hcore/hfile.hxx"
#include "hcore/hmultimap.hxx"
#include "tools/escape.hxx"

using namespace yaal::hcore;
using namespace yaal::tools::util;

namespace yaal {

namespace tools {

namespace executing_parser {

namespace {

static char const _raw_[] = "\n\r\t\b\a\f\033\v";
static char const _safe_[] = "nrtbafev";
static EscapeTable const _escapes_( _raw_,  static_cast<int>( sizeof ( _raw_ ) )  - 1,
                                    _safe_, static_cast<int>( sizeof ( _safe_ ) ) - 1 );

}

class HRecursionDetector {
	visited_t _visited;
	typedef yaal::hcore::HStack<visited_t> checkpoints_t;
	checkpoints_t _checkpoints;
public:
	HRecursionDetector( void )
		: _visited(), _checkpoints() {
		_checkpoints.push( visited_t() );
	}
	bool visit( HRuleBase const* rule_ ) {
		M_PROLOG
		if ( ! _checkpoints.top().insert( rule_ ).second )
			throw HRecursionDetectorException( "Infinite recursion detected." );
		return ( _visited.insert( rule_ ).second );
		M_EPILOG
	}
	void reset_visits( void ) {
		M_PROLOG
		_checkpoints.top().clear();
		return;
		M_EPILOG
	}
	void checkpoints_push( void ) {
		M_PROLOG
		_checkpoints.push( _checkpoints.top() );
		return;
		M_EPILOG
	}
	void checkpoints_pop( void ) {
		M_PROLOG
		_checkpoints.pop();
		M_EPILOG
	}
};

class HRuleAggregator {
	typedef yaal::hcore::HArray<HNamedRule*> named_rules_t;
	typedef yaal::hcore::HMap<HRuleBase::ptr_t, named_rules_t> recursions_t;
	recursions_t _recursions;
	visited_t _visited;
public:
	HRuleAggregator( HRuleBase* rule_ )
		: _recursions(), _visited() {
		rule_->find_recursions( *this );
		merge();
	}
	bool visit( HRuleBase const* rule_ ) {
		M_PROLOG
		return ( _visited.insert( rule_ ).second );
		M_EPILOG
	}
	void verify( HNamedRule& nr_ ) {
		M_PROLOG
		HRuleBase::ptr_t r( nr_.rule() );
		if ( !! r ) {
			HRuleRef const* rref( dynamic_cast<HRuleRef const*>( r.raw() ) );
			if ( rref ) {
				HRuleBase::ptr_t rb( rref->get_rule() );
				M_ASSERT( !! rb );
				if ( ! have_strict( rb ) ) {
					nr_.reset( r = rb );
				}
			}
			add( r, &nr_ );
			r->find_recursions( *this );
		}
		return;
		M_EPILOG
	}
private:
	void add( HRuleBase::ptr_t rule_, HNamedRule* parent_ ) {
		M_PROLOG
		_recursions[ rule_ ].push_back( parent_ );
		return;
		M_EPILOG
	}
	bool have_strict( HRuleBase::ptr_t rule_ ) {
		M_PROLOG
		return ( _recursions.count( rule_ ) > 0 );
		M_EPILOG
	}
	void merge( void ) {
		M_PROLOG
		for ( recursions_t::iterator it( _recursions.begin() ), end( _recursions.end() ); it != end; ++ it ) {
			if ( it->second.size() > 1 ) {
				named_rules_t& nrs( it->second );
				named_rules_t::iterator i( nrs.begin() );
				++ i;
				for ( named_rules_t::iterator e( nrs.end() ); i != e; ++ i ) {
					(*i)->reset( make_pointer<HRuleRef>( it->first ) );
				}
			}
		}
		return;
		M_EPILOG
	}
};

}

HExecutingParser::HExecutingParser( executing_parser::HRuleBase const& rule_ )
	: _grammar( rule_.clone() ), _excutors(), _matched( false ),
	_errorPosition( yaal::hcore::HString::npos ), _errorMessages(),
	_inputStart( NULL ) {
	M_PROLOG
	sanitize();
	M_EPILOG
}

bool HExecutingParser::operator()( yaal::hcore::HString const& input_ ) {
	M_PROLOG
	return ( parse( input_.begin(), input_.end() ) );
	M_EPILOG
}

bool HExecutingParser::operator()( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	return ( parse( first_, last_ ) );
	M_EPILOG
}

void HExecutingParser::operator()( void ) {
	M_PROLOG
	execute();
	return;
	M_EPILOG
}

void HExecutingParser::sanitize( void ) {
	M_PROLOG
	try {
		executing_parser::HGrammarDescription gd( *_grammar );
		M_ENSURE( ! gd.is_empty() );
		executing_parser::HRecursionDetector recursionDetector;
		_grammar->detect_recursion( recursionDetector );
	} catch ( executing_parser::HRuleBaseException const& e ) {
		throw HExecutingParserException( e.what() );
	} catch ( executing_parser::HRecursionDetectorException const& e ) {
		throw HExecutingParserException( e.what() );
	} catch ( executing_parser::HGrammarDescriptionException const& e ) {
		throw HExecutingParserException( e.what() );
	}
	return;
	M_EPILOG
}

void HExecutingParser::execute( void ) {
	M_PROLOG
	M_ENSURE( _matched );
	for ( execution_steps_t::iterator it( _excutors.begin() ), end( _excutors.end() ); it != end; ++ it )
		it->second();
	return;
	M_EPILOG
}

bool HExecutingParser::parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	_excutors.clear();
	_inputStart = first_;
	_errorPosition = yaal::hcore::HString::npos;
	_errorMessages.clear();
	yaal::hcore::HString::const_iterator it( _grammar->parse( this, first_, last_ ) );
	if ( executing_parser::HRuleBase::skip_space( it, last_ ) == last_ ) {
		it = last_;
		_errorPosition = yaal::hcore::HString::npos;
		_errorMessages.clear();
	}
	_inputStart = NULL;
	_matched = ( it == last_ );
	return ( _matched );
	M_EPILOG
}

void HExecutingParser::add_execution_step( yaal::hcore::HString::const_iterator position_, executor_t const& executor_ ) {
	M_PROLOG
	_excutors.push_back( make_pair( position_, executor_ ) );
	return;
	M_EPILOG
}

void HExecutingParser::drop_execution_steps( yaal::hcore::HString::const_iterator it_ ) {
	M_PROLOG
	execution_steps_t::iterator it( begin( _excutors ) );
	execution_steps_t::iterator e( end( _excutors ) );
	while ( ( it != e ) && ( it->first != it_ ) )
		++ it;
	_excutors.erase( it, e );
	return;
	M_EPILOG
}

void HExecutingParser::report_error( yaal::hcore::HString::const_iterator position_, yaal::hcore::HString const& message_ ) {
	M_PROLOG
	int long position( position_ - _inputStart );
	if ( position > _errorPosition ) {
		_errorMessages.clear();
		_errorPosition = position;
	}
	if ( position == _errorPosition )
		_errorMessages.push_back( message_ );
	return;
	M_EPILOG
}

int HExecutingParser::error_position( void ) const {
	M_PROLOG
	return ( static_cast<int>( _errorPosition ) );
	M_EPILOG
}

HExecutingParser::messages_t const& HExecutingParser::error_messages( void ) const {
	M_PROLOG
	return ( _errorMessages );
	M_EPILOG
}

void HExecutingParser::HProxy::add_execution_step( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator position_, executor_t const& executor_ ) {
	M_PROLOG
	executingParser_->add_execution_step( position_, executor_ );
	return;
	M_EPILOG
}

void HExecutingParser::HProxy::drop_execution_steps( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator position_ ) {
	M_PROLOG
	executingParser_->drop_execution_steps( position_ );
	return;
	M_EPILOG
}

void HExecutingParser::HProxy::report_error( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator position_, yaal::hcore::HString const& message_ ) {
	M_PROLOG
	executingParser_->report_error( position_, message_ );
	return;
	M_EPILOG
}

namespace executing_parser {

HRuleBase::HRuleBase( void )
	: _action()
	{}

HRuleBase::HRuleBase( action_t const& action_ )
	: _action( action_ )
	{}

HRuleBase::ptr_t HRuleBase::clone( void ) const {
	M_PROLOG
	return ( do_clone() );
	M_EPILOG
}

bool HRuleBase::is_optional( void ) const {
	M_PROLOG
	return ( do_is_optional() );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HRuleBase::parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ASSERT( first_ && last_ );
	yaal::hcore::HString::const_iterator it( do_parse( executingParser_, first_, last_ ) );
	if ( it == first_ )
		HExecutingParser::HProxy::drop_execution_steps( executingParser_, first_ );
	M_ASSERT( it );
	return ( it );
	M_EPILOG
}

void HRuleBase::add_execution_step( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator position_, action_t const& executor_ ) {
	M_PROLOG
	HExecutingParser::HProxy::add_execution_step( executingParser_, position_, executor_ );
	return;
	M_EPILOG
}

void HRuleBase::report_error( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator position_, yaal::hcore::HString const& message_ ) {
	M_PROLOG
	HExecutingParser::HProxy::report_error( executingParser_, position_, message_ );
	return;
	M_EPILOG
}


void HRuleBase::describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	int long descriptionLen( rd_.description().get_length() );
	int long childCount( rd_.children().get_size() );
	do_describe( rd_, ru_ );
	M_ENSURE_EX( ( rd_.description().get_length() > descriptionLen ) || ( rd_.children().get_size() > childCount ), "empty rule" );
	return;
	M_EPILOG
}

void HRuleBase::rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	do_rule_use( ruleUse_ );
	return;
	M_EPILOG
}

void HRuleBase::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	++ ruleUse_[ this ];
	return;
	M_EPILOG
}

bool HRuleBase::do_is_optional( void ) const
	{ return ( false ); }

yaal::hcore::HString::const_iterator HRuleBase::skip_space( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ASSERT( first_ && last_ );
	while ( ( first_ != last_ ) && isspace( *first_ ) )
		++ first_;
	return ( first_ );
	M_EPILOG
}

void HRuleBase::detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	visited_t::insert_result ir( visited_.insert( this ) );
	if ( ir.second )
		do_detach( rule_, visited_, detachAll_ );
	return;
	M_EPILOG
}

void HRuleBase::detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	if ( recursionDetector_.visit( this ) )
		do_detect_recursion( recursionDetector_ );
	return;
	M_EPILOG
}

void HRuleBase::find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	if ( recursions_.visit( this ) )
		do_find_recursions( recursions_ );
	return;
	M_EPILOG
}

HNamedRule::HNamedRule( yaal::hcore::HString const& name_, ptr_t rule_ )
	: _name( name_ ), _rule( rule_ ) {
	M_ENSURE( name_.is_empty() || ( name_[name_.get_length() - 1] != '_' ) );
}

HNamedRule::HNamedRule( HRuleBase const& rule_ )
	: _name( dynamic_cast<HRule const*>( &rule_ ) ? dynamic_cast<HRule const*>( &rule_ )->get_name() : yaal::hcore::HString() ), _rule( rule_.clone() )
	{}

yaal::hcore::HString const& HNamedRule::name( void ) const {
	M_PROLOG
	return ( _name );
	M_EPILOG
}

HRuleBase const* HNamedRule::id( void ) const {
	M_PROLOG
	HRuleRef const* rr( dynamic_cast<HRuleRef const*>( _rule.raw() ) );
	return ( rr ? rr->get_rule().raw() : _rule.raw() );
	M_EPILOG
}

void HNamedRule::reset( ptr_t const& rule_ ) {
	M_PROLOG
	_rule = rule_;
	return;
	M_EPILOG
}

HNamedRule::ptr_t const& HNamedRule::rule( void ) const {
	M_PROLOG
	return ( _rule );
	M_EPILOG
}

HNamedRule::ptr_t& HNamedRule::rule( void ) {
	M_PROLOG
	return ( _rule );
	M_EPILOG
}

void HNamedRule::describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	M_ASSERT( !! _rule );
	rule_use_t::const_iterator it( ru_.find( _rule.get() ) );
	HRuleRef const* rr( dynamic_cast<HRuleRef const*>( &*_rule ) );
	if ( ( ( it != ru_.end() ) && ( it->second > 1 ) ) || ( ! _name.is_empty() ) || rr ) {
		if ( ! rr )
			rd_.add( this );
		rd_.desc( rd_.make_name( *this ) );
	} else
		_rule->describe( rd_, ru_ );
	return;
	M_EPILOG
}

HRule::HRule( yaal::hcore::HString const& name_ )
	: HRuleBase(), _rule( name_, make_pointer<HRecursiveRule>() ), _completelyDefined( false ) {
}

HRule::HRule( void )
	: HRuleBase(), _rule( make_pointer<HRecursiveRule>() ), _completelyDefined( false ) {
}

HRule::HRule( HRule const& rule_ )
	: HRuleBase( rule_._action ), _rule( rule_._rule ), _completelyDefined( rule_._completelyDefined ) {
}

HRule::HRule( HRuleBase const& rule_ )
	: HRuleBase(), _rule( rule_.clone() ), _completelyDefined( true ) {
}

HRule::HRule( ptr_t const& rule_ )
	: _rule( rule_ ), _completelyDefined( true ) {
}

HRule::HRule( yaal::hcore::HString const& name_, HRuleBase const& rule_ )
	: HRuleBase(), _rule( name_, rule_.clone() ), _completelyDefined( true ) {
}

HRule::HRule( yaal::hcore::HString const& name_, ptr_t const& rule_ )
	: _rule( name_, rule_ ), _completelyDefined( true ) {
}

HRule::HRule( yaal::hcore::HString const& name_, ptr_t const& rule_, action_t const& action_ )
	: HRuleBase( action_ ), _rule( name_, rule_ ), _completelyDefined( true ) {
}

HRule::~HRule( void ) {
	M_PROLOG
	return;
	M_DESTRUCTOR_EPILOG
}

HRule& HRule::operator %= ( HRuleBase const& rule_ ) {
	M_PROLOG
	M_ENSURE( ! _completelyDefined );
	HRecursiveRule* rr( dynamic_cast<HRecursiveRule*>( _rule.rule().raw() ) );
	M_ENSURE( rr );
	rr->set_rule( rule_.clone() );
	HRuleAggregator( this );
	_completelyDefined = true;
	return ( *this );
	M_EPILOG
}

HRule HRule::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ! _action );
	M_ENSURE( !! _rule );
	return ( HRule( _rule.name(), _rule.rule(), action_ ) );
	M_EPILOG
}

yaal::hcore::HString const& HRule::get_name( void ) const {
	M_PROLOG
	return ( _rule.name() );
	M_EPILOG
}

HNamedRule const* HRule::get_named_rule( void ) const {
	M_PROLOG
	return ( &_rule );
	M_EPILOG
}

HRule::ptr_t HRule::do_clone( void ) const {
	M_PROLOG
	M_ENSURE( !! _rule );
	return ( ! _action ? _rule.rule() : pointer_static_cast<HRuleBase>( make_pointer<HRule>( _rule.name(), _rule.rule(), _action ) ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HRule::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator ret( !! _rule ? _rule->parse( executingParser_, first_, last_ ) : first_ );
	if ( ( ret != first_ ) && !! _action )
		add_execution_step( executingParser_, first_, _action );
	return ( ret );
	M_EPILOG
}

void HRule::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r ) {
		bool detachThis( r.raw() == rule_ );
		if ( detachThis && detachAll_ )
			_rule.reset( make_pointer<HRuleRef>( r ) );
		else {
			if ( detachThis )
				detachAll_ = true;
			r->detach( rule_, visited_, detachAll_ );
		}
	}
	return;
	M_EPILOG
}

void HRule::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r )
		r->detect_recursion( recursionDetector_ );
	return;
	M_EPILOG
}

void HRule::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	recursions_.verify( _rule );
	return;
	M_EPILOG
}

bool HRule::do_is_optional( void ) const
	{ return ( _rule->is_optional() ); }

void HRule::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	if ( !! _rule ) {
		rd_.desc( rd_.make_name( _rule ) );
		rd_.desc( " = " );
		_rule.describe( rd_, ru_ );
	}
	return;
	M_EPILOG
}

void HRule::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( ( use == 1 ) && !! _rule )
		_rule->rule_use( ruleUse_ );
	return;
	M_EPILOG
}

HRecursiveRule::HRecursiveRule( void )
	: _rule() {
}

void HRecursiveRule::set_rule( HRuleBase::ptr_t const& rule_ ) {
	M_PROLOG
	/*
	 * HRecursiveRule can be set to other HRecursiveRule! :(
	 */
	_rule = rule_;
	visited_t visited;
	bool detachAll( true );
	_rule->detach( this, visited, detachAll );
	return;
	M_EPILOG
}

yaal::hcore::HString::const_iterator HRecursiveRule::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ENSURE( !! _rule );
	return ( _rule->parse( executingParser_, first_, last_ ) );
	M_EPILOG
}

HRuleBase::ptr_t HRecursiveRule::do_clone( void ) const {
	M_PROLOG
	return ( get_pointer() );
	M_EPILOG
}

bool HRecursiveRule::do_is_optional( void ) const {
	M_PROLOG
	M_ENSURE( _rule );
	return ( _rule->is_optional() );
	M_EPILOG
}

void HRecursiveRule::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	if ( !! _rule )
		_rule.describe( rd_, ru_ );
	return;
	M_EPILOG
}

void HRecursiveRule::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( ( use == 1 ) && !! _rule )
		_rule->rule_use( ruleUse_ );
	return;
	M_EPILOG
}

void HRecursiveRule::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	/*
	 * HRecursiveRule can be omitted once while detaching!
	 * It is impossible then to perform this test: M_ENSURE( rule_ != this ).
	 */
	if ( !! _rule )
		_rule->detach( rule_, visited_, detachAll_ );
	return;
	M_EPILOG
}

void HRecursiveRule::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	if ( !! _rule )
		_rule->detect_recursion( recursionDetector_ );
	return;
	M_EPILOG
}

void HRecursiveRule::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	if ( !! _rule )
		_rule->find_recursions( recursions_ );
	return;
	M_EPILOG
}

HRuleRef::HRuleRef( HRuleBase::ptr_t rule_ )
	: _rule( rule_ ) {
	M_ASSERT( !!rule_ );
	return;
}

yaal::hcore::HString::const_iterator HRuleRef::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	return ( r->parse( executingParser_, first_, last_ ) );
	M_EPILOG
}

HRuleBase::ptr_t HRuleRef::do_clone( void ) const {
	M_PROLOG
	return ( get_pointer() );
	M_EPILOG
}

bool HRuleRef::do_is_optional( void ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	return ( r->is_optional() );
	M_EPILOG
}

void HRuleRef::do_describe( HRuleDescription&, rule_use_t const& ) const {
	M_PROLOG
	M_ASSERT( 0 && "recursive describe on rule reference" );
#ifdef NDEBUG
	return;
#endif /* #ifdef NDEBUG */
	M_EPILOG
}

void HRuleRef::do_rule_use( rule_use_t& ru_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	r->rule_use( ru_ );
	return;
	M_EPILOG
}

void HRuleRef::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	if ( r.raw() != rule_ )
		r->detach( rule_, visited_, detachAll_ );
	return;
	M_EPILOG
}

void HRuleRef::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	r->detect_recursion( recursionDetector_ );
	return;
	M_EPILOG
}

void HRuleRef::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	M_ASSERT( !! r );
	r->find_recursions( recursions_ );
	return;
	M_EPILOG
}

HRuleBase::ptr_t HRuleRef::get_rule( void ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule );
	return ( r );
	M_EPILOG
}

HFollows::HFollows( HRuleBase const& predecessor_, HRuleBase const& successor_ )
	: HRuleBase(), _rules() {
	M_PROLOG
	_rules.push_back( predecessor_ );
	_rules.push_back( successor_ );
	HRuleAggregator rra( this );
	return;
	M_EPILOG
}

HFollows::HFollows( HFollows const& follows_ )
	: HRuleBase( follows_._action ), _rules() {
	M_PROLOG
	for ( rules_t::const_iterator it( follows_._rules.begin() ), end( follows_._rules.end() ); it != end; ++ it )
		_rules.push_back( *it );
	return;
	M_EPILOG
}

HFollows::HFollows( HFollows const& predecessors_, HRuleBase const& successor_ )
	: HRuleBase(), _rules() {
	M_PROLOG
	for ( rules_t::const_iterator it( predecessors_._rules.begin() ), end( predecessors_._rules.end() ); it != end; ++ it )
		_rules.push_back( *it );
	_rules.push_back( successor_ );
	HRuleAggregator rra( this );
	return;
	M_EPILOG
}

HFollows::HFollows( HFollows::rules_t const& rules_, action_t const& action_ )
	: HRuleBase( action_ ), _rules( rules_ ) {
	return;
}

HFollows HFollows::operator[]( action_t const& action_ ) const {
	M_PROLOG
	return ( HFollows( _rules, action_ ) );
	M_EPILOG
}

HRuleBase::ptr_t HFollows::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HFollows>( *this ) );
	M_EPILOG
}

void HFollows::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	bool next( false );
	for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		if ( next )
			rd_.desc( " >> " );
		it->describe( rd_, ru_ );
		next = true;
	}
	return;
	M_EPILOG
}

void HFollows::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( use == 1 ) {
		for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it )
			(*it)->rule_use( ruleUse_ );
	}
	return;
	M_EPILOG
}

yaal::hcore::HString::const_iterator HFollows::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator orig( first_ );
	bool matched( true );
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		yaal::hcore::HString::const_iterator old( first_ );
		first_ = (*it)->parse( executingParser_, first_, last_ );
		M_ASSERT( first_ );
		if ( ( first_ == old ) && ( !(*it)->is_optional() ) ) {
			matched = false;
			break;
		}
	}
	if ( matched && !! _action )
		add_execution_step( executingParser_, orig, _action );
	return ( matched ? first_ : orig );
	M_EPILOG
}

void HFollows::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		HRuleBase::ptr_t r( it->rule() );
		if ( !! r ) {
			bool detachThis( r.raw() == rule_ );
			if ( detachThis && detachAll_ )
				it->reset( make_pointer<HRuleRef>( r ) );
			else {
				if ( detachThis )
					detachAll_ = true;
				r->detach( rule_, visited_, detachAll_ );
			}
		}
	}
	return;
	M_EPILOG
}

void HFollows::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		HRuleBase::ptr_t r( it->rule() );
		if ( !! r ) {
			r->detect_recursion( recursionDetector_ );
		}
	}
	return;
	M_EPILOG
}

void HFollows::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		recursions_.verify( *it );
	}
	return;
	M_EPILOG
}

HKleeneBase::HKleeneBase( HRuleBase const& rule_ )
	: HRuleBase(), _rule( rule_ )
	{}

HKleeneBase::HKleeneBase( HNamedRule const& rule_, action_t const& action_ )
	: HRuleBase( action_ ), _rule( rule_ )
	{}

yaal::hcore::HString::const_iterator HKleeneBase::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	yaal::hcore::HString::const_iterator origScan( scan );
	yaal::hcore::HString::const_iterator old( scan );
	while ( scan != last_ ) {
		scan = _rule->parse( executingParser_, old, last_ );
		if ( scan == old )
			break;
		old = scan;
	}
	if ( scan != origScan ) {
		if ( !! _action )
			add_execution_step( executingParser_, first_, _action );
		first_ = scan;
	}
	return ( first_ );
	M_EPILOG
}

void HKleeneBase::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( ( use == 1 ) && !! _rule )
		_rule->rule_use( ruleUse_ );
	return;
	M_EPILOG
}

void HKleeneBase::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r ) {
		bool detachThis( r.raw() == rule_ );
		if ( detachThis && detachAll_ )
			_rule.reset( make_pointer<HRuleRef>( r ) );
		else {
			if ( detachThis )
				detachAll_ = true;
			r->detach( rule_, visited_, detachAll_ );
		}
	}
	return;
	M_EPILOG
}

void HKleeneBase::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	recursions_.verify( _rule );
	return;
	M_EPILOG
}

HKleeneStar::HKleeneStar( HRuleBase const& rule_ )
	: HKleeneBase( rule_ )
	{}

HKleeneStar::HKleeneStar( HNamedRule const& rule_, action_t const& action_ )
	: HKleeneBase( rule_, action_ )
	{}

HKleeneStar::HKleeneStar( HKleeneStar const& kleeneStar_ )
	: HKleeneBase( kleeneStar_._rule, kleeneStar_._action )
	{}

HKleeneStar HKleeneStar::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ! _action );
	return ( HKleeneStar( _rule, action_ ) );
	M_EPILOG
}

HRuleBase::ptr_t HKleeneStar::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HKleeneStar>( *this ) );
	M_EPILOG
}

bool HKleeneStar::do_is_optional( void ) const
	{ return ( true ); }

void HKleeneStar::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	rd_.desc( "*( " );
	_rule.describe( rd_, ru_ );
	rd_.desc( " )" );
	return;
	M_EPILOG
}

void HKleeneStar::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r ) {
		recursionDetector_.checkpoints_push();
		r->detect_recursion( recursionDetector_ );
		recursionDetector_.checkpoints_pop();
	}
	return;
	M_EPILOG
}

HKleenePlus::HKleenePlus( HRuleBase const& rule_ )
	: HKleeneBase( rule_ )
	{}

HKleenePlus::HKleenePlus( HNamedRule const& rule_, action_t const& action_ )
	: HKleeneBase( rule_, action_ )
	{}

HKleenePlus HKleenePlus::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ! _action );
	return ( HKleenePlus( _rule, action_ ) );
	M_EPILOG
}

HKleenePlus::HKleenePlus( HKleenePlus const& kleenePlus_ )
	: HKleeneBase( kleenePlus_._rule, kleenePlus_._action )
	{}

HRuleBase::ptr_t HKleenePlus::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HKleenePlus>( *this ) );
	M_EPILOG
}

void HKleenePlus::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	rd_.desc( "+( " );
	_rule.describe( rd_, ru_ );
	rd_.desc( " )" );
	return;
	M_EPILOG
}

void HKleenePlus::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r )
		r->detect_recursion( recursionDetector_ );
	return;
	M_EPILOG
}

HAlternative::HAlternative( HRuleBase const& choice1_, HRuleBase const& choice2_ )
	: HRuleBase(), _rules() {
	M_PROLOG
	_rules.push_back( choice1_ );
	_rules.push_back( choice2_ );
	HRuleAggregator rra( this );
	return;
	M_EPILOG
}

HAlternative::HAlternative( HAlternative const& alternative_ )
	: HRuleBase( alternative_._action ), _rules() {
	M_PROLOG
	for ( rules_t::const_iterator it( alternative_._rules.begin() ), end( alternative_._rules.end() ); it != end; ++ it )
		_rules.push_back( *it );
	return;
	M_EPILOG
}

HAlternative::HAlternative( HAlternative const& alternative_, HRuleBase const& choice_ )
	: HRuleBase(), _rules() {
	M_PROLOG
	for ( rules_t::const_iterator it( alternative_._rules.begin() ), end( alternative_._rules.end() ); it != end; ++ it )
		_rules.push_back( *it );
	_rules.push_back( choice_ );
	HRuleAggregator rra( this );
	return;
	M_EPILOG
}

HRuleBase::ptr_t HAlternative::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HAlternative>( *this ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HAlternative::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	yaal::hcore::HString::const_iterator scan( first_ );
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		if ( ( scan = (*it)->parse( executingParser_, first_, last_ ) ) != first_ )
			break;
	}
	if ( scan != first_ ) {
		if( !! _action )
			add_execution_step( executingParser_, first_, _action );
		first_ = scan;
	}
	return ( first_ );
}

void HAlternative::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	bool next( false );
	for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		if ( next )
			rd_.desc( " | " );
		it->describe( rd_, ru_ );
		next = true;
	}
	return;
	M_EPILOG
}

void HAlternative::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( use == 1 ) {
		for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it )
			(*it)->rule_use( ruleUse_ );
	}
	return;
	M_EPILOG
}

void HAlternative::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		HRuleBase::ptr_t r( it->rule() );
		if ( !! r ) {
			bool detachThis( r.raw() == rule_ );
			if ( detachThis && detachAll_ )
				it->reset( make_pointer<HRuleRef>( r ) );
			else {
				if ( detachThis )
					detachAll_ = true;
				r->detach( rule_, visited_, detachAll_ );
			}
		}
	}
	return;
	M_EPILOG
}

void HAlternative::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	for ( rules_t::const_iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		HRuleBase::ptr_t r( it->rule() );
		if ( !! r ) {
			recursionDetector_.checkpoints_push();
			r->detect_recursion( recursionDetector_ );
			recursionDetector_.checkpoints_pop();
		}
	}
	return;
	M_EPILOG
}

void HAlternative::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	for ( rules_t::iterator it( _rules.begin() ), end( _rules.end() ); it != end; ++ it ) {
		recursions_.verify( *it );
	}
	return;
	M_EPILOG
}

HOptional::HOptional( HRuleBase const& rule_ )
	: HRuleBase(), _rule( rule_ ) {
}

HOptional::HOptional( HOptional const& optional_ )
	: HRuleBase( optional_._action ), _rule( optional_._rule ) {
}

HOptional::HOptional( HNamedRule const& rule_, action_t const& action_ )
	: HRuleBase( action_ ), _rule( rule_ ) {
}

HRuleBase::ptr_t HOptional::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HOptional>( *this ) );
	M_EPILOG
}

HOptional HOptional::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ! _action );
	return ( HOptional( _rule, action_ ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HOptional::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator ret( !! _rule ? _rule->parse( executingParser_, first_, last_ ) : first_ );
	if ( ( ret != first_ ) && !! _action )
		add_execution_step( executingParser_, first_, _action );
	return ( ret );
	M_EPILOG
}

bool HOptional::do_is_optional( void ) const
	{ return ( true ); }

void HOptional::do_describe( HRuleDescription& rd_, rule_use_t const& ru_ ) const {
	M_PROLOG
	rd_.desc( "-( " );
	_rule.describe( rd_, ru_ );
	rd_.desc( " )" );
	return;
	M_EPILOG
}

void HOptional::do_rule_use( rule_use_t& ruleUse_ ) const {
	M_PROLOG
	int use( ++ ruleUse_[ this ] );
	if ( ( use == 1 ) && !! _rule )
		_rule->rule_use( ruleUse_ );
	return;
	M_EPILOG
}

void HOptional::do_detach( HRuleBase const* rule_, visited_t& visited_, bool& detachAll_ ) {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r ) {
		bool detachThis( r.raw() == rule_ );
		if ( detachThis && detachAll_ )
			_rule.reset( make_pointer<HRuleRef>( r ) );
		else {
			if ( detachThis )
				detachAll_ = true;
			r->detach( rule_, visited_, detachAll_ );
		}
	}
	return;
	M_EPILOG
}

void HOptional::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	HRuleBase::ptr_t r( _rule.rule() );
	if ( !! r ) {
		recursionDetector_.checkpoints_push();
		r->detect_recursion( recursionDetector_ );
		recursionDetector_.checkpoints_pop();
	}
	return;
	M_EPILOG
}

void HOptional::do_find_recursions( HRuleAggregator& recursions_ ) {
	M_PROLOG
	recursions_.verify( _rule );
	return;
	M_EPILOG
}

HFollows operator >> ( HRuleBase const& predecessor_, HRuleBase const& successor_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, successor_ ) );
	M_EPILOG
}

HFollows operator >> ( HFollows const& predecessors_, HRuleBase const& successor_ ) {
	M_PROLOG
	return ( HFollows( predecessors_, successor_ ) );
	M_EPILOG
}

HAlternative operator | ( HRuleBase const& choice1_, HRuleBase const& choice2_ ) {
	M_PROLOG
	return ( HAlternative( choice1_, choice2_ ) );
	M_EPILOG
}

HAlternative operator | ( HAlternative const& alternative_, HRuleBase const& choice_ ) {
	M_PROLOG
	return ( HAlternative( alternative_, choice_ ) );
	M_EPILOG
}

HKleeneStar operator* ( HRuleBase const& rule_ ) {
	M_PROLOG
	return ( HKleeneStar( rule_ ) );
	M_EPILOG
}

HKleenePlus operator+ ( HRuleBase const& rule_ ) {
	M_PROLOG
	return ( HKleenePlus( rule_ ) );
	M_EPILOG
}

HOptional operator - ( HRuleBase const& rule_ ) {
	M_PROLOG
	return ( HOptional( rule_ ) );
	M_EPILOG
}

HReal::HReal( void )
	: HRuleBase(), _actionDouble(), _actionDoubleLong(),
	_actionNumber(), _actionString(), _cache()
	{}

HReal::HReal( action_t const& action_ )
	: HRuleBase( action_ ), _actionDouble(), _actionDoubleLong(),
	_actionNumber(), _actionString(), _cache()
	{}

HReal::HReal( action_double_t const& action_ )
	: HRuleBase(), _actionDouble( action_ ), _actionDoubleLong(),
	_actionNumber(), _actionString(), _cache()
	{}

HReal::HReal( action_double_long_t const& action_ )
	: HRuleBase(), _actionDouble(), _actionDoubleLong( action_ ),
	_actionNumber(), _actionString(), _cache()
	{}

HReal::HReal( action_number_t const& action_ )
	: HRuleBase(), _actionDouble(), _actionDoubleLong(),
	_actionNumber( action_ ), _actionString(), _cache()
	{}

HReal::HReal( action_string_t const& action_ )
	: HRuleBase(), _actionDouble(), _actionDoubleLong(),
	_actionNumber(), _actionString( action_ ), _cache()
	{}

HReal::HReal( HReal const& real_ )
	: HRuleBase( real_._action ), _actionDouble( real_._actionDouble ), _actionDoubleLong( real_._actionDoubleLong ),
	_actionNumber( real_._actionNumber ), _actionString( real_._actionString ), _cache( real_._cache )
	{}

HReal HReal::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionDouble ) && ( ! _actionDoubleLong ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HReal( action_ ) );
	M_EPILOG
}

HReal HReal::operator[]( action_double_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionDouble ) && ( ! _actionDoubleLong ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HReal( action_ ) );
	M_EPILOG
}

HReal HReal::operator[]( action_double_long_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionDouble ) && ( ! _actionDoubleLong ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HReal( action_ ) );
	M_EPILOG
}

HReal HReal::operator[]( action_number_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionDouble ) && ( ! _actionDoubleLong ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HReal( action_ ) );
	M_EPILOG
}

HReal HReal::operator[]( action_string_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionDouble ) && ( ! _actionDoubleLong ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HReal( action_ ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HReal::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	_cache.clear();
	real_paring_state_t state( START );
	while ( scan != last_ ) {
		bool stop( false );
		char ch( *scan );
		switch ( state ) {
			case ( START ): {
				if ( isdigit( ch ) )
					state = INTEGRAL;
				else if ( ch == '-' )
					state = MINUS;
				else if ( ch == '.' )
					state = DOT;
				else
					stop = true;
			} break;
			case ( MINUS ):
			case ( INTEGRAL ): {
				if ( isdigit( ch ) )
					state = INTEGRAL;
				else if ( ch == '.' )
					state = DOT;
				else
					stop = true;
			} break;
			case ( DOT ):
			case ( DECIMAL ): {
				if ( isdigit( ch ) )
					state = DECIMAL;
				else
					stop = true;
			} break;
			default: {
				M_ASSERT( ! "invalid hardcoded state"[0] );
			}
		}
		if ( stop )
			break;
		_cache.push_back( *scan );
		++ scan;
	}
	if ( state >= INTEGRAL ) {
		if ( !! _actionDouble ) {
			double d( lexical_cast<double>( _cache ) );
			add_execution_step( executingParser_, first_, call( _actionDouble, d ) );
		} else if ( !! _actionDoubleLong ) {
			double long dl( lexical_cast<double long>( _cache ) );
			add_execution_step( executingParser_, first_, call( _actionDoubleLong, dl ) );
		} else if ( !! _actionNumber ) {
			add_execution_step( executingParser_, first_, call( _actionNumber, _cache ) );
		} else if ( !! _actionString ) {
			add_execution_step( executingParser_, first_, call( _actionString, _cache ) );
		} else if ( !! _action )
			add_execution_step( executingParser_, first_, call( _action ) );
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected real number" );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HReal::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HReal>( *this ) );
	M_EPILOG
}

void HReal::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "real" );
	return;
	M_EPILOG
}

void HReal::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HReal::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HReal::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HReal const& get_real_instance( void ) {
	M_PROLOG
	static HReal realInstance;
	return ( realInstance );
	M_EPILOG
}

HReal const& real( get_real_instance() );

HInteger::HInteger( void )
	: HRuleBase(), _actionIntLong(), _actionInt(),
	_actionNumber(), _actionString(), _cache()
	{}

HInteger::HInteger( action_t const& action_ )
	: HRuleBase( action_ ), _actionIntLong(), _actionInt(),
	_actionNumber(), _actionString(), _cache()
	{}

HInteger::HInteger( action_int_long_t const& action_ )
	: HRuleBase(), _actionIntLong( action_ ), _actionInt(),
	_actionNumber(), _actionString(), _cache()
	{}

HInteger::HInteger( action_int_t const& action_ )
	: HRuleBase(), _actionIntLong(), _actionInt( action_ ),
	_actionNumber(), _actionString(), _cache()
	{}

HInteger::HInteger( action_number_t const& action_ )
	: HRuleBase(), _actionIntLong(), _actionInt(),
	_actionNumber( action_ ), _actionString(), _cache()
	{}

HInteger::HInteger( action_string_t const& action_ )
	: HRuleBase(), _actionIntLong(), _actionInt(),
	_actionNumber(), _actionString( action_ ), _cache()
	{}

HInteger::HInteger( HInteger const& integer_ )
	: HRuleBase( integer_._action ), _actionIntLong( integer_._actionIntLong ), _actionInt( integer_._actionInt ),
	_actionNumber( integer_._actionNumber ), _actionString( integer_._actionString ), _cache( integer_._cache )
	{}

HInteger HInteger::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionIntLong ) && ( ! _actionInt ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HInteger( action_ ) );
	M_EPILOG
}

HInteger HInteger::operator[]( action_int_long_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionIntLong ) && ( ! _actionInt ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HInteger( action_ ) );
	M_EPILOG
}

HInteger HInteger::operator[]( action_int_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionIntLong ) && ( ! _actionInt ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HInteger( action_ ) );
	M_EPILOG
}

HInteger HInteger::operator[]( action_number_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionIntLong ) && ( ! _actionInt ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HInteger( action_ ) );
	M_EPILOG
}

HInteger HInteger::operator[]( action_string_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionIntLong ) && ( ! _actionInt ) && ( ! _actionNumber ) && ( ! _actionString ) );
	return ( HInteger( action_ ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HInteger::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	_cache.clear();
	real_paring_state_t state( START );
	while ( scan != last_ ) {
		bool stop( false );
		char ch( *scan );
		switch ( state ) {
			case ( START ): {
				if ( isdigit( ch ) )
					state = DIGIT;
				else if ( ch == '-' )
					state = MINUS;
				else
					stop = true;
			} break;
			case ( MINUS ):
			case ( DIGIT ): {
				if ( isdigit( ch ) )
					state = DIGIT;
				else
					stop = true;
			} break;
			default: {
				M_ASSERT( ! "invalid hardcoded state"[0] );
			}
		}
		if ( stop )
			break;
		_cache.push_back( *scan );
		++ scan;
	}
	if ( state >= DIGIT ) {
		if ( !! _actionIntLong ) {
			int long il( lexical_cast<int long>( _cache ) );
			add_execution_step( executingParser_, first_, call( _actionIntLong, il ) );
		} else if ( !! _actionInt ) {
			int i( lexical_cast<int>( _cache ) );
			add_execution_step( executingParser_, first_, call( _actionInt, i ) );
		} else if ( !! _actionNumber ) {
			add_execution_step( executingParser_, first_, call( _actionNumber, _cache ) );
		} else if ( !! _actionString ) {
			add_execution_step( executingParser_, first_, call( _actionString, _cache ) );
		} else if ( !! _action )
			add_execution_step( executingParser_, first_, call( _action ) );
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected integral number" );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HInteger::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HInteger>( *this ) );
	M_EPILOG
}

void HInteger::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "integer" );
	return;
	M_EPILOG
}

void HInteger::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HInteger::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HInteger::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HInteger const& get_integer_instance( void ) {
	M_PROLOG
	static HInteger integerInstance;
	return ( integerInstance );
	M_EPILOG
}

HInteger const& integer( get_integer_instance() );

HStringLiteral::HStringLiteral( void )
	: HRuleBase(), _actionString(), _cache() {
}

HStringLiteral::HStringLiteral( action_t const& action_ )
	: HRuleBase( action_ ), _actionString(), _cache() {
}

HStringLiteral::HStringLiteral( action_string_t const& action_ )
	: HRuleBase(), _actionString( action_ ), _cache() {
}

HStringLiteral::HStringLiteral( HStringLiteral const& stringLiteral_ )
	: HRuleBase( stringLiteral_._action ), _actionString( stringLiteral_._actionString ),
	_cache( stringLiteral_._cache ) {
}

HStringLiteral HStringLiteral::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HStringLiteral( action_ ) );
	M_EPILOG
}

HStringLiteral HStringLiteral::operator[]( action_string_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HStringLiteral( action_ ) );
	M_EPILOG
}

namespace {

/*
 * Only printable characters and horizontal tab are valid in string or character literals.
 */
bool is_known_character( char char_ ) {
	char unsigned c( static_cast<char unsigned>( char_ ) );
	return ( ( c >= ' ' ) || ( c == '\t' ) );
}

}

yaal::hcore::HString::const_iterator HStringLiteral::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	bool valid( false );
	yaal::hcore::HString::const_iterator start( NULL );
	do {
		if ( ! ( ( scan != last_ ) && ( *scan == '"' ) ) ) {
			break;
		}
		++ scan;
		start = scan;
		while ( ( scan != last_ ) && ( *scan != '"' ) ) {
			if ( *scan == '\\' ) {
				++ scan;
				if ( ! ( scan != last_ ) ) {
					break;
				}
			}
			if ( ! is_known_character( *scan ) ) {
				break;
			}
			++ scan;
		}
		if ( scan != last_ ) {
			if ( *scan != '"' ) {
				break;
			}
			valid = true;
		}
	} while ( false );
	if ( valid ) {
		_cache.assign( start, scan );
		++ scan;
		unescape( _cache, _escapes_ );
		if ( !! _actionString )
			add_execution_step( executingParser_, first_, call( _actionString, _cache ) );
		else if ( !! _action )
			add_execution_step( executingParser_, first_, _action );
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected literal string" );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HStringLiteral::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HStringLiteral>( *this ) );
	M_EPILOG
}

void HStringLiteral::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "string_literal" );
	return;
	M_EPILOG
}

void HStringLiteral::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HStringLiteral::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HStringLiteral::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HStringLiteral const& get_string_literal_instance( void ) {
	M_PROLOG
	static HStringLiteral stringLiteralInstance;
	return ( stringLiteralInstance );
	M_EPILOG
}

HStringLiteral const& string_literal( get_string_literal_instance() );

HCharacterLiteral::HCharacterLiteral( void )
	: HRuleBase(), _actionCharacter(), _cache() {
}

HCharacterLiteral::HCharacterLiteral( action_t const& action_ )
	: HRuleBase( action_ ), _actionCharacter(), _cache() {
}

HCharacterLiteral::HCharacterLiteral( action_character_t const& action_ )
	: HRuleBase(), _actionCharacter( action_ ), _cache() {
}

HCharacterLiteral::HCharacterLiteral( HCharacterLiteral const& stringLiteral_ )
	: HRuleBase( stringLiteral_._action ), _actionCharacter( stringLiteral_._actionCharacter ),
	_cache( stringLiteral_._cache ) {
}

HCharacterLiteral HCharacterLiteral::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionCharacter ) );
	return ( HCharacterLiteral( action_ ) );
	M_EPILOG
}

HCharacterLiteral HCharacterLiteral::operator[]( action_character_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionCharacter ) );
	return ( HCharacterLiteral( action_ ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HCharacterLiteral::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	bool valid( false );
	yaal::hcore::HString::const_iterator start( NULL );
	do {
		if ( ! ( ( scan != last_ ) && ( *scan == '\'' ) ) ) {
			break;
		}
		++ scan;
		start = scan;
		if ( scan != last_ ) {
			if ( *scan == '\'' ) {
				break;
			}
			if ( *scan == '\\' ) {
				++ scan;
				if ( ! ( scan != last_ ) ) {
					break;
				}
			}
			if ( ! is_known_character( *scan ) ) {
				break;
			}
			++ scan;
		}
		if ( scan != last_ ) {
			if ( *scan != '\'' ) {
				break;
			}
			valid = true;
		}
	} while ( false );
	if ( valid ) {
		_cache.assign( start, scan );
		++ scan;
		unescape( _cache, _escapes_ );
		if ( !! _actionCharacter )
			add_execution_step( executingParser_, first_, call( _actionCharacter, _cache[0] ) );
		else if ( !! _action )
			add_execution_step( executingParser_, first_, _action );
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected literal string" );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HCharacterLiteral::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HCharacterLiteral>( *this ) );
	M_EPILOG
}

void HCharacterLiteral::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "character_literal" );
	return;
	M_EPILOG
}

void HCharacterLiteral::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HCharacterLiteral::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HCharacterLiteral::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HCharacterLiteral const& get_character_literal_instance( void ) {
	M_PROLOG
	static HCharacterLiteral characterLiteralInstance;
	return ( characterLiteralInstance );
	M_EPILOG
}

HCharacterLiteral const& character_literal( get_character_literal_instance() );

HCharacter::HCharacter( void )
	: HRuleBase(), _characters(), _actionChar()
	{}

HCharacter::HCharacter( hcore::HString const& characters_, action_t const& action_ )
	: HRuleBase( action_ ), _characters( characters_ ), _actionChar()
	{}

HCharacter::HCharacter( hcore::HString const& characters_, action_char_t const& action_ )
	: HRuleBase(), _characters( characters_ ), _actionChar( action_ )
	{}

HCharacter::HCharacter( HCharacter const& character_ )
	: HRuleBase( character_._action ), _characters( character_._characters ), _actionChar( character_._actionChar )
	{}

HCharacter HCharacter::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionChar ) );
	return ( HCharacter( _characters, action_ ) );
	M_EPILOG
}

HCharacter HCharacter::operator[]( action_char_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionChar ) );
	return ( HCharacter( _characters, action_ ) );
	M_EPILOG
}

HCharacter HCharacter::operator() ( hcore::HString const& characters_ ) const {
	M_PROLOG
	return ( HCharacter( characters_, _action ) );
	M_EPILOG
}

yaal::hcore::HString::const_iterator HCharacter::do_parse( HExecutingParser* executingParser_, yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ENSURE( first_ != last_ );
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	char c( *scan );
	if ( _characters.is_empty() || ( _characters.find( *scan ) != hcore::HString::npos ) ) {
		if ( !! _actionChar )
			add_execution_step( executingParser_, first_, call( _actionChar, c ) );
		else if ( !! _action )
			add_execution_step( executingParser_, first_, call( _action ) );
		++ scan;
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected one of characters: " + _characters );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HCharacter::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HCharacter>( *this ) );
	M_EPILOG
}

void HCharacter::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	if ( ! _characters.is_empty() ) {
		rd_.desc( "'" );
		rd_.desc( _characters );
		rd_.desc( "'" );
	} else
		rd_.desc( "character" );
	return;
	M_EPILOG
}

void HCharacter::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HCharacter::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HCharacter::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HCharacter const& get_character_instance( void ) {
	M_PROLOG
	static HCharacter characterInstance;
	return ( characterInstance );
	M_EPILOG
}

HCharacter const& character( get_character_instance() );

HFollows operator >> ( char character_, HRuleBase const& successor_ ) {
	M_PROLOG
	return ( HFollows( character( character_ ), successor_  ) );
	M_EPILOG
}

HFollows operator >> ( HFollows const& predecessor_, char character_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, character( character_ ) ) );
	M_EPILOG
}

HFollows operator >> ( HRuleBase const& predecessor_, char character_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, character( character_ ) ) );
	M_EPILOG
}

HString::HString( hcore::HString const& string_ )
	: HRuleBase(), _string( string_ ), _actionString()
	{}

HString::HString( hcore::HString const& string_, action_t const& action_ )
	: HRuleBase( action_ ), _string( string_ ), _actionString()
	{}

HString::HString( hcore::HString const& string_, action_string_t const& action_ )
	: HRuleBase(), _string( string_ ), _actionString( action_ )
	{}

HString::HString( HString const& string_ )
	: HRuleBase( string_._action ), _string( string_._string ), _actionString( string_._actionString )
	{}

HString HString::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HString( _string, action_ ) );
	M_EPILOG
}

HString HString::operator[]( action_string_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HString( _string, action_ ) );
	M_EPILOG
}

HString HString::operator() ( hcore::HString const& string_ ) const {
	M_PROLOG
	return ( HString( string_, _action ) );
	M_EPILOG
}

hcore::HString::const_iterator HString::do_parse( HExecutingParser* executingParser_, hcore::HString::const_iterator first_, hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ENSURE( first_ != last_ );
	M_ASSERT( ! _string.is_empty() );
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	bool ok( true );
	for ( hcore::HString::const_iterator it( _string.begin() ), end( _string.end() ); it != end; ++ it, ++ scan ) {
		if ( ( scan == last_ ) || ( *scan != *it ) ) {
			ok = false;
			break;
		}
	}
	if ( ok ) {
		if ( !! _actionString )
			add_execution_step( executingParser_, first_, call( _actionString, _string ) );
		else if ( !! _action )
			add_execution_step( executingParser_, first_, call( _action ) );
		first_ = scan;
	} else {
		report_error( executingParser_, scan, "expected string: " + _string );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HString::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HString>( *this ) );
	M_EPILOG
}

void HString::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "\"" );
	rd_.desc( _string );
	rd_.desc( "\"" );
	return;
	M_EPILOG
}

void HString::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HString::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HString::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HString string( yaal::hcore::HString const& string_ ) {
	M_PROLOG
	return ( HString( string_ ) );
	M_EPILOG
}

HRegex::HRegex( hcore::HString const& string_ )
	: HRuleBase(), _regex( make_pointer<hcore::HRegex>( string_, hcore::HRegex::COMPILE::EXTENDED ) ), _actionString()
	{}

HRegex::HRegex( regex_t const& regex_, action_t const& action_ )
	: HRuleBase( action_ ), _regex( regex_ ), _actionString()
	{}

HRegex::HRegex( regex_t const& regex_, action_string_t const& action_ )
	: HRuleBase(), _regex( regex_ ), _actionString( action_ )
	{}

HRegex::HRegex( HRegex const& regex_ )
	: HRuleBase( regex_._action ), _regex( regex_._regex ), _actionString( regex_._actionString )
	{}

HRegex HRegex::operator[]( action_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HRegex( _regex, action_ ) );
	M_EPILOG
}

HRegex HRegex::operator[]( action_string_t const& action_ ) const {
	M_PROLOG
	M_ENSURE( ( ! _action ) && ( ! _actionString ) );
	return ( HRegex( _regex, action_ ) );
	M_EPILOG
}

hcore::HString::const_iterator HRegex::do_parse( HExecutingParser* executingParser_, hcore::HString::const_iterator first_, hcore::HString::const_iterator last_ ) {
	M_PROLOG
	M_ENSURE( first_ != last_ );
	yaal::hcore::HString::const_iterator scan( skip_space( first_, last_ ) );
	hcore::HRegex::HMatchIterator it( _regex->find( scan ) );
	if ( ( it != _regex->end() ) && ( it->size() <= ( last_ - scan ) ) ) {
		if ( !! _actionString )
			add_execution_step( executingParser_, first_, call( _actionString, hcore::HString( it->raw(), it->size() ) ) );
		else if ( !! _action )
			add_execution_step( executingParser_, first_, call( _action ) );
		first_ = it->raw() + it->size();
	} else {
		report_error( executingParser_, scan, "expected character sequence matching regular expression: " + _regex->pattern() );
	}
	return ( first_ );
	M_EPILOG
}

HRuleBase::ptr_t HRegex::do_clone( void ) const {
	M_PROLOG
	return ( make_pointer<HRegex>( *this ) );
	M_EPILOG
}

void HRegex::do_describe( HRuleDescription& rd_, rule_use_t const& ) const {
	M_PROLOG
	rd_.desc( "regex( \"" );
	rd_.desc( _regex->pattern().substr( 1 ) );
	rd_.desc( "\" )" );
	return;
	M_EPILOG
}

void HRegex::do_detach( HRuleBase const*, visited_t&, bool& ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HRegex::do_detect_recursion( HRecursionDetector& recursionDetector_ ) const {
	M_PROLOG
	recursionDetector_.reset_visits();
	return;
	M_EPILOG
}

void HRegex::do_find_recursions( HRuleAggregator& ) {
	M_PROLOG
	return;
	M_EPILOG
}

HFollows operator >> ( char const* string_, HRuleBase const& successor_ ) {
	M_PROLOG
	return ( HFollows( string( string_ ), successor_  ) );
	M_EPILOG
}

HFollows operator >> ( hcore::HString const& string_, HRuleBase const& successor_ ) {
	M_PROLOG
	return ( HFollows( string( string_ ), successor_  ) );
	M_EPILOG
}

HFollows operator >> ( HFollows const& predecessor_, char const* string_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, string( string_ ) ) );
	M_EPILOG
}

HFollows operator >> ( HFollows const& predecessor_, hcore::HString const& string_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, string( string_ ) ) );
	M_EPILOG
}

HFollows operator >> ( HRuleBase const& predecessor_, char const* string_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, string( string_ ) ) );
	M_EPILOG
}

HFollows operator >> ( HRuleBase const& predecessor_, hcore::HString const& string_ ) {
	M_PROLOG
	return ( HFollows( predecessor_, string( string_ ) ) );
	M_EPILOG
}

HCharacter constant( char character_ ) {
	M_PROLOG
	return ( character( character_ ) );
	M_EPILOG
}

HString constant( yaal::hcore::HString const& string_ ) {
	M_PROLOG
	M_ENSURE( ! string_.is_empty() );
	return ( string( string_ ) );
	M_EPILOG
}

HRegex regex( yaal::hcore::HString const& pattern_ ) {
	M_PROLOG
	M_ENSURE( ! pattern_.is_empty() );
	return ( HRegex( pattern_[0] == '^' ? pattern_  : "^" + pattern_ ) );
	M_EPILOG
}

HRuleDescription::HRuleDescription( void )
	: _children(), _description(), _automaticNames()
	{}

void HRuleDescription::desc( yaal::hcore::HString const& desc_ ) {
	M_PROLOG
	_description += desc_;
	return;
	M_EPILOG
}

yaal::hcore::HString const& HRuleDescription::description( void ) const {
	M_PROLOG
	return ( _description );
	M_EPILOG
}

HRuleDescription::named_children_t const& HRuleDescription::children( void ) const {
	M_PROLOG
	return ( _children );
	M_EPILOG
}

void HRuleDescription::clear( void ) {
	M_PROLOG
	_children.clear();
	_description.clear();
	return;
	M_EPILOG
}

void HRuleDescription::add( HNamedRule const* nr_ ) {
	M_PROLOG
	_children.push_back( nr_ );
	return;
	M_EPILOG
}

yaal::hcore::HString const& HRuleDescription::make_name( HNamedRule const& nr_ ) {
	M_PROLOG
	hcore::HString const* name( NULL );

	if ( ! nr_.name().is_empty() )
		name = &nr_.name();
	else {
		name = &make_name_auto( nr_.id() );
	}
	return ( *name );
	M_EPILOG
}

yaal::hcore::HString const& HRuleDescription::make_name_auto( HRuleBase const* rule_ ) {
	M_PROLOG
	hcore::HString const* name( NULL );
	automatic_names_t::const_iterator a( _automaticNames.find( rule_ ) );
	if ( a != _automaticNames.end() )
		name = &a->second;
	else {
		static int const MAX_AUTO_NAMES_COUNT = 26;
		int autoNamesCount( static_cast<int>( _automaticNames.get_size() ) );
		name = &( _automaticNames[rule_] = ( _automaticNames.get_size() < MAX_AUTO_NAMES_COUNT ) ?
				hcore::HString( static_cast<char>( 'A' + autoNamesCount ) ) + '_'
				: hcore::HString( "rule" ) + rule_ );
	}
	return ( *name );
	M_EPILOG
}

HGrammarDescription::HGrammarDescription( HRuleBase const& rule_ )
	: _rules(), _visited(), _ruleOrder() {
	M_PROLOG
	HRuleDescription rd;
	rule_use_t ru;
	rule_.rule_use( ru );
	HRule const* rule( dynamic_cast<HRule const*>( &rule_ ) );
	hcore::HString rootName( ! rule ? rd.make_name_auto( &rule_ ) + " = " : "" );
	rule_.describe( rd, ru );
	if ( ( ! rule || rule->get_name().is_empty() ) && ! ( rule && ( rd.children().size() == 1 ) && ( rd.children()[0] == rule->get_named_rule() ) ) )
		_rules.push_back( rootName + rd.description() );
	copy( rd.children().begin(), rd.children().end(), push_insert_iterator( _ruleOrder ) );
	_visited.insert( &rule_ );
	while ( ! _ruleOrder.is_empty() ) {
		HNamedRule const* r( _ruleOrder.front() );
		_ruleOrder.pop();
		if ( _visited.insert( r->id() ).second ) {
			rd.clear();
			rd.desc( rd.make_name( *r ) );
			rd.desc( " = " );
			r->id()->describe( rd, ru );
			_rules.push_back( rd.description() );
			copy( rd.children().begin(), rd.children().end(), push_insert_iterator( _ruleOrder ) );
		}
	}
	return;
	M_EPILOG
}

bool HGrammarDescription::is_empty( void ) const {
	M_PROLOG
	return ( _rules.is_empty() );
	M_EPILOG
}

HGrammarDescription::const_iterator HGrammarDescription::begin( void ) const {
	M_PROLOG
	return ( _rules.begin() );
	M_EPILOG
}

HGrammarDescription::const_iterator HGrammarDescription::end( void ) const {
	M_PROLOG
	return ( _rules.end() );
	M_EPILOG
}

}

}

}

