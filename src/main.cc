#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "trie.h"
#include "token.h"

using namespace CatLang;

// map type names to int values
TrieN <char, int> typeids;

TrieN <char, int> ids;

void dummyParse (FILE* in)
{
	int idIdx = TokenID::NAME + 1;
	
	SetSource (in);
	Scanner scanner;
	
	for (int i = 0; (1); ++i)
	{
			Token w = scanner.get ();
			int id = w.kind;
			
			if (w.kind == TokenID::NAME)
			{
				int err;
				id = ids.find (w.str, 0, &err);
			
				if (err)
					id = idIdx,
					ids.insert (w.str, (char)0, idIdx++);
			}
			
			if (w.kind == TokenID(-1)) break;
			printf ("%u\t\t%s\n", (unsigned short)(id), (char*)w.str);
	}
}

int main (int argc, char** argv)
{
	FILE* in;
	
	SetSource (stdin);
	
	for (int i = 0; i < argc; ++i)
	{
		if (!strcmp(argv[i], "--help"))
		{
			printf ("mcatc [--help | SRC_FILE1 SRC_FILE2 ...]\n"
							"SRC_FILE defaults to standard input\n");
			return 0;
		} 
	}
	
	if (argc > 1)
	{
		for (int i = 1; i <	argc; ++i)
		{
			in = fopen (argv[i], "r");
			if (!in) { fprintf (stderr, "File couldn't open (%s)\n", argv[i]); return 1; }
			printf ("-- Parsing '%s' --\n", argv[i]);
			//dummyParse (in);
			ParseSource (in);
			fclose (in);
		}
	}
	else
	{
		in = stdin;
		printf ("mcatc - Reading from standard input\n");
		//dummyParse (in);
		auto tree = ParseSource (in);
		tree = tree;
	}
	
	return 0;
}
