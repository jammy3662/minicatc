#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "table.h"

// ====----====----====----====----====----====----

enum SymbolT
{
	ENDF=(-1),
	
	// start symbol enumerations after word ones
	// this way single-char symbols like + and -
	// can be mapped to the same id as their word id 
	OBJ = (WordID::COM_BLOCK+1), STRUCT, ALIAS, ENUM, FUNC,
	TYPE, QUALIFIER, KEYWORD, EXPRESSION,
	POINT /* (^) or (*) */,
};

struct Expr; // expression

// ====----====----====----====----====----====----

enum Opcode
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
};

typedef long Typeid;

// universal symbol identifier
typedef unsigned long iid;

enum Types
{
	INT = (Typeid) 1,  BIT,  CHAR,  BYTE,  SHORT,  LONG,
	LONGLONG,  FLOAT,  DOUBLE,  LONGDOUBLE,
	PTR,
	TypesCOUNT, // number of builtin enum values
};

inline
char isBuiltin (Typeid type)
{  return (type <= PTR);  }

union Integral
{
	int i;  char c;  short s;  long l;
	long long ll;  float f;  double d;  long double ld;
	void* ptr;
};

enum StorageClass
{
	LOCAL = 1, AUTO = (LOCAL),
	STATIC,
	EXTERN,
	REGISTER,
};

enum TypeClass
{
	CONST = 1,
	VOLATILE,
};

struct TypeHeader
{
	StorageClass storage;
	TypeClass flags;
	
	Typeid type;
};

// * modules (namespaces)
// * struct objects (variables)
// * variable instances (objects)
// * function calls (return values)
// * generic values (expressions)
struct Object
{
	TypeHeader type;
	char* name;
	
	Integral integral;
	arr <Object> fields;
	
	char isIntegral: 1,
	     isConstant: 1;
};
typedef Object* Symbol;

struct Var
{
	char* name;
	Symbol value;
};

struct Func
{
	TypeHeader returnType;
	char* name;
	
	TypeHeader object;
	arr <Var> args; // use vars to track arg names (metadata)
	
	Trie <char, int> argTable;
};

struct Enum
{
	TypeHeader type;
	char* name;
	
	arr <Symbol> constants; // enum values can't be variable
	Trie <char, int> index;
};

struct Tuple
{
	// index into internal type list
	Typeid id;
	char* name;
	
	char sharedBits; // if set, treat as a union
	arr <Typeid> fields;
	Trie <char, int> fieldTable;
};

struct Operator
{
	TypeHeader returnType;
	int opcode;
	Var left, right;
};

// ====----====----====----====----====----====----

Symbol getProgram ();

#endif
