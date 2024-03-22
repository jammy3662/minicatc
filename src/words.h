#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>
#include "container.h"

#define no !

extern FILE* source;

#define WordDOUBLEOPBit  ( (1 << sizeof (char)) << 0 )
#define WordTRIPLEOPBit  ( (1 << sizeof (char)) << 1 )
#define WordCharBits  (sizeof(char) * 8)
#define WordINTBit  (1 << WordCharBits)

enum WordID
{
	INT_LIT = WordINTBit,  FLOAT_LIT,  STR,
	TXT,  PLACEHOLDER, COM_LINE,  COM_BLOCK,
	WordEND,
};

struct Word
{
	long  id;
	arr <char>  str;
};

Word getword ();
void putwordback (Word);

#endif
