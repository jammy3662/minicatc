#include "symbol.h"

#include "words.h"
#include <stdlib.h>

arr <Operator> operators;

Symbol* getSymbol ();

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
	Var res;
	
	return res;
}

Symbol* getProgram ()
{
	// this is a short function because getSymbol is recursive
	Symbol* prog = getSymbol ();
	
	return prog;
}

// might call itself many times
Symbol* getSymbol ()
{
	Symbol* res = 0;
	
	// TODO:
	// This is where it grabs tokens one by one
	// and organizes everything into the tables
	
	Word w;
	w = getword();
	printf ("wr: %i\n", w.id);

	// ...
	
	if (w.id >= WordID::INT && w.id <= WordID::STR)
{
		Var* var = (Var*) malloc (sizeof ( Var ));
		
		var->kind = OBJ;
		var->isLiteral = 1;
		var->isObject = 1; // not an expression
		
		if (w.id == WordID::INT)
	{
			var->type.id = Types::INT;
			// use the largest type to account for all sizes
			long long num = atoll (w.str);
			var->constant.ll = num;
	}
		else if (w.id == WordID::FLOAT)
	{
			var->type.id = Types::DOUBLE; // default to double like c		
			float num = atof (w.str);
			var->constant.f = num;
	}
		else if (w.id == WordID::STR)
	{
			var->type.id = Types::PTR;
			var->constant.ptr = w.str;
	}
		return var;
}

	if (w.id != WordID::TXT && w.id >= WordID::INT)
{
		// not a symbol and not an identifier
		// it's either a comment or the _ special character
		return 0;
}
	
	return res;
}
