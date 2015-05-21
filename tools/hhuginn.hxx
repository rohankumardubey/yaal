/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  tools/hhuginn.hxx - this file is integral part of `yaal' project.

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
/*! \file tools/hhuginn.hxx
 * Declaration of HHuginn class and all its nested classes.
 */

#ifndef YAAL_TOOLS_HHUGINN_HXX_INCLUDED
#define YAAL_TOOLS_HHUGINN_HXX_INCLUDED 1

#include <atomic>

#include "hcore/hmap.hxx"
#include "hcore/hhashmap.hxx"
#include "hcore/hstack.hxx"
#include "hcore/hstreaminterface.hxx"
#include "hcore/hthread.hxx"
#include "hcore/htaggedpod.hxx"
#include "tools/executingparser.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HSource;
class HFrame;
class HThread;
class HExpression;
class HScope;
struct OCompiler;

namespace ERR_CODE {
enum {
	OP_NOT_SUM = 0,
	OP_NOT_SUB,
	OP_NOT_MUL,
	OP_NOT_DIV,
	OP_NOT_EXP,
	OP_NOT_CMP,
	OPS_NOT_BOOL,
	OP_NOT_BOOL,
	IDX_NOT_INT
};
}

extern M_YAAL_TOOLS_PUBLIC_API char const* _errMsgHHuginn_[];

}

/*! \brief Huginn programming language implementation.
 */
class HHuginn {
public:
	typedef HHuginn this_type;
	class HType {
	public:
		typedef HType this_type;
		typedef HType const* type_t;
		typedef yaal::hcore::HResource<HType const> type_holder_t;
		typedef yaal::hcore::HTaggedPOD<int, HType> id_t;
		typedef std::atomic<id_t::value_type> id_generator_t;
		typedef yaal::hcore::HHashMap<yaal::hcore::HString, type_holder_t> type_dict_t;
	private:
		yaal::hcore::HString _name;
		id_t _id;
		static id_generator_t _idGenerator;
		static type_dict_t _builtin;
	public:
		yaal::hcore::HString const& name( void ) const {
			return ( _name );
		}
		id_t id( void ) const {
			return ( _id );
		}
		static type_t register_type( yaal::hcore::HString const&, HHuginn* );
		static int builtin_type_count( void );
	private:
		HType( yaal::hcore::HString const&, int );
		HType( HType const& ) = delete;
		HType& operator = ( HType const& ) = delete;
		HType( HType&& ) = delete;
		HType& operator = ( HType&& ) = delete;
	};
	typedef HType::type_t type_t;
	typedef yaal::hcore::HPointer<HHuginn> ptr_t;
	class HIterable;
	typedef yaal::hcore::HPointer<HIterable> iterable_t;
	typedef yaal::hcore::HPointer<huginn::HScope> scope_t;
	class HIf;
	class HWhile;
	class HFor;
	class HSwitch;
	class HBreak;
	class HReturn;
	class HClass;
	typedef yaal::hcore::HPointer<HClass> class_t;
	class HMethod;
	class HFunction;
	class HReference;
	class HFunctionReference;
	class HValue;
	typedef yaal::hcore::HPointer<HValue> value_t;
	class HBoolean;
	class HInteger;
	class HReal;
	class HString;
	class HCharacter;
	class HNumber;
	class HList;
	typedef yaal::hcore::HPointer<HList> list_t;
	class HMap;
	class HObject;
	class HBooleanEvaluator;
	class HTernaryEvaluator;
	typedef yaal::hcore::HPointer<huginn::HExpression> expression_t;
	typedef yaal::hcore::HArray<HHuginn::expression_t> expressions_t;
	class HErrorCoordinate;
	typedef yaal::hcore::HArray<value_t> values_t;
	typedef yaal::hcore::HPointer<huginn::HFrame> frame_t;
	typedef yaal::hcore::HBoundCall<value_t ( huginn::HThread*, values_t const&, int )> function_t;
	typedef yaal::hcore::HPointer<huginn::HThread> thread_t;
	typedef yaal::hcore::HHashMap<yaal::hcore::HThread::id_t, thread_t> threads_t;
	struct TYPE {
		static type_t const NONE;
		static type_t const BOOLEAN;
		static type_t const INTEGER;
		static type_t const REAL;
		static type_t const STRING;
		static type_t const NUMBER;
		static type_t const CHARACTER;
		static type_t const LIST;
		static type_t const MAP;
		static type_t const REFERENCE;
		static type_t const FUNCTION_REFERENCE;
		static type_t const METHOD;
		static type_t const UNKNOWN;
		static type_t const NOT_BOOLEAN;
	};
	class HHuginnRuntimeException;
	typedef yaal::hcore::HResource<huginn::HSource> source_t;
	typedef yaal::hcore::HResource<huginn::OCompiler> compiler_t;
private:
	typedef yaal::hcore::HMap<yaal::hcore::HString, class_t> classes_t;
	typedef yaal::hcore::HMap<yaal::hcore::HString, function_t> functions_t;
	struct STATE {
		typedef enum {
			EMPTY,
			LOADED,
			PREPROCESSED,
			PARSED,
			COMPILED
		} state_t;
	};
	STATE::state_t _state;
	HType::id_generator_t _idGenerator;
	HType::type_dict_t _userTypeDict;
	classes_t _classes;
	functions_t _functions;
	source_t _source;
	compiler_t _compiler;
	HExecutingParser _engine;
	threads_t _threads;
	list_t _argv;
	value_t _result;
	yaal::hcore::HString _errorMessage;
	int _errorPosition;
	typedef std::atomic<bool> flag_t;
	static flag_t _grammarVerified;
	yaal::hcore::HStreamInterface::ptr_t _inputStream;
	yaal::hcore::HStreamInterface* _inputStreamRaw;
	yaal::hcore::HStreamInterface::ptr_t _outputStream;
	yaal::hcore::HStreamInterface* _outputStreamRaw;
	yaal::hcore::HStreamInterface::ptr_t _errorStream;
	yaal::hcore::HStreamInterface* _errorStreamRaw;
public:
	HHuginn( void );
	virtual ~HHuginn( void );
	/*! \brief Store source in internal buffer.
	 *
	 * \param  stream_ - stream that contains program source.
	 */
	void load( yaal::hcore::HStreamInterface& stream_ );
	/*! \brief Preprocess loaded program source.
	 */
	void preprocess( void );
	/*! \brief Parse preprocessed program source.
	 *
	 * \return True iff whole input source was parsed successfully.
	 */
	bool parse( void );
	/*! \brief Compile parsed program.
	 *
	 * \return True iff compilation finished with no errors.
	 */
	bool compile( void );
	/*! \brief Execute compiled program.
	 *
	 * \return True iff execution finished with no errors.
	 */
	bool execute( void );
	/*! \brief Dump Huginn Virtual Machine state.
	 */
	value_t call( yaal::hcore::HString const&, values_t const&, int );
	value_t result( void ) const;
	void dump_vm_state( yaal::hcore::HStreamInterface& );
	huginn::HFrame* current_frame( void );
	void create_function( executing_parser::position_t );
	void create_class( executing_parser::position_t );
	void add_argument( yaal::hcore::HString const& );
	void dump_preprocessed_source( yaal::hcore::HStreamInterface& );
	int error_position( void ) const;
	HErrorCoordinate error_coordinate( void ) const;
	HErrorCoordinate get_coordinate( int ) const;
	yaal::hcore::HString error_message( void ) const;
	void set_input_stream( yaal::hcore::HStreamInterface& );
	void set_input_stream( yaal::hcore::HStreamInterface::ptr_t );
	void set_output_stream( yaal::hcore::HStreamInterface& );
	void set_output_stream( yaal::hcore::HStreamInterface::ptr_t );
	void set_error_stream( yaal::hcore::HStreamInterface& );
	void set_error_stream( yaal::hcore::HStreamInterface::ptr_t );
	executing_parser::HRule make_engine( void );
	yaal::hcore::HStreamInterface& input_stream( void );
	yaal::hcore::HStreamInterface& output_stream( void );
	yaal::hcore::HStreamInterface& error_stream( void );
	function_t get_function( yaal::hcore::HString const& );
private:
	void commit_classes( void );
	HClass const* commit_class( yaal::hcore::HString const& );
	void register_builtins( void );
	char const* error_message( int ) const;
	friend type_t HType::register_type( yaal::hcore::HString const&, HHuginn* );
	HHuginn( HHuginn const& ) = delete;
	HHuginn& operator = ( HHuginn const& ) = delete;
};

typedef yaal::hcore::HExceptionT<HHuginn> HHuginnException;

class HHuginn::HErrorCoordinate {
public:
	typedef HErrorCoordinate this_type;
private:
	int _line;
	int _column;
public:
	HErrorCoordinate( int line_, int column_ )
		: _line( line_ ), _column( column_ ) {
	}
	int line( void ) const {
		return ( _line );
	}
	int column( void ) const {
		return ( _column );
	}
};

class HHuginn::HHuginnRuntimeException {
public:
	typedef HHuginn::HHuginnRuntimeException this_type;
private:
	yaal::hcore::HString _message;
	int _position;
public:
	HHuginnRuntimeException( yaal::hcore::HString const&, int );
	yaal::hcore::HString const& message( void ) const;
	int position( void ) const;
};

class HHuginn::HValue {
public:
	typedef HHuginn::HValue this_type;
private:
	type_t const _type;
public:
	HValue( type_t );
	virtual ~HValue( void ) {
		return;
	}
	type_t type( void ) const;
	value_t clone( void ) const;
private:
	virtual value_t do_clone( void ) const;
	HValue( HValue const& ) = delete;
	HValue& operator = ( HValue const& ) = delete;
};

class HHuginn::HReference : public HHuginn::HValue {
public:
	typedef HHuginn::HReference this_type;
	typedef HHuginn::HValue base_type;
private:
	HHuginn::value_t& _value;
public:
	HReference( HHuginn::value_t& );
	HHuginn::value_t& value( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HTernaryEvaluator : public HHuginn::HValue {
public:
	typedef HHuginn::HTernaryEvaluator this_type;
	typedef HHuginn::HValue base_type;
private:
	expression_t _condition;
	expression_t _ifTrue;
	expression_t _ifFalse;
public:
	HTernaryEvaluator( expression_t const&, expression_t const&, expression_t const& );
	value_t execute( huginn::HThread* );
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HIterable : public HHuginn::HValue {
public:
	typedef HHuginn::HIterable this_type;
	typedef HHuginn::HValue base_type;
	class HIterator;
	HIterable( type_t );
	HIterator iterator( void );
protected:
	virtual HIterator do_iterator( void ) = 0;
};

class HHuginn::HBoolean : public HHuginn::HValue {
public:
	typedef HHuginn::HBoolean this_type;
	typedef HHuginn::HValue base_type;
private:
	bool _value;
public:
	HBoolean( bool );
	bool value( void ) const;
	void to_string( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HInteger : public HHuginn::HValue {
public:
	typedef HHuginn::HInteger this_type;
	typedef HHuginn::HValue base_type;
private:
	int long long _value;
public:
	HInteger( int long long );
	int long long value( void ) const;
	void to_character( void ) const;
	void to_integer( void ) const;
	void to_number( void ) const;
	void to_real( void ) const;
	void to_string( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HReal : public HHuginn::HValue {
public:
	typedef HHuginn::HReal this_type;
	typedef HHuginn::HValue base_type;
private:
	double long _value;
public:
	HReal( double long );
	double long value( void ) const;
	void to_integer( void ) const;
	void to_number( void ) const;
	void to_real( void ) const;
	void to_string( void ) const;
	/* There is no direct convertion to character. */
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HString : public HHuginn::HIterable {
public:
	typedef HHuginn::HString this_type;
	typedef HHuginn::HValue base_type;
private:
	yaal::hcore::HString _value;
public:
	HString( yaal::hcore::HString const& );
	yaal::hcore::HString const& value( void ) const;
	yaal::hcore::HString& value( void );
	void to_integer( void ) const;
	void to_number( void ) const;
	void to_real( void ) const;
	void to_string( void ) const;
protected:
	virtual HIterator do_iterator( void ) override;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HCharacter : public HHuginn::HValue {
public:
	typedef HHuginn::HCharacter this_type;
	typedef HHuginn::HValue base_type;
private:
	char _value;
public:
	HCharacter( char );
	char value( void ) const;
	void to_character( void ) const;
	void to_integer( void ) const;
	void to_string( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HNumber : public HHuginn::HValue {
public:
	typedef HHuginn::HNumber this_type;
	typedef HHuginn::HValue base_type;
private:
	yaal::hcore::HNumber _value;
public:
	HNumber( yaal::hcore::HNumber const& );
	yaal::hcore::HNumber const& value( void ) const;
	void to_integer( void ) const;
	void to_number( void ) const;
	void to_real( void ) const;
	void to_string( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HList : public HHuginn::HIterable {
public:
	typedef HHuginn::HList this_type;
	typedef HHuginn::HIterable base_type;
private:
	typedef yaal::hcore::HDeque<HHuginn::value_t> values_t;
	values_t _data;
public:
	HList( void );
	HList( values_t const& );
	void push_back( value_t const& );
	int long size( void ) const;
	value_t get( int long long );
	value_t get_ref( int long long );
protected:
	virtual HIterator do_iterator( void ) override;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HMap : public HHuginn::HIterable {
public:
	typedef HHuginn::HMap this_type;
	typedef HHuginn::HIterable base_type;
	typedef bool (*cmp_t)( HHuginn::value_t const&, HHuginn::value_t const& );
	typedef yaal::hcore::HMap<HHuginn::value_t, HHuginn::value_t, cmp_t> values_t;
private:
	values_t _data;
	type_t _keyType;
public:
	HMap( void );
	HMap( values_t const&, type_t );
	int long size( void ) const;
	value_t get( HHuginn::value_t const&, int );
	value_t get_ref( HHuginn::value_t const&, int );
	void insert( HHuginn::value_t const&, HHuginn::value_t const&, int );
protected:
	virtual HIterator do_iterator( void ) override;
private:
	void verify_key_type( HHuginn::type_t, int );
	HMap( HMap const& ) = delete;
	HMap& operator = ( HMap const& ) = delete;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HFunctionReference : public HHuginn::HValue {
	typedef HHuginn::HFunctionReference this_type;
	typedef HHuginn::HValue base_type;
private:
	yaal::hcore::HString _name;
	HHuginn::function_t _function;
public:
	HFunctionReference( yaal::hcore::HString const&, HHuginn::function_t const& );
	yaal::hcore::HString const& name( void ) const;
	HHuginn::function_t const& function( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HClass {
public:
	typedef HHuginn::HClass this_type;
	typedef yaal::hcore::HArray<yaal::hcore::HString> field_names_t;
	typedef yaal::hcore::HHashMap<yaal::hcore::HString, int> field_indexes_t;
	class HMethod;
private:
	type_t _type;
	HClass const* _super;
	yaal::hcore::HString _name;
	field_names_t _fieldNames;
	field_indexes_t _fieldIndexes;
	values_t _fieldDefinitions;
public:
	HClass( type_t, HClass const*, yaal::hcore::HString const&, field_names_t const&, values_t const& );
	HClass const* super( void ) const;
	yaal::hcore::HString const& name( void ) const;
	type_t type( void ) const;
	field_names_t const& field_names( void ) const;
	value_t create_instance( huginn::HThread*, values_t const&, int ) const;
private:
	HClass( HClass const& ) = delete;
	HClass& operator = ( HClass const& ) = delete;
};

class HHuginn::HClass::HMethod : public HHuginn::HValue {
	typedef HHuginn::HClass::HMethod this_type;
	typedef HHuginn::HValue base_type;
private:
	HHuginn::function_t _function;
public:
	HMethod( HHuginn::function_t const& );
	HHuginn::function_t const& function( void ) const;
private:
	virtual value_t do_clone( void ) const override;
};

class HHuginn::HObject : public HHuginn::HValue {
public:
	typedef HHuginn::HObject this_type;
	typedef HHuginn::HValue base_type;
	typedef yaal::hcore::HArray<value_t> fields_t;
private:
	HClass const* _class;
	fields_t _fields;
public:
	HObject( HClass const*, fields_t const& );
private:
	HObject( HObject const& ) = delete;
	HObject& operator = ( HObject const& ) = delete;
private:
	virtual value_t do_clone( void ) const override;
};

extern HHuginn::value_t _none_;

}

}

#endif /* #ifndef YAAL_TOOLS_HHUGINN_HXX_INCLUDED */

