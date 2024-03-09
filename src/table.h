#ifndef TABLE_DOT_H
#define TABLE_DOT_H

#include "container.h"
#include "trie.h"

template <typename K>
struct Table
{
	typedef unsigned short I;
	
	Trie <K, I>  index;
	arr <K*>  contents;
	
	K* key (I value);
	I val (K* key);
	
	K*& operator [] (I idx) { return contents [idx]; }
};

#endif
