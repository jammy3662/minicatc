#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "symbol.h"
#include "cext/trie.h"

namespace CatLang {

// return the root of a syntax tree
Scope ParseSource (char*);
Scope ParseSource (FILE*);

}

#endif
