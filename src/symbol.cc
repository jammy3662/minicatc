#include "symbol.h"

#include "trie.h"

// identifies a type signature with a number
Trie <Typeid, Typeid>
typetable;

arr <Typeid*>
types;

Typeid getType (Typeid* sig)
{
	Typeid ret = 0;
	
	// if not present, add to type list
	// and store its index
	if (!typetable.find (sig, 0, &ret))
	{
		ret = types.count;
		typetable.insert (sig, 0, ret);
		types.append (sig);
	}
	
	return ret;
}

Typeid* getFields (Typeid t)
{
	return types [t];
}
