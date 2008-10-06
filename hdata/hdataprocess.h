/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hdataprocess.h - this file is integral part of `yaal' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You can not use any part of sources of this software.
  2. You can not redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you can not demand any fees
     for this software.
     You can not even demand cost of the carrier (CD for example).
  5. You can not include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#ifndef __YAAL_HDATA_HDATAPROCESS_H
#define __YAAL_HDATA_HDATAPROCESS_H

#include "hconsole/htuiprocess.h"
#include "hconsole/omenuitem.h"
#include "tools/hxml.h"
#include "dbwrapper/hdatabase.h"
#include "hdata/oresource.h"

namespace yaal
{

namespace hdata
{

#define M_REGISTER_MENU_HANDLER( handler ) \
	{ \
	typedef typeof ( *this ) this_t; \
	l_oHandlers[ #handler ] = static_cast<OMenuItem::HANDLER_t>( \
			&this_t::handler ); \
	}

typedef yaal::hcore::HHashMap<yaal::hcore::HString,
				yaal::hconsole::OMenuItem::HANDLER_t> menu_handlers_map_t;

class HDataProcess : public hconsole::HTUIProcess
	{
protected:
	typedef HDataProcess self_t;
	typedef HTUIProcess hier_t;
	typedef yaal::hcore::HList<yaal::hcore::HString> param_cache_t;
	typedef yaal::hconsole::HWindow::ptr_t ( *window_factory_t )( yaal::hcore::HString const&, HDataProcess*, OResource* );
	typedef yaal::hcore::HPool<OResource> resource_pool_t;
	typedef yaal::hcore::HMap<yaal::hcore::HString, resource_pool_t> resource_cache_t;
	typedef yaal::hcore::HPool<OColumnInfo> column_pool_t;
	typedef yaal::hcore::HList<OEditControlResource> edit_cache_t;
	typedef yaal::hcore::HList<OListControlResource> list_cache_t;
	typedef yaal::hcore::HList<column_pool_t> column_cache_t;
	dbwrapper::database_ptr_t f_oDataBase;
	menu_handlers_map_t f_oAutoHandlers;
	yaal::tools::HXml f_oResource;
	resource_cache_t f_oResourceCache;
	column_cache_t f_oColumnCache;
	edit_cache_t f_oEditCache;
	list_cache_t f_oListCache;
private:
	yaal::hconsole::OMenuItem* f_psRootMenu;
public:
	HDataProcess( void );
	virtual ~HDataProcess( void );
	virtual int init_xrc( char const*, char const*,
			menu_handlers_map_t const& );
	dbwrapper::database_ptr_t data_base( void );
protected:
	virtual int handler_quit( int, void const* = NULL );
	virtual int handler_close_window( int, void const* = NULL );
private:
	yaal::hconsole::OMenuItem* build_sub_menu( yaal::tools::HXml::HConstNodeProxy const&,
			menu_handlers_map_t const& );
	void build_menu_item( yaal::tools::HXml::HConstNodeProxy const&,
			yaal::hconsole::OMenuItem&, menu_handlers_map_t const& );
	void destroy_menu( yaal::hconsole::OMenuItem* );
	OResource* get_resource( yaal::hcore::HString const&, yaal::tools::HXml::HConstNodeProxy const& );
	OResource* build_resource( yaal::hcore::HString const&, yaal::tools::HXml::HConstNodeProxy const& );
	int create_window( void* );
private:
	HDataProcess( HDataProcess const& );
	HDataProcess& operator = ( HDataProcess const& );
	};

}

}

#endif /* not __YAAL_HDATA_HDATAPROCESS_H */
