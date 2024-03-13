#include <stdio.h>
#include <string.h>

#include "trie.h"
#include "words.h"
#include "symbol.h"

// map type names to int values
Trie <char, int> typeids;

Trie <char, int> ids;

void dummyParse (FILE* in)
{
	int idIdx = WordID::TXT + 1;
	
	source = in;
	
	for (int i = 0; (1); ++i)
	{
		Word w = getword ();
		int id = w.id;
		
		if (w.id == WordID::TXT)
		{
			int err;
			id = ids.find (w.str, 0, &err);
			if (err)
				id = idIdx,
				ids.insert (w.str, (char)0, idIdx++);
		}
		
		if (w.id == -1) break;
		printf ("%u\t\t%s\n", (unsigned short)(id), (char*)w.str);
	}
}

int main (int argc, char** argv)
{
	FILE* in;
	
for (int i = 0; i < argc; ++i)
	if (!strcmp(argv[i], "--help"))
	{
		printf ("mcatc [--help | SRC_FILE1 SRC_FILE2 ...]\nSRC_FILE defaults to standard input\n");
		return 0;
	} 
	
	if (argc > 1)
	{
		for (int i = 1; i <	argc; ++i)
		{
			in = fopen (argv[i], "r");
			if (!in) { fprintf (stderr, "File couldn't open (%s)\n", argv[i]); return 1; }
			printf ("-- Parsing '%s' --\n", argv[i]);
			dummyParse (in);
			fclose (in);
		}
	}
	else
	{
		in = stdin;
		printf ("mcatc - Reading from standard input\n");
	}
	char* str;
	
	return 0;
}
