#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "table.h"

typedef unsigned long Typeid;

struct Type
{
	Typeid  id;	
	struct
{
	char
	fconst      :  1,
	flocal      :  1,
	fstatic     :  1,
	fextern     :  1,
	fsigned     :  1,
	fshort      :  2,
	flong       :  2,
	
	fcregister  :  1,
	fcrestrict  :  1,
	fcvolatile  :  1;
};
	
	char* name;
	Type* fields;
	
	char* printf ();
	
	operator Typeid () {return id;}
};

namespace Types  { enum
{
	NONE = 0,
	SYMBOL,  MODULE,  FUNC,  STRUCT,  ENUM,  ALIAS,
	INT,  BIT,  CHAR,  BYTE,  SHORT,  LONG,
	LONGLONG,  FLOAT,  DOUBLE,  LONGDOUBLE,  PTR,
};
	const Typeid tpOff = (Types::PTR + 1);
}

inline
char isBuiltin (Typeid type)
{
	return (type <= Types::PTR);
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

// ====----====----====----====----====----====----

struct Section;
struct Symbol;
struct Var;

struct Expr; // expression

enum SymbolT
{
	END=(-1), NONE=(0),
	
	OBJ, MODULE, ALIAS, ENUM, FUNC,
};

struct Symbol
{
	int  kind; // of SymbolT, not to be confused with a datatype (ex. int)
	char*  name;
	
	Trie <char, Symbol*>  fields;
	
	Symbol* get (char* name);
	Symbol& operator / (char* name) {return *get(name);}
};

// a typed chunk of memory
struct Object: Symbol
{
	Type type;
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
}
	value;
};

struct Enum
{
	Trie <char, Var>  values;
};

struct Func: Object
{
	Type  args, target;
	
	Expr*  body;
};

// recursive structure for all values, expressions, and operations
struct Expr: Object
{
	Type  type;
	
	int  operation;
// refer to constants in 'Opcode'
	
	Var  left;
	Var  right;
	
	Var evaluate ();
};



struct Section;

struct Func
{
	Type  ret;  // return type
	Type  args;  // arguments / function signature
	Type  target;  // type-specific functions
	
	Section*  body;
	arr <Value>  expressions;

	char*  name;
};

// a user-defined operation is internally a function
// with two arguments: the left & right operand
struct Operator
{
	int  opcode; // constant from 'Opcode'
	
	Type  left, right;
	Type  result;
	
	Func*  body;
};

union Symbol::Ref
{
	Type  type;  // datatype lookup by name
	Enum  enm;  // map names to values / expressions
	Symbol alias; // handle to another symbol
	Var  var;  // stored value
	Value  value;  // expression
	Func  function;  // expression with inputs
	Section*  section;  // module or struct
	char* header;
};

struct Section
{
	// if static, this is a standalone set of symbols, like a namespace
	// otherwise, it is a struct with fields instantiated as an object
	char isStatic;
	
	Type  fields;
	
	Trie  <char, Symbol>  index;
	Trie  <Type, Type*>  typesigs;
	
	// for compile metadata
	typedef int Errc;
	arr  <Errc>  errors;

	char*  name;

	Symbol insert (Symbol);
	Symbol get (char* name);
	Symbol lookup (char** name);
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
