/* Read yaal/LICENSE.md file for copyright and licensing information. */

/*! \file hcore/hstreaminterface.hxx
 * \brief Declaration of HStreamInterface class.
 */

#ifndef YAAL_HCORE_HSTREAMINTERFACE_HXX_INCLUDED
#define YAAL_HCORE_HSTREAMINTERFACE_HXX_INCLUDED 1

#include "hcore/hstring.hxx"
#include "hcore/hchunk.hxx"
#include "hcore/hexception.hxx"
#include "hcore/hpointer.hxx"

namespace yaal {

namespace hcore {

class HStreamInterface;
HStreamInterface& endl( HStreamInterface& );
HStreamInterface& flush( HStreamInterface& );
HStreamInterface& text( HStreamInterface& );
HStreamInterface& binary( HStreamInterface& );
HStreamInterface& dec( HStreamInterface& );
HStreamInterface& hex( HStreamInterface& );
HStreamInterface& oct( HStreamInterface& );
HStreamInterface& natural( HStreamInterface& );
HStreamInterface& fixed( HStreamInterface& );
HStreamInterface& scientific( HStreamInterface& );
HStreamInterface& boolalpha( HStreamInterface& );
HStreamInterface& noboolalpha( HStreamInterface& );
HStreamInterface& skipws( HStreamInterface& );
HStreamInterface& noskipws( HStreamInterface& );
HStreamInterface& left( HStreamInterface& );
HStreamInterface& right( HStreamInterface& );
HStreamInterface& center( HStreamInterface& );

/*! \brief Interface for stream based IO.
 */
class HStreamInterface {
public:
	typedef HStreamInterface this_type;
	class HManipulator {
		int _value;
		typedef void ( HManipulator::* ACTION_t )( HStreamInterface& ) const;
		ACTION_t _action;
	public:
		HManipulator( int, ACTION_t );
		void operator()( HStreamInterface& ) const;
		void set_fill( HStreamInterface& ) const;
		void set_width( HStreamInterface& ) const;
		void set_precision( HStreamInterface& ) const;
	};
	typedef HPointer<HStreamInterface> ptr_t;
	enum class MODE {
		TEXT,
		BINARY
	};
	enum class POLL_TYPE {
		NATIVE,
		EMULATED,
		INVALID
	};
	enum class BASE {
		DEC,
		HEX,
		OCT
	};
	enum class FLOAT_FORMAT {
		NATURAL,   /*!< Use as many decimal digits as it makes sense for given value. */
		FIXED,     /*!< Use exactly `precision` count decimal digits. */
		SCIENTIFIC /*!< Use scientific notation (1.2345e+009). */
	};
	enum class ADJUST {
		LEFT,
		RIGHT,
		CENTER
	};
	/*! \brief Kinds of seek operation.
	 */
	enum class SEEK {
		BEGIN,
		CURRENT,
		END
	};
	typedef HStreamInterface& ( *manipulator_t )( HStreamInterface& );
private:
	enum class CACHE_CONTENT {
		READ,
		WRITE
	};
	HString _wordCache;  /*!< Cache for operator >> () and operator << (). */
	HUTF8String _conversionCache; /*!< Cache used for converting between UTF-8 and UCS */
	code_point_t _fill;  /*!< Fill character for output operations. */
	int _width;          /*!< Next output operation width. */
	int _precision;      /*!< Set number of significant digits to display for all subsequent outputs of floating point type values. */
	MODE _mode;
	BASE _base;
	FLOAT_FORMAT _floatFormat;
	ADJUST _adjust;
	bool _skipWS;
	bool _boolAlpha;
	bool _bufferedIO;
	int _ioBufferSize;
protected:
	bool _valid;      /*!< Tells if further low-level IO is possible. */
	bool _fail;       /*!< Tells if most recently performed data extraction failed at logical level. */
	HChunk _cache;    /*!< Read/write buffer. */
	int _cachedBytes; /*!< Position of where continued read (another read_until invocation after interrupted one) shall store consecutive bytes. */
	CACHE_CONTENT _cacheContent;
public:
	HStreamInterface( void );
	virtual ~HStreamInterface( void );
	HStreamInterface& operator << ( HString const& val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( char const* val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( bool val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( code_point_t val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( char val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( char unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int short val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int short unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int long unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int long long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( int long long unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( double val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( double long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( float val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( void const* val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( manipulator_t const& val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator << ( HManipulator const& val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& operator >> ( HString&  val_) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( bool& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( code_point_t& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( char& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( char unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int short& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int short unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int long unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int long long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( int long long unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( double& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( double long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( float& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( void const*& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& operator >> ( manipulator_t const& val_ ) {
		return ( do_input( val_ ) );
	}
	/*! \brief Read data from stream until end of it or until delimiter is encountered.
	 *
	 * \param store - Store read date here.
	 * \param delim - Stop reading data at any character in this set.
	 * \param extract - Extract delimiting stop char from input stream.
	 * \return number of bytes read.
	 */
	int long read_until( yaal::hcore::HString& store, char const* delim = eol, bool extract = true ) {
		return ( do_read_until( store, delim, extract ) );
	}
	/*! \brief Read data from stream until end of it or until delimiter is encountered or enough data has been acquired.
	 *
	 * \param store - Store read date here.
	 * \param maxcount - A maximum number of bytes to read.
	 * \param delim - Stop reading data at any character in this set.
	 * \param extract - Extract delimiting stop char from input stream.
	 * \return number of bytes read.
	 */
	int long read_until_n( yaal::hcore::HString& store, int long maxcount, char const* delim = eol, bool extract = true ) {
		return ( do_read_until_n( store, maxcount, delim, extract ) );
	}
	/*! \brief Read data from stream as long as read characters are in set and end of stream has not been reached.
	 *
	 * \param store - Store read date here.
	 * \param acquire - Read only those characters.
	 * \return number of bytes read.
	 */
	int long read_while( yaal::hcore::HString& store, char const* acquire ) {
		return ( do_read_while( store, acquire ) );
	}
	/*! \brief Read data from stream as long as read characters are in set and neither end of stream nor given limit has been reached.
	 *
	 * \param store - Store read date here.
	 * \param maxcount - A maximum number of bytes to read.
	 * \param acquire - Read only those characters.
	 * \return number of bytes read.
	 */
	int long read_while_n( yaal::hcore::HString& store, int long maxcount, char const* acquire ) {
		return ( do_read_while_n( store, maxcount, acquire ) );
	}
	HStreamInterface& read( HString&  val_) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( bool& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( code_point_t& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( char& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( char unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int short& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int short unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int long unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int long long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( int long long unsigned& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( double& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( double long& val_ ) {
		return ( do_input( val_ ) );
	}
	HStreamInterface& read( float& val_ ) {
		return ( do_input( val_ ) );
	}
	int long read( void* buffer_, int long size_ ) {
		return ( do_input( buffer_, size_ ) );
	}
	/*! \brief Read available data in non-blocking mode.
	 *
	 * Read all data that is readily available, but not more
	 * than \e size_ bytes.
	 *
	 * \param buffer_ - read data into this buffer
	 * \param size_ - read at most that amount of bytes
	 * \return Number of bytes actually read, 0 if stream had been closed, -1 on error.
	 */
	int long read_some( void* buffer_, int long size_ ) {
		return ( do_read_some( buffer_, size_ ) );
	}
	void unread( void const* buffer_, int long size_ ) {
		do_unread( buffer_, size_ );
	}
	HStreamInterface& write( HString const& val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( char const* val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( bool val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( code_point_t val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( char val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( char unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int short val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int short unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int long unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int long long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( int long long unsigned val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( double val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( double long val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( float val_ ) {
		return ( do_output( val_ ) );
	}
	HStreamInterface& write( void const* val_ ) {
		return ( do_output( val_ ) );
	}
	int long write( void const* buffer_, int long size_ ) {
		return ( do_output( buffer_, size_ ) );
	}
	/*! \brief Move reading/writing position on seekable stream.
	 *
	 * \param offset - new reading/writing position.
	 * \param anchor - count offset relative to given anchor.
	 */
	void seek( int long offset, SEEK anchor = SEEK::BEGIN );
	/*! \brief Read data from stream as long as it matches given pattern.
	 *
	 * \post Sets fail-bit if pattern could not be read in its entirety.
	 *
	 * \param pattern_ - a pattern to be read from this stream.
	 * \return Self.
	 */
	HStreamInterface& consume( yaal::hcore::HString const& pattern_ ) {
		return ( do_consume( pattern_ ) );
	}
	/*! \brief Discard some data from the stream.
	 *
	 * \param count_ - how many bytes shall be discarded.
	 * \return Self.
	 */
	HStreamInterface& ignore( int count_ ) {
		return ( do_ignore( count_ ) );
	}
	/*! \brief Skip whitespace characters from input if this stream is so configured.
	 *
	 * \return Self.
	 */
	HStreamInterface& skipws( void ) {
		return ( do_skipws() );
	}
	M_YAAL_HCORE_PUBLIC_API static char const eol[];
	/*! \brief Tell if given stream instance if a valid stream object.
	 *
	 * Note: Implementation is not a trivial forward.
	 */
	bool is_valid( void ) const;
	POLL_TYPE poll_type( void ) const {
		return ( do_poll_type() );
	}
	void const* data( void ) const;
	/*! \brief Tell if all operations until now succeeded.
	 *
	 * \return True iff all operations until now succeeded.
	 */
	bool good( void ) const {
		return ( do_good() );
	}
	/*! \brief Tell if any problem occurred with stream at system or semantic level.
	 *
	 * \return True iff stream had experienced problems at system of semantic level.
	 */
	bool fail( void ) const {
		return ( do_fail() );
	}
	/*! \brief Tell if stream operation failed at system level.
	 *
	 * \return True iff stream operation failed at system level.
	 */
	bool bad( void ) const {
		return ( do_bad() );
	}
	/*! \brief Clear state bits.
	 */
	void clear( void ) {
		do_clear();
		return;
	}
	/*! \brief Reset internal state so stream can be reused.
	 */
	void reset( void ) {
		do_reset();
	}
	/*! \brief Flush all pending output.
	 */
	void flush( void );
	int peek( void ) {
		return ( do_peek() );
	}
	/*! \brief Get number of bytes available immediately for reading.
	 *
	 * Get number of bytes that were already read from the underlying medium and are ready for immediate extraction.
	 */
	int immediate_read_size( void ) const {
		return ( do_immediate_read_size() );
	}
	/*! \brief Get number of bytes pending for write operation.
	 *
	 * In other words, get number of bytes that are buffered and ready to be flushed to the underlying medium.
	 */
	int pending_write_size( void ) const {
		return ( do_pending_write_size() );
	}
	HStreamInterface& set_skipws( bool skipWS_ ) {
		return ( do_set_skipws( skipWS_ ) );
	}
	HStreamInterface& set_boolalpha( bool boolalpha_ ) {
		return ( do_set_boolalpha( boolalpha_ ) );
	}
	HStreamInterface& set_buffered_io( bool bufferedIo_ ) {
		return ( do_set_buffered_io( bufferedIo_ ) );
	}
	HStreamInterface& set_io_buffer_size( int ioBufferSize_ ) {
		return ( do_set_io_buffer_size( ioBufferSize_ ) );
	}
	HStreamInterface& set_fill( code_point_t val_ ) {
		return ( do_set_fill( val_ ) );
	}
	HStreamInterface& set_width( int val_ ) {
		return ( do_set_width( val_ ) );
	}
	HStreamInterface& set_precision( int precision_ ) {
		return ( do_set_precision( precision_ ) );
	}
	HStreamInterface& set_mode( MODE val_ ) {
		return ( do_set_mode( val_ ) );
	}
	HStreamInterface& set_base( BASE val_ ) {
		return ( do_set_base( val_ ) );
	}
	HStreamInterface& set_float_format( FLOAT_FORMAT val_ ) {
		return ( do_set_float_format( val_ ) );
	}
	HStreamInterface& set_adjust( ADJUST val_ ) {
		return ( do_set_adjust( val_ ) );
	}
	bool get_skipws( void ) const {
		return ( do_get_skipws() );
	}
	bool get_boolalpha( void ) const {
		return ( do_get_boolalpha() );
	}
	bool get_buffered_io( void ) const {
		return ( do_get_buffered_io() );
	}
	int get_io_buffer_size( void ) const {
		return ( do_get_io_buffer_size() );
	}
	code_point_t get_fill( void ) const {
		return ( do_get_fill() );
	}
	int get_width( void ) const {
		return ( do_get_width() );
	}
	int get_precision( void ) const {
		return ( do_get_precision() );
	}
	MODE get_mode( void ) const {
		return ( do_get_mode() );
	}
	BASE get_base( void ) const {
		return ( do_get_base() );
	}
	FLOAT_FORMAT get_float_format( void ) const {
		return ( do_get_float_format() );
	}
	ADJUST get_adjust( void ) const {
		return ( do_get_adjust() );
	}
protected:
	virtual HStreamInterface& do_output( HString const& );
	virtual HStreamInterface& do_output( char const* );
	virtual HStreamInterface& do_output( bool );
	virtual HStreamInterface& do_output( code_point_t );
	virtual HStreamInterface& do_output( char );
	virtual HStreamInterface& do_output( char unsigned );
	virtual HStreamInterface& do_output( int short );
	virtual HStreamInterface& do_output( int short unsigned );
	virtual HStreamInterface& do_output( int );
	virtual HStreamInterface& do_output( int unsigned );
	virtual HStreamInterface& do_output( int long );
	virtual HStreamInterface& do_output( int long unsigned );
	virtual HStreamInterface& do_output( int long long );
	virtual HStreamInterface& do_output( int long long unsigned );
	virtual HStreamInterface& do_output( double );
	virtual HStreamInterface& do_output( double long );
	virtual HStreamInterface& do_output( float );
	virtual HStreamInterface& do_output( void const* );
	virtual HStreamInterface& do_output( manipulator_t const& );
	virtual HStreamInterface& do_output( HManipulator const& );
	virtual int long do_output( void const*, int long );
	virtual HStreamInterface& do_input( HString& );
	virtual HStreamInterface& do_input( bool& );
	virtual HStreamInterface& do_input( code_point_t& );
	virtual HStreamInterface& do_input( char& );
	virtual HStreamInterface& do_input( char unsigned & );
	virtual HStreamInterface& do_input( int short& );
	virtual HStreamInterface& do_input( int short unsigned& );
	virtual HStreamInterface& do_input( int& );
	virtual HStreamInterface& do_input( int unsigned& );
	virtual HStreamInterface& do_input( int long& );
	virtual HStreamInterface& do_input( int long unsigned& );
	virtual HStreamInterface& do_input( int long long& );
	virtual HStreamInterface& do_input( int long long unsigned& );
	virtual HStreamInterface& do_input( double& );
	virtual HStreamInterface& do_input( double long& );
	virtual HStreamInterface& do_input( float& );
	virtual HStreamInterface& do_input( void const*& );
	virtual HStreamInterface& do_input( manipulator_t const& );
	virtual int long do_input( void*, int long );
	virtual int long do_read_until( yaal::hcore::HString&, char const*, bool );
	virtual int long do_read_until_n( yaal::hcore::HString&, int long, char const*, bool );
	virtual int long do_read_while( yaal::hcore::HString&, char const* );
	virtual int long do_read_while_n( yaal::hcore::HString&, int long, char const* );
	virtual int long do_read_some( void*, int long );
	virtual void do_unread( void const*, int long );
	virtual HStreamInterface& do_consume( yaal::hcore::HString const& );
	virtual HStreamInterface& do_ignore( int );
	virtual HStreamInterface& do_skipws( void );
	virtual int do_peek( void );
	virtual int do_immediate_read_size( void ) const;
	virtual int do_pending_write_size( void ) const;
	virtual HStreamInterface& do_set_fill( code_point_t );
	virtual HStreamInterface& do_set_width( int );
	virtual HStreamInterface& do_set_precision( int );
	virtual HStreamInterface& do_set_mode( MODE );
	virtual HStreamInterface& do_set_base( BASE );
	virtual HStreamInterface& do_set_float_format( FLOAT_FORMAT );
	virtual HStreamInterface& do_set_adjust( ADJUST );
	virtual HStreamInterface& do_set_skipws( bool );
	virtual HStreamInterface& do_set_boolalpha( bool );
	virtual HStreamInterface& do_set_buffered_io( bool );
	virtual HStreamInterface& do_set_io_buffer_size( int );
	virtual bool do_get_skipws( void ) const;
	virtual bool do_get_boolalpha( void ) const;
	virtual bool do_get_buffered_io( void ) const;
	virtual int do_get_io_buffer_size( void ) const;
	virtual code_point_t do_get_fill( void ) const;
	virtual int do_get_width( void ) const;
	virtual int do_get_precision( void ) const;
	virtual MODE do_get_mode( void ) const;
	virtual BASE do_get_base( void ) const;
	virtual FLOAT_FORMAT do_get_float_format( void ) const;
	virtual ADJUST do_get_adjust( void ) const;
	virtual bool do_good( void ) const;
	virtual bool do_fail( void ) const;
	virtual bool do_bad( void ) const;
	virtual void do_clear( void );
	virtual void do_reset( void );
	void flush_write_buffer( void );
private:
	int long read_while_retry( yaal::hcore::HString&, char const* );
	int long read_until_retry( yaal::hcore::HString&, char const*, bool );
	bool read_word( void );
	bool read_integer( void );
	bool read_floating_point( void );
	int long semantic_read( yaal::hcore::HString&, int long, char const*, bool, bool );
	void reformat( void );
	void apply_precision( void );
	virtual int long do_write( void const*, int long ) = 0;
	virtual int long do_read( void*, int long ) = 0;
	virtual void do_seek( int long, SEEK );
	virtual void do_flush( void ) = 0;
	virtual bool do_is_valid( void ) const = 0;
	virtual POLL_TYPE do_poll_type( void ) const = 0;
	virtual void const* do_data( void ) const = 0;
	friend HStreamInterface& endl( HStreamInterface& );
	friend HStreamInterface& flush( HStreamInterface& );
};

HStreamInterface::HManipulator setfill( code_point_t );
HStreamInterface::HManipulator setw( int );
HStreamInterface::HManipulator setprecision( int );

typedef HExceptionT<HStreamInterface> HStreamInterfaceException;

/*! \brief Read one line of input.
 *
 * \param stream - stream to read from.
 * \param store - variable where read line should be stored.
 * \return Stream that where read.
 */
HStreamInterface& getline( HStreamInterface& stream, yaal::hcore::HString& store );

/*! \brief Iterator adaptor for streams.
 *
 * This helper may be useful in algorithms.
 *
 * \tparam out_t - type of elements accepted/traversed by this iterator.
 * \tparam delim_t - type of optional delimiter separating each iteration.
 */
template<typename out_t, typename delim_t = char const*>
class HStreamIterator {
	mutable HStreamInterface* _stream;
	delim_t _delim;
	mutable out_t _valueCache;
	mutable int long _skip;
	mutable bool _inCache;
public:
	HStreamIterator( void )
		: _stream( nullptr ), _delim(), _valueCache(),
		_skip( 0 ), _inCache( false ) {
	}
	explicit HStreamIterator( HStreamInterface& stream, delim_t const& delim = delim_t() )
		: _stream( &stream ), _delim( delim ),
		_valueCache(), _skip( 0 ), _inCache( false ) {
	}
	HStreamIterator( HStreamIterator const& it_ )
		: _stream( it_._stream ), _delim( it_._delim ),
		_valueCache( it_._valueCache ), _skip( it_._skip ),
		_inCache( false ) {
	}
	HStreamIterator& operator = ( HStreamIterator const& it_ ) {
		if ( &it_ != this ) {
			HStreamIterator tmp( it_ );
			swap( tmp );
		}
		return ( *this );
	}
	HStreamIterator& operator* ( void ) {
		return ( *this );
	}
	HStreamIterator& operator* ( void ) const {
		return ( *this );
	}
	template<typename item_t>
	HStreamIterator& operator = ( item_t const& item ) {
		*_stream << item << _delim;
		return ( *this );
	}
	operator out_t ( void ) const {
		M_PROLOG
		while ( _skip > 0 ) {
			read_value();
			-- _skip;
		}
		if ( ! _inCache ) {
			read_value();
		}
		return ( _valueCache );
		M_EPILOG
	}
	bool operator == ( HStreamIterator const& it_ ) const {
		return ( operator != (  it_ ) );
	}
	bool operator != ( HStreamIterator const& it_ ) const {
		return ( _stream != it_._stream );
	}
	void swap( HStreamIterator& it_ ) {
		if ( &it_ != this ) {
			using yaal::swap;
			swap( _stream, it_._stream );
			swap( _delim, it_._delim );
			swap( _valueCache, it_._valueCache );
			swap( _skip, it_._skip );
			swap( _inCache, it_._inCache );
		}
		return;
	}
	HStreamIterator const& operator ++ ( void ) const {
		M_PROLOG
		++ _skip;
		_inCache = false;
		return ( *this );
		M_EPILOG
	}
private:
	void read_value( void ) const {
		M_PROLOG
		if ( _stream ) {
			*_stream >> _valueCache;
			if ( ! _stream->good() ) {
				_stream = nullptr;
			} else {
				_inCache = true;
			}
		}
		return;
		M_EPILOG
	}
};

/*! \brief Output stream iterator with explicit delimiter type.
 */
template<typename delim_t>
inline HStreamIterator<trait::no_type, delim_t> stream_iterator( HStreamInterface& stream, delim_t delim ) {
	return ( HStreamIterator<trait::no_type, delim_t>( stream, delim ) );
}

/*! \brief Output stream iterator with default delimiter type.
 */
inline HStreamIterator<trait::no_type, char const*> stream_iterator( HStreamInterface& stream ) {
	return ( HStreamIterator<trait::no_type, char const*>( stream, "" ) );
}

/*! \brief Input stream iterator with explicit delimiter type.
 */
template<typename out_t, typename delim_t>
inline HStreamIterator<out_t, delim_t> stream_iterator( HStreamInterface& stream, delim_t delim ) {
	return ( HStreamIterator<out_t, delim_t>( stream, delim ) );
}

/*! \brief Input stream iterator with default delimiter type.
 */
template<typename out_t>
inline HStreamIterator<out_t, char const*> stream_iterator( HStreamInterface& stream ) {
	return ( HStreamIterator<out_t, char const*>( stream, "" ) );
}

/*! \brief End input stream iterator with explicit delimiter type.
 */
template<typename out_t, typename delim_t>
inline HStreamIterator<out_t, delim_t> stream_iterator( void ) {
	return ( HStreamIterator<out_t, delim_t>() );
}

/*! \brief End input stream iterator with default delimiter type.
 */
template<typename out_t>
inline HStreamIterator<out_t, char const*> stream_iterator( void ) {
	return ( HStreamIterator<out_t, char const*>() );
}

}

}

#endif /* #ifndef YAAL_HCORE_HSTREAMINTERFACE_HXX_INCLUDED */

