/*
---          `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hcontrol.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_HCONSOLE_HCONTROL_H
#define __YAAL_HCONSOLE_HCONTROL_H

#include "hconsole/console.h"
#include "hcore/hpointer.h"
#include "hcore/hstring.h"
#include "hcore/hinfo.h"

namespace yaal
{

namespace hconsole
{

class HWindow;

class HControl
	{
public:
	struct BITS
		{
		struct ALIGN
			{
			typedef enum
				{
				D_LEFT,
				D_CENTER,
				D_RIGHT
				} align_t;
			};
		};
	static int const D_DEFAULT_ATTRS = -1;
	typedef yaal::hcore::HPointer<HControl,yaal::hcore::HPointerScalar,yaal::hcore::HPointerRelaxed> ptr_t;
protected:
	bool			f_bEnabled;						/* is cotrol enabled, focus can go 
																		 only to enabled control */
	bool			f_bFocused;						/* has control focus */
	bool			f_bDrawLabel;					/* will be label driven */
	bool 			f_bSingleLine;				/* is label in the same line as top of
																		 control */
	int unsigned f_uiAttributeDisabled;	/* attribute of disabled cotrol */
	int unsigned f_uiAttributeEnabled;	/* attribute of enabled cotrol */	
	int unsigned f_uiAttributeFocused;	/* attribute of focused cotrol */
																	/* high byte of attribute in all
																		 three cases keeps label (control title)
																 		 color, low byte keeps work place color */
	int				f_iRow;								/* control top coordinate relative
																		 to parent window (top or bottom border) */
	int				f_iColumn;						/* control left coordinate relative
																		 to parent window (left or right border) */
	int				f_iHeight;						/* height or coorinate of bottom of control */
	int				f_iWidth;							/* width or coorinate of right of control */
	int				f_iRowRaw;						/* control top coordinate relative
																		 to parent window (after repositioning) */
	int				f_iColumnRaw;					/* control left coordinate relative
																		 to parent window (after repositioning) */
	int				f_iHeightRaw;					/* self explanary */
	int				f_iWidthRaw;					/* self explanary */
	hcore::HString	f_oLabel;					/* control title */
	hcore::HString	f_oVarTmpBuffer;	/* buffer for temporary operations, used
																			 to keep memory fragmentation low */
	HWindow* f_poParent;						/* window that will hold this control */
/* this is used locally, there is no way to modify this variables explicitly */
	int				f_iLabelLength;	 			/* length of the label */
	int				f_iShortcutIndex;			/* index of shortcut char in label */
public:
/* parent, row, col, height, width, label */
	HControl ( HWindow*, int, int, int, int, char const* );
	virtual ~HControl ( void );
	void enable ( bool );
	virtual int process_input ( int );
	/* set focus if control is enabled or if shortcut char is specified */
	virtual int set_focus ( char = 0 ); /* focused and shortcut char match */
	virtual int kill_focus ( void );
	virtual void set ( hcore::HInfo const& );
	virtual hcore::HInfo get ( void );
	virtual bool is_searchable ( void );
	void refresh ( void );
	int click ( mouse::OMouse& );
	bool hit_test ( int, int ) const;
	void set_attributes ( int = D_DEFAULT_ATTRS, int = D_DEFAULT_ATTRS, int = D_DEFAULT_ATTRS );
	void set_draw_label ( bool );
	void move ( int, int, int, int );
	int attr_label ( void ) const;
	int attr_shortcut ( void ) const;
	int attr_data ( void ) const;
	void set_attr_label ( void ) const;
	void set_attr_shortcut ( void ) const;
	void set_attr_data ( void ) const;
	void schedule_refresh( void ) const;
protected:
	virtual void do_refresh ( void ) = 0;
	virtual int do_click ( mouse::OMouse& );
	virtual void draw_label ( void );
private:
	HControl ( HControl const& );
	HControl& operator = ( HControl const& );
	};

}

}

#endif /* not __YAAL_HCONSOLE_HCONTROL_H */

