/* Read yaal/LICENSE.md file for copyright and licensing information. */

/*! \file tools/huginn/lookup.hxx
 * \brief Declaration of huginn::HLookup class.
 */

#ifndef YAAL_TOOLS_HUGINN_LOOKUP_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_LOOKUP_HXX_INCLUDED 1

#include "tools/huginn/iterable.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HLookup : public HInvalidatingIterable {
public:
	typedef HLookup this_type;
	typedef HInvalidatingIterable base_type;
	typedef yaal::hcore::HOrderedHashMap<HHuginn::value_t, HHuginn::value_t, HValueHashHelper&, HValueHashHelper&> values_t;
private:
	mutable HValueHashHelper _helper;
	values_t _data;
public:
	HLookup( HClass const* );
	void insert( huginn::HThread*, HHuginn::value_t const&, HHuginn::value_t const&, int );
	bool has_key( huginn::HThread*, HHuginn::value_t const&, int ) const;
	void erase( huginn::HThread*, HHuginn::value_t const&, int );
	value_t get( huginn::HThread*, HHuginn::value_t const&, int ) const;
	bool try_get( huginn::HThread*, HHuginn::value_t const&, HHuginn::value_t&, int );
	values_t const& value( void ) const {
		return ( _data );
	}
	values_t& value( void ) {
		return ( _data );
	}
	void clear( void );
	void anchor( huginn::HThread* thread_, int position_ ) const {
		_helper.anchor( thread_, position_ );
	}
	void detach( void ) const {
		_helper.detach();
	}
protected:
	virtual iterator_t do_iterator( huginn::HThread*, int ) override;
	virtual int long do_size( huginn::HThread*, int ) const override;
private:
	HLookup( HLookup const& ) = delete;
	HLookup& operator = ( HLookup const& ) = delete;
private:
	virtual value_t do_clone( huginn::HThread*, HHuginn::value_t*, int ) const override;
	virtual bool do_operator_equals( HThread*, HHuginn::value_t const&, HHuginn::value_t const&, int ) const override;
	virtual void do_operator_add( HThread*, HHuginn::value_t&, HHuginn::value_t const&, int ) override;
	virtual bool do_operator_contains( HThread*, HHuginn::value_t const&, HHuginn::value_t const&, int ) const override;
	virtual HHuginn::value_t do_operator_subscript( HThread*, HHuginn::value_t const&, HHuginn::value_t const&, int ) const override;
	virtual void do_operator_subscript_assign( HThread*, HHuginn::value_t&, HHuginn::value_t const&, HHuginn::value_t&&, int ) override;
	virtual hash_value_t do_operator_hash( HThread*, HHuginn::value_t const&, int ) const override;
	virtual yaal::hcore::HString do_code( HThread*, HHuginn::value_t const&, HCycleTracker&, int ) const override;
	virtual yaal::hcore::HString do_to_string( HThread*, HHuginn::value_t const&, HCycleTracker&, int ) const override;
};

namespace lookup {

HHuginn::value_t key_values_view( huginn::HThread*, HHuginn::value_t const& );
HHuginn::value_t reversed_view( huginn::HThread*, HHuginn::value_t const& );

}

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_LOOKUP_HXX_INCLUDED */

