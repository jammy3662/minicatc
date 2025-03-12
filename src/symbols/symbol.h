#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "symbol.def.h"

namespace CatLang {

struct Symbol
{
	struct Error {
		char* message;	
		least severity;
		least code; }
	error;
	
	// internal representation for comments
	struct Note {
		char* text; }
	comment;
	
	char* name;
	
	Container* parent;
};

struct Reference
{
	Symbol* symbol;
	SymbolType symboltype;
};

struct Type
{
	enum DataType
	{
		//- Special types -//
		VOID = 0,
		
		STRUCTURED, // struct, union, or enum
		FUNCTION,
		
		//- Arithmetic types -//
		CHAR,
		SHORT,
		INT,
		LONG,
		LONG_L, // long long
		FLOAT,
		DOUBLE,
		DOUBLE_L, // double long
	};
	
	struct Qualifiers
	{
		byte
		CONST: 1,
		REGISTER: 1,
		RESTRICT: 1,
		VOLATILE:	1,
		EXTERN: 1,
		STATIC: 1,
		INLINE:	1;
		
		enum Math
		{
			SIGNED = 0,
			UNSIGNED = 1,
			COMPLEX = 2,
			IMAGINARY = 3,
		};
		byte MATH: 2;
	}
	qualifiers;
	
	Container* structure; // struct, union, function, or enum
	byte datatype;
	byte indirection; // pointers (# of)
};

struct Variable: Symbol
{
	Type type;
	Expression* initial;
};

struct Expression: Symbol
{
	enum Opcode
	{
		// NOTE: this implementation just copies
		// the c operator precedence rules
		// https://en.cppreference.com/w/c/language/operator_precedence
		
		LITERAL, // handwritten or constant value
		BLOCK, // stack frame to evaluate within
		MARKER, // marks the next expression
		VALUEOF, // variable access
		
		// selection and control expressions
		SWICH, IF, IF_ELSE, WHILE, DO_WHILE,
		FOR, RETURN, GOTO, BREAK, CONTINUE,
		
		POST_INCREMENT = 1, // ++
		POST_DECREMENT = 1, // --
		CALL = 1, // ()
		SUBSCRIPT = 1, // []
		INDEX = SUBSCRIPT, // []
		ACCESS = 1, // ->
		
		PRE_INREMENT = 2, // ++
		PRE_DECREMENT = 2, // --
		POSITIVE = 2, // +
		NEGATIVE = 2, // -
		NOT = 2, // !
		COMPLEMENT = 2, // ~
		DEREFERENCE = 2, // *
		ADDRESS = 2, // &
		SIZEOF = 2, // sizeof
		
		MULTIPLY = 3, // *
		DIVIDE = 3, // /
		MODULO = 3, // %
		MOD = MODULO, // %
		
		PLUS = 4, // +
		MINUS	= 4, // -
		
		SHIFT_LEFT = 5, // <<
		SHIFT_RIGHT	= 5, // >>
		ROTATE_LEFT	= 5, // <<<
		ROTATE_RIGHT = 5, // >>>
		
		LESS = 6, // <
		MORE = 6, // >
		LESS_OR_EQUAL = 6, // <=
		MORE_OR_EQUAL = 6, // >=
		
		EQUAL = 7, // ==
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
		
		COMMA = 15, // ,
	};
	
	union Literal
	{
		most int i, Int;
		double f, Float;
		char* str,* Str;
		char c, Char;
	};

	Type result; // resulting datatype of operation
	std::vector <Expression> subexpressions;
	Opcode opcode;
	
	union {
		Expression* operand [2];
		Literal literal;
		Reference label; };
};

// complex object (with fields/locals)
// has a namespace and symbol lookups
// - struct
// - module (namespace)
// - union
// - function
// - stack frame
// - tuple (stripped-down struct)
struct Container: Symbol
{
	std::vector <Variable> locals;
	std::vector <Expression> expressions; // expressions defined at this scope (subexpressions are stored within each expression)
	std::vector <Container> definitions;
	
	// refers to locals, expressions, and definitions
	std::vector <Reference> members;
	
	//- function-related -//
	Container* arguments;
	Type receiever; // void for non-method functions
	 
	std::multimap
	<std::string, Reference> nametable;
	
	Reference lookup (char* name);
	Reference lookup (char* name, SymbolType symboltype);
};

}

#endif
