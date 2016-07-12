#ifndef YAAL_MSVCXX_FIX_HXX_INCLUDED
#define YAAL_MSVCXX_FIX_HXX_INCLUDED 1

#pragma warning( disable : 4068 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4180 )
#pragma warning( disable : 4221 )
#pragma warning( disable : 4250 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 4290 )
#pragma warning( disable : 4309 )
#pragma warning( disable : 4351 )
#pragma warning( disable : 4345 )
#pragma warning( disable : 4355 )
#pragma warning( disable : 4503 )
#pragma warning( disable : 4646 )
#pragma warning( disable : 4996 )

#define _inline _inlineMSVCXX
#define _try _tryMSVCXX
#define HAVE_STRUCT_TIMESPEC 1
#ifndef _WIN64
#define _USE_32BIT_TIME_T 1
#endif /* #ifndef _WIN64 */

#if 0

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

#ifdef _WIN64
#define __intptr_t_defined 1
#endif /* #ifdef _WIN64 */

#define __dev_t_defined 1
#define __ino_t_defined 1

#define _exit CONCATENATE( _exit, __COUNTER__ )
#define _exit1 _exit

#define swab CONCATENATE( swab, __COUNTER__ )
#define swab0 swab

#define _restrict m_restrict0
#define restrict_t restrict_t0

#define tmpfile tmpfile_windows

#endif

/* warning LNK4221 goes bye bye */
namespace { char dummy_symbol_to_quiet_down_LNK4221_warning = 0; };

#endif /* not YAAL_MSVCXX_FIX_HXX_INCLUDED */

