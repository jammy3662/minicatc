#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "symbol.def"
#include "token.h"

namespace CatLang {

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

struct Scope;

struct Type
{
	enum DataType
	{	//- Special types -//
		INVALID	= -1,
		VOID = 0, STRUCTURED /* struct, union, enum, or function */,
		//- Arithmetic types -//
		CHAR, SHORT, INT, LONG, LONG_L /* long long */,
		FLOAT, DOUBLE, DOUBLE_L /* long double */,
	};
	
	Scope* definition; // struct, union, function, or enum
	DataType data;
	
	byte indirection; // # of pointers to underlying type
	bool const_indirection; // whether pointer can be offset

	byte
	CONST: 1, REGISTER: 1, RESTRICT: 1, VOLATILE: 1,
	EXTERN: 1, STATIC: 1, INLINE: 1, SIGNED: 1, RCI: 2 /* real, complex, imaginary */;
	
	string print ();
};

struct Scope;

struct Symbol
{
	enum Kind
	{
		INVALID = 0xFF, ANY = -1, EMPTY = 0,
		
		PLACEHOLDER,
		MARKER,
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
		IF, WHILE, FOR, SWITCH, CASE, DEFAULT,
		// jump symbols //
		CONTINUE, BREAK, RETURN, GOTO,
		// compile time variables //
		META,
		
		MACRO /* symbol with compile parameters (analagous to templates in c++) */,
		TEMPLATE = MACRO,
		
		UNRESOLVED,
	};
	
	Location loc; // source location
	Scope* parent;
	
	char* comment;
	
	std::vector <Error> errors;
	
	char* name = "unnamed symbol";
	Kind kind;
};

Symbol* const last_symbol = (Symbol*) -1;

struct Marker: Symbol
{
	fast target_index;
	
	Marker () {kind = MARKER;}
};

struct Expression;

struct Object: Symbol
{
	Type datatype;
};

struct Variable: Object
{
	Variable () {kind = VARIABLE;}
	
	Expression* Initializer;
};

struct Expression: Object
{
	Expression () {kind = EXPRESSION;}
	
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
	Meta () {kind = META;}
	
	enum { SIZE, TYPE, COUNT, NAME, FIELDS }
	property;
	
	Symbol* symbol;
};

// simplest complex type
// contains variables with optional names
// ---
// WARNING: dont actually use this as it
// complicates semantic analysis; use Scope instead
struct Tuple: Symbol
{
	Tuple () {kind = TUPLE;}
	
	Array <Symbol*> Fields;
	
	Table <string, fast> Tags;
	
	int Insert (Symbol*, char* name = 0);
};

struct Tag;

// note: Symbol.kind must be set manually
// because a Scope describes so many different symbols
struct Scope: Tuple
{
	Table <string, Tag*> Aliases;
	
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
