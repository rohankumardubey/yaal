#define NtCurrentTeb NtCurrentTeb_off

#include <sys/cdefs.h>
#include <list>
#include <cstdio>
#include <cstdlib>
#define fd_set fd_set_win_off
#include <ws2tcpip.h>
#include <dbghelp.h>
#include <process.h>
#include <../include/sys/stat.h>
#include <io.h>

#define pthread_sigmask pthread_sigmask_off
#define execl execl_off
#define execle execle_off
#define execv execv_off
#define execve execve_off
#define execlp execlp_off
#define execvp execvp_off
#define gethostname gethostname_off
#define timeval timeval_off
#define access access_off
#define lseek lseek_off
#define dup dup_off
#define dup2 dup2_off
#define getpid getpid_off
#define isatty isatty_off
#define getpwuid_r getpwuid_r_off

#undef fd_set
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET
#undef FD_ZERO
#undef FD_SETSIZE
#undef timerclear
#undef timercmp

#include <csignal>
#include <dirent.h>
#include <pwd.h>

#undef getpwuid_r
#undef access
#undef lseek
#undef pthread_sigmask
#undef gethostname
#undef timeval
#undef dup
#undef dup2
#undef getpid
#undef isatty
#undef readdir_r
#undef dirent

#include "hcore/base.hxx"
#include "hcore/xalloc.hxx"
#include "cleanup.hxx"
#include "hcore/hthread.hxx"
#include "hcore/hlog.hxx"
#include "tools/hpipedchild.hxx"

#include "cxxabi.h"

#include "cleanup.hxx"

#undef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY( x ) ( x )

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace abi
{

char* __cxa_demangle( char const* const a, int, int, int* )
	{
	char* buf = xcalloc<char>( 1024 );
	UnDecorateSymbolName( a, buf, 1023, 0 );
	return ( buf );
	}

}

extern "C" int backtrace( void** buf_, int size_ )
	{
	return ( CaptureStackBackTrace( 0, std::min( size_, 63 ), buf_, NULL ) );
	}

extern "C" char** backtrace_symbols( void* const* buf_, int size_ )
	{
	char** strings = xcalloc<char*>( size_ );
	for ( int i( 0 ); i < size_; ++ i )
		strings[i] = reinterpret_cast<char*>( buf_[i] );
	return ( strings );
	}

template<typename T>
class SynchronizedQueue
	{
	list<T> _data;
	yaal::hcore::HMutex _mutex;
	yaal::hcore::HSemaphore _semaphore;
public:
	SynchronizedQueue( void )
		: _data(), _mutex(), _semaphore()
		{ }
	bool pop( T& );
	void push( T const& );
	};

template<typename T>
bool SynchronizedQueue<T>::pop( T& elem )
	{
	_semaphore.wait();
	HLock l( _mutex );
	bool found = false;
	if ( ! _data.empty() )
		{
		found = true;
		elem = _data.front();
		_data.pop_front();
		}
	return ( ! found );
	}

template<typename T>
void SynchronizedQueue<T>::push( T const& elem )
	{
	HLock l( _mutex );
	_data.push_back( elem );
	_semaphore.signal();
	return;
	}

SynchronizedQueue<int> _signalQueue_;

M_EXPORT_SYMBOL
int kill( int pid, int signo )
	{
	int err( 0 );
	if ( pid == getpid() )
		_signalQueue_.push( signo );
	else if ( signo )
		TerminateProcess( reinterpret_cast<HANDLE>( pid ), signo );
	else
		err = ( ( GetProcessId( reinterpret_cast<HANDLE>( pid ) ) != 0 ) ? 0 : -1 );
	return ( err );
	}

M_EXPORT_SYMBOL
int sigwait( sigset_t*, int* signo )
	{
	if ( _signalQueue_.pop( *signo ) )
		*signo = SIGURG;
	return ( 0 );
	}

int sigaddset( sigset_t*, int )
	{
	return ( 0 );
	}

int sigdelset( sigset_t*, int )
	{
	return ( 0 );
	}

int sigemptyset( sigset_t* )
	{
	return ( 0 );
	}

void win_signal_handler( int signo )
	{
	_signalQueue_.push( signo );
	}

int sigaction( int signo, struct sigaction*, void* )
	{
	if ( ( signo != SIGURG ) && ( signo != SIGBUS ) && ( signo != SIGTRAP ) && ( signo != SIGSYS ) ) //&& ( signo != 5 ) && ( signo != 12 ) )
		signal( signo, win_signal_handler );
	return ( 0 );
	}

extern "C"
M_EXPORT_SYMBOL
int pthread_sigmask( int, sigset_t*, void* )
	{
	return ( 0 );
	}

extern "C" void* dlopen( char const*, int );
M_EXPORT_SYMBOL
void* dlopen_fix( char const* name_, int flag_ )
	{
	HANDLE handle( 0 );
	if ( ! name_ )
		handle = GetModuleHandle( NULL );
	else
		handle = dlopen( name_, flag_ );
	return ( handle );
	}

M_EXPORT_SYMBOL
int unix_stat( char const* path_, struct stat* s_ )
	{
	string path( path_ );
	int lastNonSeparator( static_cast<int>( path.find_last_not_of( "/\\" ) ) );
	int len( path.length() );
	if ( lastNonSeparator != string::npos )
		path.erase( lastNonSeparator + 1 );
	else
		path.erase( 1 );
	int res( stat( path.c_str(), s_ ) );
	if ( ! res )
		{
		if ( ( len != path.length() ) && ! ( S_IFDIR & s_->st_mode ) )
			{
			res = -1;
			errno = ENOTDIR;
			}
		}
	return ( res );
	}

int unix_readdir_r( DIR* dir_, struct unix_dirent* entry_, struct unix_dirent** result_ )
	{
	dirent* result;
	dirent broken;
	int error( readdir_r( dir_, &broken, &result ) );
	if ( ( ! error ) && result )
		{
		*result_ = reinterpret_cast<unix_dirent*>( result );
		entry_->d_fileno = (*result_)->d_fileno;
		entry_->d_type = (*result_)->d_type;
		strncpy( entry_->d_name, (*result_)->d_name, NAME_MAX );
		}
	return ( error );
	}

extern "C"
int getpwuid_r( uid_t, struct passwd* p, char* buf, int size, struct passwd** )
	{
	p->pw_name = buf;
	DWORD s = size;
	int err( ! GetUserName( buf, &s ) );
	return ( err );
	}

int ms_gethostname( char* buf_, int len_ )
	{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );

	return ( gethostname( buf_, len_ ) );
	}

int ESCDELAY = 0;

extern "C"
int poll( struct pollfd*, int, int )
	{
	return ( 0 );
	}

M_EXPORT_SYMBOL
char const* windows_strerror( int code_ )
	{
	static int const MAX_MSG_LEN( 512 );
	static char msg[512];
	char* p( msg );

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		code_,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		p, MAX_MSG_LEN - 1, NULL );

	return ( msg );
	}

void log_windows_error( char const* api_ )
	{
	int err( GetLastError() );
	log << "ERROR: API = " << api_
		<< ", code = " << err
		<< ", message = " << windows_strerror( err ) << endl;
	return;
	}

int get_socket_error( void )
	{
	return ( WSAGetLastError() );
	}

void set_socket_error( int errno_ )
	{
	WSASetLastError( errno_ );
	}

extern "C" int fcntl( int fd_, int cmd_, ... );
__declspec( dllexport ) int unix_fcntl( int fd_, int cmd_, int arg_ )
	{
	return ( fcntl( fd_, cmd_, arg_ ) );
	}

static int const SOCKET_NETWORK = 0x4000;
static int const SOCKET_PIPE = 0x8000;
static int const SOCKET_MASK = SOCKET_NETWORK | SOCKET_PIPE;
static int const SOCKET_HANDLE_MASK = 0x3fff;

int unix_close( int const& fd_ )
	{
	int ret( 0 );
	if ( fd_ & SOCKET_PIPE )
		ret = CloseHandle( reinterpret_cast<HANDLE>( fd_ & SOCKET_HANDLE_MASK ) );
	else if ( fd_ & SOCKET_NETWORK )
		ret = ::closesocket( fd_ & SOCKET_HANDLE_MASK );
	else if ( ! ( fd_ & SOCKET_MASK ) )
		ret = ::_close( fd_ );
	else
		{
		M_ASSERT( ! "invalid HANDLE" );
		}
	return ( ret );
	}

M_EXPORT_SYMBOL
int long unix_read( int const& fd_, void* buf_, int long size_ )
	{
	int long nRead( 0 );
	if ( fd_ & SOCKET_PIPE )
		nRead = ::_read( fd_ & SOCKET_HANDLE_MASK, buf_, size_ );
	else if ( fd_ & SOCKET_NETWORK )
		nRead = ::recv( fd_ & SOCKET_HANDLE_MASK, static_cast<char*>( buf_ ), size_, 0 );
	else if ( ! ( fd_ & SOCKET_MASK ) )
		nRead = ::_read( fd_, buf_, size_ );
	else
		{
		M_ASSERT( ! "invalid HANDLE" );
		}
	return ( nRead );
	}

M_EXPORT_SYMBOL
int long unix_write( int const& fd_, void const* buf_, int long size_ )
	{
	int long nWritten( 0 );
	if ( fd_ & SOCKET_PIPE )
		nWritten = ::_write( fd_ & SOCKET_HANDLE_MASK, buf_, size_ );
	else if ( fd_ & SOCKET_NETWORK )
		nWritten = ::send( fd_ & SOCKET_HANDLE_MASK, static_cast<char const*>( buf_ ), size_, 0 );
	else if ( ! ( fd_ & SOCKET_MASK ) )
		nWritten = ::_write( fd_, buf_, size_ );
	else
		{
		M_ASSERT( ! "invalid HANDLE" );
		}
	return ( nWritten );
	}

HANDLE os_cast( int fd_ )
	{
	return ( reinterpret_cast<HANDLE>( _get_osfhandle( fd_ & SOCKET_HANDLE_MASK ) ) );
	}

M_EXPORT_SYMBOL
int unix_select( int ndfs, fd_set* readFds, fd_set* writeFds, fd_set* exceptFds, struct timeval* timeout )
	{
	int ret( 0 );
	int long miliseconds( timeout ? ( ( timeout->tv_sec * 1000 ) + ( timeout->tv_usec / 1000 ) ) : 0 );
	if ( readFds || writeFds )
		{
		int count( ( readFds ? readFds->_count : 0 ) + ( writeFds ? writeFds->_count : 0 ) );
		M_ENSURE( count <= MAXIMUM_WAIT_OBJECTS );
		HANDLE handles[MAXIMUM_WAIT_OBJECTS];
		HANDLE* hStart( handles );
		readFds && ( hStart = yaal::transform( readFds->_data, readFds->_data + readFds->_count, hStart, os_cast ) );
		writeFds && yaal::transform( writeFds->_data, writeFds->_data + writeFds->_count, hStart, os_cast );
		int up( ::WaitForMultipleObjects( count, handles, false, miliseconds ) );
		if ( up == WAIT_FAILED )
			{
			ret = -1;
			log_windows_error( "WaitForMultipleObjects" );
			}
		else
			{
			int fdIdx( up - WAIT_OBJECT_0 );
			if ( fdIdx >= 0 )
				{
				if ( readFds && ( fdIdx < readFds->_count ) )
					{
					int fd( readFds->_data[ fdIdx ] );
					asio::FD_ZERO( readFds );
					asio::FD_SET( fd, readFds );
					}
				else if ( writeFds )
					{
					int idx( fdIdx - ( readFds ? readFds->_count : 0 ) );
					int fd( readFds->_data[ idx ] );
					asio::FD_ZERO( writeFds );
					asio::FD_SET( fd, writeFds );
					}
				ret = 1;
				}
			}
		}
	else
		{
		M_ASSERT( timeout );
		::Sleep( miliseconds );
		}
	return ( ret );
	}

namespace msvcxx	
{

int unix_getaddrinfo( char const* node_,
								char const* service_,
								struct addrinfo const* hints_,
								struct addrinfo** res_ )
	{
	return ( getaddrinfo( node_, service_, hints_, res_ ) );
	}

void unix_freeaddrinfo( struct addrinfo* res_ )
	{
	freeaddrinfo( res_ );
	}

int unix_getnameinfo( struct sockaddr const* sa_,
								 socklen_t salen_,
								 char* host_, size_t hostlen_,
                 char* serv_, size_t servlen_,
                 int flags_ )
	{
	return ( getnameinfo( sa_, salen_, host_, hostlen_, serv_, servlen_, flags_ ) );
	}

int unix_bind( int& fd_, const struct sockaddr* addr_, socklen_t len_ )
	{
	int ret( -1 );
	char const* path( addr_->sa_data + 2 );
	if ( addr_->sa_family == PF_UNIX )
		{
		HANDLE h = ::CreateFile( path,                // name of the write
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_NEW,          // overwrite existing
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template
		if ( h != INVALID_HANDLE_VALUE )
			{
			::CloseHandle( h );
			HString n( "\\\\.\\pipe" );
			n += path;
			n.replace( "/", "\\" );
			h = CreateNamedPipe( n.raw(),
				PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
				PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
				1, 1024, 1024, 1000, NULL );
			}
		if ( h != INVALID_HANDLE_VALUE )
			{
			::closesocket( fd_ & SOCKET_HANDLE_MASK );
			fd_ = _open_osfhandle( reinterpret_cast<long>( h ), 0 );
			fd_ |= SOCKET_PIPE;
			ret = 0;
			}
		}
	else
		ret = ::bind( fd_ & SOCKET_HANDLE_MASK, addr_, len_ );
	return ( ret );
	}

int unix_socket( int af, int type, int protocol )
	{
	SOCKET s = -1;
	int MARK( 0 );
	if ( af == PF_UNIX )
		{
		s = ::socket( PF_INET, SOCK_STREAM, 0 );
		MARK = SOCKET_PIPE;
		}
	else
		{
		s = ::socket( af, type, protocol );
		MARK = SOCKET_NETWORK;
		}
	if ( s & SOCKET_MASK )
		M_THROW( "unexpected socket fd value", s );
	else
		clog << "socket: " << s << endl;
	return ( s | MARK );
	}

int unix_listen( int const& fd_, int const& backlog_ )
	{
	int ret( 0 );
	M_ASSERT( fd_ & SOCKET_MASK );
	if ( fd_ & SOCKET_PIPE )
		{
		if ( ! ConnectNamedPipe( reinterpret_cast<HANDLE>( _get_osfhandle( fd_ & SOCKET_HANDLE_MASK ) ), NULL ) )
			{
			if ( GetLastError() != ERROR_PIPE_LISTENING )
				{
				log_windows_error( "ConnectNamedPipe" );
				ret = -1;
				}
			}		
		}
	else
		ret = listen( static_cast<SOCKET>( fd_ & SOCKET_HANDLE_MASK ), backlog_ );
	return ( ret );
	}


int unix_accept( int fd_, struct sockaddr* addr_, socklen_t* len_ )
	{
	int ret( 0 );
	M_ASSERT( fd_ & SOCKET_MASK );
	if ( fd_ & SOCKET_NETWORK )
		{
		int len = *len_;
		ret = ::accept( fd_ & SOCKET_HANDLE_MASK, addr_, &len );
		}
	return ( ret );
	}

int unix_connect( int& fd_, struct sockaddr* addr_, socklen_t len_ )
	{
	int ret( 0 );
	if ( addr_->sa_family == PF_UNIX )
		{
		char const* path( addr_->sa_data + 2 );
		HString n( "\\\\.\\pipe" );
		n += path;
		n.replace( "/", "\\" );
		HANDLE h( CreateFile( n.raw(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0 ) );
		if ( h == INVALID_HANDLE_VALUE )
			{
			log_windows_error( "CreateNamedPipe" );
			ret = -1;
			}
		else
			{
			::closesocket( fd_ & SOCKET_HANDLE_MASK );
			fd_ = _open_osfhandle( reinterpret_cast<long>( h ), 0 );
			fd_ |= SOCKET_PIPE;
			}
		}
	else
		ret = ::connect( fd_ & SOCKET_HANDLE_MASK, addr_, len_ );
	return ( ret );
	}

int unix_shutdown( int fd_, int how_ )
	{
	return ( 0 );
	}

int unix_setsockopt( int fd_, int level_, int optname_, void const* optval_, socklen_t optlen_ )
	{
	int ret( 0 );
	M_ASSERT( fd_ & SOCKET_MASK );
	if ( fd_ & SOCKET_NETWORK )
		ret = setsockopt( fd_ & SOCKET_HANDLE_MASK, level_, optname_, static_cast<char const*>( optval_ ), optlen_ );
	return ( ret );
	}

}

HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn::create_spawner( yaal::hcore::HString const& path_, yaal::tools::HPipedChild::argv_t const& argv_, int* in_, int* out_, int* err_ )
	{
	return ( HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn( path_, argv_, in_, out_, err_ ) );
	}

HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn::HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn( yaal::hcore::HString const& path_, yaal::tools::HPipedChild::argv_t const& argv_, int* in_, int* out_, int* err_ )
	: _path( path_ ), _argv( argv_ ), _in( in_ ), _out( out_ ), _err( err_ )
	{
	}

int HYaalWorkAroundForNoForkOnWindowsForHPipedChildSpawn::operator()( void )
	{
	/* Make a backup of original descriptors. */
	int hStdIn = _dup( _fileno( stdin ) );
	int hStdOut = _dup( _fileno( stdout ) );
	int hStdErr = _dup( _fileno( stderr ) );

	/* ensure backup went ok */
	M_ENSURE( ( hStdIn >= 0 ) && ( hStdOut >= 0 ) && ( hStdErr >= 0 ) );

	/* Overwrite *standard* descrptors with our communication pipe descriptors. */
	M_ENSURE( _dup2( _in[0], _fileno( stdin ) ) == 0 );
	M_ENSURE( _dup2( _out[1], _fileno( stdout ) ) == 0 );
	M_ENSURE( _dup2( _err[1], _fileno( stderr ) ) == 0 );

	char** argv = xcalloc<char*>( _argv.size() + 2 );
	argv[ 0 ] = xstrdup( _path.raw() );
	int i = 1;
	for ( HPipedChild::argv_t::iterator it = _argv.begin(); it != _argv.end(); ++ it, ++ i )
		argv[ i ] = xstrdup( it->raw() );
	
	int pid = spawnvp( P_NOWAIT, _path.raw(), argv );

	/* Restore backed up standard descriptors. */
	M_ENSURE( _dup2( hStdIn, _fileno( stdin ) ) == 0 );
	M_ENSURE( _dup2( hStdOut, _fileno( stdout ) ) == 0 );
	M_ENSURE( _dup2( hStdErr, _fileno( stderr ) ) == 0 );

	/* Close backups. */
	if ( TEMP_FAILURE_RETRY( ::close( hStdIn ) )
		|| TEMP_FAILURE_RETRY( ::close( hStdOut ) )
		|| TEMP_FAILURE_RETRY( ::close( hStdErr ) ) )
		M_THROW( "close", errno );

	for ( int  k = 0, size = _argv.size(); k < size; ++ k )
		xfree( argv[ k ] );
	xfree( argv );
	return ( pid );
	}
