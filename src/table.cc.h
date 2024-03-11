#ifndef TABLE_DOT_H_IMPL
#define TABLE_DOT_H_IMPL

#include "table.h"

template <typename K>
K* Table<K>::key (I val)
{
	return contents [val];
}

template <typename K>
Table<int>::I Table<K>::val (K* key)
{
	I res;
	K end = {};
	
	int err;
	res = index.find (key, end, &err);
	
	// if not present, add to
	// list & store its index
	if (err)
	{
		res = contents.count;
		index.insert (key, end, res);
		contents.append (key);
	}
	
	return res;
}

#endif
