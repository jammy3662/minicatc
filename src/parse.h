#ifndef PARSE_DOT_H
#define PARSE_DOT_H

#include "symbol.h"

#include "cext/trie.h"
#include "token.h"
#include "ints.h"

namespace CatLang {

// return the root of a syntax tree
Symbol* ParseSource (char*);
Symbol* ParseSource (FILE*);

}

#endif
