#include "trie.h"
#include "words.h"
#include <stdio.h>

// map type names to int values
Trie <char, 0, int, -1> typeids;

Trie <char,0, int,-1> ids;

int main (int argc, char** argv)
{
	FILE* in;
	if (argc >= 2) in = fopen (argv[1], "r");
	else in = stdin;
	
	char* str;
	
	int idIdx = 0;
	int wordct = 200;
	
	for (int i = 0; i < wordct; ++i)
	{
		Word w = getword (in);
		int id = w.id;
		if (w.id == Word::TXT)
		{
			id = ids.find (w.str);
			if (id == -1) id = idIdx, ids.insert (w.str, idIdx++);
		}
		printf ("%i\t\t%s\n", id, (char*)w.str);
		if (w.id == -1) break;
	}
	
	fclose (in);
	
	return 0;
}
