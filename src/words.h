#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>
#include "cext/container.h"

// Enum values that don't conflict/collide 
#define WordCharBits  (__CHAR_BIT__)
#define WordDOUBLEOPBit  ((1 << ((WordCharBits) * 1))+ WordCharBits*2)
#define WordTRIPLEOPBit  ((1 << ((WordCharBits) * 2))+ 0)
#define WordINTBit  (1 << ((WordCharBits) * 1))

extern FILE* source;

struct Word
{
// Enum values start past the largest character value
// Shortcut: storing symbols like '(' and ';' as their literal values
	enum ID
	{
		x2 = WordDOUBLEOPBit,
		x3 = WordTRIPLEOPBit,
		
		BANG = '!',
		HASH = '#',
		USD = '$',
		MOD = '%',
		AND = '&',
		STAR = '*',
		PLUS = '+',
		COMMA = ',',
		MINUS = '-',
		DOT = '.',
		SLASH = '/',
		COLON = ':',
		SEMI = ';',
		LEFT = '<',
		RIGHT = '>',
		EQUAL = '=',
		QUESTION = '?',
		AT = '@',
		POWER = '^',
		UNDERSCORE = '_',
		OR = '|',
		TILDE = '~',
		
		GROUP_L = '(',
		GROUP = GROUP_L,
		GROUP_R = ')',
		LIST_L = '[',
		LIST = LIST_L,
		LIST_R = ']',
		BRACE_L = '{',
		BRACE_R = '}',
		
		ANDx2 = AND|x2,
		ORx2 = OR|x2,
		PLUSx2 = PLUS|x2,
		MINUSx2 = MINUS|x2,
		LEFTx2 = LEFT|x2,
		LEFTx3 = LEFT|x3,
		RIGHTx2 = RIGHT|x2,
		RIGHTx3 = RIGHT|x3,
		EQUALx2 = EQUAL|x3, // value produced by EQUAL|x2 conflicts with MINUS
		
		INT_LIT = WordINTBit,
		FLOAT_LIT,
		CHAR_LIT,
		STR_LIT,
		LABEL,
		PLACEHOLDER,
		COM_LINE,
		COM_BLOCK,
	};
	
	ID id; // encompasses raw characters and enum values
	array <char> str;
	
	bool
	isLabel(),
	isSyntax(),
	isLiteral(),
	isEmpty(),
	isInvalid();
};

struct Scanner
{
	static array <Word> source;
	array <Word> buffer;
	
	bool putback;
	
	~Scanner ();
	
	Word top (bool discard_comments = true);
	Word get (bool discard_comments = true);
	void unget (Word);
};

#endif
