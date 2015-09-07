/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  stream.cxx - this file is integral part of `yaal' project.

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
#include "stream.hxx"
#include "iterator.hxx"
#include "helper.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

class HStreamIterator : public HIteratorInterface {
	HStream* _stream;
	HString _lineCache;
public:
	HStreamIterator( HStream* stream_ )
		: _stream( stream_ )
		, _lineCache( _stream->read_line_impl() ) {
		return;
	}
protected:
	virtual HHuginn::value_t do_value( void ) override {
		return ( make_pointer<HHuginn::HString>( _lineCache ) );
	}
	virtual bool do_is_valid( void ) override {
		return ( _stream->is_valid() );
	}
	virtual void do_next( void ) override {
		_lineCache = _stream->read_line_impl();
	}
private:
	HStreamIterator( HStreamIterator const& ) = delete;
	HStreamIterator& operator = ( HStreamIterator const& ) = delete;
};

HStream::HStream( HHuginn::HClass* class_, HStreamInterface::ptr_t stream_ )
	: HIterable( class_ )
	, _stream( stream_ )
	, _buffer()
	, _lineBuffer() {
	return;
}

HHuginn::value_t HStream::read( huginn::HThread*, HHuginn::HObject* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Stream.read";
	verify_arg_count( name, values_, 1, 1, position_ );
	verify_arg_type( name, values_, 0, HHuginn::TYPE::INTEGER, true, position_ );
	int size( static_cast<int>( get_integer( values_[0] ) ) );
	HStream* s( static_cast<HStream*>( object_ ) );
	return ( make_pointer<HHuginn::HString>( s->read_impl( size ) ) );
	M_EPILOG
}

HHuginn::value_t HStream::read_line( huginn::HThread*, HHuginn::HObject* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	verify_arg_count( "Stream.read_line", values_, 0, 0, position_ );
	HStream* s( static_cast<HStream*>( object_ ) );
	return ( make_pointer<HHuginn::HString>( s->read_line_impl() ) );
	M_EPILOG
}

HHuginn::value_t HStream::write( huginn::HThread*, HHuginn::HObject* object_, HHuginn::values_t const& values_, int position_ ) {
	M_PROLOG
	char const name[] = "Stream.write";
	verify_arg_count( name, values_, 1, 1, position_ );
	verify_arg_type( name, values_, 0, HHuginn::TYPE::STRING, true, position_ );
	HString const& val( get_string( values_[0] ) );
	HStream* s( static_cast<HStream*>( object_ ) );
	s->write_impl( val );
	return ( _none_ );
	M_EPILOG
}

HString HStream::read_impl( int long size_ ) {
	M_PROLOG
	_buffer.realloc( size_ );
	int long nRead( _stream->read( _buffer.raw(), size_ ) );
	HString s( _buffer.get<char>(), nRead );
	return ( s );
	M_EPILOG
}

void HStream::write_impl( HString const& val_ ) {
	M_PROLOG
	_stream->write( val_.raw(), val_.get_length() );
	M_EPILOG
}

yaal::hcore::HString const& HStream::read_line_impl( void ) {
	M_PROLOG
	_stream->read_until( _lineBuffer, HStreamInterface::eols, false );
	return ( _lineBuffer );
	M_EPILOG
}

bool HStream::is_valid( void ) const {
	M_PROLOG
	return ( _stream->is_valid() );
	M_EPILOG
}

HHuginn::HIterable::HIterator HStream::do_iterator( void ) {
	HIterator::iterator_implementation_t impl( new ( memory::yaal ) HStreamIterator( this ) );
	return ( HIterator( yaal::move( impl ) ) );
}

int long HStream::do_size( void ) const {
	return ( 0 );
}

HHuginn::class_t HStream::get_class( HHuginn* huginn_ ) {
	M_PROLOG
	char const name[] = "Stream";
	HHuginn::class_t c( huginn_->get_class( name ) );
	if ( ! c ) {
		c =	make_pointer<HHuginn::HClass>(
			huginn_,
			HHuginn::HType::register_type( name, huginn_ ),
			nullptr,
			HHuginn::HClass::field_names_t{
				"read",
				"read_line",
				"write"
			},
			HHuginn::values_t{
				make_pointer<HHuginn::HClass::HMethod>( hcore::call( &HStream::read, _1, _2, _3, _4 ) ),
				make_pointer<HHuginn::HClass::HMethod>( hcore::call( &HStream::read_line, _1, _2, _3, _4 ) ),
				make_pointer<HHuginn::HClass::HMethod>( hcore::call( &HStream::write, _1, _2, _3, _4 ) )
			}
		);
		huginn_->register_class( c );
	}
	return ( c );
	M_EPILOG
}

}

}

}

