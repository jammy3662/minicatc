#ifndef LANGUAGE_DOT_H
#define LANGUAGE_DOT_H

#include "symbol1.h"

#define range(TYPE, TARGET, LIST, BLOCK) for(int i=0;i<sizeof(LIST)/sizeof(*LIST);++i){ const TYPE & TARGET = LIST [i]; BLOCK }

static
Object language;

enum Types
{
	INT = (Typeid) WordCOUNT + 1,  BIT,  CHAR,
	FLOAT,  DOUBLE,
	PTR, VOID,
	VAR, // meta-program constructs
	TypesCOUNT,
};

inline
char isBuiltin (Typeid type)
{  return (type <= PTR);  }

enum Keywords
{
	LOCAL, AUTO=(LOCAL),
	STATIC, EXTERN,
	REGISTER, CONST, VOLATILE,
	INLINE,
	
	SHORT, LONG,
	SIGNED, UNSIGNED,
	
	INCLUDE,
	MERGE,
	
	SIZEOF, TYPEOF,
	COUNTOF, NAMEOF,
	FIELDSOF,
	
	STRUCT,
	UNION,
	ENUM,
	MODULE,
	
	DO, END,
	IF, ELSE,
	WHILE, SWITCH,
	CASE, DEFAULT,
	BREAK, CONTINUE,
	FOR,
	RETURN,
};

static
struct LanguageInit
{ LanguageInit(); } _;

#endif
