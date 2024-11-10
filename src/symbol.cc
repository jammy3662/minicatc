#include "symbol.h"
#include "words.h"
#include "expression.h"

#include <stdlib.h>

#include <stdexcept>
using namespace std; // go to hell c++

using namespace CatLang;

typedef Word::ID ID;

Reference Symbol::insert (Symbol* s)
{
	s->parent = this;
	s->index = contents.size();
	contents.push_back (s);
	
	if (s->name != 0x0)
	{
		pair <string, fast>
			entry (s->name, s->index);
	
		nametable.insert (entry);
	}
	
	return contents.back();
}

Symbol* Symbol::find (char* name, fast expectType)
{
	if (nametable.count (name) < 1)
		return (Symbol*)(0);
	
	auto lookup = nametable.equal_range (name);
	
	if (not expectType)
		// return the first search result
		return contents [lookup.first->second];
	
	// look for specific type of symbol
	for (auto i = lookup.first; i != lookup.second; ++i)
		if (i->second == expectType)
			return contents [i->second];
	
	// match not found
	return (Symbol*)(0);
}

bool Symbol::isInstance ()
{
	switch (type)
	{
		default: break;
		case (INSTANCE):
		case (VALUE):
		case (OPERATION):
		return true;
	}
	return false;
}
