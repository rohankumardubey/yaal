/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "tools/hhuginn.hxx"
#include "runtime.hxx"
#include "tools/stringalgo.hxx"
#include "iterator.hxx"
#include "helper.hxx"
#include "keyword.hxx"
#include "thread.hxx"
#include "exception.hxx"
#include "packagefactory.hxx"
#include "objectfactory.hxx"
#include "commit_id.hxx"

#include "hcore/hfile.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

class HIntrospection : public HHuginn::HValue {
private:
	enumeration::HEnumerationClass::ptr_t _kindClass;
	HHuginn::class_t _exceptionClass;
public:
	enum class KIND {
		CLASS,
		ENUM,
		FUNCTION,
		PACKAGE,
		LOCAL
	};
	HIntrospection( HHuginn::HClass* class_ )
		: HValue( class_ )
		, _kindClass(
			add_enumeration_as_member(
				class_,
				enumeration::create_class(
					class_->runtime(),
					"KIND",
					enumeration::descriptions_t{
						{ "CLASS",    "A class.",        static_cast<int>( KIND::CLASS ) },
						{ "ENUM",     "An enumeration.", static_cast<int>( KIND::ENUM ) },
						{ "FUNCTION", "A function.",     static_cast<int>( KIND::FUNCTION ) },
						{ "PACKAGE",  "A package",       static_cast<int>( KIND::PACKAGE ) },
						{ "LOCAL",    "A local.",        static_cast<int>( KIND::LOCAL ) }
					},
					"The `KIND` is set of possible value conext kinds.",
					HHuginn::VISIBILITY::PACKAGE,
					class_
				),
				"set of all possible context kinds."
			)
		)
		, _exceptionClass( class_exception( class_ ) ) {
		return;
	}
	static HHuginn::value_t version( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Introspection.version", values_, 0, 0, thread_, position_ );
		HObjectFactory& of( *thread_->runtime().object_factory() );
		HHuginn::value_t ver( of.create_string( PACKAGE_VERSION ) );
		HHuginn::value_t ci( of.create_string( COMMIT_ID ) );
		return ( of.create_tuple( { ver, ci } ) );
		M_EPILOG
	}
	static HHuginn::value_t symbol( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Introspection.symbol", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
		HRuntime& r( thread_->runtime() );
		HHuginn::value_t v( r.none_value() );
		HString const& name( get_string( values_[0] ) );
		HHuginn::identifier_id_t id( r.try_identifier_id( name ) );
		HHuginn::value_t const* pv( nullptr );
		if ( id != INVALID_IDENTIFIER ) {
			pv = r.get_global( id );
		}
		if ( pv ) {
			v = *pv;
		}
		return ( v );
		M_EPILOG
	}
	static HHuginn::value_t attribute( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Introspection.attribute", values_, { HHuginn::TYPE::UNKNOWN, HHuginn::TYPE::STRING }, thread_, position_ );
		HRuntime& r( thread_->runtime() );
		HHuginn::value_t v( r.none_value() );
		HString const& name( get_string( values_[1] ) );
		HHuginn::identifier_id_t id( r.try_identifier_id( name ) );
		if ( id != INVALID_IDENTIFIER ) {
			HHuginn::value_t& s( values_[0] );
			HHuginn::type_id_t t( s->type_id() );
			if ( t == HHuginn::TYPE::FUNCTION_REFERENCE ) {
				HHuginn::class_t c( r.get_class( static_cast<HHuginn::HFunctionReference*>( s.raw() )->identifier_id() ) );
				if ( !! c ) {
					int fi( c->field_index( id, HHuginn::HClass::MEMBER_TYPE::STATIC ) );
					if ( fi >= 0 ) {
						HHuginn::value_t const& f( c->field( fi ) );
						v = f->type_id() == HHuginn::TYPE::METHOD
							? r.object_factory()->create_unbound_method( c.raw(), static_cast<HHuginn::HClass::HMethod const*>( f.raw() )->function() )
							: f;
					}
				}
			} else {
				int idx( s->field_index( id ) );
				if ( idx >= 0 ) {
					v = s->field( s, idx );
				}
			}
		}
		return ( v );
		M_EPILOG
	}
	static HHuginn::value_t list_attributes( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Introspection.list_attributes", values_, 1, 1, thread_, position_ );
		HRuntime& r( thread_->runtime() );
		HObjectFactory& of( *r.object_factory() );
		HHuginn::values_t attrs;
		HHuginn::value_t& v( values_[0] );
		HHuginn::HClass const* c( nullptr );
		if ( v->type_id() == HHuginn::TYPE::FUNCTION_REFERENCE ) {
			c = r.get_class( static_cast<HHuginn::HFunctionReference*>( v.raw() )->identifier_id() ).raw();
		}
		if ( ! c ) {
			c = v->get_class();
		}
		for ( HHuginn::identifier_id_t id : c->field_identifiers() ) {
			attrs.emplace_back( of.create_string( r.identifier_name( id ) ) );
		}
		return ( of.create_tuple( yaal::move( attrs ) ) );
		M_EPILOG
	}
	static HHuginn::value_t subject( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Introspection.subject", values_, { HHuginn::TYPE::BOUND_METHOD }, thread_, position_ );
		HHuginn::HClass::HBoundMethod const& bm( *static_cast<HHuginn::HClass::HBoundMethod const*>( values_[0].raw() ) );
		return ( bm.subject() );
		M_EPILOG
	}
	static HHuginn::value_t kind( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Introspection.kind", values_, 1, 1, thread_, position_ );
		HRuntime& r( thread_->runtime() );
		enumeration::HEnumerationClass const& ec( *static_cast<enumeration::HEnumerationClass const*>( static_cast<HIntrospection const*>( object_->raw() )->_kindClass.raw() ) );
		HHuginn::type_id_t t( values_[0]->type_id() );
		HHuginn::HClass const* c( values_[0]->get_class() );
		HHuginn::identifier_id_t typeNameId( c->identifier_id() );
		KIND k( KIND::LOCAL );
		if ( t == HHuginn::TYPE::BOUND_METHOD ) {
			HHuginn::HClass::HBoundMethod const& bm( *static_cast<HHuginn::HClass::HBoundMethod const*>( values_[0].raw() ) );
			if ( bm.function().id() == bit_cast<void const*>( &package::value ) ) {
				k = KIND::PACKAGE;
			} else if ( bm.function().id() == bit_cast<void const*>( &package::instance ) ) {
				k = KIND::CLASS;
			} else {
				k = KIND::FUNCTION;
				c = bm.subject()->get_class();
				for ( HHuginn::identifier_id_t fi : c->field_identifiers() ) {
					int idx( c->field_index( fi ) );
					HHuginn::value_t const& f( c->field_definitions()[idx] );
					if ( ( f->type_id() == HHuginn::TYPE::METHOD ) && ( static_cast<HHuginn::HClass::HMethod const*>( f.raw() )->function() == bm.function() ) ) {
						HHuginn::class_t cls( r.get_class( fi ) );
						if ( !! cls && ( static_cast<HHuginn::HFunctionReference const*>( cls->constructor().raw() )->function() == bm.function() ) ) {
							k = KIND::CLASS;
						}
						break;
					}
				}
			}
		} else if ( is_enum_class( values_[0].raw() ) ) {
			k = KIND::ENUM;
		} else if ( t == HHuginn::TYPE::FUNCTION_REFERENCE ) {
			typeNameId = static_cast<HHuginn::HFunctionReference const*>( values_[0].raw() )->identifier_id();
			HHuginn::class_t cls( r.get_class( typeNameId ) );
			k = !! cls ? KIND::CLASS : KIND::FUNCTION;
		} else if ( !! r.find_package( typeNameId ) ) {
			k = KIND::PACKAGE;
		}
		return ( ec.enumeral( static_cast<int>( k ) ) );
		M_EPILOG
	}
	static HHuginn::value_t import( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Introspection.import", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
		HRuntime& r( thread_->runtime() );
		HString const& package( get_string( values_[0] ) );
		HHuginn::identifier_id_t id( r.try_identifier_id( package ) );
		bool classExists( !! r.get_class( id ) );
		HHuginn::value_t const* g( r.get_global( id ) );
		bool enumerationExists( g && is_enum_class( g ) );
		bool functionExists( g && ( (*g)->type_id() == HHuginn::TYPE::FUNCTION_REFERENCE ) );
		if ( is_restricted( package ) ) {
			throw HHuginn::HHuginnRuntimeException(
				"`"_ys.append( package ).append( "' is restricted keyword." ),
				thread_->current_frame()->file_id(),
				position_
			);
		}
		if ( classExists || functionExists || g ) {
			throw HHuginn::HHuginnRuntimeException(
				hcore::to_string(
					enumerationExists ? "Enumeration" : ( classExists ? "Class" : ( functionExists ? "Function" : "Package alias" ) )
				).append( " of the same name already exists." ),
				thread_->current_frame()->file_id(),
				position_
			);
		}
		HHuginn::value_t v( r.none_value() );
		try {
			v = HPackageFactory::get_instance().create_package( &r, package, position_ );
		} catch ( HHuginn::HHuginnRuntimeException const& e ) {
			thread_->raise( static_cast<HIntrospection*>( object_->raw() )->_exceptionClass.raw(), e.message(), position_ );
		} catch ( HException const& e ) {
			thread_->raise( static_cast<HIntrospection*>( object_->raw() )->_exceptionClass.raw(), e.what(), position_ );
		}
		return ( v );
		M_EPILOG
	}
	static HHuginn::value_t call_stack( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Introspection.call_stack", values_, 0, 0, thread_, position_ );
		HIntrospecteeInterface::call_stack_t callStack( thread_->runtime().get_call_stack( thread_ ) );
		HObjectFactory& of( thread_->object_factory() );
		HHuginn::HList::values_t data;
		for ( HIntrospecteeInterface::HCallSite const& cs : callStack ) {
			data.push_back( of.create<exception::HStackFrameInfo>( of.stack_frame_info_class(), cs ) );
		}
		return ( of.create_list( yaal::move( data ) ) );
		M_EPILOG
	}
};

namespace package_factory {

class HTntrospectionCreator : public HPackageCreatorInterface {
protected:
	virtual HHuginn::value_t do_new_instance( HRuntime* );
} introspectionCreator;

HHuginn::value_t HTntrospectionCreator::do_new_instance( HRuntime* runtime_ ) {
	M_PROLOG
	HHuginn::class_t c(
		runtime_->create_class(
			"Introspection",
			"The `Introspection` package provides run time introspection facility.",
			HHuginn::ACCESS::PRIVATE
		)
	);
	HHuginn::field_definitions_t fd{
		{ "version",         runtime_->create_method( &HIntrospection::version ),         "return runtime version information." },
		{ "symbol",          runtime_->create_method( &HIntrospection::symbol ),          "( *name* ) - get global symbol by *name*." },
		{ "attribute",       runtime_->create_method( &HIntrospection::attribute ),       "( *object*, *name* ) - get *object*'s attribute (a field or method) by *name*." },
		{ "list_attributes", runtime_->create_method( &HIntrospection::list_attributes ), "( *object* ) - list attributes of given *object*." },
		{ "subject",         runtime_->create_method( &HIntrospection::subject ),         "( *bound_method* ) - get subject of *bound_method*." },
		{ "kind",            runtime_->create_method( &HIntrospection::kind ),            "( *object* ) - tell what *kind* of *object* it is." },
		{ "import",          runtime_->create_method( &HIntrospection::import ),          "( *package* ) - import given *package*." },
		{ "call_stack",      runtime_->create_method( &HIntrospection::call_stack ),      "get current call stack." }
	};
	c->redefine( nullptr, fd );
	runtime_->huginn()->register_class( c );
	return ( runtime_->object_factory()->create<HIntrospection>( c.raw() ) );
	M_EPILOG
}

namespace {

bool registrar( void ) {
	M_PROLOG
	bool volatile failed = false;
	HPackageFactory& factory = HPackageFactory::get_instance();
	factory.register_package_creator( "Introspection", &introspectionCreator );
	return ( failed );
	M_EPILOG
}

bool volatile registered = registrar();

}

}

}

}

}
