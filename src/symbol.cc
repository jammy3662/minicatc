#include "symbol.h"

#include "words.h"
#include <stdlib.h>

// dummy definition because there isn't
// a specific datatype for this
#ifdef _12345678901234567890_
{
	{"void", VOID_T}, {"let", LET_T},
	{"auto", LET_T}, {"var", VAR_T},
	{"class", STRUCT_T}, {"struct", STRUCT_T},
	{"union", UNION_T}, {"enum", ENUM_T},
	{"char", CHAR_T},	{"byte", CHAR_T},
	{"short", SHORT_T}, {"int", INT_T},
	{"long", LONG_T},	{"float", DOUBLE_T},
	{"double", DOUBLE_T}, {"<=", LESSEQUAL},
	{">=", MOREEQUAL}, {"+=", PLUSEQUAL},
	{"-=", MINUSEQUAL}, {"*=", TIMESEQUAL},
	{"/=", DIVEQUAL}, {"|=", OREQUAL},
	{"&=", ANDEQUAL}, {"<<", SHIFT_L},
	{"<<<", ROTATE_L}, {">>>", ROTATE_R},
	{">>", SHIFT_R}, {"==", COMPARE},
	{"++", INCREMENT}, {"--", DECREMENT},
	{"&&", AND}, {"and", AND}, {"||", OR}, {"or", OR},
	{"<-", LARROW}, {"->", RARROW},
	{"..", TAIL}, {"...", ELLIPSIS},
	{"if", IF}, {"else", ELSE},
	{"while", WHILE}, {"switch", SWITCH},
	{"case", CASE}, {"do", DO},
	{"break", BREAK}, {"default", DEFAULT},
	{"continue", CONTINUE}, {"for", FOR},
	{"in", IN}, {"return", RETURN},
	{"end", END}, {"include", INCLUDE},
	{"inline", INLINE}, {"const", CONST},
	{"static", STATIC}, {"extern", EXTERN},
	{"sizeof", SIZEOF}, {"typeof", TYPEOF},
	{"countof", COUNTOF}, {"nameof", NAMEOF},
};
#endif

Symbol* program;
arr <Operator> operators;

const Symbol emptySymbol =
{
	NONE,
	0x0,
	
	emptySymbol.fields
};

Symbol* getSymbol (Symbol* scope = 0x0);
Symbol* getNamedSymbol (Symbol* scope = 0x0);

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
	Symbol* prog;
	program = prog;
	
	prog = getSymbol ();
	
	return prog;
}

// might call itself many times
Symbol* getSymbol (Symbol* scope)
{
// This is where it grabs tokens one by one
// and organizes everything into the tables

	Symbol* res = 0x0;
	
	Word w;
	w = getword();
	
	if (w.id == WordID::TXT)
{
	return getNamedSymbol (scope);
}
	else
	if (w.id < WordID::TXT)
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
	else
	if (w.id >= WordID::PLACEHOLDER)
{
	// not a symbol and not an identifier
	// it's either a comment or the _ special character
	// no need to handle these tokens
	return (Symbol*) &emptySymbol;
	// return an empty symbol rather than nullptr
	// to indicate a valid token (to ignore)
}
	
	// TODO:
	// All other symbols, like brackets and commas, etc.

	return res;
}

Symbol* getNamedSymbol (Symbol* scope)
{
	
}
