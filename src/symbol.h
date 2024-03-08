#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"

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

// recursive structure for all values, expressions, and operations
struct Value
{
	Typeid  type;
	Integral  literal;  // constant value for integral types (float, int, etc)
	int  operation;  // when unset, value is 'returned' from expression
	// refer to constants in 'Opcode'
	
	Value*  left;
	Value*  right;
};

struct Var
{
	Typeid  type;
	Value*  initial;
	
	char*  name;
};

struct Scope;

struct Func
{
	int  op;
	
	Typeid  ret;  // return type
	Typeid  args;  // arguments / function signature
	Typeid  target;  // type-specific functions
	
	Scope*  body;

	char*  name;
};

struct Operator
{
	int operation; // constant from 'Opcode'
	
	Typeid operands [3]; // 3rd char is null terminator
	
	Scope* body;
};

int getTypeid (Typeid* fields, Typeid* aID);
int getFields (Typeid id, Typeid** aFields);

int getType (Typeid* _type_);

#endif
