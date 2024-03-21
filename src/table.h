#ifndef TABLE_DOT_H
#define TABLE_DOT_H

#include "container.h"
#include "trie.h"

template <typename K>
struct Table
{
	typedef unsigned short I;
	
	TrieN <K, I>  index;
	arr <K*>  contents;
	
	I val (K* key);
	K* key (I value);
	
	K*& operator [] (I idx) { return contents [idx]; }
};

#ifndef TABLE_DOT_H_HEADER_ONLY
#define TABLE_DOT_H_HEADER_ONLY

#include "table.cc.h"

#endif



#endif
