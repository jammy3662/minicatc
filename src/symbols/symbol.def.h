#ifndef SYMBOL_DEF_H
#define SYMBOL_DEF_H

#include "../cext/ints.h"
#include "../cext/list.h"

#include <vector>
#include <map>
#include <string>

// TODO: replace C++ STL functionality
template <typename Key, typename Val>
using table = std::multimap <Key, Val>;

struct Variable;
struct Expression;
struct Statement;
struct Container;

enum SymbolType
{
	INVALID = 0,
	PLACEHOLDER,
	
	VARIABLE, // data object
	EXPRESSION, // operation on object(s)
	STRUCT, MODULE, UNION, FUNCTION,
	
	ALIAS, // copy of existing symbol
	MACRO, // symbol with compile parameters (analagous to templates in c++)
	TEMPLATE = MACRO,
};

#endif
