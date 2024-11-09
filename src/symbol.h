#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "words.h"
#include "trie.h"

// symbol id
enum Keyid
{
	LOCAL, AUTO,
	STATIC, EXTERN,
	REGISTER, CONST, VOLATILE,
	INLINE,
	
//	SHORT, LONG,
	SIGNED, UNSIGNED,
	
	INCLUDE, MERGE,
	
	SIZEOF, TYPEOF,
	COUNTOF, NAMEOF,
	FIELDSOF,
	
	STRUCT, UNION,
	ENUM, MODULE, LIB,
	
	DO, END,
	IF, ELSE,
	WHILE, SWITCH,
	CASE, DEFAULT,
	BREAK, CONTINUE,
	FOR,
	RETURN,
	
	ENDSYMBOL,
};

static
Trie <char, Keyid> keywords;

struct Stack;

struct Buffer // 
{
	struct Data
	{
		enum Type
		{
			VOID = 0,
			INT, CHAR, SHORT, LONG, LONGLONG,
			FLOAT, DOUBLE, DOUBLEDOUBLE,
			STRUCT, FUNCTION,
		}
		type;
		
		Stack* locals; // fields of a struct
		Stack* args; // argument list
	}
	data;
	
	// count is nonzero for an array
	short count;
	
	// number of pointers to base type
	// ex: (int**) has (indirection = 2)
	short indirection;
	bool pointing:	1; // pointer or reference?
	
	char storage: 3;
	char flags: 3;
};

struct Expr;

union Integral
{
	int i;  char c;  short s;  long l; long long ll;
	float f;  double d;  long double ld;
	char* str; void* ptr;
}; // literal int / float / pointer value

struct Stack
{
	//array <Buffer> ;
	
	struct Object
	{
		Buffer type; // evaluates to this type
		Expr* init; // an initial value or function body
		
	};
	
	bool pod:	1; // 'plain old data'; no expressions; true for structs, false for function bodies
	bool constant: 1; // true for an enum
	bool overlap: 1; // true for a union
	
	// contained within this scope
	array <Object> locals;
	
	// all objects defined in this scope
	Trie <char, Object*> fields;
};

struct Expr
{
	enum Opcode
	{
		// modifiers used so all opcodes are unique
		// but their enum values are readable
		
		x2 = WordDOUBLEOPBit,
		x3 = WordTRIPLEOPBit,
		
		NOOP = 0,
		
		ASSIGN  = ('='),      /*  set value of symbol    */
		ADROF   = ('&'),      /*  c address of           */
		INDIR   = ('*'),      /*  c pointer              */
		COMMA   = (','),      /*  list separator         */
		TAIL2   = ('.'|x2),   /*  tail w/ 2 periods ..   */
		TAIL3   = ('.'|x3),   /*  tail w/ 3 periods ...  */
		INDEX   = ('['*']'),  /*  array subscript        */
		ADD     = ('+'),      SUB   = ('-'),
		MUL     = ('*'),      DIV   = ('/'),      MOD  = ('%'),
		INC     = ('+'|x2),   DEC   = ('-'|x2),
		EQ      = ('='|x2),   MORE  = ('>'),      LESS = ('<'),
		NEQ     = ('!'*'='),  GREQ  = ('>'*'='),  LEQ  = ('<'*'='),
		AND     = ('&'|x2),   OR    = ('|'|x2),   NOT  = ('!'),
		BAND    = ('&'),      BOR   = ('|'),      XOR  = ('^'),
		LSHF    = ('<'|x2),   RSHF  = ('>'|x2),
		LROT    = ('<'|x3),   RROT  = ('>'|x3),
		LEFT    = ('<'*'-'),  RIGHT = ('-'*'>'), 
	};
	
	int operation;
	
	Stack::Object* left, right;
};

#endif
