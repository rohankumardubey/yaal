/*
---          `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

	hcontrol.hxx - this file is integral part of `yaal' project.

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

#ifndef YAAL_HCONSOLE_HCONTROL_HXX_INCLUDED
#define YAAL_HCONSOLE_HCONTROL_HXX_INCLUDED 1

#include "hconsole/console.hxx"
#include "hcore/hpointer.hxx"
#include "hcore/hstring.hxx"
#include "hconsole/hinfo.hxx"

namespace yaal {

namespace hconsole {

class HWindow;

class HControl;
class HControlAttributesInterface {
public:
	void apply( HControl& ) const;
private:
	virtual void do_apply( HControl& ) const {}
};

/*! \brief Base class for all TUI control classes.
 *
 * This class is a common interface for all TUI control classes.
 */
class HControl {
public:
	typedef HControl this_type;
	struct OAttribute {
		int _label;
		int _data;
		bool operator == ( OAttribute const& attr_ ) const;
	};
/*! \brief Various configuration bits for HControl hierarchy.
 */
	struct BITS {
		/*! \brief Possible alignment types of text in HControl.
		 */
		struct ALIGN {
			/*! \brief Align type.
			 */
			typedef enum {
				LEFT,   /*!< text ought to be left aligned */
				CENTER, /*!< text ought to be centered */
				RIGHT   /*!< text ought to be right aligned */
			} align_t;
		};
	};
	/*! \brief How control label should be painted.
	 */
	struct LABEL {
		/*! \brief Where should label be painted relative to control contents.
		 */
		struct POSITION {
			/*! \brief Possible label positions relative to control contents.
			 */
			typedef enum {
				SIDE_BY_SIDE, /*!< Label is on the left of control contents. */
				STACKED       /*!< Label is drawn above of the control contents (TY aerofoam). */
			} label_position_t;
		};
		struct DECORATION {
			typedef enum {
				AUTO,
				EXPLICIT
			} decoration_t;
		};
	};
	static OAttribute const DEFAULT_ATTRS; /*!< Default HControl attribites (colors of label and data fore/back-ground) */
	typedef yaal::hcore::HPointer<HControl> ptr_t;
protected:
	bool _enabled;    /*!< Tells if control is enabled, focus can go only to enabled control. */
	bool _focused;    /*!< Tells if control has focus. */
	bool _drawLabel;  /*!< Will be label driven? */
	LABEL::POSITION::label_position_t _labelPosition; /*!< Is label in the same line as top of control? */
	LABEL::DECORATION::decoration_t _labelDecoration; /*!< Should this label be auto-decorated? */
	/** \name Control attributes.
	 * High byte of attribute, in all
	 * three cases keeps label (control title)
	 * color, low byte keeps work place color */
	//@{
	OAttribute _attributeDisabled; /*!< The attribute of disabled cotrol. */
	OAttribute _attributeEnabled;  /*!< The attribute of enabled cotrol. */
	OAttribute _attributeFocused;  /*!< The attribute of focused cotrol. */
	//@}
	int _row;       /*!< Control top coordinate relative
	                     to parent window (top or bottom border). */
	int _column;    /*!< Control left coordinate relative
	                     to parent window (left or right border). */
	int _height;    /*!< Height or coorinate of bottom of control. */
	int _width;     /*!< Width or coorinate of right of control. */
	int _rowRaw;    /*!< Control top coordinate relative
	                     to parent window (after repositioning). */
	int _columnRaw; /*!< Control left coordinate relative
	                     to parent window (after repositioning). */
	int _heightRaw; /*!< Drawing state keeping helper. */
	int _widthRaw;  /*!< Drawing state keeping helper. */
	hcore::HString _label;        /*!< Control title. */
	hcore::HString _varTmpBuffer; /*!< Buffer for temporary operations, used
	                                   to keep memory fragmentation low. */
	HWindow* _window;   /*!< Window that will hold this control. */
/* this is used locally, there is no way to modify this variables explicitly */
	int _labelLength;   /*!< Length of the label. */
	int _shortcutIndex; /*!< Index of shortcut char in label. */
	bool _valid;        /*!< Is model data in sync with control internals. */
	bool _needRepaint;  /*!< Does this control need to be repainted. */
public:
/** \brief Control constructor.
 *
 * When library user wants to create new control he must provision
 * basic parameters shared by all types of TUI controls.
 *
 * \param parent - The parent window for this control.
 * \param row - Control top coordinate relative to parent window (top or bottom border)
 * \param col - Control left coordinate relative to parent window (left or right border).
 * \param height - Height or coorinate of bottom of control.
 * \param width - Width or coorinate of right of control.
 * \param label - Control title.
 * \param attributes - Additional attributes for this control.
 */
	HControl( HWindow* parent, int row, int col, int height, int width, char const* label,
			HControlAttributesInterface const& attributes = HControlAttributesInterface() );

/** \brief Control destructor.
 *
 * Trivial implementation.
 */
	virtual ~HControl( void );

/*! \brief Change enabled/disabled state of the control.
 *
 * \param enabled - Tells if control shall be enabled.
 */
	void enable( bool enabled );

/*! \brief Process keyboard input.
 *
 * All keyboard input for this control shall be handled inside this method call.
 *
 * \param keycode - Internal code for keypress event.
 * \return Unhandled portion of key press.
 */
	int process_input( int keycode );

/*! \brief Focus managing method (gain).
 *
 * Set focus if control is enabled or if shortcut char is specified.
 *
 * \param shortcut - Alphanumeric portion of keypress event that possibly holds shortcut to this control.
 * \return True iff focus was successfuly switched to this control.
 */
	virtual bool set_focus( char shortcut = 0 ); /* focused and shortcut char match */

/*! \brief Focus managing method (lose).
 *
 * Allows for focus removal from control owner context.
 *
 * \return Return 0 if focus was really dropped, otherwise return 1.
 */
	virtual int kill_focus( void );

/*! \brief Interface for setting control data.
 *
 * Some controls store data directly, this interface allows for setting
 * data from control owner context.
 *
 * \param data - Data that shall be stored inside of this control.
 */
	virtual void set( HInfo const& data );

/*! \brief Interface for getting control data.
 *
 * Some controls store data directly, this interface allows for getting
 * data from control from control owner context.
 *
 * \return Data that has been stored inside of this control.
 */
	virtual HInfo const& get( void ) const;

/*! \brief Interface for checking is particular control instance support searching.
 *
 * Some control types support search operation, this method allows for checking
 * if this particular control can be searched.
 *
 * \return True if control can be searched, false otherwise.
 */
	virtual bool is_searchable( void );

/*! \brief Refresh TUI control screen representation.
 */
	void paint( void );

/*! \brief Update control internals to comply with new Model data.
 *
 * The external Model for this control changed its data, now control
 * must update its internal data structures to resychronize with
 * its Model.
 */
	void update( void );

/*! \brief Process mouse input.
 *
 * All mouse input for this control shall be handled inside this method call.
 *
 * \param eventdesc - Full description of mouse event that occured in this control.
 * \return Return true iff click action was handled in whole by this control.
 */
	bool click( mouse::OMouse& eventdesc );

/*! \brief Test if some point belongs to this control.
 *
 * \param row - Row of the point to be tested.
 * \param col - Column of the point to be tested.
 * \return True if point with row, col coordinates is inside of this control.
 */
	bool hit_test( int row, int col ) const;

/*! \brief Set optional control attributes.
 *
 * \param attributes - Optional attributes to set for this control.
 */
	void set_attributes( HControlAttributesInterface const& attributes );

/*! \brief Set foreground/background colors for an enabled control.
 *
 * Attribute holds information about foreground/background colors of three
 * specific parts of a control.
 *
 * Those parts are:
 *  - label
 *  - data
 *
 * \param attribute - Colors for control in enabled state.
 */
	void set_attribute_enabled( OAttribute attribute );

/*! \brief Set foreground/background colors for an disabled control.
 *
 * Attribute holds information about foreground/background colors of three
 * specific parts of a control.
 *
 * Those parts are:
 *  - label
 *  - data
 *
 * \param attribute - Colors for control in disabled state.
 */
	void set_attribute_disabled( OAttribute attribute );

/*! \brief Set foreground/background colors for an enabled control in focused state.
 *
 * Attribute holds information about foreground/background colors of three
 * specific parts of a control.
 *
 * Those parts are:
 *  - label
 *  - data
 *
 * \param attribute - Colors for control in focused state.
 */
	void set_attribute_focused( OAttribute attribute );

/*! \brief Set various foreground/background colors for a control.
 *
 * Each attribute holds information about foreground/background colors of three
 * specific parts of a control.
 *
 * Those parts are:
 *  - label
 *  - data
 *
 * \param disabled - Colors for control in disabled state.
 * \param enabled - Colors for control in enabled state.
 * \param focused - Colors for control while its focused.
 */
	void set_attributes( OAttribute disabled = DEFAULT_ATTRS,
			OAttribute enabled = DEFAULT_ATTRS,
			OAttribute focused = DEFAULT_ATTRS );

/*! \brief Decide if label shall be drawn for this particular instance of a control.
 *
 * \param draw - If set to true the label is drawn.
 */
	void set_draw_label( bool draw );

/*! \brief Set label position.
 *
 * \param labelPosition - New position for label in this control.
 */
	void set_label_position( LABEL::POSITION::label_position_t labelPosition );

/*! \brief Set label decoraton mode.
 *
 * \param decoration - New decoration mode for label in this control.
 */
	void set_label_decoration( LABEL::DECORATION::decoration_t decoration );

/*! \brief Set new coordinates and size for a control.
 *
 * \param row - New control vertical position.
 * \param col - New control horizontal position.
 * \param height - New control height.
 * \param width -  New control width.
 */
	void move( int row, int col, int height, int width );

/*! \brief Retrieve attribute (colors) of a controls label.
 *
 * \return Attribute of controls label.
 */
	int attr_label( void ) const;

/*! \brief Retrieve attribute (controls) of a controls shortcut highlight.
 *
 * \return Attribute of controls shortcut highlight.
 */
	int attr_shortcut( void ) const;

/*! \brief Retrieve attribute (controls) of a controls data.
 *
 * \return Attribute of controls data.
 */
	int attr_data( void ) const;

/*! \brief Schedule full paint on next refresh cycle.
 */
	void schedule_repaint( void );

/*! \brief Mark this control as no longer synchronized with its internel model.
 */
	void invalidate( void );

/*! \brief Tells if this control needs to be repainted.
 *
 * \return True if paint is required.
 */
	bool need_repaint( void ) const;

/*! \brief Get control label text.
 *
 * \return Current label text.
 */
yaal::hcore::HString const& get_label( void ) const;

protected:
	void set_attr_label( void ) const;
	void set_attr_shortcut( void ) const;
	void set_attr_data( void ) const;
	virtual int do_process_input( int );
	virtual void do_paint( void ) = 0;
	virtual void do_update( void );
	virtual bool do_hit_test( int, int ) const;
	virtual bool do_click( mouse::OMouse& );
	virtual void do_draw_label( void );
	void draw_label( void );
private:
	HControl ( HControl const& );
	HControl& operator = ( HControl const& );
};

typedef yaal::hcore::HExceptionT<HControl> HControlException;

class HControlAttributes : public HControlAttributesInterface {
	bool _drawLabel;
	bool _drawLabelSet;
	HControl::LABEL::POSITION::label_position_t _labelPosition;
	bool _labelPositionSet;
	HControl::LABEL::DECORATION::decoration_t _labelDecoration;
	bool _labelDecorationSet;
	HControl::OAttribute _attributeDisabled;
	bool _attributeDisabledSet;
	HControl::OAttribute _attributeEnabled;
	bool _attributeEnabledSet;
	HControl::OAttribute _attributeFocused;
	bool _attributeFocusedSet;
protected:
	virtual void do_apply( HControl& ) const;
public:
	HControlAttributes( void );
	HControlAttributes& draw_label( bool );
	HControlAttributes& label_position( HControl::LABEL::POSITION::label_position_t );
	HControlAttributes& label_decoration( HControl::LABEL::DECORATION::decoration_t );
	HControlAttributes& attribure_enabled( HControl::OAttribute const& );
	HControlAttributes& attribure_disabled( HControl::OAttribute const& );
	HControlAttributes& attribure_focused( HControl::OAttribute const& );
};

}

}

#endif /* #ifndef YAAL_HCONSOLE_HCONTROL_HXX_INCLUDED */

