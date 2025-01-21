#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>
#include "cext/container.h"
#include "ints.h"

extern FILE* source;

struct Token
{
// Enum values start past the largest character value
// Shortcut: storing symbols like '(' and ';' as their literal values
	enum ID
	{
		PAREN_L = '(', // (
		PAREN = PAREN_L, // ()
		PAREN_R = ')', // )
		BRACKET_L = '[', // [
		BRACKET = BRACKET_L, // []
		BRACKET_R = ']', // ]
		BRACE_L = '{', // {
		BRACE_R = '}', // }
		
		BANG = '!', // !
		HASH = '#', // #
		USD = '$', // $
		MOD = '%', // %
		AND = '&', // &
		STAR = '*', // *
		PLUS = '+', // +
		COMMA = ',', // ,
		MINUS = '-', // -
		DOT = '.', // .
		SLASH = '/', // /
		COLON = ':', // :
		SEMI = ';', // ;
		LEFT = '<', // <
		RIGHT = '>', // >
		EQUAL = '=', // =
		QUESTION = '?', // ?
		AT = '@', // @
		POWER = '^', // ^
		UNDERSCORE = '_', // _
		OR = '|', // |
		TILDE = '~', // ~
		
		x2 = 128,
		x3 = 256,
		
		ANDx2 = AND + x2, // &&
		ORx2 = OR + x2, // ||
		PLUSx2 = PLUS + x2, // ++
		MINUSx2 = MINUS + x2, // --
		LEFTx2 = LEFT + x2, // <<
		RIGHTx2 = RIGHT + x2, // >>
		EQUALx2 = EQUAL + x2, // ==
		
		LEFTx3 = LEFT + x3, // <<<
		RIGHTx3 = RIGHT + x3, // >>>
		
		TAIL, // ..
		ELLIPSES, // ...
		
		ARROW, // -> (legacy / c)
		
		LABEL,
		PLACEHOLDER,
		
		INT_LIT,
		FLOAT_LIT,
		CHAR_LIT,
		STR_LIT,
		
		COM_LINE,
		COM_BLOCK,
	};
	
	ID id; // encompasses raw characters and enum values
	array <char> str;
};

enum TokenType
{
	NONE,
	LABEL,
	PUNCTUATION,
	LITERAL,
	COMMENT,
};
TokenType typeOf (Token);

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
