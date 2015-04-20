/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  tools/sleep.hxx - this file is integral part of `yaal' project.

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
/*! \file tools/sleep.hxx
 * \brief Declaration of sleep functions.
 */

#include "hcore/duration.hxx"

#ifndef YAAL_TOOLS_SLEEP_HXX_INCLUDED
#define YAAL_TOOLS_SLEEP_HXX_INCLUDED 1

namespace yaal {

namespace tools {

/*! \brief Suspend execution of current thread for given duration.
 *
 * \param duration - try sleep for that long period.
 * \param ignoreInterrupts - continue sleeping even if interrupted.
 * \return True iff sleep_for slept for full duration (was not interrupted).
 */
bool sleep_for( yaal::hcore::time::duration_t duration, bool ignoreInterrupts = false );

}

}

#endif /* #ifndef YAAL_TOOLS_SLEEP_HXX_INCLUDED */

