#ifndef PARSE_DEF_H
#define PARSE_DEF_H

#ifdef OLD_KEY_ID
enum Keyid
{
	LOCAL,
	STATIC,
	CONST,
	
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
#endif // OLD_KEY_ID

#endif
