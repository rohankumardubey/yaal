/*
---         `yaal' (c) 1978 by Marcin 'Amok' Konarski           ---

	hprogramoptionshandler.cxx - this file is integral part of `yaal' project.

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

#include <cstdlib>  /* getenv() */
#include <cstring>  /* strcpy(), strcat() */
#include <cstdio>   /* fopen() */
#include <getopt.h>
#include <libintl.h> /* gettext() */

#include "base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "config.hxx"
#include "hprogramoptionshandler.hxx"
#include "functional.hxx"
#include "hstring.hxx"
#include "hchunk.hxx"
#include "hregex.hxx"
#include "hlog.hxx"
#include "hcore.hxx"

using namespace yaal;
using namespace yaal::hcore;

namespace yaal {

namespace hcore {

struct RC_PATHER {
	typedef enum {
		ETC,
		ENV,
		HOME_ETC,
		HOME
	} placement_t;

	typedef enum {
		NONE = 0,
		GLOBAL = 1,
		LOCAL = 2
	} enum_t;
};

typedef HStrongEnum<RC_PATHER> placement_bit_t;

int read_rc_line( HString&, HString&, HFile&, int& );

namespace {

HString make_path( HString const& rcName_,
		RC_PATHER::placement_t placement_ ) {
	M_PROLOG
	static const char RC[] = "rc";
	HString rcPath;
	switch ( placement_ ) {
		case ( RC_PATHER::ETC ): {
			rcPath = SYSCONFDIR;
			rcPath += "/";
			rcPath += rcName_;
			rcPath += "rc";
		}
		break;
		case ( RC_PATHER::ENV ): {
			HString envName( rcName_ );
			envName += RC;
			envName.upper();
			char const* envPath( ::getenv( envName.c_str() ) );
			if ( envPath ) {
				rcPath = envPath;
			}
		}
		break;
		case ( RC_PATHER::HOME_ETC ):
		case ( RC_PATHER::HOME ): {
#if ! defined( __MSVCXX__ )
			char const USER_HOME_ENV[] = "HOME";
#else /* #if ! defined( __MSVCXX__ ) */
			char const USER_HOME_ENV[] = "USERPROFILE";
#endif /* #else #if ! defined( __MSVCXX__ ) */
			char const* homePath( ::getenv( USER_HOME_ENV ) );
			M_ENSURE( homePath );
			rcPath = homePath;
			if ( placement_ == RC_PATHER::HOME_ETC )
				rcPath += "/etc/conf/";
			else
				rcPath += "/.";
			rcPath += rcName_;
			rcPath += RC;
		}
		break;
		default:
		break;
	}
	return ( rcPath );
	M_EPILOG
}

int rc_open( HString const& rcPath_, HFile& file_ ) {
	M_PROLOG
	int error = 0;
	if ( !! file_ )
		file_.close();
	HScopedValueReplacement<int> saveErrno( errno, 0 );
	error = file_.open( rcPath_, HFile::OPEN::READING );
	HString message( rcPath_ );
	if ( error )
		message += " not found, ";
	else {
		message = "config read from: " + rcPath_;
		message += ", ";
	}
	log << message;
	return ( error );
	M_EPILOG
}

bool substitute_environment( HString& string_ ) {
	M_PROLOG
	bool envVarRefFound = false;
	if ( ! string_.is_empty() ) {
		HRegex pattern( "${[^{}]\\{1,\\}}" );
		HRegex::HMatchIterator it = pattern.find( string_.raw() );
		if ( it != pattern.end() ) {
			HString var = string_.mid( static_cast<int long>( it->raw() - string_.raw() ), it->size() );
			HString name = var.mid( 2, it->size() - 3 );
			char const* start = ::getenv( name.raw() );
			string_.replace( var, start ? start : "" );
			envVarRefFound = true;
		}
	}
	return ( envVarRefFound );
	M_EPILOG
}

}

namespace program_options_helper {

struct ORCLoader {
	HProgramOptionsHandler _optionHandler;
	HString _path;
	HString _section;
	HProgramOptionsHandler::RC_CALLBACK_t rc_callback;
 	ORCLoader( void ) :
		_optionHandler(), _path(), _section(), rc_callback( NULL ) { }
 	ORCLoader( HProgramOptionsHandler const& optionHandler_, HString const& rcName_,
		HString const& section_, HProgramOptionsHandler::RC_CALLBACK_t callback )
		: _optionHandler( optionHandler_ ),
		_path( rcName_ ), _section( section_ ),
		rc_callback( callback ) { }
 	ORCLoader( ORCLoader const& loader )
		: _optionHandler( loader._optionHandler ),
		_path( loader._path ), _section( loader._section ),
		rc_callback( loader.rc_callback ) {}
	ORCLoader& operator = ( ORCLoader const& loader ) {
		if ( &loader != this ) {
			_optionHandler = loader._optionHandler;
			_path = loader._path;
			_section = loader._section;
			rc_callback = loader.rc_callback;
		}
		return ( *this );
	}
};

class HSetup : public HSingleton<HSetup> {
public:
	typedef HSingleton<HSetup> base_type;
	typedef HSetup this_type;
	typedef HMap<HString, ORCLoader> rc_loaders_t;
	typedef rc_loaders_t::iterator iterator;
private:
	rc_loaders_t _rCLoaders;
	bool _locked;
	HSetup( void )
		: _rCLoaders(), _locked( false ) {}
	virtual ~HSetup( void ) {}
public:
	void add_section( HString const& section_, ORCLoader const& orcLoader_ ) {
		M_PROLOG
		_rCLoaders.insert( make_pair( section_, orcLoader_ ) );
		M_EPILOG
	}
	ORCLoader& operator()( HString const& section_ ) {
		M_PROLOG
		rc_loaders_t::iterator it( _rCLoaders.find( section_ ) );
		M_ENSURE( it != _rCLoaders.end() );
		return ( it->second );
		M_EPILOG
	}
	iterator begin() {
		M_PROLOG
		return ( _rCLoaders.begin() );
		M_EPILOG
	}
	iterator end() {
		M_PROLOG
		return ( _rCLoaders.end() );
		M_EPILOG
	}
	void lock( void ) {
		_locked = true;
	}
	void unlock( void ) {
		_locked = false;
	}
	bool is_locked( void ) const {
		return ( _locked );
	}
private:
	friend class HSingleton<HSetup>;
	friend class HDestructor<HSetup>;
};

class HLocker {
	HSetup& _setup;
public:
	HLocker( HSetup& setup_ ) : _setup( setup_ )
		{ _setup.lock(); }
	~HLocker( void )
		{ _setup.unlock(); }
};

}

HProgramOptionsHandler::OOption::OOption( void )
	: _name( NULL ), _value(),
	_shortForm( 0 ), _switchType( TYPE::NONE ),
	_description( NULL ), _argument( NULL ),
	CALLBACK() {}

HProgramOptionsHandler::OOption::OOption(
		char const* name_,
		HOptionValueInterface::ptr_t value_,
		int shortForm_,
		TYPE::enum_t switchType_,
		char const* argument_,
		char const* description_,
		HProgramOptionsHandler::simple_callback_t const& a_CALLBACK )
	: _name( name_ ), _value( value_ ),
	_shortForm( shortForm_ ), _switchType( switchType_ ),
	_description( description_ ), _argument( argument_ ),
	CALLBACK( a_CALLBACK ) {}

HProgramOptionsHandler::OOption::OOption( HProgramOptionsHandler::OOption const& o )
	: _name( o._name ), _value( o._value ),
	_shortForm( o._shortForm ), _switchType( o._switchType ),
	_description( o._description ), _argument( o._argument ),
	CALLBACK( o.CALLBACK ) {}

HProgramOptionsHandler::OOption& HProgramOptionsHandler::OOption::operator = ( HProgramOptionsHandler::OOption const& o ) {
	M_PROLOG
	if ( &o != this ) {
		OOption tmp( o );
		swap( tmp );
	}
	return ( *this );
	M_EPILOG
}

void HProgramOptionsHandler::OOption::swap( HProgramOptionsHandler::OOption& o ) {
	M_PROLOG
	if ( &o != this ) {
		using yaal::swap;
		swap( _name, o._name );
		swap( _value, o._value );
		swap( _shortForm, o._shortForm );
		swap( _switchType, o._switchType );
		swap( _description, o._description );
		swap( _argument, o._argument );
		swap( CALLBACK, o.CALLBACK );
	}
	return;
	M_EPILOG
}

void HProgramOptionsHandler::HOptionValueInterface::set( HString const& val ) {
	M_PROLOG
	do_set( val );
	return;
	M_EPILOG
}

type_id_t HProgramOptionsHandler::HOptionValueInterface::get_type( void ) const {
	M_PROLOG
	return ( do_get_type() );
	M_EPILOG
}

int HProgramOptionsHandler::process_rc_file( HString const& rcName_,
		HString const& section_, RC_CALLBACK_t rc_callback ) {
	M_PROLOG
	program_options_helper::HSetup& setup( program_options_helper::HSetup::get_instance() );
	if ( ! setup.is_locked() )
		setup.add_section( section_, program_options_helper::ORCLoader( *this, rcName_, section_, rc_callback ) );
	struct OPlacement {
		RC_PATHER::placement_t _placement;
		placement_bit_t _placementBit;
	} const placementTab[] = {
		{ RC_PATHER::ETC, RC_PATHER::GLOBAL },
		{ RC_PATHER::ENV, RC_PATHER::LOCAL },
		{ RC_PATHER::HOME_ETC, RC_PATHER::LOCAL },
		{ RC_PATHER::HOME, RC_PATHER::LOCAL }
	};
	bool section( false );
	bool optionOK( false );
	placement_bit_t successStory( RC_PATHER::NONE );
	HFile rc;
	HString option, value, message;
	log( LOG_TYPE::INFO ) << __FUNCTION__ << ": ";
	if ( _options.is_empty() )
		M_THROW( _( "bad variable count" ), _options.size() );
	typedef HSet<HString> paths_t;
	paths_t paths;
	for ( int placementIdx( 0 ); placementIdx < static_cast<int>( sizeof ( placementTab ) / sizeof ( OPlacement ) ); placementIdx ++ ) {
		if ( ( !! ( successStory & RC_PATHER::GLOBAL ) )
				&& ( placementTab[ placementIdx ]._placementBit == RC_PATHER::GLOBAL ) )
			continue;
		if ( !! ( successStory & RC_PATHER::LOCAL ) )
			break;
		HString rcPath( make_path( rcName_, placementTab[ placementIdx ]._placement ) );
		if ( rcPath.is_empty() )
			continue;
		if ( paths.insert( rcPath ).second && ! rc_open( rcPath, rc ) ) {
			successStory |= placementTab [ placementIdx ]._placementBit;
			int line = 0;
			while ( read_rc_line( option, value, rc, line ) ) {
				if ( ! section_.is_empty() ) {
					if ( value.is_empty() ) {
						value.format( "[%s]", section_.raw() );
						if ( option == value ) {
							if ( _debugLevel_ >= DEBUG_LEVEL::PRINT_PROGRAM_OPTIONS )
								::fprintf( stderr, "section: [%s]\n", option.raw() );
							log << "section: " << section_ << ", ";
							section = true;
							continue;
						} else
							section = false;
					}
					if ( ! section )
						continue;
				}
				while ( substitute_environment( value ) )
					;
				if ( _debugLevel_ >= DEBUG_LEVEL::PRINT_PROGRAM_OPTIONS )
					::fprintf( stderr, "option: [%s], value [%s]\n",
							option.raw(), value.raw() );
				optionOK = false;
				for ( options_t::iterator it = _options.begin(), end = _options.end(); it != end; ++ it ) {
					if ( ! strcasecmp( option, it->_name ) )
						optionOK = true, set_option( *it, value );
				}
				if ( rc_callback && rc_callback( option, value )
						&& ! optionOK ) {
					log << "failed." << endl;
					message.format( "Error: unknown option found: `%s', "
								"with value: `%s', on line %d.\n",
								option.raw(), value.raw(), line );
					log( LOG_TYPE::ERROR ) << message;
					::fputs( message.raw(), stderr );
				}
			}
		}
	}
	if ( !! rc )
		rc.close();
	log << "done." << endl;
	return ( 0 );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		int shortForm, OOption::TYPE::enum_t type,
		char const* desc, char const* arg,
		simple_callback_t const& callback ) {
	M_PROLOG
	/* If user does not specify short form by hand an automatic short form value will by assigned.
	 * Automatic short form value must be outside of byte range.
	 */
	int sf = shortForm ? shortForm : static_cast<int>( _options.size() ) + meta::max_unsigned<char unsigned>::value + 1;
	OOption o( name, value, sf, type, arg, desc, callback );
	if ( ! ( name || sf ) )
		throw HProgramOptionsHandlerException( "unnamed option encountered" );
	if ( ( ! value ) && ( ! callback.first ) )
		throw HProgramOptionsHandlerException( HString( "unused option: " ) + ( name ? HString( name ) : HString( static_cast<char>( sf ) ) ) );
	for ( options_t::const_iterator it( _options.begin() ), end( _options.end() ); it != end; ++ it ) {
		if ( ( !! value && ! it->_value ) || ( ! value && !! it->_value ) || ( !! value && ( value->id() != it->_value->id() ) ) || ( callback != it->CALLBACK ) ) {
			if ( name && it->_name && ! ::strcasecmp( it->_name, name ) )
				throw HProgramOptionsHandlerException( HString( "duplicated long option: " ) + name );
			if ( it->_shortForm == sf )
				throw HProgramOptionsHandlerException( HString( "duplicated short option: " ) + static_cast<char>( sf ) );
		}
	}
	_options.push_back( o );
	return ( *this );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		char const* shortForm, OOption::TYPE::enum_t type,
		char const* desc, char const* arg,
		simple_callback_t const& callback ) {
	M_PROLOG
	return ( operator()( name, value, shortForm[0], type, desc, arg, callback ) );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		char const* shortForm, OOption::TYPE::enum_t type,
		char const* desc, simple_callback_t const& callback ) {
	M_PROLOG
	return ( operator()( name, value, shortForm[0], type, desc, NULL, callback ) );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		int shortForm, OOption::TYPE::enum_t type,
		char const* desc, simple_callback_t const& callback ) {
	M_PROLOG
	return ( operator()( name, value, shortForm, type, desc, NULL, callback ) );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		OOption::TYPE::enum_t type,
		char const* desc, simple_callback_t const& callback ) {
	M_PROLOG
	return ( operator()( name, value, 0, type, desc, NULL, callback ) );
	M_EPILOG
}

HProgramOptionsHandler& HProgramOptionsHandler::operator()(
		char const* name, HOptionValueInterface::ptr_t value,
		OOption::TYPE::enum_t type,
		char const* desc, char const* arg,
		simple_callback_t const& callback ) {
	M_PROLOG
	return ( operator()( name, value, 0, type, desc, arg, callback ) );
	M_EPILOG
}

void const* HProgramOptionsHandler::HOptionValueInterface::id( void ) const {
	M_PROLOG
	return ( do_get() );
	M_EPILOG
}

namespace program_options_helper {

namespace {

void process_loader( ORCLoader& loader ) {
	M_PROLOG
	loader._optionHandler.process_rc_file( loader._path, loader._section, loader.rc_callback );
	return;
	M_EPILOG
}

}

int reload_configuration( void ) {
	M_PROLOG
	HSetup& setup( HSetup::get_instance() );
	HLocker lock( setup );
	log << "Reloading configuration." << endl;
	for ( HSetup::iterator it( setup.begin() ), end( setup.end() ); it != end; ++ it )
		process_loader( it->second );
	return ( 0 );
	M_EPILOG
}

}

namespace {

/* Reads one line from file_, stores beginning of line in option_,
 * stores rest of line in value_, returns 1 if there are more lines
 * to read and 0 in other case. */

void strip_comment( HString& line_ ) {
	M_PROLOG
	bool apostrophe = false, quotation = false;
	int long ctr = 0, lenght = line_.get_length();
	for ( ctr = 0; ctr < lenght; ctr ++ ) {
		switch ( line_[ ctr ] ) {
			case ( '\'' ):
				apostrophe = ! apostrophe;
			break;
			case ( '"' ):
				quotation = ! quotation;
			break;
			case ( '#' ): {
				if ( ! ( quotation || apostrophe ) ) {
					line_.set_at( ctr, 0 );
					return;
				}
			}
			break;
			default:
			break;
		}
	}
	return;
	M_EPILOG
}

char const _keyValueSep_[] = " \t=";

}

int read_rc_line( HString& option_, HString& value_, HFile& file_,
		int& line_ ) {
	M_PROLOG
	int long index = 0, lenght = 0, end = 0;
	option_ = value_ = "";
	while ( file_.read_line( option_ ) >= 0 ) {
		line_ ++;
		index = 0;
		if ( ! option_[ index ] )
			continue; /* empty line */
		/* we are looking for first non-whitespace on the line */
		index = option_.find_other_than( _whiteSpace_.data() );
		if ( ! option_[ index ] || ( option_[ index ] == '#' ) )
			continue; /* there is only white spaces or comments on that line */
		/* at this point we know we have _some_ option */
		strip_comment( option_ );
		/* strip comment from end of line */
		lenght = option_.get_length();
		if ( index ) {
			option_.shift_left( index );
			lenght -= index;
		}
		/* now we look for first key-value separator after option */
		if ( ( index = option_.find_one_of( _keyValueSep_ ) ) > 0 ) {
			/* we have found a whitespace, so there is probability that */
			/* have a value :-o */
			int long endOfOption = index;
			index = option_.find_other_than( _keyValueSep_, index );
			if ( ( index > 0 ) && option_[ index ] ) {
				if ( count( option_.raw() + endOfOption, option_.raw() + index, '=' ) > 1 )
					throw HProgramOptionsHandlerException( "Syntax error: redundant `=' sign.", line_ );
				/* we have found a non-whitespace, so there certainly is a value */
				end = ( lenght - 1 ) - option_.reverse_find_other_than( _whiteSpace_.data() );
				/* now we strip apostrophe or quotation marks */
				if ( ( ( option_[ end ] == '\'' )
							|| ( option_[ end ] == '"' ) )
						&& ( option_[ end ] == option_[ index ] ) )
					index ++, end --;
				if ( ( end + 1 ) < lenght )
					option_.set_at( end + 1, 0 );
				value_ = option_.mid( index );
			}
			option_.set_at( endOfOption, 0 );
		}
		return ( 1 );
	}
	return ( 0 );
	M_EPILOG
}

void HProgramOptionsHandler::set_option( OOption& option_, HString const& value_ ) {
	M_PROLOG
	if ( !! option_._value ) {
		if ( option_._switchType == OOption::TYPE::NONE ) {
			M_ENSURE( option_._value->get_type() == TYPE::BOOL );
			option_._value->set( "true" );
		} else
			option_._value->set( value_ );
	}
	if ( option_.CALLBACK.first )
		option_.CALLBACK.first( option_.CALLBACK.second );
	return;
	M_EPILOG
}

namespace {

char const* make_short_opts( HProgramOptionsHandler::options_t const& options_, HString& buffer_ ) {
	M_PROLOG
	buffer_ = "";
	for ( HProgramOptionsHandler::options_t::const_iterator it = options_.begin(),
			end = options_.end(); it != end; ++ it ) {
		if ( it->_shortForm > static_cast<int>( meta::max_unsigned<char unsigned>::value ) )
			continue;
		buffer_ += static_cast<char>( it->_shortForm );
		switch ( it->_switchType ) {
			case ( HProgramOptionsHandler::OOption::TYPE::REQUIRED ):
				buffer_ += ':';
			break;
			case ( HProgramOptionsHandler::OOption::TYPE::OPTIONAL ):
				buffer_ += "::";
			break;
			case ( HProgramOptionsHandler::OOption::TYPE::NONE ):
			default :
				break;
		}
	}
	return ( buffer_.raw() );
	M_EPILOG
}

option* make_option_array( HProgramOptionsHandler::options_t const& options_, HChunk& buffer_ ) {
	M_PROLOG
	option* options = NULL;
	options = buffer_.get<option>();
	int ctr = 0;
	for ( HProgramOptionsHandler::options_t::const_iterator it = options_.begin(),
			end = options_.end(); it != end; ++ it, ++ ctr ) {
		memset( &options[ ctr ], 0, sizeof ( option ) );
		/* Solaris version of `struct option' is braindead broken.
		 * Another proof that Solaris sucks big time.
		 */
		options[ ctr ].name = const_cast<char*>( it->_name );
		switch ( it->_switchType ) {
			case ( HProgramOptionsHandler::OOption::TYPE::REQUIRED ):
				options[ ctr ].has_arg = required_argument;
			break;
			case ( HProgramOptionsHandler::OOption::TYPE::OPTIONAL ):
				options[ ctr ].has_arg = optional_argument;
			break;
			case ( HProgramOptionsHandler::OOption::TYPE::NONE ):
			default :
				options[ ctr ].has_arg = no_argument;
			break;
		}
		options[ ctr ].val = it->_shortForm;
	}
	return ( options );
	M_EPILOG
}

}

int HProgramOptionsHandler::process_command_line( int argc_,
		char* const* const argv_,
		int* const unknown_ ) {
	M_PROLOG
	int val( 0 );
	HString shortOptBuffer;
	HChunk longOptBuffer( chunk_size<option>( _options.size() + 1 ) ); /* + 1 for array terminator */
	hcore::log( LOG_TYPE::INFO ) << "Decoding switches ... ";
	char const* shortOpts( make_short_opts( _options, shortOptBuffer ) );
	option* optionArray( make_option_array( _options, longOptBuffer ) );
	while ( ( val = ::getopt_long( argc_, argv_, shortOpts,
					optionArray, NULL ) ) != EOF ) {
		bool validSwitch( false );
		for ( options_t::iterator it = _options.begin(), end = _options.end(); it != end; ++ it ) {
			if ( it->_shortForm == val )
				validSwitch = true, set_option( *it, optarg );
		}
		if ( ! validSwitch && unknown_ )
			( *unknown_ ) ++;
	}
	hcore::log << "done." << endl;
	return ( optind );
	M_EPILOG
}

}

}

