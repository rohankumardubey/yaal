/* Read yaal/LICENSE.md file for copyright and licensing information. */

/* YAAL_PRIVATE_IMPLEMENTATION_DETAIL */

#ifndef YAAL_TOOLS_HUGINN_SWITCH_HXX_INCLUDED
#define YAAL_TOOLS_HUGINN_SWITCH_HXX_INCLUDED 1

#include "tools/huginn/compiler.hxx"
#include "tools/huginn/statement.hxx"

namespace yaal {

namespace tools {

namespace huginn {

class HSwitch : public HStatement {
public:
	typedef HSwitch this_type;
	typedef HStatement base_type;
	typedef OCompiler::OScopeContext::active_scopes_t cases_t;
private:
	HHuginn::expression_t _expression;
	cases_t _cases;
	HHuginn::scope_t _default;
public:
	HSwitch( HStatement::statement_id_t, HHuginn::expression_t const&, cases_t const&, HHuginn::scope_t const&, int, int );
protected:
	virtual void do_execute( huginn::HThread* ) const override;
};

}

}

}

#endif /* #ifndef YAAL_TOOLS_HUGINN_SWITCH_HXX_INCLUDED */

