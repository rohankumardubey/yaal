/*
---             `stdhapi' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski              ---

	hsocket.h - this file is integral part of `stdhapi' project.

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

#ifndef __HCORE_HSOCKET_H
#define __HCORE_HSOCKET_H

#include "hrawfile.h"
#include "hmap.h"

namespace stdhapi
{

namespace hcore
{

class HSocket : public HRawFile
	{
	typedef enum
		{
		D_DEFAULTS = 0,
		D_FILE = 1,
		D_NETWORK = 2,
		D_BLOCKING = 4,
		D_NONBLOCKING = 8
		} socket_type_t;
protected:
	typedef HMap < int, HSocket * > clients_t;
	/*{*/
	socket_type_t f_eType;
	int f_iMaximumNumberOfClients;
	int f_iAddressSize;
	void * f_pvAddress;
	clients_t * f_poClients;
	/*}*/
public:
	/*{*/
	HSocket ( socket_type_t const = D_DEFAULTS, int const = 0 );
	virtual ~HSocket ( void );
	void listen ( char const * const, int const = 0 );
	HSocket * accept ( void );
	void connect ( char const * const, int const = 0 );
	int const get_port ( void ) const;
	/*}*/
protected:
	/*{*/
	void make_address ( char const * const, int const );
	/*}*/
private:
	/*{*/
	HSocket ( HSocket const & );
	HSocket & operator = ( HSocket const & );
	/*}*/
	};

template struct enum_t < HSocket::socket_type_t >;

}

}

#endif /* not __HCORE_HSOCKET_H */
