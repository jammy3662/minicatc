#include "symbol.h"
#include "language.h"

#include "words.h"
#include <stdlib.h>

Symbol* program;
arr <Operator> operators;

// denotes comments and placeholders without extra data
const Symbol emptySymbol =
{
	NONE, // kind
	0x0, // name
	
	emptySymbol.fields
};

Symbol* getSymbol (Symbol* scope);

Symbol* getNamedSymbol (Symbol* scope);
Symbol* getParenSymbol (Symbol* scope); // ( ... )
Symbol* getBraceSymbol (Symbol* scope); // { ... }
Symbol* getArraySymbol (Symbol* scope); // [ ... ]
Symbol* getMacroSymbol (Symbol* scope); // | ... |

// type signatures and tuple structs
Typesig* getTypesig (Symbol* scope);

// variable, function or alias
Object* getObject (Symbol* scope);

Enum* getEnum (Symbol* scope);


Symbol* Symbol::get (char* name)
{
	Symbol* res;
	int err;
	
	res = fields.find (name, &err);
	if (err) return 0;
	
	return res;
}

int Symbol::insert (char* name, Symbol* s)
{
	int available;
	fields.find (name, &available);
	
	if (available)
{	
		fields.insert (name, s);
		return 0;
}	
	else
	return 1;
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
	Symbol* prog;
	program = prog;
	
	prog = getSymbol ((Symbol*)languageScope);
	
	return prog;
}

// might call itself many times
Symbol* getSymbol (Symbol* scope)
{
// This is where it grabs tokens one by one
// and organizes everything into the tables
	
	Word w = {};
	Symbol* res = 0x0;
	
	w = getword();

_NAME_:
	if (w.id == WordID::TXT)
{
	putwordback (w); // leave first word for lookup
	return getNamedSymbol (scope);
}
	else

_CONSTANT_:
	if (w.id < WordID::TXT)
{
	Var* var = (Var*) malloc (sizeof ( Var ));
	
	var->kind = OBJ;
	var->isLiteral = 1;
	var->isObject = 1; // not an expression
	
	_INT_CONSTANT_:	
		if (w.id == WordID::INT)
	{
			var->type.id = Types::INT;
			// use the largest type to account for all sizes
			long long num = atoll (w.str);
			var->constant.ll = num;
	}
		else
		
	_FLOAT_CONSTANT_:
		if (w.id == WordID::FLOAT)
	{
			var->type.id = Types::DOUBLE; // default to double like c		
			float num = atof (w.str);
			var->constant.f = num;
	}
		else
		
	_STRING_CONSTANT_:
		if (w.id == WordID::STR)
	{
			var->type.id = Types::PTR;
			var->constant.ptr = w.str;
			w.str = 0x0;
	}
	
	if (w.str != 0x0) free (w.str);
	return var;
}
	else
	
	_COMMENT_:
		if (w.id >= WordID::PLACEHOLDER)
	{
		// either a comment or the _ special character
		// indicate a valid token (to ignore) with empty symbol
		return (Symbol*) &emptySymbol;
	}
	
	// TODO:
	// All other symbols, like brackets and commas, etc.
	
	_SEPARATOR_:
	
	return res;
}

Symbol* getNamedSymbol (Symbol* scope)
{
	Word w = {};
	Symbol* res = 0x0;
	
	w = getword();
	
	if (w.id != WordID::TXT)
{	
		putwordback (w);
		return res;
}	
	res = scope->get (w.str);
	
	// check if there's another name to lookup
	w = getword();
	
	if (w.id == '.')
{
		// check in the resulting symbol from this lookup
		return getNamedSymbol (res);
}
	else putwordback (w);
	
	return res;
}

Typesig* getTypesig (Symbol* scope)
{
	
}
