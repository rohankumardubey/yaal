/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hdatatreewidget.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_HDATA_HDATATREEWIDGET_HXX_INCLUDED
#define YAAL_HDATA_HDATATREEWIDGET_HXX_INCLUDED 1

#include "hconsole/htreewidget.hxx"
#include "hdata/hdatawidget.hxx"

namespace yaal {

namespace hdata {

class HDataWindow;

/*! \brief Database connected tree based TUI widget.
 */
class HDataTreeWidget : public HDataWidget, public virtual hconsole::HTreeWidget {
public:
	HDataTreeWidget( HDataWindow*, int, int, int, int, yaal::hcore::HString const&,
			hconsole::HWidgetAttributesInterface const& = hconsole::HWidgetAttributesInterface() );
	virtual ~HDataTreeWidget( void );
	virtual void do_load( int long ) override;
};

class HDataTreeWidgetCreator : virtual public yaal::hconsole::HTreeWidgetCreator {
protected:
	virtual hconsole::HWidget::ptr_t do_new_instance( hconsole::HWindow*, yaal::tools::HXml::HConstNodeProxy const& ) override;
};

}

}

#endif /* #ifndef YAAL_HDATA_HDATATREEWIDGET_HXX_INCLUDED */

