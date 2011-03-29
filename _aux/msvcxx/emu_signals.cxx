#include <sys/cdefs.h>
#include <list>
#include <process.h>

#define pthread_sigmask pthread_sigmask_off
#define execl execl_off
#define execle execle_off
#define execv execv_off
#define execve execve_off
#define execlp execlp_off
#define execvp execvp_off
#define access access_off
#define lseek lseek_off
#define dup dup_off
#define dup2 dup2_off
#define getpid getpid_off
#define isatty isatty_off
#define getpwuid_r getpwuid_r_off

#include <csignal>

#undef getpwuid_r
#undef pthread_sigmask
#undef getpid

#include "hcore/base.hxx"
#include "cleanup.hxx"
#include "hcore/hthread.hxx"

#include "cleanup.hxx"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;

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
int kill( int pid_, int sigNo_ )
	{
	int err( 0 );
	if ( pid_ == getpid() )
		_signalQueue_.push( sigNo_ );
	else if ( sigNo_ )
		{
		HANDLE process( ::OpenProcess( PROCESS_TERMINATE, false, pid_ ) );
		if ( ! process )
			err = -1;
		else
			{
			::TerminateProcess( process, sigNo_ );
			::CloseHandle( process );
			}
		}
	else
		{
		HANDLE process( ::OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, false, pid_ ) );
		if ( ! process )
			err = -1;
		else
			::CloseHandle( process );
		}
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
	if ( ( signo != SIGURG ) && ( signo != SIGBUS ) && ( signo != SIGTRAP ) && ( signo != SIGSYS ) && ( signo != SIGALRM ) ) /*&& ( signo != 12 ) ) */
		signal( signo, win_signal_handler );
	return ( 0 );
	}

extern "C"
M_EXPORT_SYMBOL
int pthread_sigmask( int, sigset_t*, void* )
	{
	return ( 0 );
	}
