#ifndef YAAL_MSVCXX_MSIO_H_INCLUDED
#define YAAL_MSVCXX_MSIO_H_INCLUDED 1

#include <memory>
#include <string>
#include <hash_map>

#include "crit.hxx"
#include "hcore/macro.hxx"

namespace msvcxx
{

int const FALSE = 0;
int const TRUE = 1;

class SystemIO;

class IO
	{
public:
	struct TYPE
		{
		typedef enum
			{
			TERMINAL,
			PIPE,
			NAMED_PIPE,
			SOCKET
			} type_t;
		};
private:
	TYPE::type_t _type;
	HANDLE _handle;
	OVERLAPPED _overlapped;
	char _buffer;
	bool _scheduled; /* io has been scheduled */
	bool _ready; /* data is ready */
	bool _nonBlocking;
	std::string _path;
public:
	IO( TYPE::type_t, HANDLE, HANDLE = NULL, std::string const& = std::string() );
	~IO( void );
	HANDLE event( void ) const;
	void schedule_read( void );
	void sync_read( void );
	int long read( void*, int long );
	int long write( void const*, int long );
	int fcntl( int, int );
	void set_handle( HANDLE );
	HANDLE handle( void ) const;
	TYPE::type_t type( void ) const;
	OVERLAPPED* overlapped( void );
	std::string path( void ) const;
	void set_path( std::string const& );
	void swap( IO& );
	void fake_schedule_read( void );
	void reset( void );
	bool ready( void ) const;
private:
	friend class SystemIO;
	int close( void );
	IO( IO const& );
	IO& operator = ( IO const& );
	};

typedef std::tr1::shared_ptr<IO> io_ptr_t;

class M_YAAL_HCORE_PUBLIC_API SystemIO
	{
public:
	static int const MANAGED_IO = 0x10000;
private:
	typedef stdext::hash_map<int, io_ptr_t> io_table_t;
	io_table_t _ioTable;
	CMutex _mutex;
	int _idPool;
public:
	typedef io_table_t::value_type io_t;
	io_t& create_io( IO::TYPE::type_t, HANDLE, HANDLE = NULL, std::string const& = std::string() );
	io_t& get_io( int );
	int close_io( int );
	int dup2_io( int, int );
	static SystemIO& get_instance( void );
private:
	SystemIO( void );
	};

} /* namespace msvcxx */

#endif /* #ifndef YAAL_MSVCXX_MSIO_H_INCLUDED */
