/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hdatalistwidget.cxx - this file is integral part of `yaal' project.

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

#include <cstring>

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "hdatalistwidget.hxx"
#include "hdatawindow.hxx"
#include "hdataprocess.hxx"
#include "hcore/hset.hxx"
#include "hcore/hlog.hxx"
#include "tools/stringalgo.hxx"

using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::hconsole;
using namespace yaal::hconsole::list_widget_helper;
using namespace yaal::dbwrapper;

namespace yaal {

namespace hdata {

HDataListWidget::HDataColumnInfo::HDataColumnInfo(
		yaal::hcore::HString const& columnName_,
		int width_,
		BITS::ALIGN::align_t const& align_,
		type_id_t type_,
		hcore::HString const& format_,
		HWidget* associatedWidget_,
		HDictionary::ptr_t const& dict_ )
	: HColumnInfo( columnName_, width_, align_, type_, format_, associatedWidget_ ),
	_dict( dict_ ) {
	return;
}

HDataListWidget::HDataListWidget(
		HDataWindow* window_, int row_, int column_, int height_,
		int width_, yaal::hcore::HString const& title_, HWidgetAttributesInterface const& attr_ )
	: HWidget( window_, row_, column_, height_, width_, title_ ),
	HSearchableWidget( HSearchableWidgetAttributes().searchable( true ) ),
	HListWidget( NULL, 0, 0, 0, 0, HString(), attr_ ),
	HDataWidget(),
	_table(),
	_columns(),
	_filterColumn(),
	_idColumn(),
	_sort(),
	_dataModel( _model ) {
	attr_.apply( *this );
	M_PROLOG
	return;
	M_EPILOG
}

HDataListWidget::~HDataListWidget ( void ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HDataListWidget::make_crud( int long id_ ) {
	M_PROLOG
	HDataWindow* parent( dynamic_cast<HDataWindow*>( _window ) );
	M_ASSERT( parent );
	if ( _idColumn.is_empty() ) {
		_idColumn = parent->id_column_name();
	}
	if ( ! _crud ) {
		HDataProcess* proc( dynamic_cast<HDataProcess*>( _window->get_tui() ) );
		_crud = make_pointer<HCRUDDescriptor>( proc->data_base() );
	}
	if ( ! ( _table.is_empty() || _columns.is_empty() || _idColumn.is_empty() ) ) {
		_crud->set_table( _table );
		_crud->set_columns( _columns );
		if ( ! _filterColumn.is_empty() ) {
			HString filter( _filterColumn );
			filter.append( " = " ).append( to_string( id_ ) );
			_crud->set_filter( filter );
		}
		_crud->set_sort( _sort );
	}
	return;
	M_EPILOG
}

void HDataListWidget::do_load( int long id_ ) {
	M_PROLOG
	make_crud( id_ );
	HDataWindow* parent( dynamic_cast<HDataWindow*>( _window ) );
	M_ASSERT( parent );
	HRecordSet::ptr_t rs( _viewQuery.is_empty() ? _crud->execute( HCRUDDescriptor::MODE::SELECT ) : _crud->execute( _viewQuery ) );
	int idColNo( -1 );
	int const colCount = rs->get_field_count();
	for ( int i = 0; i < colCount; ++ i ) {
		if (  rs->get_column_name( i ) == _idColumn ) {
			idColNo = i;
			break;
		}
	}
	int columnCount( static_cast<int>( _header.size() ) );
	parent->status_bar()->init_progress( 0., "Collecting ..." );
	HAsIsValueListModel<>::data_ptr_t data( _dataModel->get_data() );
	HAsIsValueListModel<>::data_t::iterator it( data->begin() );
	int ctr( 0 );
	int size( static_cast<int>( _dataModel->size() ) );
	HItem<> item( columnCount );
	for ( HRecordSet::iterator row( rs->begin() ), end( rs->end() ); row != end; ++ row ) {
		for ( int i( 0 ); i < columnCount; ++ i ) {
			HDataListWidget::HDataColumnInfo* ci( static_cast<HDataListWidget::HDataColumnInfo*>( _header[i].get() ) );
			if ( !! ci->_dict ) {
				int id( lexical_cast<int>( *row[i] ) );
				item[i].set_integer( id );
				item[i].set_string( (*(ci->_dict))[id] );
			} else if ( ci->type() == yaal::TYPE::HTIME ) {
				item[ i ].set_time( HTime( row[i] ? *row[ i ] : "", _iso8601DateFormat_ ) );
			} else {
				item[ i ].set_string( row[i] ? *row[ i ] : "" );
			}
		}
		if ( idColNo >= 0 ) {
			item._id = lexical_cast<int>( *row[ idColNo ] );
		}
		parent->status_bar()->update_progress();
		if ( it != data->end() ) {
			(*it) = item;
			++ it;
		} else
			data->push_back( item );
		++ ctr;
	} while ( ctr ++ < size )
		_dataModel->remove_tail();
	reset();
	schedule_repaint();
	return;
	M_EPILOG
}

void HDataListWidget::do_save( int long id_ ) {
	M_PROLOG
	make_crud( id_ );
	_crud->set_columns( _idColumn );
	HRecordSet::ptr_t rs( _crud->execute( HCRUDDescriptor::MODE::SELECT ) );
	typedef HSet<int> ids_t;
	ids_t ids;
	for ( HRecordSet::values_t const& values : *rs ) {
		ids.insert( lexical_cast<int>( *(values[0]) ) );
	}
	rs.reset();
	HCRUDDescriptor::field_names_t columnNames( string::split<HCRUDDescriptor::field_names_t>( _columns, "," ) );
	_crud->set_columns( columnNames );
	HAsIsValueListModel<>::data_ptr_t data( _dataModel->get_data() );
	M_ENSURE( columnNames.back().trim() == _idColumn );
	columnNames.pop_back(); /* remove `id' column */
	int columns( static_cast<int>( columnNames.get_size() ) );
	HString filter;
	for ( HItem<> const& item : *data ) {
		if ( item._id > 0 ) {
			filter.assign( _idColumn ).append( " = " ).append( to_string( item._id ) );
			_crud->set_filter( filter );
			for ( int i( 0 ); i < columns; ++ i ) {
				sync( (*_crud)[i], item[i], _header[i]->type() );
			}
			_crud->execute( HCRUDDescriptor::MODE::UPDATE );
			M_ENSURE( ids.erase( static_cast<int>( item._id ) ) );
		}
	}
	columnNames.emplace_back( _filterColumn );
	_crud->set_columns( yaal::move( columnNames ) );
	for ( HItem<> const& item : *data ) {
		if ( item._id <= 0 ) {
			for ( int i( 0 ); i < columns; ++ i ) {
				sync( (*_crud)[i], item[i], _header[i]->type() );
			}
			(*_crud)[columns] = to_string( id_ );
			_crud->execute( HCRUDDescriptor::MODE::INSERT );
		}
	}
	for ( int id : ids ) {
		filter.assign( _idColumn ).append( " = " ).append( to_string( id ) );
		_crud->set_filter( filter );
		_crud->execute( HCRUDDescriptor::MODE::DELETE );
	}
	return;
	M_EPILOG
}

int long HDataListWidget::do_get_current_id( void ) {
	M_PROLOG
	return ( _cursor.is_valid() ? _cursor->get_id() : -1 );
	M_EPILOG
}

void HDataListWidget::do_add_new( void ) {
	M_PROLOG
	int size( static_cast<int>( _header.size() ) );
	HInfoItem item( size );
	for ( int i( 0 ); i < size; ++ i ) {
		item[i].set_string( "" );
		item[i].set_integer( -1 );
	}
	_dataModel->add_tail( item );
	process_input( KEY_CODES::HOME );
	process_input( KEY_CODES::END );
	return;
	M_EPILOG
}

void HDataListWidget::do_cancel_new( void ) {
	M_PROLOG
	_dataModel->remove_tail();
	if ( _dataModel->size() ) {
		process_input( KEY_CODES::HOME );
		process_input( KEY_CODES::END );
	}
	return;
	M_EPILOG
}

void HDataListWidget::set_record_descriptor( yaal::hcore::HString const& table_,
		yaal::hcore::HString const& columns_,
		yaal::hcore::HString const& filterColumn_,
		yaal::hcore::HString const& idColumn_,
		yaal::hcore::HString const& sort_ ) {
	M_PROLOG
	_table = table_;
	_columns = columns_;
	_filterColumn = filterColumn_;
	_idColumn = idColumn_;
	_sort = sort_;
	return;
	M_EPILOG
}

HWidget::ptr_t HDataListWidgetCreator::do_new_instance( HWindow* window_, yaal::tools::HXml::HConstNodeProxy const& node_ ) {
	M_PROLOG
	HListWidgetAttributes attrs;
	HDataWindow* window( dynamic_cast<HDataWindow*>( window_ ) );
	M_ENSURE( window );
	prepare_attributes( attrs, node_ );
	OResource r( get_resource( node_ ) );
	attrs.label_position( r._labelPosition ).label_decoration( r._labelDecoration );
	HDataListWidget* list( new ( memory::yaal ) HDataListWidget( window, r._row, r._column, r._height, r._width, r._label, attrs ) );
	apply_resources( list->get_pointer(), node_ );
	apply_role( window, list, node_ );
	return ( list->get_pointer() );
	M_EPILOG
}

bool HDataListWidgetCreator::do_apply_resources( hconsole::HWidget::ptr_t widget_, yaal::tools::HXml::HConstNodeProxy const& node_ ) {
	M_PROLOG
	bool ok( HListWidgetCreator::do_apply_resources( widget_, node_ ) );
	HString nodeName( node_.get_name() );
	HDataListWidget* dl( dynamic_cast<HDataListWidget*>( widget_.raw() ) );
	if ( nodeName == "db" ) {
		HString table( xml::attr_val( node_, "table" ) );
		HString columns( xml::attr_val( node_, "column" ) );
		HString idColumn( xml::attr_val( node_, "id_column" ) );
		xml::value_t filterColumn( xml::try_attr_val( node_, "filter_column" ) );
		xml::value_t sort( xml::try_attr_val( node_, "sort" ) );
		dl->set_record_descriptor( table, columns, filterColumn ? *filterColumn : "", idColumn, sort ? *sort : "" );
	}
	return ( ok );
	M_EPILOG
}

HListWidget::HColumnInfo::ptr_t HDataListWidgetCreator::do_make_column(
		yaal::tools::HXml::HConstNodeProxy const& node_,
		HListWidget* widget_,
		yaal::hcore::HString const& columnName,
		int width,
		HListWidget::BITS::ALIGN::align_t const& align,
		type_id_t type,
		yaal::hcore::HString const& format_,
		HWidget* associatedWidget ) {
	M_PROLOG
	M_ASSERT( node_.get_name() == "column" );
	xml::value_t dictName( xml::try_attr_val( node_, "dict" ) );
	HDictionary::ptr_t dict;
	if ( !! dictName ) {
		HDataWindow* window( dynamic_cast<HDataWindow*>( widget_->get_window() ) );
		M_ASSERT( window );
		dict = window->get_dictionary( *dictName );
	}
	return ( HListWidget::HColumnInfo::ptr_t(
			new ( memory::yaal ) HDataListWidget::HDataColumnInfo( columnName, width, align, type, format_, associatedWidget, dict ) ) );
	M_EPILOG
}

namespace {

bool register_creator( void ) {
	HWidgetFactory::get_instance().register_widget_creator( "datalist",
			HWidgetCreatorInterface::ptr_t( static_cast<HWidgetCreatorInterface*>( new ( memory::yaal ) HDataListWidgetCreator() ) ) );
	return ( true );
}

bool volatile registered = register_creator();

}

}

}


