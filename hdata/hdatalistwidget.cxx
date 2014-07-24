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

using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::hconsole::list_widget_helper;
using namespace yaal::dbwrapper;

namespace yaal {

namespace hdata {

HDataListWidget::HDataListWidget(
		HDataWindow* window_, int row_, int column_, int height_,
		int width_, char const* title_ )
								: HWidget ( window_, row_, column_, height_,
										width_, title_ ),
								HSearchableWidget( true ),
								HListWidget( NULL, 0, 0, 0, 0, NULL ),
								HDataWidget(), _dataModel( _model ) {
	M_PROLOG
	return;
	M_EPILOG
}

HDataListWidget::~HDataListWidget ( void ) {
	M_PROLOG
	return;
	M_EPILOG
}

void HDataListWidget::load( int long /*id_*/ ) {
	M_PROLOG
	_SQL->set_filter( "" );
	HDataWindow* parent = dynamic_cast<HDataWindow*>( _window );
	M_ASSERT( parent );
	HRecordSet::ptr_t rs = _SQL->execute( HSQLDescriptor::MODE::SELECT );
	int idColNo = -1;
	int const colCount = rs->get_field_count();
	for ( int i = 0; i < colCount; ++ i )
		if ( ! ::strcmp( rs->get_column_name( i ), _resource->_id ) )
			idColNo = i;
	HDataWindow::ORowBuffer rb( idColNo, static_cast<int>( _header.size() ) );
	parent->set_sync_store( &rb );
	parent->status_bar()->init_progress( 0., "Collecting ..." );
	HAsIsValueListModel<>::data_ptr_t data( _dataModel->get_data() );
	HAsIsValueListModel<>::data_t::iterator it( data->begin() );
	int ctr( 0 );
	int size( static_cast<int>( _dataModel->size() ) );
	for ( HRecordSet::iterator row( rs->begin() ), end( rs->end() ); row != end; ++ row ) {
		parent->sync( row );
		parent->status_bar()->update_progress();
		if ( it != data->end() )
			{
			(*it) = rb._item;
			++ it;
		} else
			data->push_back( rb._item );
		++ ctr;
	} while ( ctr ++ < size )
		_dataModel->remove_tail();
	reset();
	parent->set_sync_store();
	return;
	M_EPILOG
}

int long HDataListWidget::get_current_id( void ) {
	M_PROLOG
	return ( _cursor->get_id() );
	M_EPILOG
}

void HDataListWidget::add_new( void ) {
	M_PROLOG
	_dataModel->add_tail( HInfoItem( static_cast<int>( _header.size() ) ) );
	process_input( KEY_CODES::HOME );
	process_input( KEY_CODES::END );
	return;
	M_EPILOG
}

void HDataListWidget::cancel_new( void ) {
	M_PROLOG
	_dataModel->remove_tail();
	if ( _dataModel->size() ) {
		process_input( KEY_CODES::HOME );
		process_input( KEY_CODES::END );
	}
	return;
	M_EPILOG
}

}

}
