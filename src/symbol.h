#ifndef SYMBOL_DOT_H
#define SYMBOL_DOT_H

#include "ints.h"

#include <vector>
#include <map>
#include <string>

namespace CatLang
{
	struct Symbol;
	
	// TODO: write a more portable implementation
	// for references (pointers may not be a given)
	typedef Symbol* Reference;
	typedef Reference Ref;
	
	// implementation specific containers
	template <typename T>
	using list = std::vector <T>;
	template <typename Key, typename Val>
	using table = std::multimap <Key, Val>;
	
	struct Symbol
	{
		char* name;
		fast index;
		Reference parent;
		list <Symbol*> contents;
		table <std::string, fast> nametable;
		
		enum type
		{
			ERROR = (fast)(0),
			PLACEHOLDER = (1),
			ALIAS,
			MACRO,
			STRUCT,
			ENUM,
			UNION,
			INSTANCE,
			VALUE,
			OPERATION,
		}
		type;
		
		bool isInstance ();
		
		Reference insert (Symbol*);
		Reference find (char* name, fast expect_type = 0);
	};
};

#endif
