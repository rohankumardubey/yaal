/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hcomboboxcontrol.hxx - this file is integral part of `yaal' project.

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
/*! \file hconsole/hcomboboxcontrol.hxx
 * \brief Declaration of HComboboxControl class.
 */

#ifndef YAAL_HCONSOLE_HCOMBOBOXCONTROL_HXX_INCLUDED
#define YAAL_HCONSOLE_HCOMBOBOXCONTROL_HXX_INCLUDED 1

#include "hconsole/heditcontrol.hxx"
#include "hconsole/hlistcontrol.hxx"

namespace yaal {

namespace hconsole {

/*! \brief Implementation of TUI Combobox control class.
 *
 * Combobox control allows fancy representation of row based data with handful
 * of display alteration methods and convenient text editing together with
 * advanced automatic text operations.
 */
class HComboboxControl : public virtual HEditControl,
	public virtual HListControl {
public:
	typedef HComboboxControl this_type;
	typedef HListControl base_type;
	/*! \brief Current combobox state.
	 */
	struct MODE {
		/*! \brief Current combobox state.
		 */
		typedef enum {
			EDITCONTROL, /*!< Combobox retracted. */
			LISTCONTROL  /*!< Combobox expanded. */
		} mode_t;
	};
protected:
	MODE::mode_t _mode; /* operating mode (MODE_EDITCONTROL|MODE_LISTCONTROL) */
	int _droppedWidth;	/* width of dropped list */
public:
	HComboboxControl ( HWindow *,			/* parent */
										 int,						/* row */
										 int,						/* col */
										 int,						/* height */
										 int,						/* width */
										 char const *,	/* label */
										 int = 0, 			/* dropped width */
										 int = 127,			/* max string length */
										 char const * = _maskDefault_,	/* regexp (mask) */
										 bool = true		/* searchable */ );		/* focused attribute */
	virtual ~HComboboxControl ( void );
	virtual int set_focus ( char = 0 );
	virtual int kill_focus ( void ); /* this one will be used here */
protected:
	virtual int do_process_input ( int );
	virtual void do_refresh ( void );
	virtual int do_click ( mouse::OMouse & );
private:
	void close_combo ( void );
};

typedef yaal::hcore::HExceptionT<HComboboxControl, HListControlException> HComboboxControlException;

}

}

#endif /* #ifndef YAAL_HCONSOLE_HCOMBOBOXCONTROL_HXX_INCLUDED */

