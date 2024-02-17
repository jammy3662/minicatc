#include "trie.h"
#include "words.h"
#include <stdio.h>

// map type names to int values
Trie <char, 0, int, -1> typeids;

Trie <char,0, int,-1> ids;

int main (int argc, char** argv)
{
	FILE* in;
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "--help"))
		{
			printf ("mcatc [SRC_FILE]\nIf you leave out SRC_FILE, it will read from standard input\n");
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
	
	int idIdx = Word::TXT + 1;
	int wordct = 2000;
	
	for (int i = 0; i < wordct; ++i)
	{
		Word w = getword (in);
		int id = w.id;
		if (w.id == Word::TXT)
		{
			id = ids.find (w.str);
			if (id == -1) id = idIdx, ids.insert (w.str, idIdx++);
		}
		if (w.id == -1) break;
		printf ("%i\t\t%s\n", id, (char*)w.str);
	}
	
	fclose (in);
	
	return 0;
}
