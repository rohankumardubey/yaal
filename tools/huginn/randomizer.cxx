/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "randomizer.hxx"
#include "runtime.hxx"
#include "helper.hxx"
#include "thread.hxx"
#include "objectfactory.hxx"
#include "instruction.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

class HRandomizerClass : public HClass {
public:
	typedef HRandomizerClass this_type;
	typedef HClass base_type;
private:
	enumeration::HEnumerationClass::ptr_t _distributionEnumerationClass;
	HHuginn::class_t _exceptionClass;
public:
	HRandomizerClass( HRuntime* runtime_, HHuginn::type_id_t typeId_, HHuginn::identifier_id_t identifierId_, HClass const* origin_ )
		: HClass(
			runtime_,
			typeId_,
			identifierId_,
			"The `Randomizer` class represents a random number generator concept. `Randomizer` can generate uniform distribution of either `integer` or `real` values from given range.",
			HHuginn::ACCESS::PUBLIC,
			HClass::TYPE::BUILTIN,
			origin_,
			&HRandomizer::create_instance
		)
		, _distributionEnumerationClass()
		, _exceptionClass() {
		HHuginn::field_definitions_t fd{
			{ "next",      runtime_->create_method( &HRandomizer::next ),      "get next random number from this distribution" },
			{ "seed",      runtime_->create_method( &HRandomizer::seed ),      "([ *data* ]) - initialize random generator internal state based on supplied *data*, or using system entropy if no data is given" }
		};
		redefine( nullptr, fd );
		_distributionEnumerationClass = add_enumeration_as_member(
			this,
			enumeration::create_class(
				runtime_,
				"DISTRIBUTION",
				enumeration::descriptions_t{
					{ "DISCRETE", "an uniform discrete distribution of `integer` values, parametrized by [a, b]",     static_cast<int>( HRandomizer::DISTRIBUTION::DISCRETE ) },
					{ "UNIFORM",  "an uniform continuous distribution of `real` values, parametrized by [a, b]",      static_cast<int>( HRandomizer::DISTRIBUTION::UNIFORM ) },
					{ "TRIANGLE", "a triangle continuous distribution of `real` values, parametrized by [a, b, mod]", static_cast<int>( HRandomizer::DISTRIBUTION::TRIANGLE ) },
					{ "NORMAL",   "a normal (Gaussian) distribution of `real` values, parametrized by [mu, sigma]",   static_cast<int>( HRandomizer::DISTRIBUTION::NORMAL ) },
					{ "POISSON",  "a Poisson distribution of `real` values, parametrized by [lambda]",                static_cast<int>( HRandomizer::DISTRIBUTION::POISSON ) }
				},
				"The `DISTRIBUTION` is set of possible random variable distributions generated by a `Randomizer`.",
				HHuginn::VISIBILITY::PACKAGE,
				this
			),
			"set of available distributions used for Randomizer.",
			HClass::MEMBER_TYPE::STATIC
		);
	}
	HClass const* distribution_class( void ) const {
		return ( _distributionEnumerationClass->enumeral_class() );
	}
};

HRandomizer::HRandomizer( HClass const* class_, DISTRIBUTION distribution_, distribution_t&& generator_ )
	: HValue( class_ )
	, _distribution( distribution_ )
	, _generator( yaal::move( generator_ ) ) {
	return;
}

HHuginn::value_t HRandomizer::create_instance( HClass const* class_, huginn::HThread* thread_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Randomizer.constructor";
	verify_arg_count( name, values_, 2, 4, thread_, position_ );
	HObjectFactory& of( thread_->object_factory() );
	HClass const* dc( static_cast<HRandomizerClass const*>( class_ )->distribution_class() );
	HClass const* ic( of.integer_class() );
	HClass const* rc( of.real_class() );
	verify_arg_type( name, values_, 0, dc, ARITY::MULTIPLE, thread_, position_ );
	DISTRIBUTION distribution( static_cast<DISTRIBUTION>( get_enumeral( values_[0] ) ) );
	distribution_t generator;
	switch ( distribution ) {
		case ( DISTRIBUTION::DISCRETE ): {
			verify_signature_by_class( name, values_, { dc, ic, ic }, thread_, position_ );
			yaal::i64_t from( get_integer( values_[1] ) );
			yaal::i64_t to( get_integer( values_[2] ) );
			if ( to < from ) {
				throw HHuginn::HHuginnRuntimeException(
					"Invalid DISCRETE parametrization.",
					thread_->file_id(),
					position_
				);
			}
			generator = make_resource<random::distribution::HDiscrete>( from, to );
		} break;
		case ( DISTRIBUTION::UNIFORM ): {
			verify_signature_by_class( name, values_, { dc, rc, rc }, thread_, position_ );
			double long infimum( get_real( values_[1] ) );
			double long supremum( get_real( values_[2] ) );
			if ( infimum >= supremum ) {
				throw HHuginn::HHuginnRuntimeException(
					"Invalid UNIFORM parametrization.",
					thread_->file_id(),
					position_
				);
			}
			generator = make_resource<random::distribution::HUniform>( infimum, supremum );
		} break;
		case ( DISTRIBUTION::TRIANGLE ): {
			verify_signature_by_class( name, values_, { dc, rc, rc, rc }, thread_, position_ );
			double long infimum( get_real( values_[1] ) );
			double long supremum( get_real( values_[2] ) );
			double long mode( get_real( values_[3] ) );
			if ( ( infimum >= supremum ) || ( mode < infimum ) || ( mode > supremum ) ) {
				throw HHuginn::HHuginnRuntimeException(
					"Invalid TRIANGLE parametrization.",
					thread_->file_id(),
					position_
				);
			}
			generator = make_resource<random::distribution::HTriangle>( infimum, supremum, mode );
		} break;
		case ( DISTRIBUTION::NORMAL ): {
			verify_signature_by_class( name, values_, { dc, rc, rc }, thread_, position_ );
			double long mu( get_real( values_[1] ) );
			double long sigma( get_real( values_[2] ) );
			if ( sigma <= 0 ) {
				throw HHuginn::HHuginnRuntimeException(
					"Invalid NORMAL parametrization.",
					thread_->file_id(),
					position_
				);
			}
			generator = make_resource<random::distribution::HNormal>( mu, sigma );
		} break;
		case ( DISTRIBUTION::POISSON ): {
			verify_signature_by_class( name, values_, { dc, rc }, thread_, position_ );
			double long lambda( get_real( values_[1] ) );
			if ( lambda <= 0 ) {
				throw HHuginn::HHuginnRuntimeException(
					"Invalid POISSON parametrization.",
					thread_->file_id(),
					position_
				);
			}
			generator = make_resource<random::distribution::HPoisson>( lambda );
		} break;
	}
	return ( of.create<HRandomizer>( class_, distribution, yaal::move( generator ) ) );
	M_EPILOG
}

HHuginn::value_t HRandomizer::seed( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Randomizer.seed";
	verify_arg_count( name, values_, 0, 1, thread_, position_ );
	HRandomizer* o( static_cast<HRandomizer*>( object_->raw() ) );
	if ( ! values_.is_empty() ) {
		verify_arg_type( name, values_, 0, HHuginn::TYPE::INTEGER, ARITY::UNARY, thread_, position_ );
		yaal::u64_t data( static_cast<yaal::u64_t>( get_integer( values_[0] ) ) );
		o->_generator->generator()->set_seed( data );
	} else {
		o->_generator->set_generator( make_pointer<random::HRandomNumberGenerator>() );
	}
	return ( *object_ );
	M_EPILOG
}

HHuginn::value_t HRandomizer::next( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Randomizer.next";
	verify_arg_count( name, values_, 0, 0, thread_, position_ );
	HRandomizer* o( static_cast<HRandomizer*>( object_->raw() ) );
	return (
		( ( o->_distribution == DISTRIBUTION::DISCRETE ) || ( o->_distribution == DISTRIBUTION::POISSON ) )
			? thread_->object_factory().create_integer( o->_generator->next_discrete() )
			: thread_->object_factory().create_real( o->_generator->next_continuous() )
	);
	M_EPILOG
}


yaal::hcore::HString HRandomizer::do_code( huginn::HThread* thread_, HHuginn::value_t const&, HCycleTracker&, int ) const {
	M_PROLOG
	HRuntime& rt( thread_->runtime() );
	hcore::HString s( full_class_name( rt, HValue::get_class() ) );
	s.append( "(" ).append( full_class_name( rt, HValue::get_class() ) ).append( ".DISTRIBUTION." );
	random::distribution::HDistribution const* distribution( _generator.raw() );
	switch ( _distribution ) {
		case ( DISTRIBUTION::DISCRETE ): {
			random::distribution::HDiscrete const& d( *static_cast<random::distribution::HDiscrete const*>( distribution ) );
			s.append( "DISCRETE, " ).append( d.from() ).append( ", " ).append( d.to() );
		} break;
		case ( DISTRIBUTION::UNIFORM ): {
			random::distribution::HUniform const& u( *static_cast<random::distribution::HUniform const*>( distribution ) );
			s.append( "UNIFORM, " ).append( u.infimum() ).append( ", " ).append( u.supremum() );
		} break;
		case ( DISTRIBUTION::TRIANGLE ): {
			random::distribution::HTriangle const& t( *static_cast<random::distribution::HTriangle const*>( distribution ) );
			s.append( "TRIANGLE, " ).append( t.infimum() ).append( ", " ).append( t.supremum() ).append( ", " ).append( t.mode() );
		} break;
		case ( DISTRIBUTION::NORMAL ): {
			random::distribution::HNormal const& n( *static_cast<random::distribution::HNormal const*>( distribution ) );
			s.append( "NORMAL, " ).append( n.mu() ).append( ", " ).append( n.sigma() );
		} break;
		case ( DISTRIBUTION::POISSON ): {
			random::distribution::HPoisson const& p( *static_cast<random::distribution::HPoisson const*>( distribution ) );
			s.append( "POISSON, " ).append( p.lambda() );
		} break;
	}
	s.append( ")" );
	return s;
	M_EPILOG
}

HHuginn::class_t HRandomizer::get_class( HRuntime* runtime_, HClass const* origin_ ) {
	M_PROLOG
	HHuginn::identifier_id_t classIdentifier( runtime_->identifier_id( "Randomizer" ) );
	HHuginn::class_t c( runtime_->get_class( classIdentifier ) );
	if ( ! c ) {
		c = runtime_->create_class(
			HRuntime::class_constructor_t(
				[&runtime_, &classIdentifier, &origin_] ( HHuginn::type_id_t typeId_ ) -> HHuginn::class_t {
					return (
						make_pointer<HRandomizerClass>(
							runtime_,
							typeId_,
							classIdentifier,
							origin_
						)
					);
				}
			)
		);
		runtime_->huginn()->register_class( c );
	}
	return c;
	M_EPILOG
}

HHuginn::value_t HRandomizer::do_clone( huginn::HThread* thread_, HHuginn::value_t* object_, int ) const {
	HRandomizer* o( static_cast<HRandomizer*>( object_->raw() ) );
	distribution_t distribution;
	random::distribution::HDistribution const* source( o->_generator.raw() );
	switch ( o->_distribution ) {
		case ( DISTRIBUTION::DISCRETE ): {
			random::distribution::HDiscrete const& d( *static_cast<random::distribution::HDiscrete const*>( source ) );
			distribution = make_resource<random::distribution::HDiscrete>( d.from(), d.to() );
		} break;
		case ( DISTRIBUTION::UNIFORM ): {
			random::distribution::HUniform const& u( *static_cast<random::distribution::HUniform const*>( source ) );
			distribution = make_resource<random::distribution::HUniform>( u.infimum(), u.supremum() );
		} break;
		case ( DISTRIBUTION::TRIANGLE ): {
			random::distribution::HTriangle const& t( *static_cast<random::distribution::HTriangle const*>( source ) );
			distribution = make_resource<random::distribution::HTriangle>( t.infimum(), t.supremum(), t.mode() );
		} break;
		case ( DISTRIBUTION::NORMAL ): {
			random::distribution::HNormal const& n( *static_cast<random::distribution::HNormal const*>( source ) );
			distribution = make_resource<random::distribution::HNormal>( n.mu(), n.sigma() );
		} break;
		case ( DISTRIBUTION::POISSON ): {
			random::distribution::HPoisson const& p( *static_cast<random::distribution::HPoisson const*>( source ) );
			distribution = make_resource<random::distribution::HPoisson>( p.lambda() );
		} break;
	}
	distribution->set_generator( make_pointer<random::HRandomNumberGenerator>( *source->generator() ) );
	return ( thread_->object_factory().create<HRandomizer>( HValue::get_class(), o->_distribution, yaal::move( distribution ) ) );
}

}

}

}
