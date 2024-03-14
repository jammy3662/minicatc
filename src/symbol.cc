#include "symbol.h"

Program getProgram ();

Symbol* Symbol::get (char* name)
{
	Symbol* res;
	int err;
	
	res = fields.find (name, &err);
	if (err) return 0;
	
	return res;
}

Var Func::evaluate ()
{
	return body.evaluate ();
}

Var Expr::evaluate ()
{
	/* ... */
}
