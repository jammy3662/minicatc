#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "symbol.def"
#include "../token.h"

namespace CatLang {

struct Tuple;

struct Type
{
	enum DataType
	{	//- Special types -//
		VOID = 0, STRUCTURED /* struct, union, enum, or function */,
		//- Arithmetic types -//
		CHAR, SHORT, INT, LONG, LONG_L /* long long */,
		FLOAT, DOUBLE, DOUBLE_L /* long double */,
	};
	
	Tuple* definition; // struct, union, function, or enum
	DataType data;
	
	byte indirection; // pointers (# of)
	bool const_indirection; // whether pointer can be offset

	byte
	CONST: 1, REGISTER: 1, RESTRICT: 1, VOLATILE: 1,
	EXTERN: 1, STATIC: 1, INLINE: 1, SIGNED: 1, RCI: 2 /* real, complex, imaginary */;
};

struct Scope;

struct Error
{
	Token token;
	char* message;
	
	enum Level
	{ NOTE, WARNING, ERROR, }
	severity;
	
	enum Code
	{ SYNTAX, REFERENCE, UNDEFINED, }
	code;
};

struct Symbol
{
	enum Kind
	{
		INVALID = 0xFF, ANY = -1, EMPTY = 0,
		
		PLACEHOLDER,
		
		OBJECT,
		TYPE,
		
		// objects //
		VARIABLE /* data object */, 
		EXPRESSION /* operation on object(s) */,
		FUNCTION,
		// scopes //
		MODULE, STACKFRAME,
		// types //
		TUPLE,
		// scope types //
		STRUCT, UNION, ENUM,
		// selection symbols //
		IF, WHILE, FOR, SWITCH,
		// jump symbols //
		CONTINUE, BREAK, RETURN, GOTO,
		
		MACRO /* symbol with compile parameters (analagous to templates in c++) */,
		TEMPLATE = MACRO,
		
		UNRESOLVED,
	};
	
	Location loc; // source location
	Scope* parent;
	
	char* comment;
	
	std::vector <Error> errors;
	
	char* name = "unnamed object";
	Kind kind;
};

Symbol* const last_symbol = (Symbol*) -1;

struct Goto: Symbol
{
	fast target_index;
};

struct Expression;

struct Object: Symbol
{
	Type datatype;
	
	enum { VARIABLE, EXPRESSION }
	object_kind;
};

struct Variable: Object
{
	Expression* Initializer;
};

struct Expression: Object
{
	enum Opcode {
	// NOTE: this implementation just copies
	// the c operator precedence rules
	// https://en.cppreference.com/w/c/language/operator_precedence
	
	LITERAL, // handwritten / constant value
	VALUEOF, // object / variable access
	
	POST_INCREMENT = 1 /* ++ */,
	POST_DECREMENT = 1 /* -- */,
	CALL = 1 /* () */,
	INDEX = 1 /* [] */,
	ACCESS = 1 /* -> */,
	
	PRE_INREMENT = 2 /* ++ */,
	PRE_DECREMENT = 2 /* -- */,
	POSITIVE = 2 /* + */,
	NEGATIVE = 2 /* - */,
	NOT = 2 /* ! */,
	COMPLEMENT = 2 /* ~ */,
	DEREFERENCE = 2, // * A = 2, // &
	SIZEOF = 2, TYPEOF = 2, COUNTOF = 2 /* arrays only */, NAMEOF = 2,
	FIELDSOF = 2,  // TODO: split this into more useful specializations, like localsof, defsof, enumsof, etc
	
	MULTIPLY = 3, // *
	DIVIDE = 3, // /
	MODULO = 3, // %
	
	ADD = 4, // +
	SUBTRACT	= 4, // -
	
	SHIFT_LEFT = 5, // <<
	SHIFT_RIGHT	= 5, // >>
	ROTATE_LEFT	= 5, // <<<
	ROTATE_RIGHT = 5, // >>>
	
	LESS = 6, // <
	MORE = 6, // >
	LESS_OR_EQUAL = 6, // <=
	MORE_OR_EQUAL = 6, // >=
	
	EQUALS = 7, // ==
	INEQUAL = 7, // !=
	
	AND = 8, // &
	XOR = 9, // ^
	OR = 10, // |
	
	BOTH = 11, // &&
	EITHER = 12, // ||
	
	TERNARY = 13, // ?:
	
	ASSIGN = 14, // =
	ASSIGN_PLUS = 14, // +=
	ASSIGN_MINUS = 14, // -=
	ASSIGN_MULTIPLY = 14, // *=
	ASSIGN_DIVIDE = 14, // /=
	ASSIGN_MODULO = 14, // %=
	ASSIGN_MOD = ASSIGN_MODULO, // %=
	ASSIGN_SHIFT_LEFT = 14, // <<=
	ASSIGN_SHIFT_RIGHT = 14, // >>=
	ASSIGN_ROTATE_LEFT = 14, // <<<=
	ASSIGN_ROTATE_RIGHT = 14, // >>>=
	ASSIGN_AND = 14, // &=
	ASSIGN_XOR = 14, // ^=
	ASSIGN_OR = 14, // |=
	
	APPEND = 15, // ,
	};
	
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
		Object* object;
		Constant literal;
		Expression* operands [2];
	};
	
	Opcode opcode;
	bool constant_value;
};

//  access sizeof, typeof, countof, nameof, or fieldsof an object
struct Meta: Symbol
{
	enum { SIZE, TYPE, COUNT, NAME, FIELDS }
	property;
	
	Symbol* symbol;
};

// simplest complex type
// contains variables with optional names
struct Tuple: Symbol
{
	Array <Symbol*> Fields;
	
	Table <string, fast> Tags;
	
	void Insert (Symbol*, char* name = 0);
};

struct Scope: Tuple
{
	Table <string, fast> Gotos;
	
	Type Receiver; // for methods like int.sign()
	Tuple Parameters; // fields passed in for functions
	
	bool symbolic: 1; // false for modules (single instance)
	bool overlap: 1; // whether fields share memory space (unions)
	bool braced: 1; // true when scope starts with {
	bool closed: 1; // true when } or ...
};

struct Selection: Symbol
{
	
};

Symbol* findin (char* name, Symbol* scope); // look for a symbol only within the scope, not its outer scopes
Symbol* lookup (char* name, Symbol* scope);
Symbol* lookup (char* name, Symbol* scope, byte type);

Error Log (char* message, Token, Error::Level level, Symbol* scope);
Error Log (char* message, Token, Error::Level level, Error::Code code, Symbol* scope);
Error Log (Error err, Symbol* scope);

}

#endif
