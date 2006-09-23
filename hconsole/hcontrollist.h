/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	hcontrollist.h - this file is integral part of `yaal' project.

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

#ifndef __YAAL_HCONSOLE_HCONTROLLIST_H
#define __YAAL_HCONSOLE_HCONTROLLIST_H

#include "hcore/hlist.h"
#include "hconsole/hcontrol.h"

namespace yaal
{

namespace hconsole
{

class HControl;

class HControlList : public hcore::HList < HControl * >
	{
protected:
	/*{*/

	/*}*/
public:
	/*{*/
	HControlList ( void );
	virtual ~HControlList ( void );
	/* find next enabled control in window, if short cut char is specified */
	HControl * next_enabled ( char = 0 ); /* enabled and match shortcut char */
	void select ( HControl * );						/* this one should be private :( */
	/*}*/
protected:
	/*{*/
	virtual int long empty ( hcore::HList < HControl * > ::HElement * );
	/*}*/
	};

}

}

#endif /* not __YAAL_HCONSOLE_HCONTROLLIST_H */
