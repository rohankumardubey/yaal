/* Read yaal/LICENSE.md file for copyright and licensing information. */

/* YAAL_PRIVATE_IMPLEMENTATION_DETAIL */

#ifndef YAAL_TOOLS_HUGINN_FILTER_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_FILTER_HXX_INCLUDED 1

#include "tools/hhuginn.hxx"
#include "tools/huginn/iterator.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HFilter : public huginn::HIterable {
	HHuginn::value_t _source;
	HHuginn::value_t _callable;
public:
	HFilter( huginn::HClass const* class_, HHuginn::value_t source_, HHuginn::value_t callable_ )
		: HIterable( class_ )
		, _source( source_ )
		, _callable( callable_ ) {
	}
	static HHuginn::class_t get_class( HRuntime*, huginn::HClass const* );
protected:
	virtual int long do_size( huginn::HThread*, int ) const override;
private:
	virtual iterator_t do_iterator( HThread*, int ) override;
private:
	virtual HHuginn::value_t do_clone( huginn::HThread*, HHuginn::value_t*, int ) const override;
};

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_FILTER_HXX_INCLUDED */

