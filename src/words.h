#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>

#define no !

extern FILE* source;

#define WordDOUBLEOPBit  ( (1 << sizeof (char)) << 0 )
#define WordTRIPLEOPBit  ( (1 << sizeof (char)) << 1 )
#define WordCharBits  (sizeof(char) * 8)
#define WordINTBit  (1 << WordCharBits)

namespace WordID  { enum
{
	INT = WordINTBit,  FLOAT,  STR,
	TXT,  PLACEHOLDER, COM_LINE,  COM_BLOCK,
};}

struct Word
{
	short  id;
	char*  str;
};

Word getword ();
void putwordback (Word);

#endif
