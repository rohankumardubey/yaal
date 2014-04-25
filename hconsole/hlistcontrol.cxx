/*
---          `yaal' (c) 1978 by Marcin 'Amok' Konarski           ---

	hlistcontrol.cxx - this file is integral part of `yaal' project.

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
#include <cctype>
#include <libintl.h>

#include "hcore/base.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "hlistcontrol.hxx"
#include "hcore/memory.hxx"
#include "hconsole.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole::list_control_helper;
using namespace yaal::hconsole::mouse;

namespace yaal {

namespace hconsole {

HListControl::HColumnInfo::HColumnInfo( void )
	: _descending( false ), _widthRaw( 0 ), _width( 0 ), _align( BITS::ALIGN::LEFT ),
	_shortcutIndex( 0 ), _shortcut( 0 ), _type( TYPE::HSTRING ), _name(),
	_control( NULL ) {
	M_PROLOG
	return;
	M_EPILOG
}

HListControl::HColumnInfo::~HColumnInfo( void ) {
	M_PROLOG
	_control = NULL;
	return;
	M_EPILOG
}

HListControl::HColumnInfo::HColumnInfo( HColumnInfo const& columnInfo_ )
	: _descending( false ), _widthRaw( 0 ), _width( 0 ), _align( BITS::ALIGN::LEFT ),
	_shortcutIndex( 0 ), _shortcut( 0 ), _type( TYPE::HSTRING ), _name(),
	_control( NULL ) {
	M_PROLOG
	( *this ) = columnInfo_;
	return;
	M_EPILOG
}

HListControl::HColumnInfo& HListControl::HColumnInfo::operator = ( HColumnInfo const& columnInfo_ ) {
	M_PROLOG
	if ( this != & columnInfo_ ) {
		_descending = columnInfo_._descending;
		_widthRaw = columnInfo_._widthRaw;
		_width = columnInfo_._width;
		_type = columnInfo_._type;
		_align = columnInfo_._align;
		_shortcutIndex = columnInfo_._shortcutIndex;
		_shortcut = columnInfo_._shortcut;
		_name = columnInfo_._name;
		_control = columnInfo_._control;
	}
	return ( *this );
	M_EPILOG
}

HListControl::HListControl( HWindow* parent_, int row_, int column_,
		int height_, int width_, char const* label_, HAbstractControler::ptr_t const& data_ )
						: HControl( parent_, row_, column_, height_, width_,
								label_ ),
							HSearchableControl( true ),
	_checkable( false ), _sortable( true ),
	_drawHeader( true ), _editable( false ),
	_controlOffset( 0 ), _cursorPosition( 0 ), _sumForOne( 0 ),
	_header(), _sortColumn( -1 ), _match(),
	_cursor(), _firstVisibleRow(), _controler( data_ ) {
	M_PROLOG
	_controler->set_control( this );
	schedule_refresh();
	return;
	M_EPILOG
}

HListControl::~HListControl( void ) {
	return;
}

void HListControl::do_refresh( void ) {
	M_PROLOG
	int columnOffset = 0;
	int columns = static_cast<int>( _header.size() );
	int hR = _drawHeader ? 1 : 0; /* HR stands for header row */
	int size = static_cast<int>( _controler->size() );
	HColumnInfo * columnInfo = NULL;
	HConsole& cons = HConsole::get_instance();
	int tmp( _widthRaw );
	if ( _focused )
		cons.curs_set( CURSOR::INVISIBLE );
	draw_label(); /* raw* set here */
	_rowRaw += hR;
	if ( ! _sumForOne )
		return;
	if ( _widthRaw != tmp )
		recalculate_column_widths();
/* we need to decrement _heightRaw because we have additional row,
 * the list control header */
	if ( _drawHeader )
		_heightRaw --;
	_varTmpBuffer.hs_realloc( _widthRaw + 1 );
	int ctr( 0 );
	if ( size > 0 ) {
		iterator_t it = _firstVisibleRow;
		for ( ctr = 0;
					ctr < ( size > _heightRaw ? _heightRaw : size );
					++ ctr, ++ it ) {
			columnOffset = 0;
			for ( int ctrLoc( 0 ); ctrLoc < columns; ctrLoc ++ ) {
				columnInfo = & _header[ ctrLoc ];
				if ( columnInfo->_widthRaw ) {
					bool checked( get_text_for_cell( it, ctrLoc, columnInfo->_type ) );
					draw_cell( it, ctr, ctrLoc, columnOffset, columnInfo, checked );
					columnOffset += columnInfo->_widthRaw;
				}
				if ( ( it == _cursor ) && columnInfo->_control )
					(*it)[ ctrLoc ].set_child_control_data( columnInfo->_control );
			}
		}
	}
	draw_background( ctr );
	_rowRaw -= hR;
	draw_header( columns );
	_rowRaw += hR;
	cons.set_attr( ! _enabled ?
			( ! _focused ? _attributeFocused._data : _attributeEnabled._data )
			: _attributeDisabled._data );
	if ( size > _heightRaw )
		draw_scroll( _columnRaw + columnOffset - 1 );
	_rowRaw -= hR;
	return;
	M_EPILOG
}

void HListControl::draw_header( int columns_ ) {
	M_PROLOG
	int ctr = 0;
	int ctrLoc = 0;
	int columnOffset = 0;
	int hR = _drawHeader ? 1 : 0; /* HR stands for header row */
	HColumnInfo * columnInfo = NULL;
	HConsole& cons = HConsole::get_instance();
	for ( ctr = 0; ctr < columns_; ctr ++ ) {
		columnInfo = & _header[ ctr ];
		if ( columnInfo->_widthRaw ) {
			if ( _drawHeader ) {
				_varTmpBuffer = columnInfo->_name;
				set_attr_label();
				_varTmpBuffer.format( "%%-%ds",
							columnInfo->_widthRaw );
				cons.mvprintf( _rowRaw, _columnRaw + columnOffset,
							_varTmpBuffer.raw(),
								columnInfo->_name.left(
									columnInfo->_widthRaw ).raw() );
				set_attr_shortcut();
				cons.mvprintf( _rowRaw,
							_columnRaw + columnOffset + columnInfo->_shortcutIndex,
							"%c", columnInfo->_shortcut );
				if ( _sortColumn == ctr )
					cons.mvprintf( _rowRaw,
								_columnRaw + columnOffset
								+ columnInfo->_widthRaw - 2,
								"%c", columnInfo->_descending ? '^' : 'v' );
			}
			columnOffset += columnInfo->_widthRaw;
			if ( ctr < columns_ ) {
				cons.set_attr( _attributeDisabled._data );
				for ( ctrLoc = 0; ctrLoc < ( _heightRaw + hR );
						ctrLoc ++ ) {
					cons.move( _rowRaw + ctrLoc, _columnRaw + columnOffset - 1 );
					cons.addch( GLYPHS::VERTICAL_LINE );
				}
			}
		}
	}
	if ( ! _singleLine ) {
		_varTmpBuffer.format( " %d/%d ", _controlOffset + _cursorPosition + 1, static_cast<int>( _controler->size() ) );
		if ( _labelLength < _widthRaw ) {
			int clip = static_cast<int>( ( ( _widthRaw - _labelLength ) < _varTmpBuffer.get_length() ) ? _varTmpBuffer.get_length() - ( _widthRaw - _labelLength ) : 0 );
			cons.mvprintf( _rowRaw - 1, static_cast<int>( _columnRaw + _widthRaw + clip - _varTmpBuffer.get_length() ), _varTmpBuffer.raw() + clip );
		}
	}
	return;
	M_EPILOG
}

void HListControl::draw_background( int from_ ) {
	M_PROLOG
	int ctr = from_;
	set_attr_data();
	_varTmpBuffer.fillz( '.', 0, _widthRaw );
	for ( ; ctr < _heightRaw; ctr ++ )
		HConsole::get_instance().mvprintf( _rowRaw + ctr, _columnRaw, _varTmpBuffer.raw() );
	return;
	M_EPILOG
}

void HListControl::draw_scroll( int posX_ ) {
	M_PROLOG
	double scaled = 0;
	int size = static_cast<int>( _controler->size() );
	HConsole& cons = HConsole::get_instance();
	if ( _controlOffset ) {
		cons.move( _rowRaw, posX_ );
		cons.addch( GLYPHS::UP_ARROW );
	}
	if ( ( size - _controlOffset ) > _heightRaw ) {
		cons.move( _rowRaw + _heightRaw - 1, posX_ );
		cons.addch( GLYPHS::DOWN_ARROW );
	}
	scaled = _heightRaw - 3;
	scaled *= static_cast<double>(
			_controlOffset + _cursorPosition );
	scaled /= static_cast<double>( size );
	cons.mvprintf( _rowRaw + static_cast<int>( scaled + 1.5 ), posX_, "#" );
	return;
	M_EPILOG
}

void HListControl::draw_cell( iterator_t& it_, int row_, int column_, int columnOffset_, HColumnInfo const* const  columnInfo_, bool checked_ ) {
	M_PROLOG
	HConsole& cons( HConsole::get_instance() );
	int len( static_cast<int>( _varTmpBuffer.get_length() ) );
	switch ( columnInfo_->_align ) {
		case ( BITS::ALIGN::LEFT ): {
			if ( len < columnInfo_->_widthRaw )
				_varTmpBuffer.fillz( '_', len, columnInfo_->_widthRaw - len );
			else if ( len > columnInfo_->_widthRaw )
				_varTmpBuffer.set_at( columnInfo_->_widthRaw, 0 );
		}
		break;
		case ( BITS::ALIGN::CENTER ): {
			if ( len > columnInfo_->_widthRaw )
				_varTmpBuffer = _varTmpBuffer.right(
						columnInfo_->_widthRaw );
			else if ( len < columnInfo_->_widthRaw ) {
				_varTmpBuffer.insert( 0, ( columnInfo_->_widthRaw - len ) / 2, '_' );
				len = static_cast<int>( _varTmpBuffer.get_length() );
				_varTmpBuffer.fillz( '_', len, columnInfo_->_widthRaw - len );
			}
		}
		break;
		case ( BITS::ALIGN::RIGHT ): {
			if ( len > columnInfo_->_widthRaw )
				_varTmpBuffer.erase( 0, len - columnInfo_->_widthRaw );
			else if ( len < columnInfo_->_widthRaw )
				_varTmpBuffer.insert( 0, ( columnInfo_->_widthRaw - len ) - 1, '_' );
		}
		break;
		default :
			M_THROW( "unknown align", columnInfo_->_align );
		break;
	}
	if ( row_ == _cursorPosition ) {
		if ( checked_ )
			cons.set_attr( ! _enabled
					? ( ! _focused ? ~_attributeFocused._label
						: ~_attributeEnabled._label )
					: ~_attributeDisabled._label );
		else
			cons.set_attr( _enabled ? ( _focused ? ~_attributeFocused._data
						: ~_attributeEnabled._data ) : ~_attributeDisabled._data );
	} else {
		if ( checked_ )
			cons.set_attr( _enabled
					? ( _focused ? ~_attributeFocused._label
						: ~_attributeEnabled._label )
					: ~_attributeDisabled._label );
		else
			set_attr_data();
	}
	cons.mvprintf( _rowRaw + row_, _columnRaw + columnOffset_, _varTmpBuffer.raw()	);
	if ( _searchActived )
		highlight( _rowRaw + row_,
				_columnRaw + columnOffset_, _match._matchNumber,
				( it_ == _match._currentMatch )
				&& ( column_ == _match._columnWithMatch ) );
	return;
	M_EPILOG
}

namespace {

template<typename tType>
inline void increment( tType& iterator, int count ) {
	for ( int i = 0; i < count; ++ i, ++ iterator )
		;
}

template<typename tType>
inline void decrement( tType& iterator, int count ) {
	for ( int i = 0; i < count; ++ i, -- iterator )
		;
}

template<typename tType, typename ttType>
inline void cyclic_increment( ttType& model, tType& iterator, int count ) {
	for ( int i = 0; i < count; ++ i ) {
		++ iterator;
		if ( ! iterator.is_valid() )
			iterator = model->begin();
	}
}

template<typename tType, typename ttType>
inline void cyclic_decrement( ttType& model, tType& iterator, int count ) {
	for ( int i = 0; i < count; ++ i ) {
		-- iterator;
		if ( ! iterator.is_valid() )
			iterator = model->rbegin();
	}
}

}

void HListControl::handle_key_page_up( void ) {
	if ( ! _cursorPosition ) {
		if ( _controlOffset ) {
			_controlOffset -= ( _heightRaw - 1 );
			decrement( _firstVisibleRow, _heightRaw - 1 );
		} else
			HConsole::get_instance().bell();
		if ( _controlOffset < 0 ) {
			_controlOffset = 0;
			_firstVisibleRow = _controler->begin();
		}
	} else {
		_cursorPosition = 0;
		_cursor = _firstVisibleRow;
	}
	return;
}

void HListControl::handle_key_page_down( void ) {
	int size = static_cast<int>( _controler->size() );
	if ( size >= _heightRaw ) {
		if ( _cursorPosition == ( _heightRaw - 1 ) ) {
			if ( _controlOffset < ( size - _heightRaw ) ) {
				_controlOffset += ( _heightRaw - 1 );
				increment( _firstVisibleRow, _heightRaw - 1 );
				if ( _controlOffset > ( size - _heightRaw ) ) {
					_controlOffset = size - _heightRaw;
					_firstVisibleRow = _controler->rbegin();
					decrement( _firstVisibleRow, _heightRaw - 1 );
				}
			} else
				HConsole::get_instance().bell();
		} else {
			_cursorPosition = _heightRaw - 1;
			_cursor = _firstVisibleRow;
			increment( _cursor, _heightRaw - 1 );
		}
	} else {
		if ( _cursorPosition != ( size - 1 ) ) {
			_cursorPosition = size - 1;
			_cursor = _controler->rbegin();
		} else
			HConsole::get_instance().bell();
	}
	return;
}

void HListControl::handle_key_up( void ) {
	if ( ( _controlOffset + _cursorPosition ) > 0 ) {
		if ( _cursorPosition > 0 ) {
			-- _cursorPosition;
			-- _cursor;
		} else if ( _controlOffset > 0 ) {
			-- _firstVisibleRow;
			-- _cursor;
			-- _controlOffset;
		}
	} else
		HConsole::get_instance().bell();
	return;
}

void HListControl::handle_key_home( void ) {
	reset();
	return;
}

void HListControl::handle_key_end( void ) {
	int size = static_cast<int>( _controler->size() );
	_cursor = _controler->rbegin();
	if ( size >= _heightRaw ) {
		_cursorPosition = _heightRaw - 1;
		_controlOffset = size - _heightRaw;
		_firstVisibleRow = _cursor;
		decrement( _firstVisibleRow, _heightRaw - 1 );
	} else
		_cursorPosition = size - 1;
	return;
}

void HListControl::handle_key_down( void ) {
	if ( ( _cursorPosition + _controlOffset ) < ( _controler->size() - 1 ) ) {
		++ _cursorPosition;
		++ _cursor;
		if ( _cursorPosition >= _heightRaw ) {
			_cursorPosition = _heightRaw - 1;
			++ _controlOffset;
			++ _firstVisibleRow;
		}
	} else
		HConsole::get_instance().bell();
	return;
}

void HListControl::handle_key_ctrl_n ( void ) {
	if ( _backwards )
		go_to_match_previous();
	else
		go_to_match();
	return;
}

void HListControl::handle_key_ctrl_p( void ) {
	if ( _backwards )
		go_to_match();
	else
		go_to_match_previous();
	return;
}

void HListControl::handle_key_space( void ) {
	M_ASSERT( ! _controler->empty() );
	M_ASSERT( _cursor.is_valid() );
	if ( _checkable )
		_cursor->switch_state();
	return;
}

void HListControl::handle_key_tab( void ) {
	_focused = false;	/* very  */
	schedule_refresh();				/* magic */
	return;
}

int HListControl::do_process_input( int code_ ) {
	M_PROLOG
	int ctr = 0;
	int errorCode = 0;
	code_ = HControl::do_process_input( code_ );
	_varTmpBuffer = "";
	switch ( code_ ) {
		case ( KEY_CODES::PAGE_UP ):   handle_key_page_up();   break;
		case ( KEY_CODES::PAGE_DOWN ): handle_key_page_down(); break;
		case ( KEY_CODES::UP ):        handle_key_up();        break;
		case ( KEY_CODES::HOME ):      handle_key_home();      break;
		case ( KEY_CODES::END ):       handle_key_end();       break;
		case ( KEY_CODES::DOWN ):      handle_key_down();      break;
		case ( KEY<'n'>::ctrl ):       handle_key_ctrl_n();    break;
		case ( KEY<'p'>::ctrl ):       handle_key_ctrl_p();    break;
		case ( ' ' ):                  handle_key_space();     break;
		case ( '\t' ):                 handle_key_tab();
/* there is no break in previous `case():', because this list must give up
 * its focus and be refreshed and parent window must give focus
 * to another control */
		default : {
			errorCode = static_cast<int>( _header.size() );
			for ( ctr = 0; ctr < errorCode; ctr ++ )
				if ( tolower( code_ ) == tolower( _header[ ctr ]._shortcut ) )
					break;
			if ( ctr < errorCode ) {
				errorCode = 0;
				sort_by_column( ctr,
						code_ == tolower( code_ ) ? OSortHelper::ASCENDING : OSortHelper::DESCENDING );
			} else
				errorCode = code_;
		}
		break;
	}
	code_ = errorCode;
	if ( ! errorCode ) {
		schedule_refresh();
		_parent->status_bar()->message( COLORS::FG_LIGHTGRAY, "%s", _varTmpBuffer.raw() );
	}
	return ( code_ );
	M_EPILOG
}

void HListControl::add_column( int column_, char const* name_,
		int width_, BITS::ALIGN::align_t const& align_, type_id_t type_,
		HControl* control_ ) {
	M_PROLOG
	int shortcutIndex = 0;
	HColumnInfo columnInfo;
	int size = static_cast<int>( _controler->size() );
	if ( size )
		M_THROW( "cannot add new column when list not empty", size );
	_varTmpBuffer = name_;
	shortcutIndex = static_cast<int>( _varTmpBuffer.find( '&' ) );
	if ( shortcutIndex > -1 ) {
		_varTmpBuffer.set_at( shortcutIndex, 0 );
		_varTmpBuffer += name_ + shortcutIndex + 1;
	} else
		shortcutIndex = 0;
	_sumForOne += width_;
	columnInfo._width = width_;
	columnInfo._type = type_;
	columnInfo._align = align_;
	columnInfo._shortcutIndex = shortcutIndex;
	columnInfo._shortcut = _varTmpBuffer[ shortcutIndex ];
	columnInfo._name = _varTmpBuffer;
	columnInfo._control = control_;
	if ( ! _header.is_empty() && ( column_ >= 0 ) )
		_header.insert( _header.begin() + column_, columnInfo );
	else
		_header.push_back( columnInfo );
	recalculate_column_widths();
	return;
	M_EPILOG
}

int HListControl::set_focus( char shorcut_ ) {
	M_PROLOG
	return ( HControl::set_focus( shorcut_ ) );
	M_EPILOG
}

void HListControl::recalculate_column_widths( void ) {
	M_PROLOG
	int ctrLoc( 0 );
	int columnOffset( 0 );
	int columns( static_cast<int>( _header.size() ) );
	for ( int ctr( 0 ); ctr < columns; ctr ++ ) {
		int newWidth( _header[ ctr ]._width );
		if ( newWidth ) {
			if ( ! _sumForOne )
				M_THROW( "width of all columns equals 0", _sumForOne );
			newWidth *= _widthRaw;
			newWidth /= _sumForOne;
			ctrLoc = ctr; /* last one with non zero width */
			_header[ ctr ]._widthRaw = newWidth;
			columnOffset += newWidth;
		}
	}
	/* last column with non zero width should fill space */
	columnOffset -= _header[ ctrLoc ]._widthRaw;
	_header[ ctrLoc ]._widthRaw = ( _widthRaw - columnOffset );
	return;
	M_EPILOG
}

void HListControl::sort_by_column( int column_, OSortHelper::sort_order_t order_ ) {
	M_PROLOG
	if ( ! _sortable )
		return;
	_sortColumn = column_;
	_header[ column_ ]._descending = order_ == OSortHelper::DESCENDING;
	long int size = _controler->size();
	if ( size > 128 )
		_parent->status_bar()->init_progress(
				static_cast<double>( size )
				* static_cast<double>( size ) / 2.,
				" Sorting ..." );
	list_control_helper::OSortHelper helper =
		{ column_, order_, _header[ _sortColumn ]._type,
		0, _controler->size(), _parent };
	_controler->sort( helper );
	_controlOffset = _cursorPosition = 0;
	return;
	M_EPILOG
}

int HListControl::do_click( mouse::OMouse& mouse_ ) {
	M_PROLOG
	int row = 0;
	int columns( static_cast<int>( _header.size() ) );
	HColumnInfo* columnInfo = NULL;
	if ( ! HControl::do_click( mouse_ ) )
		return ( 1 );
	row = ( mouse_._row - _rowRaw ) - ( _drawHeader ? 1 : 0 );
	if ( row < 0 ) /* header clicked */ {
		int column( mouse_._column + _columnRaw - 1 );
		int width( 0 );
		for ( int ctr( 0 ); ctr < columns; ++ ctr ) {
			columnInfo = &_header [ ctr ];
			width += columnInfo->_widthRaw;
			if ( column <= width ) {
				sort_by_column( ctr,
						columnInfo->_descending ? OSortHelper::ASCENDING : OSortHelper::DESCENDING );
				schedule_refresh();
				break;
			}
		}
	} else if ( row < _controler->size() ) {
		if ( mouse_._buttons & MOUSE_BITS::BUTTON::WHEEL_UP )
			scroll_up();
		else if ( mouse_._buttons & MOUSE_BITS::BUTTON::WHEEL_DOWN )
			scroll_down();
		else if ( row == _cursorPosition )
			return ( 1 );
		else {
			if ( row > _cursorPosition ) {
				for ( int i( _cursorPosition ); i < row; ++ i )
					++ _cursor;
			} else {
				for ( int i( _cursorPosition ); i > row; -- i )
					-- _cursor;
			}
			_cursorPosition = row;
		}
		schedule_refresh();
	}
	return ( 0 );
	M_EPILOG
}

void HListControl::scroll_up( void ) {
	M_PROLOG
	if ( _controlOffset > 0 ) {
		-- _controlOffset;
		-- _firstVisibleRow;
		if ( _cursorPosition < ( _heightRaw - 1 ) )
			++ _cursorPosition;
		else
			-- _cursor;
	}
	return;
	M_EPILOG
}

void HListControl::scroll_down( void ) {
	M_PROLOG
	if ( ( _controlOffset + _heightRaw ) < _controler->size() ) {
		++ _controlOffset;
		++ _firstVisibleRow;
		if ( _cursorPosition > 0 )
			-- _cursorPosition;
		else
			++ _cursor;
	}
	return;
	M_EPILOG
}

bool HListControl::is_searchable( void ) {
	return ( _searchable );
}

void HListControl::reset( void ) {
	M_PROLOG
	_firstVisibleRow = _cursor = _controler->begin();
	_controlOffset = _cursorPosition = 0;
	M_EPILOG
}

void HListControl::go_to_match( void ) {
	M_PROLOG
	int ctr = 0, ctrLoc = 0, moveFirstRow = 0;
	int size = static_cast<int>( _controler->size() );
	int count = size + 1;
	int columns = static_cast<int>( _header.size() );
	int controlOffsetOrig = _controlOffset, cursorPositionOrig = _cursorPosition;
	char const * highlightStart = NULL;
	char const * outcome = NULL;
	iterator_t cursorOrig = _cursor;
	iterator_t firstVisibleRowOrig = _firstVisibleRow;
	if ( ! _searchActived )
		return;
	if ( _match._currentMatch != _cursor )
		_match._matchNumber = -1;
	_match._currentMatch = _cursor;
	while ( count -- ) {
		for ( ctr = _match._columnWithMatch; ctr < columns; ctr ++ ) {
			get_text_for_cell( _cursor, ctr, TYPE::HSTRING );
			highlightStart = NULL;
			ctrLoc = 0;
			for ( HPattern::HMatchIterator it = _pattern.find( _varTmpBuffer.raw() ),
					end = _pattern.end(); it != end; ++ it, ++ ctrLoc ) {
				if ( ctrLoc > _match._matchNumber ) {
					highlightStart = it->raw();
					break;
				}
			}
			if ( highlightStart )
				break;
			_match._matchNumber = -1;
		}
		if ( highlightStart )
			break;
		_match._columnWithMatch = 0;
/* this part is from process_input, but slightly modified */
		if ( ( _cursorPosition + _controlOffset ) < ( size - 1 ) ) {
			_cursorPosition ++;
			if ( _cursorPosition >= _heightRaw ) {
				_cursorPosition = _heightRaw - 1;
				_controlOffset ++;
				moveFirstRow ++;
			}
			++ _cursor;
		} else {
			_cursor = _firstVisibleRow = _controler->begin();
			_controlOffset = _cursorPosition = 0;
			moveFirstRow = 0;
			outcome = _( "search hit BOTTOM, continuing at TOP" );
		}
/* end od it */
	}
	if ( highlightStart ) {
		if ( moveFirstRow )
			increment( _firstVisibleRow, moveFirstRow );
		_match._columnWithMatch = ctr;
		_match._matchNumber = ctrLoc;
		_match._currentMatch = _cursor;
		_varTmpBuffer = "";
	} else {
		_cursor = cursorOrig;
		_firstVisibleRow = firstVisibleRowOrig;
		_cursorPosition = cursorPositionOrig;
		_controlOffset = controlOffsetOrig;
		_match._matchNumber = -1;
		_match._columnWithMatch = 0;
		_varTmpBuffer = HString( _( "pattern not found: " ) ) + _pattern.error();
	}
	if ( outcome )
		_varTmpBuffer = outcome;
	return;
	M_EPILOG
}

void HListControl::go_to_match_previous( void ) {
	M_PROLOG
	int ctr = 0, ctrLoc = 0, moveFirstRow = 0;
	int size = static_cast<int>( _controler->size() );
	int count = size + 1;
	int columns = static_cast<int>( _header.size() );
	int controlOffsetOrig = _controlOffset, cursorPositionOrig = _cursorPosition;
	char const * highlightStart = NULL;
	char const * outcome = NULL;
	iterator_t cursorOrig = _cursor;
	iterator_t firstVisibleRowOrig = _firstVisibleRow;
	if ( ! _searchActived )
		return;
	if ( _match._currentMatch != _cursor )
		_match._matchNumber = -1;
	_match._currentMatch = _cursor;
	while ( count -- ) {
		for ( ctr = _match._columnWithMatch; ctr >= 0; ctr -- ) {
			get_text_for_cell( _cursor, ctr, TYPE::HSTRING );
			highlightStart = NULL;
			ctrLoc = 0;
			if ( _match._matchNumber < 0 )
				_match._matchNumber = static_cast<int>( distance( _pattern.find( _varTmpBuffer.raw() ), _pattern.end() ) );
			for ( HPattern::HMatchIterator it = _pattern.find( _varTmpBuffer.raw() ),
					end = _pattern.end(); it != end; ++ it, ++ ctrLoc ) {
				if ( ctrLoc == ( _match._matchNumber - 1 ) ) {
					highlightStart = it->raw();
					break;
				}
				if ( ctrLoc >= _match._matchNumber ) {
					highlightStart = NULL;
					break;
				}
			}
			if ( highlightStart )
				break;
			_match._matchNumber = -1;
		}
		if ( highlightStart )
			break;
		_match._columnWithMatch = columns - 1;
/* this part is from process_input, but slightly modified */
		if ( ( _controlOffset + _cursorPosition ) > 0 ) {
			if ( _cursorPosition > 0 )
				_cursorPosition --;
			else if ( _controlOffset > 0 ) {
				_controlOffset --;
				moveFirstRow ++;
			}
			cyclic_decrement( _controler, _cursor, 1 );
		} else {
			if ( size >= _heightRaw ) {
				_cursor = _controler->begin();
				cyclic_decrement( _controler, _cursor, 1 );
				_firstVisibleRow = _cursor;
				cyclic_decrement( _controler, _firstVisibleRow, _heightRaw - 1 );
				_cursorPosition = _heightRaw - 1;
				_controlOffset = size - _heightRaw;
			} else {
				cyclic_decrement( _controler, _cursor, 1 );
				_cursorPosition = size - 1;
			}
			moveFirstRow = 0;
			outcome = _( "search hit TOP, continuing at BOTTOM" );
		}
/* end od it */
	}
	if ( highlightStart ) {
		if ( moveFirstRow )
			cyclic_decrement( _controler, _firstVisibleRow, moveFirstRow );
		_match._columnWithMatch = ctr;
		_match._matchNumber = ctrLoc;
		_match._currentMatch = _cursor;
		_varTmpBuffer = "";
	} else {
		_cursor = cursorOrig;
		_firstVisibleRow = firstVisibleRowOrig;
		_cursorPosition = cursorPositionOrig;
		_controlOffset = controlOffsetOrig;
		_match._matchNumber = -1;
		_match._columnWithMatch = 0;
		_varTmpBuffer = _( "pattern not found" );
	}
	if ( outcome )
		_varTmpBuffer = outcome;
	return;
	M_EPILOG
}

void HListControl::set_flags( flag_t flags_, flag_t mask_ ) {
	M_PROLOG
	if ( !!( mask_ & FLAG::SORTABLE ) )
		_sortable = !!( flags_ & FLAG::SORTABLE ) ? true : false;
	if ( !!( mask_ & FLAG::CHECKABLE ) )
		_checkable = !!( flags_ & FLAG::CHECKABLE ) ? true : false;
	if ( !!( mask_ & FLAG::EDITABLE ) )
		_editable = !!( flags_ & FLAG::EDITABLE ) ? true : false;
	if ( !!( mask_ & FLAG::DRAW_HEADER ) )
		_drawHeader = !!( flags_ & FLAG::DRAW_HEADER ) ? true : false;
	return;
	M_EPILOG
}

bool HListControl::get_text_for_cell( iterator_t& it_, int column_, type_id_t type_ ) {
	M_PROLOG
	M_ASSERT( it_.is_valid() );
	HAbstractRow& item = *it_;
	switch ( type_.value() ) {
		case ( TYPE::INT_LONG ):
			_varTmpBuffer = item[ column_ ].get_long();
		break;
		case ( TYPE::DOUBLE ):
			_varTmpBuffer = item[ column_ ].get_double();
		break;
		case ( TYPE::HSTRING ):
			_varTmpBuffer = item[ column_ ].get_string();
		break;
		case ( TYPE::HTIME ):
			_varTmpBuffer = item[ column_ ].get_time();
		break;
		default :
			M_THROW( "unknown type", type_.value() );
		break;
	}
	return ( item.get_checked() );
	M_EPILOG
}

list_control_helper::HAbstractControler::ptr_t& HListControl::get_controler( void ) {
	return ( _controler );
}

int long HListControl::get_row_count( void ) {
	return ( _controler->size() );
}

void HListControl::remove_current_row ( void ) {
	M_PROLOG
	bool flag = true;
	if ( _controlOffset
			&& ( ( _controlOffset + _heightRaw ) == _controler->size() ) ) {
		_controlOffset --;
		++ _firstVisibleRow;
	} else if ( _cursorPosition && ( _cursorPosition == ( _controler->size() - 1 ) ) )
		_cursorPosition --;
	else
		flag = false;
	if ( _cursor == _firstVisibleRow )
		++ _firstVisibleRow;
	iterator_t it = _cursor;
	if ( flag )
		++ _cursor;
	_controler->erase( it );
	schedule_refresh();
	return;
	M_EPILOG
}

void HListControl::do_update( void ) {
	M_PROLOG
	if ( _controler->size() ) {
		_controlOffset = _cursorPosition = 0;
		_firstVisibleRow = _controler->begin();
		_cursor = _controler->begin();
	}
	return;
	M_EPILOG
}

type_id_t HListControl::get_column_type( int column_ ) {
	M_PROLOG
	return ( _header[ column_ ]._type );
	M_EPILOG
}

namespace list_control_helper {

void OSortHelper::progress( void ) {
	M_PROLOG
	++ _comparedItems;
	if ( ( _size > 1024 ) && ! ( _comparedItems % 1024 ) )
		_window->status_bar()->update_progress( static_cast<double>( _comparedItems ) );
	return;
	M_EPILOG
}

template<>
bool compare_cells( HInfo const& left_, HInfo const& right_, OSortHelper& sortHelper_ ) {
	M_PROLOG
	if ( sortHelper_._window )
		sortHelper_.progress();
	bool lower = false;
	switch ( sortHelper_._type.value() ) {
		case ( TYPE::INT_LONG ):
			lower = left_.get_int_long() < right_.get_int_long();
		break;
		case ( TYPE::DOUBLE ):
			lower = left_.get_double() < right_.get_double();
		break;
		case ( TYPE::HSTRING ):
			lower = strcasecmp( left_.get_string(), right_.get_string() ) < 0;
		break;
		case ( TYPE::HTIME ):
			lower = left_.get_time().raw() < right_.get_time().raw();
		break;
		default:
			break;
	}
	return ( lower );
	M_EPILOG
}

template<>
HListControler<>::model_ptr_t HListControler<>::get_model( void ) {
	return ( _list );
}

HAbstractControler::~HAbstractControler( void ) {
	return;
}

HAbstractControler::HAbstractControler( void ) : _control( NULL ) {
	return;
}

void HAbstractControler::set_control( HListControl* control_ ) {
	_control = control_;
	return;
}

void HAbstractControler::erase( HAbstractControler::HModelIteratorWrapper& ) {
	return;
}

void HAbstractControler::add_tail( void ) {
	return;
}

template<>
yaal::hcore::HString const HCell<yaal::hcore::HList<HInfoItem>::iterator>::get_long( void ) {
	M_PROLOG
	return ( HString ( (*_data)[ _column ].get_int_long() ) );
	M_EPILOG
}

template<>
yaal::hcore::HString const HCell<yaal::hcore::HList<HInfoItem>::iterator>::get_double( void ) {
	M_PROLOG
	return ( HString ( (*_data)[ _column ].get_double() ) );
	M_EPILOG
}

template<>
yaal::hcore::HString const HCell<yaal::hcore::HList<HInfoItem>::iterator>::get_string( void ) {
	M_PROLOG
	return ( (*_data)[ _column ].get_string() );
	M_EPILOG
}

template<>
yaal::hcore::HString HCell<yaal::hcore::HList<HInfoItem>::iterator>::get_time( void ) {
	M_PROLOG
	return ( ( (*_data)[ _column ].get_time() ).string() );
	M_EPILOG
}

HAbstractControler::HModelIteratorWrapper::HModelIteratorWrapper( void ) : _iteratorPtr() {
	return;
}

HAbstractControler::HModelIteratorWrapper::HModelIteratorWrapper( iterator_ptr_t const& it_ ) : _iteratorPtr( it_ ) {
	return;
}

bool HAbstractControler::HModelIteratorWrapper::is_valid( void ) const {
	return ( _iteratorPtr->is_valid() );
}

bool HAbstractControler::HModelIteratorWrapper::operator==( HAbstractControler::HModelIteratorWrapper const& it_ ) {
	M_PROLOG
	return ( _iteratorPtr.raw() ? _iteratorPtr->is_equal( *it_._iteratorPtr ) : it_._iteratorPtr.raw() == NULL );
	M_EPILOG
}

bool HAbstractControler::HModelIteratorWrapper::operator!=( HAbstractControler::HModelIteratorWrapper const& it_ ) {
	M_PROLOG
	return ( _iteratorPtr.raw() ? _iteratorPtr->is_not_equal( *it_._iteratorPtr ) : it_._iteratorPtr.raw() != NULL );
	M_EPILOG
}

HAbstractControler::HModelIteratorWrapper& HAbstractControler::HModelIteratorWrapper::operator++( void ) {
	_iteratorPtr->next();
	return ( *this );
}

HAbstractControler::HModelIteratorWrapper& HAbstractControler::HModelIteratorWrapper::operator--( void ) {
	_iteratorPtr->previous();
	return ( *this );
}

HAbstractRow& HAbstractControler::HModelIteratorWrapper::operator*( void ) {
	return ( _iteratorPtr->dereference() );
}

HAbstractRow* HAbstractControler::HModelIteratorWrapper::operator->( void ) {
	return ( _iteratorPtr->call() );
}

HAbstractControler::HModelIteratorWrapper::HModelIteratorWrapper( HAbstractControler::HModelIteratorWrapper const& it_ )
	: _iteratorPtr() {
	operator=( it_ );
	return;
}

HAbstractControler::HModelIteratorWrapper& HAbstractControler::HModelIteratorWrapper::operator=( HAbstractControler::HModelIteratorWrapper const& it_ ) {
	M_PROLOG
	if ( &it_ != this ) {
		if ( it_._iteratorPtr.raw() )
			it_._iteratorPtr->assign_to( _iteratorPtr );
	}
	return ( *this );
	M_EPILOG
}

HAbstractControler::iterator_ptr_t& HAbstractControler::HModelIteratorWrapper::raw( void ) {
	return ( _iteratorPtr );
}

HAbstractControler::HAbstractModelIterator::HAbstractModelIterator( void ) {
	return;
}

HAbstractControler::HAbstractModelIterator::~HAbstractModelIterator( void ) {
	return;
}

HAbstractRow::~HAbstractRow( void ) {
	return;
}

template<>
void HRow<yaal::hcore::HList<HInfoItem>::iterator>::switch_state( void ) {
	_iterator->_checked = ! _iterator->_checked;
	return;
}

template<>
bool HRow<yaal::hcore::HList<HInfoItem>::iterator>::get_checked( void ) {
	return ( _iterator->_checked );
}

template<>
int long HRow<yaal::hcore::HList<HInfoItem>::iterator>::get_id( void ) {
	return ( _iterator->_id );
}

template<>
HRow<yaal::hcore::HList<HInfoItem>::iterator>::HRow( iterator_t& it_ ) : _iterator( it_ ), _cells( it_->get_value_count() ) {
	M_PROLOG
	int long cellCount = it_->get_value_count();
	for ( int i = 0; i < cellCount; ++ i )
		_cells[ i ] = make_pointer<HCell<yaal::hcore::HList<HInfoItem>::iterator> >( ref( _iterator ), i );
	return;
	M_EPILOG
}

HAbstractCell::~HAbstractCell( void ) {
	return;
}

template<>
void HCell<yaal::hcore::HList<HInfoItem>::iterator>::set_child_control_data( HControl* control_ ) {
	M_PROLOG
	control_->set( (*_data)[ _column ] );
	control_->refresh();
	return;
	M_EPILOG
}

template<>
bool CompareListControlItems<HInfoItem>::operator() ( HInfoItem const& left_,
		HInfoItem const& right_ ) const {
	M_PROLOG
	HInfoItem const& left = _sortHelper._order == OSortHelper::ASCENDING ? left_ : right_;
	HInfoItem const& right = _sortHelper._order == OSortHelper::ASCENDING ? right_ : left_;
	return ( list_control_helper::compare_cells( left[ _sortHelper._sortColumn ],
				right[ _sortHelper._sortColumn ], _sortHelper ) );
	M_EPILOG
}

}

}

}
