#ifndef LANGUAGE_DOT_H
#define LANGUAGE_DOT_H

#include "symbol.h"

#define range(TYPE, TARGET, LIST, BLOCK) for(int i=0;i<sizeof(LIST)/sizeof(*LIST);++i){ const TYPE & TARGET = LIST [i]; BLOCK }

static
Object language;

// symbol id
enum SID
{
	INT_T = WordEND + 1,  BIT_T,  CHAR_T,
	FLOAT_T,  DOUBLE_T,
	PTR_T, VOID_T,
	AUTO_T, VAR_T, // meta-program constructs
	
	LOCAL, AUTO,
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
	
	SIDEND,
};

inline
char isBuiltin (long id)
{  return (id < SIDEND);  }

extern
struct LanguageInit
{ LanguageInit(); } _;

Word getwordF (Object* scope = 0x0);

Object getObject (Object* parent);

#endif
