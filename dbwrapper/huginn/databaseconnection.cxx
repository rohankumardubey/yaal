/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "databaseconnection.hxx"
#include "query.hxx"
#include "dbwrapper/dbwrapper.hxx"
#include "tools/huginn/runtime.hxx"
#include "tools/huginn/helper.hxx"
#include "tools/huginn/thread.hxx"
#include "tools/huginn/objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::huginn;

namespace yaal {

namespace dbwrapper {

namespace huginn {

HDatabaseConnection::HDatabaseConnection( HHuginn::HClass const* class_, dbwrapper::database_ptr_t database_ )
	: HHuginn::HValue( class_ )
	, _database( database_ ) {
	return;
}

HHuginn::value_t HDatabaseConnection::query(
	tools::huginn::HThread* thread_,
	HHuginn::value_t* object_,
	HHuginn::values_t& values_,
	int position_
) {
	HDatabaseConnection* dbc( static_cast<HDatabaseConnection*>( object_->raw() ) );
	return ( dbc->do_query( thread_, values_, position_ ) );
}

HHuginn::value_t HDatabaseConnection::table_names(
	tools::huginn::HThread* thread_,
	HHuginn::value_t* object_,
	HHuginn::values_t& values_,
	int position_
) {
	HDatabaseConnection* dbc( static_cast<HDatabaseConnection*>( object_->raw() ) );
	return ( dbc->do_table_names( thread_, values_, position_ ) );
}

HHuginn::value_t HDatabaseConnection::column_names(
	tools::huginn::HThread* thread_,
	HHuginn::value_t* object_,
	HHuginn::values_t& values_,
	int position_
) {
	HDatabaseConnection* dbc( static_cast<HDatabaseConnection*>( object_->raw() ) );
	return ( dbc->do_column_names( thread_, values_, position_ ) );
}

class HDatabaseConnectionClass : public HHuginn::HClass {
	HHuginn::class_t const& _exceptionClass;
	HHuginn::class_t _queryClass;
public:
	typedef HDatabaseConnectionClass this_type;
	typedef HHuginn::HClass base_type;
	HDatabaseConnectionClass(
		HRuntime* runtime_,
		HHuginn::type_id_t typeId_,
		HHuginn::class_t const& exceptionClass_
	) : HHuginn::HClass(
			runtime_,
			typeId_,
			runtime_->identifier_id( "DatabaseConnection" ),
			"The `DatabaseConnection` class allows performing various type of queries on connected database.",
			HHuginn::ACCESS::PRIVATE
		)
		, _exceptionClass( exceptionClass_ )
		, _queryClass( huginn::HQuery::get_class( runtime_, exceptionClass_ ) ) {
		HHuginn::field_definitions_t fd{
			{ "query",        runtime_->create_method( &HDatabaseConnection::query ),        "( *sql* ) - create query object for given *sql* `string`" },
			{ "table_names",  runtime_->create_method( &HDatabaseConnection::table_names ),  "get list of table names available in connected database" },
			{ "column_names", runtime_->create_method( &HDatabaseConnection::column_names ), "( *table* ) - get list of column names from given *table* in connected database" }
		};
		redefine( nullptr, fd );
		return;
	}
	HHuginn::HClass const* query_class( void ) const {
		return ( _queryClass.raw() );
	}
	HHuginn::HClass const* exception_class( void ) const {
		return ( _exceptionClass.raw() );
	}
private:
	virtual HHuginn::value_t do_create_instance( tools::huginn::HThread* thread_, HHuginn::values_t& values_, int position_ ) const {
		M_PROLOG
		verify_signature( "DatabaseConnection.constructor", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
		HHuginn::value_t v( thread_->runtime().none_value() );
		try {
			dbwrapper::database_ptr_t db( dbwrapper::util::connect( get_string( values_[0] ) ) );
			v = thread_->object_factory().create<HDatabaseConnection>( this, db );
		} catch ( HException const& e ) {
			thread_->raise( exception_class(), e.what(), position_ );
		}
		return ( v );
		M_EPILOG
	}
};

HHuginn::value_t HDatabaseConnection::do_query(
	tools::huginn::HThread* thread_,
	HHuginn::values_t& values_,
	int position_
) {
	verify_signature( "DatabaseConnection.query", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
	HDatabaseConnectionClass const* dbcClass( static_cast<HDatabaseConnectionClass const*>( HValue::get_class() ) );
	HHuginn::value_t v( thread_->runtime().none_value() );
	try {
		dbwrapper::HQuery::ptr_t q( _database->prepare_query( get_string( values_[0] ) ) );
		v = thread_->object_factory().create<huginn::HQuery>( dbcClass->query_class(), q );
	} catch ( HException const& e ) {
		thread_->raise( dbcClass->exception_class(), e.what(), position_ );
	}
	return ( v );
}

HHuginn::value_t HDatabaseConnection::do_table_names(
	tools::huginn::HThread* thread_,
	HHuginn::values_t& values_,
	int position_
) {
	char const name[] = "DatabaseConnection.table_names";
	verify_arg_count( name, values_, 0, 0, thread_, position_ );
	HDatabaseConnectionClass const* dbcClass( static_cast<HDatabaseConnectionClass const*>( HValue::get_class() ) );
	HHuginn::value_t v( thread_->runtime().none_value() );
	try {
		HDataBase::table_list_t tl( _database->get_tables() );
		v = thread_->object_factory().create_list();
		HHuginn::HList* l( static_cast<HHuginn::HList*>( v.raw() ) );
		for ( yaal::hcore::HString const& tn : tl ) {
			l->push_back( thread_->object_factory().create_string( tn ) );
		}
	} catch ( HException const& e ) {
		thread_->raise( dbcClass->exception_class(), e.what(), position_ );
	}
	return ( v );
}

HHuginn::value_t HDatabaseConnection::do_column_names(
	tools::huginn::HThread* thread_,
	HHuginn::values_t& values_,
	int position_
) {
	verify_signature( "DatabaseConnection.column_names", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
	HDatabaseConnectionClass const* dbcClass( static_cast<HDatabaseConnectionClass const*>( HValue::get_class() ) );
	HHuginn::value_t v( thread_->runtime().none_value() );
	try {
		HDataBase::column_list_t cl( _database->get_columns( get_string( values_[0] ) ) );
		v = thread_->object_factory().create_list();
		HHuginn::HList* l( static_cast<HHuginn::HList*>( v.raw() ) );
		for ( yaal::hcore::HString const& cn : cl ) {
			l->push_back( thread_->object_factory().create_string( cn ) );
		}
	} catch ( HException const& e ) {
		thread_->raise( dbcClass->exception_class(), e.what(), position_ );
	}
	return ( v );
}

HHuginn::class_t HDatabaseConnection::get_class( HRuntime* runtime_, HHuginn::class_t const& exceptionClass_ ) {
	M_PROLOG
	HHuginn::class_t c(
		runtime_->create_class(
			HRuntime::class_constructor_t(
				[&runtime_, &exceptionClass_] ( HHuginn::type_id_t typeId_ ) -> HHuginn::class_t {
					return (
						make_pointer<HDatabaseConnectionClass>(
							runtime_,
							typeId_,
							exceptionClass_
						)
					);
				}
			)
		)
	);
	runtime_->huginn()->register_class( c );
	return ( c );
	M_EPILOG
}

HHuginn::value_t HDatabaseConnection::do_clone( tools::huginn::HThread* thread_, HHuginn::value_t*, int position_ ) const {
	M_PROLOG
	throw HHuginn::HHuginnRuntimeException( "Copy semantics is not supported on DatabaseConnection.", thread_->current_frame()->file_id(), position_ );
	M_EPILOG
}

}

}

}

