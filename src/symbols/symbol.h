#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "../ints.h"
#include "../list.h"

#include <vector>
#include <map>
#include <string>

namespace CatLang {

// TODO: replace C++ STL functionality
template <typename Key, typename Val>
using table = std::multimap <Key, Val>;

enum SymbolType
{
	INVALID = 0,
	PLACEHOLDER,
	
	VARIABLE, // data object
	EXPRESSION, // operation on object(s)
	CONTAINER, // arbitrary structures
	
	ALIAS, // copy of existing symbol
	MACRO, // symbol with compile parameters (analagous to templates in c++)
	TEMPLATE = MACRO,
};

struct Variable;
struct Expression;
struct Container;

struct Symbol
{
	struct Error
	{
		char* message;	
		least severity;
		least code;
	};
	
	// internal representation for comments
	struct Note
	{
		char* text;
	};
	
	Error error;
	Note comment;
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
		//- Arithmetic types -//
		CHAR,
		SHORT,
		INT,
		LONG,
		LONG_L, // long long
		FLOAT,
		DOUBLE,
		DOUBLE_L, // double long
		
		//- Special types -//
		VOID,
		
		STRUCTURED, // struct, union, or enum
		FUNCTION,
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
	};
	
	Container* structure; // struct, union, function, or enum
	DataType datatype;
	Qualifiers qualifiers;
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
		LABEL, // marks the next expression
		
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
		long long int i, Int;
		double f, Float;
		char* str,* Str;
		char c, Char;
	};

	Type result; // resulting datatype of operation between two arbitrary types
	Opcode opcode;
	
	union
	{
		Expression* operand [2];
		Literal literal;
		Container* stackframe;
	};
};

// complex object (with fields/locals)
// has a namespace and symbol lookups
// - struct
// - module (namespace)
// - union
// - function
// - stack frame
struct Container: Symbol
{
	enum ContainerType
	{
		STRUCT, MODULE, UNION, FUNCTION 
	};
	
	std::vector <Variable> locals;
	std::vector <Expression> expressions;
	std::vector <Container> definitions;
	
	// references to internal array elements
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
