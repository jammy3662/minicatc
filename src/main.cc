#include "trie.h"
#include "words.h"
#include <stdio.h>

// map type names to int values
Trie <char, 0, int, -1> typeids;

int main (int argc, char** argv)
{
	if (argc < 2) return 1;
	FILE* in = fopen (argv[1], "r");
	
	char* str;
	
	int wordct = 200;
	
	for (int i = 0; i < wordct; ++i)
	{
		Word w = getword (in);
		printf ("%s\n", w.str);
		if (w.id == -1) break;
	}
	
	fclose (in);
	
	return 0;
}
