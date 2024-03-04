#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "container.h"
#include "trie.h"

typedef unsigned long Nameid;

// Typeid is essentially a "Type pointer"
typedef unsigned long Typeid;
enum TYPEID
{
	NONE = 0,
	VAR, FUNC, ALIAS, STRUCT, ENUM,
	INT = 1, BIT, CHAR, BYTE, SHORT, LONG,
	LONGLONG, FLOAT, DOUBLE, LONGDOUBLE, PTR,
};
union Integral
{
	int i; char c; short s; long l;
	long long ll; float f; double d; long double ld;
	void* ptr;
};

Typeid getType (Typeid* signature);
Typeid* getFields (Typeid type);

enum Opcode
{
	ADD, SUB,	POS, NEG,
	MUL, DIV, MOD,
	INC, DEC,
	EQ, NEQ, GREATER, LESS,
	GREQ, LEQ,
	AND, OR, NOT,
	BAND, BOR, BNOT, BXOR,
	LSHIFT, RSHIFT,
	ASSIGN,
	MEMBER, CALL, INDEX, ADDRESS,
	ADR = BXOR, INDIR = MUL,
	COMMA,
};

// recursive structure for all operations
// these collectively form an expression or block of code
struct Expr
{
	Opcode opcode;
	Typeid result;
	Integral value; // static values like 1+2 for integral operations
	
	Expr* front;
	Expr* end;
};

struct Expr;

// declaration of a variable, function,
// method, or operator overload
struct Symbol
{
	struct
	{
		char
		fconst : 1,
		flocal : 1,
		fstatic : 1,
		fsigned : 1,
		fshort : 2,
		flong: 2,
		fcextern :	1,
		fcregister: 1,
		fcrestrict: 1,
		fcvolatile : 1;
	}
	flags;
	
	Typeid type; // variable type or return type
	
	Typeid args; // function signature (arguments)
	Word op; // operator overloads
	
	int count; // array size
	
	Expr*	expression; // function body or variable value
};

struct Scope
{
	Trie <char, Typeid> types;
	Trie <char, Symbol> symbols;
	
};

#endif
