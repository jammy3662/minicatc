#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "symbol.h"
#include "cext/trie.h"

namespace CatLang {

// return the root of a syntax tree
Scope ParseSource (char*);
Scope ParseSource (FILE*);

enum Keyword
{
	// meta
	INCLUDE,
	INLINE,
	SIZEOF, COUNTOF,
	NAMEOF, TYPEOF,
	// storage //
	LOCAL,
	STATIC,
	CONST,
	// alu //
	SIGNED, UNSIGNED,
	COMPLEX, IMAGINARY,
	// flow //
	BREAK, CONTINUE, RETURN, GOTO,
	// declaration //
	STRUCT, UNION,
	MODULE, ENUM,
	// loop //
	WHILE, DO, FOR,
	// selection //
	IF, ELSE, SWITCH,
	CASE, DEFAULT,
};

}

#endif
