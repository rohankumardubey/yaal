/*
---           `yaal' (c) 1978 by Marcin 'Amok' Konarski            ---

  hcore/hregex.hxx - this file is integral part of `yaal' project.

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
/*! \file hcore/hregex.hxx
 * \brief HRegex class related declarations.
 *
 * HRegex::HMatchIterator and HRegex::HMatch classes are included here.
 */

#ifndef YAAL_HCORE_HREGEX_HXX_INCLUDED
#define YAAL_HCORE_HREGEX_HXX_INCLUDED 1

#include "config.hxx"
#include "hcore/hchunk.hxx"
#include "hcore/hstring.hxx"
#include "hcore/hbitflag.hxx"

namespace yaal {

namespace hcore {

/*! \brief Regular expression string searcher.
 */
class HRegex final {
public:
	struct COMPILE;
	typedef HBitFlag<COMPILE> compile_t;
	struct COMPILE {
		static M_YAAL_HCORE_PUBLIC_API compile_t const NONE;
		static M_YAAL_HCORE_PUBLIC_API compile_t const EXTENDED;
		static M_YAAL_HCORE_PUBLIC_API compile_t const IGNORE_CASE;
		static M_YAAL_HCORE_PUBLIC_API compile_t const NEWLINE;
		static M_YAAL_HCORE_PUBLIC_API compile_t const NO_EXCEPTION;
		static M_YAAL_HCORE_PUBLIC_API compile_t const DEFAULT;
	};
	struct MATCH;
	typedef HBitFlag<MATCH> match_t;
	struct MATCH {
		static M_YAAL_HCORE_PUBLIC_API match_t const NONE;
		static M_YAAL_HCORE_PUBLIC_API match_t const NOT_BEGINNING_OF_LINE;
		static M_YAAL_HCORE_PUBLIC_API match_t const NOT_END_OF_LINE;
		static M_YAAL_HCORE_PUBLIC_API match_t const DEFAULT;
	};
	class HMatch;
	class HMatchIterator;
	typedef HMatchIterator iterator;
	typedef HRegex this_type;
private:
	bool    _initialized;          /*!< is regex initialized */
	HString _pattern;              /*!< original regex pattern */
	compile_t _flags;              /*!< flags used to compile this regex */
	HChunk  _compiled;             /*!< compiled regular expression for search patterns */
	mutable int _lastError;
	/* All fields below are conceptually a memory cache. */
	mutable HChunk _errorBuffer;   /*!< buffer for error message */
	mutable HString _errorCause;   /*!< cause of last error */
	mutable HString _errorMessage; /*!< error message of last operation */
public:
	/*! \brief Create uninitialized regex object.
	 */
	HRegex( void );
	HRegex( HRegex&& );
	HRegex& operator = ( HRegex&& );

	/*! \brief Construct regex with regex rule.
	 *
	 * \param regex - regex pattern to construct regex object.
	 * \param flags - configure various aspects of regex behavior.
	 */
	HRegex( HString const& regex, compile_t flags = COMPILE::DEFAULT );

	/*! \brief Destroy regex object and free all resources.
	 */
	~HRegex( void );

	/*! \brief Parse regular expression pattern with configuration and prepare for matches retrieval.
	 *
	 * \param regex - regular expression pattern to match against.
	 * \param flags - configure various aspects of regex behavior.
	 */
	bool compile( HString const& regex, compile_t flags = COMPILE::DEFAULT );

	/*! \brief Test if this regex object is a valid regex object.
	 *
	 * \return True iff this regex object is valid.
	 */
	bool is_valid( void ) const;
	/*! \brief Get oryginal regex patter for this regex object.
	 *
	 * \return Original regex pattern.
	 */
	HString const& pattern( void ) const;

	HString const& error( void ) const;
	int error_code( void ) const;

	/*! \brief Find substring matching this patter in given string.
	 *
	 * \param string_ - string to search through.
	 * \return HMatchIterator pointing to first match or end of matches HMatchIterator.
	 *
	 * Parameter must be raw memory pointer for HMatchIterator returns
	 * raw memory pointers to input string while dereferenced.
	 */
	HMatchIterator find( char const* string_ ) const;
	HMatchIterator find( HString const& string_ ) const;
	HMatchIterator end( void ) const;
	/*! \brief Tell if given string matches regex.
	 *
	 * \param string_ - string to check against the regex.
	 * \return True iff given string matches this regex.
	 */
	bool matches( HString const& string_ ) const;
	void swap( HRegex& );
	void clear( void );
	HRegex copy( void ) const;
private:
	HRegex( HRegex const& ) = delete;
	HRegex& operator = ( HRegex const& ) = delete;
	void error_clear( void ) const;
	char const* matches( char const*, int* ) const;
};

/*! \brief Instance of single match for given regex.
 */
class HRegex::HMatch {
	int _start;
	int _size;
public:
	int start() const;
	int size() const;
private:
	HMatch( int, int );
	friend class HRegex::HMatchIterator;
};

/*! \brief Pattern matches iterator.
 */
class HRegex::HMatchIterator : public iterator_interface<HMatch, iterator_category::forward> {
	HRegex const* _owner;
	char const* _string;
	HRegex::HMatch _match;
public:
	typedef iterator_interface<HMatch, iterator_category::forward> base_type;
	HMatch const* operator->( void ) const;
	HMatch const& operator*( void ) const;
	bool operator != ( HMatchIterator const& ) const;
	bool operator == ( HMatchIterator const& ) const;
	HMatchIterator& operator ++ ( void );
	HMatchIterator( HMatchIterator const& );
	HMatchIterator& operator = ( HMatchIterator const& );
private:
	HMatchIterator( HRegex const*, char const*, int, int );
	friend class HRegex;
};

typedef HExceptionT<HRegex> HRegexException;

HRegex operator "" _yr ( char const*, size_t );

inline void swap( HRegex& a, HRegex& b ) {
	a.swap( b );
}

}

}

#ifdef YAAL_USE_PCRE
#	define YAAL_REGEX_WORD_START "\\b"
#	define YAAL_REGEX_WORD_END   "\\b"
#elif defined ( __HOST_OS_TYPE_DARWIN__ )
#	define YAAL_REGEX_WORD_START "[[:<:]]"
#	define YAAL_REGEX_WORD_END   "[[:>:]]"
#else /* Not PCRE and not Darwin. */
#	define YAAL_REGEX_WORD_START "\\<"
#	define YAAL_REGEX_WORD_END   "\\>"
#endif


#endif /* #ifndef YAAL_HCORE_HREGEX_HXX_INCLUDED */

