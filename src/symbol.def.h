#ifndef SYMBOL_DEF_H
#define SYMBOL_DEF_H

#include <vector>
#include <map>
#include <string>

#include "cext/ints.h"
//#include "cext/list.h"

// TODO: replace C++ STL functionality
template <typename T>
using Array = std::vector <T>;

template <typename Key, typename Val>
using Table = std::multimap <Key, Val>;

using string = std::string;

enum ErrorLevel
{
	LEVEL_NOTE,
	LEVEL_WARNING,
	LEVEL_ERROR,
};

enum ErrorCode
{
	ERROR_SYNTAX,
	ERROR_REFERENCE,
	ERROR_UNDEFINED,
};

enum SymbolTypes
{
	INVALID = 0xFF,
	ANY = -1,
	
	EMPTY,
	
	PLACEHOLDER,
	
	VARIABLE, // data object
	EXPRESSION, // operation on object(s)
	TUPLE,
	STRUCT, UNION, ENUM, FUNCTION, // scope types
	MODULE, STACKFRAME, 
	
	ALIAS, // copy of existing symbol
	MACRO, // symbol with compile parameters (analagous to templates in c++)
	TEMPLATE = MACRO,
	
	UNRESOLVED,
};

enum DataType
{
	//- Special types -//
	VOID = 0,
	STRUCTURED, // struct, union, enum, or function
	
	//- Arithmetic types -//
	CHAR,
	SHORT,
	INT,
	LONG,
	LONG_L, // long long
	FLOAT,
	DOUBLE,
	DOUBLE_L, // long double
};

enum TypeMathFlags
{
	SIGNED = 0,
	UNSIGNED = 1,
	COMPLEX = 2,
	IMAGINARY = 3,
};

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

enum KeywordID
{
	KW_LOCAL,
	KW_STATIC,
	KW_CONST,
	KW_INLINE,
	
	KW_MATH_SIGNED, KW_MATH_UNSIGNED,
	KW_MATH_COMPLEX, KW_MATH_IMAGINARY,
	
	KW_INCLUDE,
	
	KW_SIZEOF, KW_TYPEOF,
	KW_COUNTOF, KW_NAMEOF,
	KW_FIELDSOF,
	
	KW_STRUCT, KW_UNION,
	KW_ENUM, KW_MODULE,
	
	KW_IF, KW_ELSE,
	KW_DO, KW_WHILE, KW_FOR,
	KW_SWITCH, KW_CASE, KW_DEFAULT,
	KW_BREAK, KW_CONTINUE,
	KW_RETURN
};

#endif
