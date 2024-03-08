#include <stdio.h>
#include <string.h>

#include "trie.h"
#include "words.h"
#include "symbol.h"

// map type names to int values
Trie <char, int> typeids;

Trie <char, int> ids;

int main (int argc, char** argv)
{
	FILE* in;
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "--help"))
		{
			printf ("mcatc [--help | SRC_FILE]\nSRC_FILE defaults to standard input\n");
			return 0;
		}
		in = fopen (argv[1], "r");
		if (!in) { fprintf (stderr, "File couldn't open\n"); return 1; }
	}
	else
	{
		in = stdin;
		printf ("mcatc - Reading from standard input\n");
	}
	char* str;
	
	int idIdx = WordID::TXT + 1;
	int wordct = 2000;
	
	source = in;
	
	for (int i = 0; 1; ++i)
	{
		Word w = getword ();
		int id = w.id;
		if (w.id == WordID::TXT)
		{
			int res = ids.find (w.str, 0, &id);
			if (!res) id = idIdx, ids.insert (w.str, (char)0, idIdx++);
		}
		if (w.id == -1) break;
		printf ("%u\t\t%s\n", (unsigned char)(id), (char*)w.str);
	}
	
	fclose (in);
	
	return 0;
}
