#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "cext/trie.h"
#include "token.h"
#include "ints.h"

namespace CatLang {

struct Symbol;

// return the root of a syntax tree
Symbol* ParseSource (char*);
Symbol* ParseSource (FILE*);

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
	// jump //
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
