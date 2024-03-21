#ifndef LANGUAGE_DOT_H
#define LANGUAGE_DOT_H

#include "symbol.h"

#define range(TYPE, TARGET, LIST, BLOCK) for(int i=0;i<sizeof(LIST)/sizeof(*LIST);++i){ const TYPE & TARGET = LIST [i]; BLOCK }

extern
Symbol language;

namespace Builtins
{
enum Keywords
{
	
	VOID_T, LET_T,
	VAR_T, STRUCT_T,
	UNION_T, ENUM_T,
	CHAR_T,
	SHORT_T, INT_T,
	LONG_T,	DOUBLE_T,
	AND, OR,
	IF, ELSE,
	WHILE, SWITCH,
	CASE, DO,
	BREAK, DEFAULT,
	CONTINUE, FOR,
	RETURN, END,
	INCLUDE,
	INLINE, CONST,
	STATIC, EXTERN,
	SIZEOF, TYPEOF,
	COUNTOF, NAMEOF,
};

struct Primitive {char* name; Typeid type;};

static const
Primitive types [] =
{
	{"void", VOID_T}, {"let", LET_T},
	{"auto", LET_T}, {"var", VAR_T},
	{"class", STRUCT_T}, {"struct", STRUCT_T},
	{"union", UNION_T}, {"enum", ENUM_T},
	{"char", CHAR_T},	{"byte", CHAR_T},
	{"short", SHORT_T}, {"int", INT_T},
	{"long", LONG_T},	{"float", DOUBLE_T},
	{"double", DOUBLE_T},
};

static const
Primitive keywords [] =
{
	{"if", IF}, {"else", ELSE},
	{"while", WHILE}, {"switch", SWITCH},
	{"case", CASE}, {"do", DO},
	{"break", BREAK}, {"default", DEFAULT},
	{"continue", CONTINUE}, {"for", FOR},
	{"return", RETURN}, {"end", END},
	{"include", INCLUDE},
	{"inline", INLINE}, {"const", CONST},
	{"static", STATIC}, {"extern", EXTERN},
	{"sizeof", SIZEOF}, {"typeof", TYPEOF},
	{"countof", COUNTOF}, {"nameof", NAMEOF},
};

}

#endif
