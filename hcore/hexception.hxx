/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hexception.hxx - this file is integral part of `yaal' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You cannot use any part of sources of this software.
  2. You cannot redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you cannot demand any fees
     for this software.
     You cannot even demand cost of the carrier (CD for example).
  5. You cannot include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/
/*! \file hcore/hexception.hxx
 * \brief HException and HExceptionT<> class declaration.
 *
 * Auto-exception-hierachy stuff is decalred here.
 */

#ifndef YAAL_HCORE_HEXCEPTION_HXX_INCLUDED
#define YAAL_HCORE_HEXCEPTION_HXX_INCLUDED

#include <cerrno>
#include <cstddef>
#include <typeinfo>

#ifdef __YAAL_BUILD__
#	include "config.hxx"
#endif /* __YAAL_BUILD__ */
#include "hcore/trait.hxx"
#include "hcore/hstring.hxx"

typedef void this_type;
typedef void base_type;

namespace yaal
{

namespace hcore
{

extern int _debugLevel_;
struct DEBUG_LEVEL
	{
	static int const PRINT_PROGRAM_OPTIONS = 3;
	static int const PRINT_EXCEPTION_STACK = 5;
	static int const GDB = 7;
	static int const ABORT_ON_ASSERT = 10;
	};

/*! \brief Base exception class for YAAL software.
 *
 * This class is base class for all exceptions used (generated)
 * in \e yaal library.
 */
class HException
	{
	mutable bool _local;
protected:
	static void* ERROR_STREAM;
	char _char;
	int	 _int;
	long _long;
	double _double;
	char* _charPtr;
	void* _voidPtr;
	int  _frame;
	char* _fileName;
	char* _functionName;
	int  _code;
	char* _message;
public:
	HException( char const* const, char const* const, int const,
			char const* const, int const = 0 );
	HException( char const* const, char const* const, int const,
			HString const&, int const = 0 );
	HException( HException const& );
	virtual ~HException( void ) ;
	void set( char const = 0, int const = 0, long const = 0, double const = 0,
			char const* const = 0, void* const = 0 );
	void set( char const* );
	void set( HString const& );
	void print_error( bool const ) const;
	void log( char const* const, char const* const, int const );
	char const* what( void ) const;
	int code( void ) const;
	static void failed_assert( char const* const, char const* const, int const,
			char const* const ) __attribute__(( __noreturn__ ));
	static void set_error_stream( void* );
	static char* get_type_name( char const* const );
	static void cleanup( char* );
private:
	HException& operator = ( HException const& );
	};

extern char const* const _exceptionType_;

/*! \brief Template used to create type specyfic exceptions.
 */
template<typename tType, typename base_type = HException>
class HExceptionT : public base_type
	{
public:
	HExceptionT( char const* const reason_, char* ptr = base_type::get_type_name( typeid( tType ).name() ) )
		: base_type( _exceptionType_, ptr, 0, reason_, errno )
		{ base_type::cleanup( ptr );	}
	HExceptionT( HString const& reason_, char* ptr = base_type::get_type_name( typeid( tType ).name() ) )
		: base_type( _exceptionType_, ptr, 0, reason_, errno )
		{ base_type::cleanup( ptr );	}
	HExceptionT( char const* const fileName_,
			char const* const functionName_, int const line_,
			char const* const reason_, int const code_ )
		: base_type( fileName_, functionName_, line_, reason_, code_ )
		{	}
	HExceptionT( char const* const fileName_,
			char const* const functionName_, int const line_,
			HString const& reason_, int const code_ )
		: base_type( fileName_, functionName_, line_, reason_, code_ )
		{	}
	};

typedef HExceptionT<HString> HStringException;

/*! \brief Failed assertion exception.
 *
 * In \e DEBUG build failuers in assertions does not abort the
 * process, insead the throw instance of HFailedAssertion.
 */
class HFailedAssertion
	{
	char const* _what;
public:
	HFailedAssertion( char const* const what_ ) : _what( what_ ) {}
	HFailedAssertion( HFailedAssertion const& fa ) : _what( fa._what ) {}
	HFailedAssertion& operator = ( HFailedAssertion const& fa )
		{
		if ( &fa != this )
			{
			HFailedAssertion n( fa );
			swap( n );
			}
		return ( *this );
		}
	char const* what( void ) { return ( _what ); }
private:
	void swap( HFailedAssertion& );
	};

/*! \brief Yaal's default exception handling policy.
 */
struct HGlobalScopeExceptionHandlingPolicy
	{
	static void handle_exception( void ) __attribute__(( __noreturn__ ));
	};

/*! \brief Exception safe wrapper for global variables.
 */
template<typename global_t, typename exception_handling_policy_t = HGlobalScopeExceptionHandlingPolicy>
class HExceptionSafeGlobal
	{
	global_t _object;
public:
	HExceptionSafeGlobal( void ) try : _object() {} catch ( ... ) { exception_handling_policy_t::handle_exception(); }
	template<typename a0_t>
	HExceptionSafeGlobal( a0_t const& a0_ ) try : _object( a0_ ) {} catch ( ... ) { exception_handling_policy_t::handle_exception(); }
	template<typename a0_t, typename a1_t>
	HExceptionSafeGlobal( a0_t const& a0_, a1_t const& a1_ ) try : _object( a0_, a1_ ) {} catch ( ... ) { exception_handling_policy_t::handle_exception(); }
	global_t& instance( void )
		{ return ( _object ); }
	};

/*
 * A,     AE,      AE = ET<A, E>
 * B : A, BE : AE, BE = ET<B, AE>
 * C : B, CE : BE, CE = ET<C, BE>
 */

/*! \brief Helper %tools for automatic creation for (YEH) yaal exception hierarchy.
 *
 * Non of members of this util namespace should be used directly.
 */
namespace exception_auto_hierarchy
{

template<typename tType>
struct parent_exception;

/*! \brief Meta-function, parent_exception helper, gets existing parent exception or creates newone.
 */
template<typename tType>
struct void_to_exception
	{
	typedef typename parent_exception<tType>::parent_exception_t parent_exception_t;
	typedef HExceptionT<tType, parent_exception_t> exception_t;
	};

/*! \cond */
template<>
struct void_to_exception<void>
	{
	typedef yaal::hcore::HException exception_t;
	};
/*! \endcond */

/*! \brief exception auto-hierarchy helper
 *
 * void_to_base metafunction resolves non void tType type to type and void to base.
 *
 * \tparam tType - main type for type resolving.
 * \tparam base_t - base type for type resolving.
 * \retval type - resolved type of base exception.
 */
template<typename tType, typename base_t>
struct void_to_base
	{
	typedef tType type;
	};

/*! \cond */
template<typename base_t>
struct void_to_base<void, base_t>
	{
	typedef base_t type;
	};
/*! \endcond */

template<int const, typename>
struct existing_hier;

/*! \brief Get superclass in hierarchy if one exists, predefined value otherwise.
 */
template<typename tType>
struct context_hier
	{
	template<typename real_class>
	static trait::true_type has_hier( typename real_class::base_type* );
	template<typename real_class>
	static trait::false_type has_hier( ... );
	typedef typename existing_hier<sizeof ( has_hier<tType>( 0 ) ), tType>::type type;
	};

/*! \brief Get type of existing superclass (in hierarchy) of a type.
 *
 * \tparam subclass - subclass type in hierarchy.
 * \retval type - superclass in hierarchy.
 */
template<typename subclass_t>
struct existing_hier<sizeof ( trait::true_type ), subclass_t>
	{
	/*! \brief Meta-function, get typedef from protected section of a type.
	 *
	 * \tparam hermetic - type to infiltrate.
	 * \retval type - superclass in hierarchy.
	 */
	template<typename hermetic>
	struct get_protected_typedef : public hermetic
		{
		typedef typename hermetic::base_type type;
		};
	typedef typename get_protected_typedef<subclass_t>::type type;
	};

/*! \cond */
template<typename subclass>
struct existing_hier<sizeof ( trait::false_type ), subclass>
	{
	typedef base_type type;
	};
/*! \endcond */

/*! \brief Meta-function to retrieve parent exception of given exception.
 *
 * \tparam tType - childe exception type.
 */
template<typename tType>
struct parent_exception
	{
	typedef typename context_hier<tType>::type proposed_type_hier_t;
	typedef typename void_to_exception<proposed_type_hier_t>::exception_t parent_exception_t;
	};

/*! \cond */
template<>
struct parent_exception<void>
	{
	typedef yaal::hcore::HException parent_exception_t;
	};
/*! \endcond */

}

template<typename tType, typename message_t, typename code_t>
void throw_exception( char const* const&, char const* const&, int const&, message_t const&, code_t const&, char const* const& = NULL ) __attribute__(( __noreturn__ ));
template<typename tType, typename message_t, typename code_t>
void throw_exception( char const* const& file, char const* const& function, int const& line, message_t const& message, code_t const& code, char const* const& reason )
	{
	typedef typename exception_auto_hierarchy::parent_exception<tType>::parent_exception_t parent_exception_t;
	typedef typename yaal::hcore::HExceptionT<tType, parent_exception_t> exception_t;
	if ( reason )
		{
		exception_t e( file, function, line, message, static_cast<int>( code ) );
	 	e.set( reason );
		throw e;
		}
	else
		throw exception_t( file, function, line, message, static_cast<int>( code ) );
	}

template<typename tType, typename message_t, typename code_t>
void throw_exception( char const* const&, char const* const&, int const&, message_t const&, code_t const&, HString const&, HString const& ) __attribute__(( __noreturn__ ));
template<typename tType, typename message_t, typename code_t>
void throw_exception( char const* const& file, char const* const& function, int const& line, message_t const& message, code_t const& code, HString const& reason, HString const& comment )
	{
	typedef typename exception_auto_hierarchy::parent_exception<tType>::parent_exception_t parent_exception_t;
	typedef typename yaal::hcore::HExceptionT<tType, parent_exception_t> exception_t;
	exception_t e( file, function, line, message, static_cast<int>( code ) );
	e.set( reason + ": " + comment );
	throw e;
	}

}

}

#endif /* not YAAL_HCORE_HEXCEPTION_HXX_INCLUDED */

