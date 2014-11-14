/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hdatacomboboxwidget.hxx - this file is integral part of `yaal' project.

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
/*! \file hdata/hdatacomboboxwidget.hxx
 * \brief Declaration of HDataComboboxWidget class.
 */

#ifndef YAAL_HDATA_HDATACOMBOBOXWIDGET_HXX_INCLUDED
#define YAAL_HDATA_HDATACOMBOBOXWIDGET_HXX_INCLUDED 1

#include "hconsole/hcomboboxwidget.hxx"
#include "hdata/hdatawindow.hxx"

namespace yaal {

namespace hdata {

/*! \brief Database enabled TUI Edit widget.
 *
 * This TUI widget support automatic synchronization with document query column.
 */
class HDataComboboxWidget : public HDataWidget, public virtual hconsole::HComboboxWidget {
public:
	typedef HDataComboboxWidget this_type;
	typedef HComboboxWidget base_type;
private:
public:
	HDataComboboxWidget( HDataWindow*, /* parent */
			int, /* top ( parent relative ) */
			int, /* left */
			int, /* height */
			int, /* width */
			yaal::hcore::HString const& = "", /* label */
			hconsole::HWidgetAttributesInterface const& = hconsole::HWidgetAttributesInterface() );
};

class HDataComboboxWidgetCreator : virtual public yaal::hconsole::HComboboxWidgetCreator {
protected:
	virtual hconsole::HWidget::ptr_t do_new_instance( hconsole::HWindow*, yaal::tools::HXml::HConstNodeProxy const& );
};

}

}

#endif /* #ifndef YAAL_HDATA_HDATACOMBOBOXWIDGET_HXX_INCLUDED */
