#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include <sstream>
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

struct Tuple;
struct Scope;

struct Tag;

struct Type
{
	enum DataType
	{	//- Special types -//
		NONE	= -1,
		VOID = 0, STRUCTURED /* struct, union, function, or enum */,
		//- Arithmetic types -//
		BIT, CHAR, BYTE, SHORT, INT, LONG,
		FLOAT,
	};
	
	DataType data;
	
	union {
	Tuple* tuple;
	Scope* definition;
	};
	
	byte indirection_ct;
	long indirection; // sequence of pointers or references to underlying type
	// (2 bits each, constness [1] and ptr/ref [0])

	enum RIC
	{ REAL = 0b00, IMAGINARY = 0b01, COMPLEX = 0b11 };

	unsigned byte
	CONST: 1, STATIC: 1, LOCAL: 1, INLINE: 1, SIGNED: 1, DOUBLE: 1, RIC: 2,
	REGISTER: 1, RESTRICT: 1, VOLATILE: 1, EXTERN: 1; // real, imaginary, complex (both) //;
	
	Tuple* parameters; // for function types
	
	void indirection_set_const (bool constness);
	void indirection_set_ref (bool refness);
	bool indirection_const (fast idx);
	bool indirection_ref (fast idx);
	
	string print ();
	string print_data ();
	
	static
	Type parse (Scope*,
	            Tag* first_tag = 0);
	
	static
	Type invalid ();
	
	bool is_valid ();
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
	
	template <typename... T>
	Error Log (Error::Level lvl, Token token, T... diagnostics);
	
	static
	Symbol* parse (Scope*);
};

Symbol* const last_symbol = (Symbol*) -1;

struct Marker: Symbol
{
	fast target_index;
	
	Marker () {kind = MARKER;}
	
	static
	Marker* parse (Scope*, Tag* name = 0);
};

struct Expression;

struct Object: Symbol
{
	Type datatype;
	
	static
	Object* parse (Scope*,
		Tag* first_tag = 0);
};

struct Variable: Object
{
	Variable () {kind = VARIABLE;}
	
	Expression* Initializer;
	
	static
	Variable* parse (Scope*,
		Type type,
		Tag* identifier,
		bool initialize);
	
	static
	Variable* parse (Scope*,
		Tag* first_tag = 0);
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
	bool constant_value = false;
	bool parenthesized = false;
	
	struct OperatorType
	{
		bool prefix: 1;
		bool infix: 1;
		bool postfix: 1;
	};

	// an operator may have multiple positions
	// example, ++ is both prefix & postfix
	static OperatorType TypeOfOp (Token);

	static bool IsOperator (Token);
	
	static
	Expression* parse
	(
		Scope*,
		bool parenthesized = false,
		Tag* first_tag = 0
	);
	
	// pre-typed expression
	static
	Expression* parse
	(
		Scope*,
		bool parenthesized = false,
		Type* type = 0,
		Tag* next_tag = 0
	);
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
	
	static
	Tuple* parse (Scope*,
	              Tag* first_tag = 0);
	
	static Type asType ();
};

// note: Symbol.kind must be set manually
// because a Scope describes so many different symbols
struct Scope: Tuple
{
	Table <string, Tag*> Aliases;
	
	Tuple* Parameters; // fields passed in for functions
	
	bool symbolic: 1; // false for modules (single instance)
	bool overlap: 1; // whether fields share memory space (unions)
	bool braced: 1; // true when scope starts with {
	bool closed: 1; // true when } or ...
	
	static
	Scope* parse (Scope* parent, bool embedded = false, bool braced = false);
	// * if embedded, parse directly into parent scope,
	//   rather than creating a new scope in parent
	// * if braced, expect a } to close instead of ...
};

struct Struct: Scope
{
	Struct () { kind = STRUCT; }
	
	static
	Struct* parse (Scope*, Tag* first_tag = 0);
};

struct Union: Struct
{
	Union () { kind = UNION; }
	
	static
	Union* parse (Scope*);
};

struct Module: Struct
{
	Module () { kind = MODULE; }
	
	static
	Module* parse (Scope*);
};

struct Enum: Scope
{
	Enum () { kind = ENUM; }
	
	static
	Enum* parse (Scope*);
};

struct Function: Scope
{
	Function () { kind = FUNCTION; }
	
	static
	Function* parse (Scope*);
};

struct Selection: Symbol
{
	
};

//  access sizeof, typeof, countof, nameof, or fieldsof an object
struct Meta: Expression
{
	Meta () {kind = META;}
	
	enum { SIZE, TYPE, COUNT, NAME }
	property;
	
	Object* object;
	
	static
	Meta* parse (Scope*);
};

Symbol* findin (char* name, Symbol* scope); // look for a symbol only within the scope, not its outer scopes
Symbol* lookup (char* name, Symbol* scope);
Symbol* lookup (char* name, Symbol* scope, byte type);

Error Log (char* message, Token, Error::Level level, Symbol* scope);
Error Log (char* message, Token, Error::Level level, Error::Code code, Symbol* scope);
Error Log (Error err, Symbol* scope);

template <typename... T>
Error Symbol::Log (Error::Level lvl, Token token, T... diagnostics)
{
	std::stringstream msg;
	
	(msg << ... << diagnostics);
		
	auto str = msg.str();
	auto cstr = (char*) str.c_str();
	
	return CatLang::Log (cstr, token, lvl, this);
}

}

#endif
