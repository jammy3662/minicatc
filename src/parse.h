#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "symbol.h"
#include "language.h"
#include "cext/trie.h"

namespace CatLang {

Trie <char, Keyword> keywords;

// return the root of a syntax tree
Scope ParseSource (char*);
Scope ParseSource (FILE*);

}

#endif
