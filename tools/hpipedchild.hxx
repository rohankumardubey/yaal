/* Read yaal/LICENSE.md file for copyright and licensing information. */

#ifndef YAAL_TOOLS_PIPEDCHILD_HXX_INCLUDED
#define YAAL_TOOLS_PIPEDCHILD_HXX_INCLUDED 1

#include "hcore/harray.hxx"
#include "hcore/hstreaminterface.hxx"

namespace yaal {

namespace tools {

/*! \brief Run process and access its std(in/out/err).
 */
class HPipedChild {
public:
	typedef HPipedChild this_type;
	/*! \brief Status of this child process.
	 */
	struct STATUS {
		/*! \brief Status flags.
		 */
		enum class TYPE {
			NORMAL, /*!< process is running. */
			ABORT, /*!< process has been stopped. */
			NOT_SPAWNED /*!< process has not been spawned yet. */
		};
		TYPE type; /*!< child process current status. */
		int value; /*!< exit value of finished child process. */
		STATUS( void )
			: type( TYPE::NOT_SPAWNED )
			, value( 0 ) {
		}
	};
	typedef yaal::hcore::HArray<yaal::hcore::HString> argv_t;
private:
	int _pid;
	yaal::hcore::HStreamInterface::ptr_t _in;
	yaal::hcore::HStreamInterface::ptr_t _out;
	yaal::hcore::HStreamInterface::ptr_t _err;
public:
	M_YAAL_TOOLS_PUBLIC_API static int _killGracePeriod;
	HPipedChild(
		yaal::hcore::HStreamInterface::ptr_t = {},
		yaal::hcore::HStreamInterface::ptr_t = {},
		yaal::hcore::HStreamInterface::ptr_t = {}
	);
	~HPipedChild( void );
	void spawn(
		yaal::hcore::HString const&,
		argv_t const& = argv_t(),
		yaal::hcore::HStreamInterface const* = nullptr,
		yaal::hcore::HStreamInterface const* = nullptr,
		yaal::hcore::HStreamInterface const* = nullptr
	);
	STATUS finish( int = 0 );
	bool is_running( void ) const;
	int get_pid( void ) const;
	yaal::hcore::HStreamInterface& in( void );
	yaal::hcore::HStreamInterface& out( void );
	yaal::hcore::HStreamInterface& err( void );
	yaal::hcore::HStreamInterface::ptr_t stream_in( void );
	yaal::hcore::HStreamInterface::ptr_t stream_out( void );
	yaal::hcore::HStreamInterface::ptr_t stream_err( void );
	void close_in( void );
	void close_out( void );
	void close_err( void );
private:
	HPipedChild( HPipedChild const& ) = delete;
	HPipedChild& operator = ( HPipedChild const& ) = delete;
};

typedef yaal::hcore::HExceptionT<HPipedChild> HPipedChildException;

}

}

#endif /* #ifndef YAAL_TOOLS_PIPEDCHILD_HXX_INCLUDED */

