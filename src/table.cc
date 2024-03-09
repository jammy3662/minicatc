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
	
	int find = index.find (key, end, &res);
	
	// if not present, add to
	// list & store its index
	if (find == 0)
	{
		res = contents.count;
		index.insert (key, end, res);
		contents.append (key);
	}
	
	return res;
}
