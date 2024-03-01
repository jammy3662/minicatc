#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>

#include "container.h"

extern FILE* source;

struct Word
{
	enum
	{
		INT=('~'+2), FLOAT, STR,
		COM_LINE, COM_BLOCK, TXT
	};
	
	arr <char> str;
	int id;
	int multiple;
	Word ();
};

Word getword ();

#endif
