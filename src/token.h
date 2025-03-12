#ifndef TOKEN_DOT_H
#define TOKEN_DOT_H

#include "token.def.h"

#include <stdio.h>
#include "cext/container.h"
#include "ints.h"

extern FILE* source;

struct Token
{
	typedef TokenID ID;
	
	ID id; // encompasses raw characters and enum values
	array <char> str;
};

TokenType tokenType (Token);

struct Scanner
{
	array <Token> buffer;
	fast consumed;
	bool undoAfter;
	
	~Scanner ();
	
	Token top (bool discard_comments = true);
	Token get (bool discard_comments = true);
	void unget (Token);
};

#endif
