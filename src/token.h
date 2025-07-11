#ifndef TOKEN_DOT_H
#define TOKEN_DOT_H

#include "token.def"

#include <stdio.h>
#include "cext/container.h"
#include "ints.h"

void SetSource (FILE*);

struct Location
{
	short line, column;
	char* filePath;
};

struct Token
{
	TokenID id; // encompasses raw characters and enum values
	Location loc;
	array <char> str;
};

TokenType TypeOf (Token);

inline
bool TokenIs (TokenID tt, Token t)
{ return t.id == tt; }

struct Scanner
{
	static array <Token> buffer;
	fast consumed;
	bool undoAfter;
	
	~Scanner ();
	
	Token top (bool discard_comments = true);
	Token get (bool discard_comments = true);
	void unget (Token);
};

#endif
