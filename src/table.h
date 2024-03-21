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

template <typename K, typename V>
struct Database
{
	arr <V> values;
	Trie <K, int>	index;
	
	V& find (K* key, K stop, int* errc = 0x0)
	{
		return values [ index.find (key, stop, errc) ];
	}
	void insert (K* key, K stop, V val)
	{
		int idx = values.count;
		values.append (val); // (!) this changes the count field
		index.insert (key, stop, idx);
	}
};

#ifndef TABLE_DOT_H_HEADER_ONLY
#define TABLE_DOT_H_HEADER_ONLY

#include "table.cc.h"

#endif



#endif
