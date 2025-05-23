#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "symbol.h"
#include "cext/trie.h"

namespace CatLang
{
	Trie <char, int2> keywords;
	
	// return the root of a syntax tree
	Reference ParseSource (char*);
	Reference ParseSource (FILE*);
}

#endif
