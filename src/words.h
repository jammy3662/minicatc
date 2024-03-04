#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>

#include "container.h"

extern FILE* source;

struct Word
{
	enum
	{
		INT=(-'~'), FLOAT, STR,
		COM_LINE, COM_BLOCK, TXT
	};
	
	char id;
	char multiple;
	arr <char> str;
};

Word getword ();

#endif
