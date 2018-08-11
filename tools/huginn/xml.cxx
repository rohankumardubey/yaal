/* Read yaal/LICENSE.md file for copyright and licensing information. */

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "tools/hhuginn.hxx"
#include "runtime.hxx"
#include "tools/hxml.hxx"
#include "iterator.hxx"
#include "helper.hxx"
#include "thread.hxx"
#include "stream.hxx"
#include "packagefactory.hxx"
#include "objectfactory.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::huginn;

namespace yaal {

namespace tools {

namespace huginn {

namespace xml {

typedef yaal::hcore::HPointer<yaal::tools::HXml> xml_t;

namespace {

HHuginn::class_t make_node_class( HRuntime* runtime_, HHuginn::HClass const* origin_, char const* name_, char const* doc_ ) {
	M_PROLOG
	HHuginn::class_t c( runtime_->create_class( name_, doc_, HHuginn::ACCESS::PRIVATE ) );
	c->set_origin( origin_ );
	c->redefine( runtime_->object_factory()->string_class(), {} );
	runtime_->huginn()->register_class( c, HHuginn::VISIBILITY::PACKAGE );
	return ( c );
	M_EPILOG
}

}

class HElement : public HHuginn::HInvalidatingIterable {
public:
	typedef yaal::hcore::HPointer<yaal::tools::HXml> xml_t;
	class HTrackerProxy : public HHuginn::HNotifableReference {
		HElement& _element;
	public:
		HTrackerProxy( HHuginn::HReferenceTracker* owner_, HElement& element_ )
			: HHuginn::HNotifableReference( owner_ )
			, _element( element_ ) {
		}
	protected:
		virtual void do_invalidate( void ) override {
			_element.do_invalidate();
		}
		virtual void const* do_id( void ) const override {
			return ( _element.do_id() );
		}
	};
private:
	HHuginn::value_t _doc;
	yaal::tools::HXml::HNodeProxy _node;
	HTrackerProxy _tracker;
public:
	HElement( HHuginn::HClass const*, HHuginn::value_t&, yaal::tools::HXml::HNodeProxy const& );
	virtual ~HElement( void );
	HHuginn::value_t name( HThread* thread_, int position_ ) const {
		ensure_valid( thread_, position_ );
		return ( !! _node ? thread_->object_factory().create_string( _node.get_name() ) : thread_->runtime().none_value() );
	}
	HHuginn::value_t nth_child( HThread*, int, int );
	HHuginn::value_t append( HThread*, HHuginn::values_t&, int );
	void remove( HThread*, HElement*, int );
	void remove_nth( HThread*, int, int );
	HHuginn::value_t parent( HThread*, int );
	HHuginn::value_t document( HThread*, int );
	yaal::tools::HXml::HNodeProxy const& node( void ) const {
		return ( _node );
	}
protected:
	virtual iterator_t do_iterator( HThread*, int ) override;
	virtual int long do_size( huginn::HThread* thread_, int position_ ) const override {
		ensure_valid( thread_, position_ );
		return ( distance( _node.begin(), _node.end() ) );
	}
	void do_invalidate( void );
	void const* do_id( void ) const {
		return ( _node.node_id() );
	}
private:
	void ensure_valid( HThread* thread_, int position_ ) const {
		if ( ! _node ) {
			throw HHuginn::HHuginnRuntimeException(
				"This `XML.Element` reference is no longer a valid node.",
				thread_->current_frame()->file_id(),
				position_
			);
		}
		return;
	}
	void do_remove( yaal::tools::HXml::HIterator& );
	HHuginn::value_t do_clone( huginn::HThread* thread_, HHuginn::value_t*, int position_ ) const override {
		M_PROLOG
		throw HHuginn::HHuginnRuntimeException(
			"Copy semantics is not supported on `XML.Element`s.",
			thread_->current_frame()->file_id(),
			position_
		);
		M_EPILOG
	}
private:
	friend class HTrackerProxy;
};

class HDocumentClass;

class HElementClass : public HHuginn::HClass {
public:
	typedef HElementClass this_type;
	typedef HHuginn::HClass base_type;
private:
	HHuginn::HClass const* _documentClass;
	HHuginn::class_t const& _exceptionClass;
public:
	HElementClass(
		HRuntime* runtime_,
		HHuginn::type_id_t typeId_,
		HHuginn::HClass const* documentClass_,
		HHuginn::class_t const& exceptionClass_,
		HHuginn::HClass const* origin_
	) : HHuginn::HClass(
			runtime_,
			typeId_,
			runtime_->identifier_id( "Element" ),
			"The `Element` class represents a single `XML` `Document` `Element` reference.",
			HHuginn::ACCESS::PRIVATE
		)
		, _documentClass( documentClass_ )
		, _exceptionClass( exceptionClass_ ) {
		HHuginn::field_definitions_t fd{
			{ "name",       runtime_->create_method( &HElementClass::element_name ), "get the name of this `Element`" },
			{ "parent",     runtime_->create_method( &HElementClass::parent ),       "get parent node of this `Element`" },
			{ "subscript",  runtime_->create_method( &HElementClass::subscript ),    "( *index* ) get *index*'th child node of this `Element`" },
			{ "document",   runtime_->create_method( &HElementClass::document ),     "get an `XML` `Document` to which this `Element` belongs to" },
			{ "append",     runtime_->create_method( &HElementClass::append ),       "( *type*, *value* ) - append new node of type *type* and value *value* at the end of this `Element`" },
			{ "remove",     runtime_->create_method( &HElementClass::remove ),       "( *node* ) - remove given `Element` child node of this `Element`" },
			{ "remove_nth", runtime_->create_method( &HElementClass::remove_nth ),   "( *nth* ) - remove *nth* child node of this `Element`" }
		};
		set_origin( origin_ );
		redefine( nullptr, fd );
		return;
	}
	static HHuginn::value_t element_name( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Element.name", values_, 0, 0, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		return ( element.name( thread_, position_ ) );
		M_EPILOG
	}
	static HHuginn::value_t parent( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Element.parent", values_, 0, 0, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		return ( element.parent( thread_, position_ ) );
		M_EPILOG
	}
	static HHuginn::value_t subscript( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Element.subscript", values_, { HHuginn::TYPE::INTEGER }, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		return ( element.nth_child( thread_, safe_int::cast<int>( get_integer( values_[0] ) ), position_ ) );
		M_EPILOG
	}
	static HHuginn::value_t document( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Element.document", values_, 0, 0, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		return ( element.document( thread_, position_ ) );
		M_EPILOG
	}
	static HHuginn::value_t append( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Element.append", values_, { HHuginn::TYPE::FUNCTION_REFERENCE, HHuginn::TYPE::STRING }, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		return ( element.append( thread_, values_, position_ ) );
		M_EPILOG
	}
	static HHuginn::value_t remove( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		verify_signature_by_class( "Element.remove", values_, { element.get_class() }, thread_, position_ );
		element.remove( thread_, static_cast<HElement*>( values_[0].raw() ), position_ );
		return ( *object_ );
		M_EPILOG
	}
	static HHuginn::value_t remove_nth( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "Element.remove_nth", values_, { HHuginn::TYPE::INTEGER }, thread_, position_ );
		HElement& element( *static_cast<HElement*>( object_->raw() ) );
		element.remove_nth( thread_, safe_int::cast<int>( get_integer( values_[0] ) ), position_ );
		return ( *object_ );
		M_EPILOG
	}
	HDocumentClass const* document_class( void ) const;
	HHuginn::HClass const* exception_class( void ) const {
		return ( _exceptionClass.raw() );
	}
	static HHuginn::class_t get_class( HRuntime* runtime_, HHuginn::HClass const* documentClass_, HHuginn::class_t const& exceptionClass_ , HHuginn::HClass const* origin_ ) {
		M_PROLOG
		HHuginn::class_t c(
			runtime_->create_class(
				HRuntime::class_constructor_t(
					[&runtime_, &documentClass_, &exceptionClass_, &origin_] ( HHuginn::type_id_t typeId_ ) -> HHuginn::class_t {
						return (
							make_pointer<HElementClass>(
								runtime_,
								typeId_,
								documentClass_,
								exceptionClass_,
								origin_
							)
						);
					}
				)
			)
		);
		runtime_->huginn()->register_class( c, HHuginn::VISIBILITY::PACKAGE );
		return ( c );
		M_EPILOG
	}
private:
	HElementClass( HElementClass const& ) = delete;
	HElementClass& operator = ( HElementClass const& ) = delete;
};

class HElementIterator : public HNotifableIterator {
	HHuginn::HClass const* _class;
	HHuginn::value_t _doc;
	yaal::tools::HXml::HNodeProxy _node;
	yaal::tools::HXml::HIterator _it;
public:
	HElementIterator( HHuginn::HClass const* class_, HHuginn::value_t const& doc_, HElement* element_ )
		: HNotifableIterator( element_ )
		, _class( class_ )
		, _doc( doc_ )
		, _node( element_->node() )
		, _it( _node.begin() ) {
		return;
	}
protected:
	virtual HHuginn::value_t do_value( HThread* thread_, int ) override;
	virtual bool do_is_valid( huginn::HThread*, int ) override {
		return ( _it != _node.end() );
	}
	virtual void do_next( HThread*, int ) override {
		if ( _skip == 0 ) {
			++ _it;
		} else {
			-- _skip;
		}
	}
	virtual void do_invalidate( void ) override {
		_it = _node.end();
	}
	virtual void do_skip( void ) override {
		++ _it;
		++ _skip;
	}
	virtual void const* do_id( void ) const override {
		return ( _it.node_id() );
	}
private:
	HElementIterator( HElementIterator const& ) = delete;
	HElementIterator& operator = ( HElementIterator const& ) = delete;
};

HHuginn::HIterable::iterator_t HElement::do_iterator( HThread* thread_, int position_ ) {
	ensure_valid( thread_, position_ );
	return ( make_pointer<HElementIterator>( get_class(), _doc, this ) );
}

class HDocument : public HHuginn::HValue, public HHuginn::HReferenceTracker {
public:
	typedef yaal::hcore::HHashMap<void const*, HHuginn::value_ref_t> element_refs_t;
private:
	xml_t _xml;
	element_refs_t _elementRefs;
public:
	HDocument( HHuginn::HClass const* class_, yaal::hcore::HString const& name_ )
		: HValue( class_ )
		, HReferenceTracker()
		, _xml( make_pointer<yaal::tools::HXml>() )
		, _elementRefs() {
		_xml->create_root( name_ );
		return;
	}
	HDocument( HHuginn::HClass const* class_, xml_t xml_ )
		: HValue( class_ )
		, _xml( xml_ )
		, _elementRefs() {
		return;
	}
	HHuginn::value_t create_element( HObjectFactory&, HHuginn::value_t&, yaal::tools::HXml::HNodeProxy const& );
	void destroy_element( yaal::tools::HXml::HNodeProxy const& node_ ) {
		_elementRefs.erase( node_.node_id() );
	}
	HHuginn::value_t root( HThread* thread_, HHuginn::value_t& object_ ) {
		return ( create_element( thread_->object_factory(), object_, _xml->get_root() ) );
	}
	void save( HStream& stream_ ) {
		M_PROLOG
		_xml->save( stream_.raw(), yaal::tools::HXml::GENERATOR::INDENT );
		return;
		M_EPILOG
	}
	HHuginn::value_t do_clone( huginn::HThread* thread_, HHuginn::value_t*, int ) const override {
		M_PROLOG
		return ( thread_->object_factory().create<HDocument>( get_class(), make_pointer<yaal::tools::HXml>( *_xml ) ) );
		M_EPILOG
	}
	xml_t const& xml( void ) const {
		return ( _xml );
	}
};

class HDocumentClass : public HHuginn::HClass {
public:
	typedef HDocumentClass this_type;
	typedef HHuginn::HClass base_type;
private:
	HHuginn::class_t _elementClass;
	HHuginn::class_t _textClass;
	HHuginn::class_t _commentClass;
	HHuginn::class_t _entityClass;
	HHuginn::class_t const& _streamClass;
	HHuginn::class_t const& _exceptionClass;
public:
	HDocumentClass(
		HRuntime* runtime_,
		HHuginn::type_id_t typeId_,
		HHuginn::class_t const& streamClass_,
		HHuginn::class_t const& exceptionClass_,
		HHuginn::HClass* origin_
	) : HHuginn::HClass(
			runtime_,
			typeId_,
			runtime_->identifier_id( "Document" ),
			"The `Document` class represents a single `XML` `Document` instance.",
			HHuginn::ACCESS::PRIVATE
		)
		, _elementClass(
			add_class_as_member(
				origin_,
				HElementClass::get_class( runtime_, this, exceptionClass_, origin_ ),
				"An `XML` `Element` node type."
			)
		)
		, _textClass(
			add_class_as_member(
				origin_,
				make_node_class( runtime_, origin_, "Text", "The `Text` class represents TEXT node in an `XML` `Document`." ),
				"An `XML` `TEXT` node type."
			)
		)
		, _commentClass(
			add_class_as_member(
				origin_,
				make_node_class( runtime_, origin_, "Comment", "The `Comment` class represents a comment string in an `XML` `Document`." ),
				"An `XML` `Comment` node type."
			)
		)
		, _entityClass(
			add_class_as_member(
				origin_,
				make_node_class( runtime_, origin_, "Entity", "The `Entity` class represents an `Entity` reference in an `XML` `Document`." ),
				"An `XML` `Entity` node type."
			)
		)
		, _streamClass( streamClass_ )
		, _exceptionClass( exceptionClass_ ) {
		HHuginn::field_definitions_t fd{
			{ "root", runtime_->create_method( &HDocumentClass::root ), "get root element of an `XML` `Document`" },
			{ "save", runtime_->create_method( &HDocumentClass::save ), "( *stream* ) - save this `Document` to given *stream*" }
		};
		set_origin( origin_ );
		redefine( nullptr, fd );
		return;
	}
	static HHuginn::value_t root( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_arg_count( "Document.root", values_, 0, 0, thread_, position_ );
		HDocument& document( *static_cast<HDocument*>( object_->raw() ) );
		return ( document.root( thread_, *object_ ) );
		M_EPILOG
	}
	static HHuginn::value_t save( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		HDocument& document( *static_cast<HDocument*>( object_->raw() ) );
		HDocumentClass const& documentClass( *static_cast<HDocumentClass const*>( document.get_class() ) );
		verify_signature_by_class( "XML.save", values_, { documentClass._streamClass.raw() }, thread_, position_ );
		document.save( *static_cast<HStream*>( values_[0].raw() ) );
		return ( *object_ );
		M_EPILOG
	}
	HHuginn::HClass const* element_class( void ) const {
		return ( _elementClass.raw() );
	}
	HHuginn::HClass const* text_class( void ) const {
		return ( _textClass.raw() );
	}
	HHuginn::HClass const* comment_class( void ) const {
		return ( _commentClass.raw() );
	}
	HHuginn::HClass const* entity_class( void ) const {
		return ( _entityClass.raw() );
	}
	HHuginn::HClass const* exception_class( void ) const {
		return ( _exceptionClass.raw() );
	}
	static HHuginn::class_t get_class( HRuntime* runtime_, HHuginn::class_t const& streamClass_, HHuginn::class_t const& exceptionClass_, HHuginn::HClass* origin_ ) {
		M_PROLOG
		HHuginn::class_t c(
			runtime_->create_class(
				HRuntime::class_constructor_t(
					[&runtime_, &streamClass_, &exceptionClass_, &origin_] ( HHuginn::type_id_t typeId_ ) -> HHuginn::class_t {
						return (
							make_pointer<HDocumentClass>(
								runtime_,
								typeId_,
								streamClass_,
								exceptionClass_,
								origin_
							)
						);
					}
				)
			)
		);
		runtime_->huginn()->register_class( c, HHuginn::VISIBILITY::PACKAGE );
		return ( c );
		M_EPILOG
	}
private:
	HDocumentClass( HDocumentClass const& ) = delete;
	HDocumentClass& operator = ( HDocumentClass const& ) = delete;
};

HDocumentClass const* HElementClass::document_class( void ) const {
	return ( static_cast<HDocumentClass const *>( _documentClass ) );
}

namespace {

HHuginn::value_t make_node_ref( HObjectFactory& of_, HDocumentClass const* dc_, HHuginn::value_t& doc_, yaal::tools::HXml::HNodeProxy const& n_ ) {
	M_PROLOG
	HHuginn::value_t v;
	switch ( n_.get_type() ) {
		case ( yaal::tools::HXml::HNode::TYPE::ELEMENT ): {
			v = static_cast<HDocument*>( doc_.raw() )->create_element( of_, doc_, n_ );
		} break;
		case ( yaal::tools::HXml::HNode::TYPE::CONTENT ): {
			v = of_.create<HHuginn::HString>( dc_->text_class(), n_.get_value() );
		} break;
		case ( yaal::tools::HXml::HNode::TYPE::COMMENT ): {
			v = of_.create<HHuginn::HString>( dc_->comment_class(), n_.get_value() );
		} break;
		case ( yaal::tools::HXml::HNode::TYPE::ENTITY ): {
			v = of_.create<HHuginn::HString>( dc_->entity_class(), n_.get_value() );
		} break;
	}
	return ( v );
	M_EPILOG
}

void get_all_child_nodes( HHuginn::HReferenceTracker::ids_t& ids_, yaal::tools::HXml::HConstNodeProxy const& node_ ) {
	M_PROLOG
	for ( yaal::tools::HXml::HConstNodeProxy const& n : node_ ) {
		ids_.push_back( n.node_id() );
		if ( n.get_type() == yaal::tools::HXml::HNode::TYPE::ELEMENT ) {
			get_all_child_nodes( ids_, n );
		}
	}
	return;
	M_EPILOG
}

}

HHuginn::value_t HDocument::create_element( HObjectFactory& of_, HHuginn::value_t& doc_, yaal::tools::HXml::HNodeProxy const& node_ ) {
	M_PROLOG
	HDocumentClass const* dc( static_cast<HDocumentClass const*>( get_class() ) );
	element_refs_t::iterator it( _elementRefs.find( node_.node_id() ) );
	HHuginn::value_t v;
	if ( it != _elementRefs.end() ) {
		v = HHuginn::value_t( it->second );
		M_ASSERT( !! v );
	} else {
		v = of_.create<HElement>( dc->element_class(), doc_, node_ );
		_elementRefs.insert( make_pair( node_.node_id(), HHuginn::value_ref_t( v ) ) );
	}
	return ( v );
	M_EPILOG
}

HElement::HElement( HHuginn::HClass const* class_, HHuginn::value_t& doc_, yaal::tools::HXml::HNodeProxy const& node_ )
	: HInvalidatingIterable( class_ )
	, _doc( doc_ )
	, _node( node_ )
	, _tracker( static_cast<HDocument*>( doc_.raw() ), *this ) {
	return;
}

HElement::~HElement( void ) {
	M_PROLOG
	do_invalidate();
	return;
	M_DESTRUCTOR_EPILOG
}

void HElement::do_invalidate( void ) {
	M_PROLOG
	if ( !! _node ) {
		static_cast<HDocument*>( _doc.raw() )->destroy_element( _node );
	}
	_node = yaal::tools::HXml::HNodeProxy();
	return;
	M_EPILOG
}

HHuginn::value_t HElement::append( HThread* thread_, HHuginn::values_t& values_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	if ( ! _node ) {
		throw HHuginn::HHuginnRuntimeException(
			"Referenced `XML.Element` was removed.",
			thread_->current_frame()->file_id(),
			position_
		);
	}
	HElementClass const* ec( static_cast<HElementClass const*>( get_class() ) );
	HDocumentClass const* dc( ec->document_class() );
	HHuginn::HFunctionReference& fr( *static_cast<HHuginn::HFunctionReference*>( values_[0].raw() ) );
	void const* id( fr.function().id() );
	HRuntime& r( thread_->runtime() );
	HString const& s( get_string( values_[1] ) );
	yaal::tools::HXml::HNode::TYPE t( yaal::tools::HXml::HNode::TYPE::ELEMENT );
	if ( id == dc->element_class() ) {
		/* node type already set */
	} else if ( id == dc->text_class() ) {
		t = yaal::tools::HXml::HNode::TYPE::CONTENT;
	} else if ( id == dc->comment_class() ) {
		t = yaal::tools::HXml::HNode::TYPE::COMMENT;
	} else if ( id == dc->entity_class() ) {
		t = yaal::tools::HXml::HNode::TYPE::ENTITY;
	} else {
		throw HHuginn::HHuginnRuntimeException(
			"Node type must be one of: `XML.Element`, `XML.Text`, `XML.Comment` or `XML.Entity`, not `"_ys.append( r.identifier_name( fr.identifier_id() ) ).append( "`." ),
			thread_->current_frame()->file_id(),
			position_
		);
	}
	return ( make_node_ref( *r.object_factory(), dc, _doc, *_node.add_node( t, s ) ) );
	M_EPILOG
}

HHuginn::value_t HElement::parent( HThread* thread_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	yaal::tools::HXml::HNodeProxy n( _node.get_parent() );
	return (
		!! n
			? static_cast<HDocument*>( _doc.raw() )->create_element( thread_->object_factory(), _doc, n )
			: thread_->runtime().none_value()
	);
	M_EPILOG
}

HHuginn::value_t HElement::nth_child( HThread* thread_, int index_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	HElementClass const* ec( static_cast<HElementClass const*>( get_class() ) );
	HDocumentClass const* dc( ec->document_class() );
	if ( ( index_ < 0 ) || ( index_ >= _node.child_count() ) ) {
		throw HHuginn::HHuginnRuntimeException(
			"Invalid `XML.Element` child index: "_ys.append( index_ ),
			thread_->current_frame()->file_id(),
			position_
		);
	}
	yaal::tools::HXml::HIterator it( _node.begin() );
	advance( it, index_ );
	return ( make_node_ref( thread_->object_factory(), dc, _doc, *it ) );
	M_EPILOG
}

void HElement::remove( HThread* thread_, HElement* element_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	yaal::tools::HXml::HIterator it(
		yaal::find_if(
			_node.begin(),
			_node.end(),
			[&element_]( yaal::tools::HXml::HConstNodeProxy const& node_ ) {
				return ( node_ == element_->_node );
			}
		)
	);
	if ( it == _node.end() ) {
		throw HHuginn::HHuginnRuntimeException(
			"Given `XML.Element` in not a child node of this `Element`. ",
			thread_->current_frame()->file_id(),
			position_
		);
	}
	do_remove( it );
	return;
	M_EPILOG
}

void HElement::remove_nth( HThread* thread_, int nth_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	if ( ( nth_ < 0 ) || ( nth_ >= _node.child_count() ) ) {
		throw HHuginn::HHuginnRuntimeException(
			"Invalid `XML.Element` child index: "_ys.append( nth_ ),
			thread_->current_frame()->file_id(),
			position_
		);
	}
	yaal::tools::HXml::HIterator it( _node.begin() );
	advance( it, nth_ );
	do_remove( it );
	return;
	M_EPILOG
}

void HElement::do_remove( yaal::tools::HXml::HIterator& it_ ) {
	M_PROLOG
	HHuginn::HReferenceTracker::ids_t ids;
	get_all_child_nodes( ids, *it_ );
	ids.push_back( (*it_).node_id() );
	static_cast<HDocument*>( _doc.raw() )->invalidate( ids );
	skip( it_.node_id() );
	_node.remove_node( it_ );
	return;
	M_EPILOG
}

HHuginn::value_t HElement::document( HThread* thread_, int position_ ) {
	M_PROLOG
	ensure_valid( thread_, position_ );
	return ( _doc );
	M_EPILOG
}

HHuginn::value_t HElementIterator::do_value( HThread* thread_, int ) {
	return ( make_node_ref( thread_->object_factory(), static_cast<HElementClass const*>( _class )->document_class(), _doc, *_it ) );
}

}

class HXml : public HHuginn::HValue {
	HHuginn::class_t _documentClass;
	HHuginn::class_t _streamClass;
	HHuginn::class_t _exceptionClass;
public:
	HXml( HHuginn::HClass* class_ )
		: HValue( class_ )
		, _documentClass( xml::HDocumentClass::get_class( class_->runtime(), _streamClass, _exceptionClass, class_ ) )
		, _streamClass( HStream::get_class( class_->runtime() ) )
		, _exceptionClass( class_exception( class_ ) ) {
		return;
	}
	static HHuginn::value_t load( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		HXml& xml( *static_cast<HXml*>( object_->raw() ) );
		verify_signature_by_class( "XML.load", values_, { xml._streamClass.raw() }, thread_, position_ );
		HHuginn::value_t v( thread_->runtime().none_value() );
		HXml& XML( *static_cast<HXml*>( object_->raw() ) );
		try {
			xml::xml_t doc( make_pointer<yaal::tools::HXml>() );
			HStream& stream( *static_cast<HStream*>( values_[0].raw() ) );
			doc->load( stream.raw() );
			v = thread_->object_factory().create<xml::HDocument>( XML._documentClass.raw(), doc );
		} catch ( HException const& e ) {
			thread_->raise( XML._exceptionClass.raw(), e.what(), position_ );
		}
		return ( v );
		M_EPILOG
	}
	static HHuginn::value_t create( huginn::HThread* thread_, HHuginn::value_t* object_, HHuginn::values_t& values_, int position_ ) {
		M_PROLOG
		verify_signature( "XML.create", values_, { HHuginn::TYPE::STRING }, thread_, position_ );
		HXml& XML( *static_cast<HXml*>( object_->raw() ) );
		return ( thread_->object_factory().create<xml::HDocument>( XML._documentClass.raw(), get_string( values_[0] ) ) );
		M_EPILOG
	}
};

namespace package_factory {

class HXmlCreator : public HPackageCreatorInterface {
protected:
	virtual HHuginn::value_t do_new_instance( HRuntime* );
} xmlCreator;

HHuginn::value_t HXmlCreator::do_new_instance( HRuntime* runtime_ ) {
	M_PROLOG
	HHuginn::class_t c(
		runtime_->create_class(
			"XML",
			"The `XML` package provides various text manipulation algorithms.",
			HHuginn::ACCESS::PRIVATE
		)
	);
	HHuginn::field_definitions_t fd{
		{ "load",   runtime_->create_method( &HXml::load ),   "( *stream* ) - load an `XML` document from given *stream*" },
		{ "create", runtime_->create_method( &HXml::create ), "( *name* ) - create new `XML` document with given root node *name*" }
	};
	c->redefine( nullptr, fd );
	runtime_->huginn()->register_class( c );
	return ( runtime_->object_factory()->create<HXml>( c.raw() ) );
	M_EPILOG
}

namespace {

bool registrar( void ) {
	M_PROLOG
	bool volatile failed = false;
	HPackageFactory& factory = HPackageFactory::get_instance();
	factory.register_package_creator( "XML", &xmlCreator );
	return ( failed );
	M_EPILOG
}

bool volatile registered = registrar();

}

}

}

}

}

