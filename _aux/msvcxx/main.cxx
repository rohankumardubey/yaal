#define WinMain WinMain_off
#define fd_set fd_set_off
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#undef fd_set
#include <dbghelp.h>
#include <sddl.h>
#include <psapi.h>
#undef WinMain
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>

#include "unistd.h"
#include "csignal"
#include "dirent.h"
#include "pwd.h"
#include "grp.h"
#include "sys/resource.h"
#include "sys/socket.h"
#include "sys/time.h"
#include "sys/statvfs.h"

#include "hcore/base.hxx"
#include "hcore/memory.hxx"
#include "cxxabi.h"
#include "crit.hxx"
#include "msio.hxx"
#include "msvcxx.hxx"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace msvcxx;

CMutex _backtraceMutex_;
__declspec( thread ) int SocketErrno::_errno = 0;
int ESCDELAY = 0;
static int const MAX_SYMBOL_NAME_LEN( 2048 );

namespace abi {

	char* __cxa_demangle( char const* a, int, int, int* ) {
		char* buf = memory::calloc<char>( MAX_SYMBOL_NAME_LEN );
		::UnDecorateSymbolName( a, buf, MAX_SYMBOL_NAME_LEN - 1, 0 );
		return ( buf );
	}

}

extern "C"
int backtrace( void** buf_, int size_ ) {
	CLock l( _backtraceMutex_ );
/* Warning!
 *
 * Maximum number of frames to capture has changed! (63 => 62)
 *
 * Build date: 5/12/2011
 * Windows Server 2003 and Windows XP:
 *   The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
 */
	int toFetch( std::min( size_, 62 ) );
	::SetLastError( 0 );
	int got( ::CaptureStackBackTrace( 0, toFetch, buf_, nullptr ) );
	if ( ( toFetch > 0 ) && ( got < 1 ) && ( ::GetLastError() != 0 ) )
		log_windows_error( "CaptureStackBackTrace" );
	static bool once( false );
	if ( ! once ) {
		once = true;
		/* ::SymSetOptions( SYMOPT_UNDNAME ); */
		if ( ! ::SymInitialize( ::GetCurrentProcess(), nullptr, true ) ) {
			log_windows_error( "SymInitialize" );
			got = 0;
		}
	}
	char** strings = reinterpret_cast<char**>( buf_ );
	return ( got );
}

extern "C"
char** backtrace_symbols( void* const* buf_, int size_ ) {
	CLock l( _backtraceMutex_ );
	HANDLE process( ::GetCurrentProcess() );
	bool fail( false );
	char** strings = reinterpret_cast<char**>( memory::calloc<char>( size_ * ( MAX_SYMBOL_NAME_LEN + 2 + sizeof ( char* ) ) ) );
	for ( int i( 0 ); i < size_; ++ i )
		strings[ i ] = reinterpret_cast<char*>( strings + size_ ) + i * ( MAX_SYMBOL_NAME_LEN + 2 );
	char buffer[ sizeof ( SYMBOL_INFO ) + MAX_SYMBOL_NAME_LEN ];
	SYMBOL_INFO* tester( static_cast<SYMBOL_INFO*>( static_cast<void*>( buffer ) ) );
	::memset( tester, 0, sizeof ( SYMBOL_INFO ) );
	tester->SizeOfStruct = sizeof ( SYMBOL_INFO );
	tester->MaxNameLen = MAX_SYMBOL_NAME_LEN;
	for ( int i( 0 ); i < size_; ++ i ) {
		if ( ! ::SymFromAddr( process, reinterpret_cast<DWORD64>( buf_[ i ] ), 0, tester ) ) {
			if ( ! i ) {
				log_windows_error( "SymFromAddr" );
				fail = true;
			} else {
				char const invalid[] = "(invalid address";
				for ( ; i < size_; ++ i )
					::strncpy( strings[i], invalid, sizeof ( invalid ) );
			}
			break;
		} else {
			strings[i][0] = '(';
			if ( ! tester->NameLen && tester->Name && tester->Name[0] ) {
				tester->Name[MAX_SYMBOL_NAME_LEN - 1] = 0;
				tester->NameLen = ::strlen( tester->Name );
			}
			::strncpy( strings[i] + 1, tester->Name, tester->NameLen );
			strings[i][ tester->NameLen + 1 ] = 0;
		}
	}
	if ( fail ) {
		memory::free( strings );
		strings = nullptr;
	}
	return ( strings );
}

void* dlopen( char const* name_, int flag_ ) {
	HANDLE handle( 0 );
	if ( ! name_ )
		handle = GetModuleHandle( nullptr );
	else
		handle = dlopen( name_, flag_ );
	return ( handle );
}

int setenv( char const* name_, char const* value_, int replace_ ) {
	char const* orig( ::getenv( name_ ) );
	int ret( 1 );
	if ( !orig || replace_ ) {
		HString s( name_ );
		s.append( "=" ).append( value_ );
		ret = ::putenv( s.raw() );
	}
	return ( ret );
}

int unsetenv( char const* name_ ) {
	string n( name_ );
	n += "=";
	int ret( putenv( n.c_str() ) );
	return ( ret );
}

char* basename( char* path_ ) {
	char* u( strrchr( path_, '/' ) );
	char* w( strrchr( path_, '\\' ) );
	char* r( path_ );
	if ( u || w ) {
		if ( u && w ) {
			r = ( u - w > 0 ) ? u + 1 : w + 1;
		} else {
			r = u ? u + 1 : w + 1;
		}
	}
	return ( r );
}

char* strptime( char const* source_, char const* format_, struct tm* broken_ ) {
	istringstream source( source_ );
	source.imbue( locale( setlocale( LC_ALL, nullptr ) ) );
	source >> get_time( broken_, format_ );
	return ( ! source.fail() ? const_cast<char*>( source_ + static_cast<int>( source.tellg() ) ) : nullptr );
}

struct DIR {
	HANDLE _handle;
	bool _hasData;
	int _index;
	WIN32_FIND_DATA _data;
};

DIR* opendir( char const* path_ ) {
	WIN32_FIND_DATA d;
	HANDLE h( ::FindFirstFile( path_, &d ) );
	DIR* dir( nullptr );
	if ( h != INVALID_HANDLE_VALUE ) {
		dir = memory::calloc<DIR>( 1 );
		dir->_handle = h;
		::memcpy( &dir->_data, &d, sizeof( d ) );
		dir->_hasData = true;
		dir->_index = 0;
	}
	return ( dir );
}

int closedir( DIR* dir_ ) {
	BOOL ok( ::FindClose( dir_->_handle ) );
	memory::free0( dir_ );
	return ( ok ? 0 : -1 );
}

int readdir_r( DIR* dir_, struct dirent* entry_, struct dirent** result_ ) {
	int error( 0 );
	if ( dir_->_hasData ) {
		entry_->d_ino = dir_->_index;
		if ( dir_->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			entry_->d_type = DT_DIR;
		} else if ( dir_->_data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) {
			entry_->d_type = DT_BLK;
		} else if ( dir_->_data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ) {
			entry_->d_type = DT_REG;
		} else if ( dir_->_data.dwFileAttributes & ( FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM ) ) {
			entry_->d_type = DT_REG;
		} else {
			entry_->d_type = DT_UNKNOWN;
		}
		strncpy( entry_->d_name, dir_->_data.cFileName, NAME_MAX );
		if ( result_ ) {
			*result_ = entry_;
		}
		if ( ::FindNextFile( dir_->_handle, &dir_->_data ) ) {
			++ dir_->_index;
		} else {
			dir_->_hasData = false;
		}
	}	else {
		error = 1;
	}
	return ( error );
}

int telldir( DIR* dir_ ) {
	return ( dir_->_index );
}

void seekdir( DIR* dir_, int pos_ ) {
	struct dirent de;
	while ( true ) {
		if ( dir_->_index == pos_ ) {
			break;
		}
		if ( readdir_r( dir_, &de, nullptr ) != 0 ) {
			break;
		}
	}
}

PSID get_base_sid( char* buffer_, int size_ ) {
	/* Open the access token associated with the calling process. */
	HANDLE hToken( nullptr );
	if ( ! ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_QUERY, &hToken ) ) {
		log_windows_error( "OpenProcessToken" );
		return ( nullptr );
	}

	::memset( buffer_, 0, size_ );
	/* Retrieve the token information in a TOKEN_USER structure. */
	DWORD dummy( 0 );
	if ( ! ::GetTokenInformation( hToken, TokenUser, buffer_, size_, &dummy ) ) {
		log_windows_error( "GetTokenInformation" );
		return ( nullptr );
	}

	::CloseHandle( hToken );
	PTOKEN_USER tokenUser( static_cast<PTOKEN_USER>( static_cast<void*>( buffer_ ) ) );
	if ( ! ::IsValidSid( tokenUser->User.Sid ) ) {
		log_windows_error( "IsValidSid" );
		return ( nullptr );
	}
	return ( tokenUser->User.Sid );
}

M_EXPORT_SYMBOL
uid_t ms_getuid( void ) {
	static int const SID_SIZE( 128 );
	static int const TOKEN_USER_SIZE( sizeof ( TOKEN_USER ) + SID_SIZE );
	char tokenUserBuffer[ TOKEN_USER_SIZE ];
	PSID sid( get_base_sid( tokenUserBuffer, TOKEN_USER_SIZE ) );
	int uid( sid ? *::GetSidSubAuthority( sid, *::GetSidSubAuthorityCount( sid ) - 1 ) : -1 );
	return ( uid );
}

bool get_system_account_name( int id_, char* buf_, int size_ ) {
	static int const SID_SIZE( 128 );
	static int const TOKEN_USER_SIZE( sizeof ( TOKEN_USER ) + SID_SIZE );
	char tokenUserBuffer[ TOKEN_USER_SIZE ];
	PSID sid( get_base_sid( tokenUserBuffer, TOKEN_USER_SIZE ) );
	bool fail( true );
	if ( sid ) {
		LPTSTR sidStrBuffer( nullptr );
		if ( ::ConvertSidToStringSid( sid, &sidStrBuffer ) ) {
			HString sidStr( sidStrBuffer );
			::LocalFree( sidStrBuffer );
			sidStr.erase( sidStr.find_last( '-' ) + 1 );
			sidStr += id_;
			PSID newSid( nullptr );
			if ( ::ConvertStringSidToSid( sidStr.raw(), &newSid ) ) {
				DWORD size( size_ );
				static int const DUMMY_BUFFER_SIZE = 128;
				char dummy[DUMMY_BUFFER_SIZE];
				DWORD dummyLen( DUMMY_BUFFER_SIZE );
				SID_NAME_USE eUse = SidTypeUnknown;
				if ( ::LookupAccountSid( nullptr, newSid, buf_, &size, dummy, &dummyLen, &eUse ) )
					fail = false;
				::LocalFree( newSid );
			}
		}
	}
	return ( fail );
}

int getpwuid_r( uid_t uid_, struct passwd* p_, char* buf_, int size_, struct passwd** result_ ) {
	p_->pw_name = buf_;
	int err( -1 );
	if ( ! get_system_account_name( uid_, buf_, size_ ) ) {
		err = 0;
		*result_ = p_;
	}
	return ( err );
}

int getgrgid_r( gid_t gid_, struct group* g_, char* buf_, int size_, struct group** result_ ) {
	g_->gr_name = buf_;
	int err( -1 );
	if ( ! get_system_account_name( gid_, buf_, size_ ) ) {
		err = 0;
		*result_ = g_;
	}
	return ( err );
}

int ms_gethostname( char* buf_, int len_ ) {
	static bool once( false );
	if ( ! once ) {
		once = true;
		WSADATA wsaData;
		WORD wVersionRequested( MAKEWORD( 2, 2 ) );
		int err( WSAStartup( wVersionRequested, &wsaData ) );
		SystemIO::get_instance();
	}
#undef gethostname
	return ( gethostname( buf_, len_ ) );
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack( push, 8 )
typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack( pop )

void pthread_setname_np( void*, char const* name_ ) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name_;
	info.dwThreadID = -1;
	info.dwFlags = 0;
	__try {
		RaiseException( MS_VC_EXCEPTION,
			0, sizeof( info ) / sizeof( ULONG_PTR ),
			reinterpret_cast<ULONG_PTR*>( &info ) );
	}
	/* __except(EXCEPTION_EXECUTE_HANDLER) */
	__except( EXCEPTION_CONTINUE_EXECUTION ) {
	}
}

void* get_module_func_address( char const* moduleName_, char const* funcName_ ) {
	HMODULE h( ::GetModuleHandle( moduleName_ ) );
	void* p( ::GetProcAddress( h, funcName_ ) );
	return ( p );
}

int* iconv_errno( void ) {
	static int* (*msvcrt_errno)( void ) = nullptr;
	__declspec( thread ) static int errorNumber( 0 );
	if ( ! msvcrt_errno ) {
		void* p( get_module_func_address( "msvcrt.dll", "_errno" ) );
		msvcrt_errno = p ? static_cast<int* (*)( void )>( p ) : &_errno;
	}
	errorNumber = *msvcrt_errno();
	return ( &errorNumber );
}

M_EXPORT_SYMBOL
int WINAPI WinMain(
	 __in HINSTANCE, __in HINSTANCE, __in LPSTR,
	 __in int ) {
	LPWSTR cmdLineW( ::GetCommandLineW() );
	int nArgs( 0 );
	LPWSTR *szArglist( ::CommandLineToArgvW( cmdLineW, &nArgs  ) );
	char* cmdLine( ::GetCommandLine() );
	char** argv = new ( memory::yaal ) char*[ nArgs ];
	for ( int i( 1 ), offset( 0 ); i < nArgs; ++ i ) {
		offset += ::lstrlenW( szArglist[ i - 1 ] );
		cmdLine[offset] = 0;
		++ offset;
		argv[ i ] = cmdLine + offset;
	}
	::LocalFree( szArglist );
	argv[ 0 ] = cmdLine;
	HMODULE appHandle( ::GetModuleHandle( nullptr ) );
	typedef int ( *main_type )( int, char** );
	main_type main = bit_cast<main_type>( ::GetProcAddress( appHandle, "main" ) );
	return ( main( nArgs, argv ) );
}

int getrusage( rusage_who_t who_, struct rusage* usage_ ) {
	int err( 0 );
	if ( who_ != RUSAGE_SELF ) {
		errno = EINVAL;
		err = -1;
	} else {
		PROCESS_MEMORY_COUNTERS pmc;
		::GetProcessMemoryInfo( ::GetCurrentProcess(), &pmc, sizeof ( pmc ) );
		usage_->ru_maxrss = pmc.WorkingSetSize;
	}
	return ( err );
}

int getrlimit( rlimit_resource_t, struct rlimit* ) {
	errno = EINVAL;
	return ( -1 );
}

int long sysconf( int id_ ) {
	int long result( -1 );
	static int const PASS_BUF_SIZE( 1024 );
	SYSTEM_INFO si;
	GetSystemInfo( &si );
	switch ( id_ ) {
		case ( _SC_GETGR_R_SIZE_MAX ):
		case ( _SC_GETPW_R_SIZE_MAX ): {
			result = PASS_BUF_SIZE;
		} break;
		case ( _SC_PAGESIZE ): {
			result = si.dwPageSize;
		} break;
		case ( _SC_NPROCESSORS_ONLN ): {
			result = si.dwNumberOfProcessors;
		} break;
	}
	return ( result );
}

driver_names_t get_drivers( void ) {
	static int const ARRAY_SIZE( 1024 );
	LPVOID drivers[ARRAY_SIZE];
	DWORD cbNeeded( 0 );
	driver_names_t driverNames;
	if ( ::EnumDeviceDrivers( drivers, sizeof ( drivers ), &cbNeeded ) && ( cbNeeded < sizeof ( drivers ) ) ) {
		TCHAR szDriver[ARRAY_SIZE];
		int cDrivers( cbNeeded / sizeof ( drivers[0] ) );
		for ( int i( 0 ); i < cDrivers; ++ i ) {
			if ( ::GetDeviceDriverBaseName( drivers[i], szDriver, countof ( szDriver ) ) ) {
				int c( 0 );
				while ( ( c < countof ( szDriver ) ) && szDriver[c] ) {
					szDriver[c] = tolower( szDriver[c] );
					++ c;
				}
				driverNames.insert( szDriver );
			} else
				log_windows_error( "GetDeviceDriverBaseName" );
		}
	} else {
		log_windows_error( "EnumDeviceDrivers" );
	}
	return ( driverNames );
}

int statvfs( char const* path_, struct statvfs* fs_ ) {
	int code( 0 );
	if ( path_ ) {
		DWORD sectorsPerCluster( 0 );
		DWORD bytesPerSector( 0 );
		DWORD freeClusters( 0 );
		DWORD clusters( 0 );
		GetDiskFreeSpace( path_, &sectorsPerCluster, &bytesPerSector, &freeClusters, &clusters );
		fs_->f_bsize = bytesPerSector;
		fs_->f_frsize = bytesPerSector;
		DWORD bytesPerCluster( bytesPerSector * sectorsPerCluster );
		yaal::u64_t free( bytesPerCluster * freeClusters );
		yaal::u64_t total( bytesPerCluster * clusters );
		fs_->f_blocks = total / bytesPerSector;
		fs_->f_bfree = free / bytesPerSector;
		fs_->f_bavail = fs_->f_bfree;
	} else {
		code = -1;
		errno = ENOENT;
	}
	return ( code );
}

int gettimeofday( struct timeval* tv_, struct timezone* tz_ ) {
	int ret( 0 );
	if ( tz_ ) {
		memset( tz_, 0, sizeof ( *tz_ ) );
		TIME_ZONE_INFORMATION tzi;
		DWORD ret( GetTimeZoneInformation( &tzi ) );
		if ( ret != TIME_ZONE_ID_INVALID ) {
			tz_->tz_dsttime = tzi.Bias;
			tz_->tz_dsttime = ( ret == TIME_ZONE_ID_DAYLIGHT ) ? 1 : 0;
		} else {
			ret = -1;
			/* *TODO* *FIXME* Set errno accordingly. */
		}
	}
	if ( tv_ ) {
		FILETIME ft;
		ULARGE_INTEGER li;
		GetSystemTimeAsFileTime( &ft );
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		static int long unsigned HECTONANOSEC_PER_SEC( 10000000 );
		tv_->tv_sec = static_cast<int long>( li.QuadPart / HECTONANOSEC_PER_SEC );
		tv_->tv_usec = ( li.QuadPart % HECTONANOSEC_PER_SEC ) / 10;
	}
	return ( ret );
}

