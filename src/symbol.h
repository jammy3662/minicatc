#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "symbol.def.h"
#include "../token.h"

namespace CatLang {

struct Symbol;

// most references point directly to a symbol,
// but unresolved references hold a path to be checked later
struct Reference
{
	Symbol* Symbol;
	byte SymbolType;
	char* Path;
};

struct Type
{
	Reference complex; // struct, union, function, or enum
	byte datatype; DataType;
	byte indirection; // pointers (# of)

	byte
	CONST: 1,
	REGISTER: 1,
	RESTRICT: 1,
	VOLATILE:	1,
	EXTERN: 1,
	STATIC: 1,
	INLINE:	1,
	MATH:	2;
};

struct Error
{	Location loc;
	char* message;
	int2 code;
	byte severity; };

struct Note
{	char* text; };

struct Symbol
{
	Location loc; // source location
	Reference parent;
	
	// internal representation for comments
	Note comment;
	
	std::vector <Error> errors;
	
	char* name;
	byte SymbolType;
};

struct Variable: Symbol
{
	Type DataType;
	Reference Initializer;
};

struct Expression: Symbol
{
	std::vector <Expression> subexpressions;
	Type result; // resulting datatype of operation
	
	union Constant
	{
		most int i, Int;
		double f, Float;
		char* str,* Str;
		char c, Char;
	};
	
	union
	{
		Reference object;
		Reference operands [2];
		
		union Constant constant;
	};
	
	byte opcode;
};

// simplest complex type
// contains variables with optional names
struct Tuple: Symbol
{
	Array <Variable>
	Locals;
	
	Table <string, unsigned short>
	Namespace; // index into locals
};

struct Scope: Tuple
{
	Array <Expression> Expressions;
	Array <Scope> Definitions;
	
	Table <string, Reference>	Aliases;
	
	Type Receiver; // for methods like int.sign()
	Tuple Parameter; // fields passed in for functions
	
	bool open; // true until closed {}
};

Reference RefFrom (Symbol* symbol);

Symbol* lookup (char* name, Symbol* scope);
Symbol* lookup (char* name, Symbol* scope, byte type);

Error Log (char* message, byte level, Symbol* scope);
Error Log (char* message, byte level, int2 code, Symbol* scope);
Error Log (Error err, Symbol* scope);
}

#endif
