#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "table.h"

struct Typeflags
{
	char
	fconst      :  1,
	flocal      :  1,
	fstatic     :  1,
	fsigned     :  1,
	fshort      :  2,
	flong       :  2,
	fcextern    :  1,
	fcregister  :  1,
	fcrestrict  :  1,
	fcvolatile  :  1,
	
	isFunc      :  1,
	isOperator  :  1;
};

struct Typeid
{
	typedef unsigned long ID;
	
	ID  id;	
	Typeflags  flags;
	
	// akin to the 'name' field of most other symbols
	Typeid* fields;
	
	operator ID () { return id; }
};

namespace Builtin  { enum
{
	NONE = 0,
	MODULE,  FUNC,  STRUCT,  ENUM,  ALIAS,
	INT,  BIT,  CHAR,  BYTE,  SHORT,  LONG,
	LONGLONG,  FLOAT,  DOUBLE,  LONGDOUBLE,  PTR,
};
	const Typeid::ID tpOff = (Builtin::PTR + 1);
}

union Integral
{
	int i;  char c;  short s;  long l;
	long long ll;  float f;  double d;  long double ld;
	void* ptr;
};

namespace Opcode  { enum
{
	x2 = WordDOUBLEOPBit,
	x3 = WordTRIPLEOPBit,
	
	NOOP = 0,
	
	ASSIGN  = ('='),      /*  set value of symbol    */
	MEMBER  = ('.'),      /*  get symbol from scope  */
	ADROF   = ('&'),      /*  c address of           */
	ADR     = ('^'),      /*  cat pointer            */
	INDIR   = ('*'),      /*  c pointer              */
	COMMA   = (','),      /*  list separator         */
	SCOLON  = (';'),      /*  expression separator   */
	TAIL2   = ('.'|x2),   /*  tail w/ 2 periods ..   */
	TAIL3   = ('.'|x3),   /*  tail w/ 3 periods ...  */
	CALL    = ('('*')'),  /*  function call          */
	INDEX   = ('['*']'),  /*  array subscript        */
	ADD     = ('+'),      SUB   = ('-'),
	MUL     = ('*'),      DIV   = ('/'),      MOD  = ('%'),
	INC     = ('+'|x2),   DEC   = ('-'|x2),
	EQ      = ('='|x2),   MORE  = ('>'),      LESS = ('<'),
	NEQ     = ('!'*'='),  GREQ  = ('>'*'='),  LEQ = ('<'*'='),
	AND     = ('&'|x2),   OR    = ('|'|x2),   NOT     = ('!'),
	BAND    = ('&'),      BOR   = ('|'),      XOR = ('^'),
	LSHF    = ('<'|x2),   RSHF  = ('>'|x2),
	LROT    = ('<'|x3),   RROT  = ('>'|x3),
	LEFT    = ('<'*'-'),  RIGHT = ('-'*'>'), 
};}

typedef char** Name;

// ====----====----====----====----====----====----

struct Section;

// recursive structure for all values, expressions, and operations
struct Value
{
	Typeid  type;
	Integral  literal;  // constant value for integral types (float, int, etc)
	int  operation;  // when unset, value is 'returned' from expression
	// refer to constants in 'Opcode'
	
	Value*  left;
	Value*  right;
	
	// in mostly all cases, values are unnamed
	// variables are used for named storage
	char* name;
};

struct Var
{
	Value  value;
	
	char*  name;
};

struct Enum
{
	Trie <char, Var>  constants;
	
	char*  name;
};

struct Section;

struct Func
{
	Typeid  ret;  // return type
	Typeid  args;  // arguments / function signature
	Typeid  target;  // type-specific functions
	
	Section*  body;
	arr < Value >  expressions;

	char*  name;
};

// a user-defined operation is internally a function
// with two arguments: the left & right operand
struct Operator
{
	int  opcode; // constant from 'Opcode'
	
	Typeid  left, right;
	Typeid  result;
	
	Func*  body;
};

// a wrapper for all valid definitions in a scope
struct Symbol
{
	enum { END=(-1), HEADER, TYPE, TYPEDEF, ENUM, VAR, VALUE, FUNC, SECTION };
	// identifies which type is being stored currently
	int symbol;
	
	typedef struct { Typeid type; char* name; } Typedef;
	
	union
	{
		Typeid  type;
		Typedef  typenm;
		Enum*  enm;
		Var  var;
		Value  value;
		Func*  function;
		Section*  section;
		char* header;
	};
};

struct Section
{
	// if fields = 0, this is a standalone set of symbols, like a namespace
	// otherwise, it is a struct with 'fields' instantiated as an object
	Typeid fields;
	
	// using a table so that a typeid can retrieve
	// the string of typeids that created it (reverse lookup)
	// v  v  v  v
	Table <Typeid>  types; // type signatures
	
	Trie <char, Typeid> typedefs; // named types
	Trie <char, Enum>  enums; // map names to values / expressions
	
	Trie <char, Var>  vars;
	Trie <char, Value>  expressions;
	
	Trie <char, Func>  functions;
	Trie <char, Section*>  sections;
	
	// for compile metadata
	typedef int Errc;
	arr <Errc> errors;
	
	// TODO: type flags might be unneccessary at this module / struct def level
	Typeflags flags;
	
	char* name;

	void insert (Symbol);
};

struct Program
{
	Section global;
	
	// user defined operator behaviors
	// only one set of 'operator overloads' per program
	arr <Operator> operators;
	
	char* name;
};

// ====----====----====----====----====----====----

Program getProgram ();


#endif
