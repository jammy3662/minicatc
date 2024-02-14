#ifndef WORDS_DOT_H
#define WORDS_DOT_H

#include <stdio.h>

struct Word
{
	enum ID { INT=('~'+2), FINT, DEC, TXT, SYMB };
	
	char* str;
	int id;
	
	union value
	{
		char c;
		short s;
		int i;
				union {
		long l;
		void* p;
				};
		float f;
		double d;
	}
	value;
};

Word getword (FILE* stream);

#endif
