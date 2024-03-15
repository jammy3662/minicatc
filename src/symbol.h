#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "table.h"

typedef unsigned long Typeid;

namespace Types  { enum
{
	NONE = 0,
	SYMBOL,  MODULE,  FUNC,  STRUCT,  ENUM,  ALIAS,
	INT,  BIT,  CHAR,  BYTE,  SHORT,  LONG,
	LONGLONG,  FLOAT,  DOUBLE,  LONGDOUBLE,  PTR,
};
	const Typeid tpOff = (Types::PTR + 1);
}

union Integral
{
	int i;  char c;  short s;  long l;
	long long ll;  float f;  double d;  long double ld;
	void* ptr;
};

inline
char isBuiltin (Typeid type)
{
	return (type <= Types::PTR);
}

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

// ====----====----====----====----====----====----

struct Expr; // expression

enum SymbolT
{
	ENDF=(-1), NONE=(0),
	
	OBJ, STRUCT, ALIAS, ENUM, FUNC,
	TYPE, QUALIFIER, KEYWORD,
};

struct Symbol
{
	int  kind; // of SymbolT, not to be confused with a datatype (ex. int)
	char*  name;
	
	Trie <char, Symbol*>  fields;
	
	Symbol* get (char* name);
	int insert (char* name, Symbol*);
	Symbol* operator / (char* name) {return get(name);}
};

struct Keyword: Symbol
{
	int id;
};

struct Typesig: Symbol
{
	Typeid  id;
	
	char
	fconst   :  1,  flocal   :  1,
	fstatic  :  1,  fextern  :  1,
	fsigned  :  1,  funion   :  1,
	fshort   :  2,  flong    :  2,
	
	fcregister  :  1,  fcrestrict  :  1,  fcvolatile  :  1;
	
	Typesig* members;
	
	operator Typeid () {return id;}
};

// a typed chunk of memory
struct Object: Symbol
{
	Typesig type;
};

// a constant / handwritten value
// the result of an expression (another value)
// a named variable from the code
struct Var: Object
{
	char  isLiteral: 1; // structs can also be written as a literal
	char  isObject: 1;
	
	union
{
	Integral  constant;
	Expr*  expression;
	Object*  object;
};
};

// recursive structure for all values, expressions, and operations
struct Expr: Object
{
	Typesig  type;
	
	int  operation;
// refer to constants in 'Opcode'
	
	Var  left;
	Var  right;
	
	Var evaluate ();
};

struct Func: Object
{
	Typesig  args, target;
	
	Expr  body;
	
	Var evaluate ();
};

struct Enum
{
	Trie <char, Var>  values;
};

// a user-defined operation is internally a function
// with two arguments: the left & right operand
struct Operator: Func
{
	int  opcode; // constant from 'Opcode'
};

// user defined operator behaviors
// only one set of 'operator overloads' per program
extern
arr <Operator> operators;

// ====----====----====----====----====----====----

Symbol* getProgram ();

#endif
