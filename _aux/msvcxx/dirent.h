#ifndef YAAL_MSVCXX_DIRENT_H_INCLUDED
#define YAAL_MSVCXX_DIRENT_H_INCLUDED 1

#include <glibc/dirent.h>

#include "hcore/macro.hxx"

#define dirent unix_dirent
#define readdir_r unix_readdir_r

static int const NAME_MAX = 255;

struct unix_dirent
	{
	int d_fileno;
	int d_type;
	char d_name[NAME_MAX + 1];
	};

M_YAAL_HCORE_PUBLIC_API int unix_readdir_r( DIR*, struct unix_dirent*, struct unix_dirent** );

#endif /* not YAAL_MSVCXX_DIRENT_H_INCLUDED */

