#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "table.h"

struct Expr; // expression

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

union Integral
{
	int i;  char c;  short s;  long l;
	long long ll;  float f;  double d;  long double ld;
	void* ptr;
};

enum StorageClass
{
	fLOCAL = 1,
	fSTATIC,
	fEXTERN,
	fREGISTER,
};

enum TypeClass
{
	fCONST = 1,
	fVOLATILE,
};

struct SymbolTable;

struct Type
{
	StorageClass storage;
	TypeClass flags;
	
	Typeid id;
};

// * modules (namespaces)
// * struct objects (variables)
// * variable instances (objects)
// * function calls (return values)
// * generic values (expressions)
struct Object
{
	Type type;
	char* name;
	
	char isIntegral: 1, // int, float, etc
	     isConstant: 1; // whether handwritten value
	
	Integral integral;
	arr <Object> fields;
	
	SymbolTable* defs;
	
	int count; // array size
};
typedef Object* Symbol;

struct Func
{
	Type returnType;
	char* name;
	
	Type objectType;
	arr <Symbol> args; // use vars to track arg names (metadata)
	
	Trie <char, int> argTable;
};

struct Enum
{
	Type type;
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
	Type returnType;
	int opcode;
	Object left, right;
};

struct SymbolTable
{
	Database <char, Type, Typeid> types;
	Trie <char, long> keywords;
	
	Database <char, Func> functions;
	Database <char, Enum> enums;
	Database <char, Tuple> tuples;
	Database <char, Operator> ops;
};



// ====----====----====----====----====----====----

void getProgram ();

#endif
