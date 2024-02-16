#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>

#include "container.h"

struct Word
{
	enum ID { INT=('~'+2), FLOAT, STR, TXT };
	
	arr <char> str;
	int id;
	
	union value
	{
		char c; short s; int i;
		long l; void* p;
		float f; double d;
	}
	value;
};

Word getword (FILE* stream);

#endif
