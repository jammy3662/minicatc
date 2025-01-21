#include "symbol.h"
#include "token.h"
#include "expression.h"

#include <stdlib.h>

#include <stdexcept>
using namespace std; // go to hell c++

using namespace CatLang;

typedef Token::ID ID;

List SymbolHeap;

// find the closest scope with given name
// result in innermost scope is returned 
Container* findScopeOfName (char* name, Container* scope)
{
	// name not found here
	while (scope->nametable.count (name) < 1)
	{
		// name is undefined
		if (scope->parent == NULL)
			// avoid checking return value in caller
			throw (0);
		
		// check the outer scope
		scope = scope->parent;
	}
	return scope;
}

Reference Container::lookup (char* name)
{
	Container* owner;
	
	try { owner = findScopeOfName (name, this); }
	catch (int err) { return (Reference){}; }
	
	// return last (most recent) result
	// same-name symbols of differing type won't be accessible by name alone
	return owner->nametable.
		upper_bound (name)->second;
}

Reference Container::lookup	(char* name, SymbolType type)
{
	Container* owner;
	
	try { owner = findScopeOfName (name, this); }
	catch (int err) { return (Reference){}; }
	
	auto lookup = owner->nametable.equal_range (name);
	
	// return the most recent matching result
	// (reverse iterator from end to begin)
	for (auto item = lookup.second; item != lookup.first; item--)
	{
		Reference& ref = item->second;
		if (ref.symboltype == type) return ref;
	}
	
	// no symbol of the specified type was found
	return (Reference){};
}
