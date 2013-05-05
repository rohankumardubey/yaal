/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	tools/executingparser.hxx - this file is integral part of `yaal' project.

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
/*! \file tools/executingparser.hxx
 * Dleclaration of executing_parser namespace.
 */

#ifndef YAAL_TOOLS_EXECUTINGPARSER_HXX_INCLUDED
#define YAAL_TOOLS_EXECUTINGPARSER_HXX_INCLUDED 1

#include "hcore/hstring.hxx"
#include "hcore/hnumber.hxx"
#include "hcore/harray.hxx"
#include "hcore/hlist.hxx"
#include "hcore/hboundcall.hxx"

namespace yaal {

namespace tools {

/*! \brief Namespace containing classes composing executing parser infrastructure.
 */
namespace executing_parser {

class HRuleBase {
protected:
	typedef yaal::hcore::HPointer<HRuleBase> ptr_t;
	typedef yaal::hcore::HBoundCall<> action_t;
	typedef yaal::hcore::HBoundCall<> executor_t;
	typedef yaal::hcore::HArray<executor_t> executors_t;
	bool _matched;
	action_t _action;
	executors_t _excutors;
public:
	typedef HRuleBase this_type;
	HRuleBase( void );
	HRuleBase( action_t const& );
	virtual ~HRuleBase( void )
		{}
	bool operator()( yaal::hcore::HString const& input_ );
	bool operator()( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
	void operator()( void );
	void execute( void );
	yaal::hcore::HString::const_iterator parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
	ptr_t clone( void ) const;
	bool is_optional( void ) const;
protected:
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
	virtual void do_execute( void );
	virtual ptr_t do_clone( void ) const = 0;
	virtual bool do_is_optional( void ) const;
	static yaal::hcore::HString::const_iterator skip_space( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
};

typedef yaal::hcore::HExceptionT<HRuleBase> HRuleBaseException;

class HRule : public HRuleBase {
	ptr_t _rule;
public:
	HRule( void );
	HRule( HRule const& );
	HRule( HRuleBase const& );
	HRule( ptr_t const& );
	HRule( ptr_t const&, action_t const& );
	HRule operator[]( action_t const& action_ ) const;
protected:
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
	virtual void do_execute( void );
	virtual ptr_t do_clone( void ) const;
	virtual bool do_is_optional( void ) const;
};

typedef yaal::hcore::HExceptionT<HRule, HRuleBaseException> HRuleException;

class HFollows : public HRuleBase {
	typedef yaal::hcore::HList<HRuleBase::ptr_t> rules_t;
	rules_t _rules;
public:
	typedef HFollows this_type;
	typedef HRuleBase base_type;
	HFollows( HRuleBase const& predecessor_, HRuleBase const& successor_ );
	HFollows( HFollows const& follows_ );
	HFollows( HFollows const& predecessors_, HRuleBase const& successor_ );
	virtual ~HFollows( void )
		{}
	HRule operator[]( action_t const& action_ ) const;
protected:
	virtual ptr_t do_clone( void ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
	virtual void do_execute( void );
};

typedef yaal::hcore::HExceptionT<HFollows, HRuleBaseException> HFollowsException;

class HKleeneStar : public HRuleBase {
	ptr_t _rule;
public:
	typedef HKleeneStar this_type;
	typedef HRuleBase base_type;
	HKleeneStar( HRuleBase const& rule_ );
	HKleeneStar( HKleeneStar const& kleeneStar_ );
	HKleeneStar operator[]( action_t const& ) const;
	virtual ~HKleeneStar( void )
		{}
protected:
	HKleeneStar( ptr_t, action_t );
	virtual ptr_t do_clone( void ) const;
	virtual void do_execute( void );
	virtual bool do_is_optional( void ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
};

typedef yaal::hcore::HExceptionT<HKleeneStar, HRuleBaseException> HKleeneStarException;

class HKleenePlus : public HRuleBase {
	ptr_t _rule;
public:
	typedef HKleenePlus this_type;
	typedef HRuleBase base_type;
	HKleenePlus( HRuleBase const& rule_ );
	HKleenePlus( HKleenePlus const& kleenePlus_ );
	HKleenePlus operator[]( action_t const& ) const;
	virtual ~HKleenePlus( void )
		{}
protected:
	HKleenePlus( ptr_t, action_t );
	virtual ptr_t do_clone( void ) const;
	virtual void do_execute( void );
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
};

typedef yaal::hcore::HExceptionT<HKleenePlus, HRuleBaseException> HKleenePlusException;

class HAlternative : public HRuleBase {
	typedef yaal::hcore::HList<HRuleBase::ptr_t> rules_t;
	rules_t _rules;
public:
	typedef HAlternative this_type;
	typedef HRuleBase base_type;
	HAlternative( HRuleBase const& choice1_, HRuleBase const& choice2_ );
	HAlternative( HAlternative const& alternative_ );
	HAlternative( HAlternative const& alternative_, HRuleBase const& choice_ );
protected:
	virtual ptr_t do_clone( void ) const;
};

typedef yaal::hcore::HExceptionT<HAlternative, HRuleBaseException> HAlternativeException;

class HOptional : public HRuleBase {
protected:
	virtual bool do_is_optional( void ) const;
};

typedef yaal::hcore::HExceptionT<HOptional, HRuleBaseException> HOptionalException;

HFollows operator >> ( HRuleBase const& predecessor_, HRuleBase const& successor_ );
HFollows operator >> ( HFollows const& predecessors_, HRuleBase const& successor_ );
HAlternative operator | ( HRuleBase const& choice1_, HRuleBase const& choice2_ );
HAlternative operator | ( HAlternative const& alternative_, HRuleBase const& choice_ );
HKleeneStar operator* ( HRuleBase const& rule_ );
HKleenePlus operator+ ( HRuleBase const& rule_ );

class HReal : public HRuleBase {
	typedef yaal::hcore::HBoundCall<void ( double )> action_double_t;
	typedef yaal::hcore::HBoundCall<void ( double long )> action_double_long_t;
	typedef yaal::hcore::HBoundCall<void ( yaal::hcore::HNumber const& )> action_number_t;
	typedef yaal::hcore::HBoundCall<void ( yaal::hcore::HString const& )> action_string_t;
	action_double_t _actionDouble;
	action_double_long_t _actionDoubleLong;
	action_number_t _actionNumber;
	action_string_t _actionString;
	yaal::hcore::HString _cache;
	typedef enum {
		START = 0,
		MINUS = 1,
		INTEGRAL = 2,
		DOT = 3,
		DECIMAL = 4
	} real_paring_state_t;
public:
	typedef HReal this_type;
	typedef HRuleBase base_type;
	HReal( void );
	HReal operator[]( action_t const& ) const;
	HReal operator[]( action_double_t const& ) const;
	HReal operator[]( action_double_long_t const& ) const;
	HReal operator[]( action_number_t const& ) const;
	HReal operator[]( action_string_t const& ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
protected:
	HReal( action_t );
	HReal( action_double_t );
	HReal( action_double_long_t );
	HReal( action_number_t );
	HReal( action_string_t );
	HReal( HReal const& );
	virtual ptr_t do_clone( void ) const;
};

extern HReal real;

typedef yaal::hcore::HExceptionT<HReal, HRuleBaseException> HRealException;

class HInteger : public HRuleBase {
	typedef yaal::hcore::HBoundCall<void ( int long )> action_int_long_t;
	typedef yaal::hcore::HBoundCall<void ( int )> action_int_t;
	typedef yaal::hcore::HBoundCall<void ( yaal::hcore::HNumber const& )> action_number_t;
	typedef yaal::hcore::HBoundCall<void ( yaal::hcore::HString const& )> action_string_t;
	action_int_long_t _actionIntLong;
	action_int_t _actionInt;
	action_number_t _actionNumber;
	action_string_t _actionString;
	yaal::hcore::HString _cache;
	typedef enum {
		START = 0,
		MINUS = 1,
		DIGIT = 2
	} real_paring_state_t;
public:
	typedef HInteger this_type;
	typedef HRuleBase base_type;
	HInteger( void );
	HInteger( HInteger const& );
	HInteger operator[]( action_t const& ) const;
	HInteger operator[]( action_int_long_t const& ) const;
	HInteger operator[]( action_int_t const& ) const;
	HInteger operator[]( action_number_t const& ) const;
	HInteger operator[]( action_string_t const& ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
protected:
	HInteger( action_t );
	HInteger( action_int_long_t );
	HInteger( action_int_t );
	HInteger( action_number_t );
	HInteger( action_string_t );
	virtual ptr_t do_clone( void ) const;
};

extern HInteger integer;

typedef yaal::hcore::HExceptionT<HInteger, HRuleBaseException> HIntegerException;

class HCharacter : public HRuleBase {
	typedef yaal::hcore::HBoundCall<void ( char )> action_char_t;
	char _character;
	action_char_t _actionChar;
public:
	typedef HCharacter this_type;
	typedef HRuleBase base_type;
	HCharacter( char character_ = 0 );
	HCharacter( HCharacter const& character_ );
	virtual ~HCharacter( void )
		{}
	HCharacter operator[]( action_t const& ) const;
	HCharacter operator[]( action_char_t const& ) const;
	HCharacter operator() ( char ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
protected:
	HCharacter( char, action_t );
	HCharacter( char, action_char_t );
	virtual ptr_t do_clone( void ) const;
};
extern HCharacter character;

typedef yaal::hcore::HExceptionT<HCharacter, HRuleBaseException> HCharacterException;

HFollows operator >> ( char character_, HRuleBase const& successor_ );
HFollows operator >> ( HRuleBase const& successor_, char );

class HString : public HRuleBase {
	typedef yaal::hcore::HBoundCall<void ( yaal::hcore::HString const& )> action_string_t;
	yaal::hcore::HString _string;
	action_string_t _actionString;
public:
	typedef HString this_type;
	typedef HRuleBase base_type;
	HString( yaal::hcore::HString const& string_ = yaal::hcore::HString() );
	HString( HString const& string_ );
	virtual ~HString( void )
		{}
	HString operator[]( action_t const& ) const;
	HString operator[]( action_string_t const& ) const;
	HString operator() ( yaal::hcore::HString const& ) const;
	virtual yaal::hcore::HString::const_iterator do_parse( yaal::hcore::HString::const_iterator first_, yaal::hcore::HString::const_iterator last_ );
protected:
	HString( yaal::hcore::HString const&, action_t );
	HString( yaal::hcore::HString const&, action_string_t );
	virtual ptr_t do_clone( void ) const;
};

extern HString string;

typedef yaal::hcore::HExceptionT<HString, HRuleBaseException> HStringException;

HFollows operator >> ( char const* string_, HRuleBase const& successor_ );
HFollows operator >> ( yaal::hcore::HString const& string_, HRuleBase const& successor_ );
HFollows operator >> ( HRuleBase const& predecessor_, char const* string_ );
HFollows operator >> ( HRuleBase const& predecessor_, yaal::hcore::HString const& string_ );

}

}

}

#endif /* #ifndef YAAL_TOOLS_EXECUTINGPARSER_HXX_INCLUDED */

