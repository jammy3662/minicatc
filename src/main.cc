#include <stdio.h>
#include <string.h>

#include "trie.h"
#include "words.h"
#include "parse.h"

using namespace CatLang;

// map type names to int values
TrieN <char, int> typeids;

TrieN <char, int> ids;

void dummyParse (FILE* in)
{
	int idIdx = Word::LABEL + 1;
	
	source = in;
	Scanner scanner;
	
	for (int i = 0; (1); ++i)
	{
			Word w = scanner.get ();
			int id = w.id;
			
			if (w.id == Word::LABEL)
			{
				int err;
				id = ids.find (w.str, 0, &err);
			
				if (err)
					id = idIdx,
					ids.insert (w.str, (char)0, idIdx++);
			}
			
			if (w.id == Word::ID(-1)) break;
			printf ("%u\t\t%s\n", (unsigned short)(id), (char*)w.str);
	}
}

int main (int argc, char** argv)
{
	FILE* in;
	
	source = stdin;
	
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
			source = in;
			//dummyParse (in);
			parseSource ();
			fclose (in);
		}
	}
	else
	{
		in = stdin;
		printf ("mcatc - Reading from standard input\n");
		source = in;
		//dummyParse (in);
		auto tree = parseSource ();
		tree = tree;
	}
	
	return 0;
}
