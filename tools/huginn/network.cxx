/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  network.cxx - this file is integral part of `yaal' project.

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
#include "tools/huginn/thread.hxx"
#include "tools/huginn/stream.hxx"
#include "helper.hxx"
#include "exception.hxx"
#include "hcore/hsocket.hxx"
#include "hcore/resolver.hxx"
#include "packagefactory.hxx"
#include "objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

class HNetwork : public HHuginn::HValue {
	struct OPERATIONS {
		static int const READING = 1;
		static int const WRITING = 2;
	};
	HHuginn::class_t _streamClass;
	HHuginn::class_t _exceptionClass;
public:
	HNetwork( HHuginn::HClass* class_ )
		: HValue( class_ )
		, _streamClass( HStream::get_class( class_->runtime() ) )
		, _exceptionClass(
			exception::create_class(
				class_->runtime(),
				"NetworkException",
				"The `NetworkException` exception type for `Network` package."
			)
		) {
		return;
	}
	static HHuginn::value_t resolve( huginn::HThread* thread_, HHuginn::value_t*, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Network.resolve", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
		return ( thread_->object_factory().create_string( resolver::ip_to_string( resolver::get_ip( get_string( values_[0] ) ) ) ) );
		M_EPILOG
	}
	static HHuginn::value_t connect( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		return ( static_cast<HNetwork*>( object_->raw() )->do_connect( thread_, values_, position_ ) );
		M_EPILOG
	}
private:
	HHuginn::value_t do_connect( huginn::HThread* thread_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		char const name[] = "Network.connect";
		verify_arg_count( name, values_, 1, 2, thread_, position_ );
		ARITY arity( values_.get_size() > 1 ? ARITY::MULTIPLE : ARITY::UNARY );
		verify_arg_type( name, values_, 0, HHuginn::TYPE::STRING, arity, thread_, position_ );
		verify_arg_type( name, values_, 1, HHuginn::TYPE::INTEGER, arity, thread_, position_ );
		int port( 0 );
		if ( arity == ARITY::MULTIPLE ) {
			port = static_cast<int>( get_integer( values_[1] ) );
			if ( ( port <= 0 ) || ( port > 65535 ) ) {
				thread_->raise( _exceptionClass.raw(), "Bad port: "_ys.append( port ), position_ );
				port = -1;
			}
		}
		HHuginn::value_t v( thread_->runtime().none_value() );
		if ( port >= 0 ) {
			HStreamInterface::ptr_t stream( make_pointer<HSocket>( port > 0 ? HSocket::TYPE::NETWORK : HSocket::TYPE::FILE ) );
			HSocket* s( static_cast<HSocket*>( stream.raw() ) );
			try {
				s->connect( get_string( values_[0] ), port );
				v = thread_->object_factory().create<HStream>( _streamClass.raw(), stream );
			} catch ( HResolverException const& e ) {
				thread_->raise( _exceptionClass.raw(), e.what(), position_ );
			} catch ( HSocketException const& e ) {
				thread_->raise( _exceptionClass.raw(), e.what(), position_ );
			}
		}
		return ( v );
		M_EPILOG
	}
};

namespace package_factory {

class HNetworkCreator : public HPackageCreatorInterface {
protected:
	virtual HHuginn::value_t do_new_instance( HRuntime* );
} networkCreator;

HHuginn::value_t HNetworkCreator::do_new_instance( HRuntime* runtime_ ) {
	M_PROLOG
	HHuginn::class_t c(
		runtime_->create_class(
			"Network",
			nullptr,
			HHuginn::field_definitions_t{
				{ "connect", runtime_->object_factory()->create_method( hcore::call( &HNetwork::connect, _1, _2, _3, _4 ) ), "( *host*, *port* ) - create a TCP connection to given *host* at given *port*" },
				{ "resolve", runtime_->object_factory()->create_method( hcore::call( &HNetwork::resolve, _1, _2, _3, _4 ) ), "( *hostName* ) - resolve IP address of given *hostName*" }
			},
			"The `Network` package provides access to various networking related functionalities."
		)
	);
	runtime_->huginn()->register_class( c );
	return ( runtime_->object_factory()->create<HNetwork>( c.raw() ) );
	M_EPILOG
}

namespace {

bool registrar( void ) {
	M_PROLOG
	bool volatile failed = false;
	HPackageFactory& factory = HPackageFactory::get_instance();
	factory.register_package_creator( "Network", &networkCreator );
	return ( failed );
	M_EPILOG
}

bool volatile registered = registrar();

}

}

}

}

}

