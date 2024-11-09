#ifndef CST_DOT_H
#define CST_DOT_H

#include "ints.h"
#include "words.h"

#include <vector>
#include <map>
#include <string>

// Cat Symbol Tree
namespace CST
{
	struct Symbol;
	
	struct Value
	{
		union
		{
			long long int i;
			double f;
			char* str;
			char c;
		};
	};
	
	struct Instance
	{
		Symbol* reference;
	};
	
	struct Operation
	{
		fast opcode; // refers to word id
		
		enum { INFIX=0, POSTFIX, PREFIX };
		fast position;
		
		inline fast isUnary () { return !!(position); }
		
		union{ Symbol* leftSide; Symbol* operand; };
		union{ Symbol* rightSide; };
	};
	
	struct Symbol
	{
		char* name;
		fast index;
		Symbol* parent;
		
		enum type
		{
			// use as const Symbol*
			NOT_FOUND = (-2),
			END_OF_FILE = (-1),
			ERROR = (0),
			
			// type values
			PLACEHOLDER = (fast)(1),
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
		
		union Data
		{
			Value literal;
			struct { Symbol* type; } instance;
			Operation expr;
		}
		data;
		
		std::vector <Symbol> contents;
		std::multimap <std::string, fast> nametable;
		
		bool isInstance ();
		
		Symbol& insert (Symbol);
		Symbol& generate (char* name = 0x0);
		Symbol* find (char* name, fast expect_type = 0);
	};
	
	// return the root of a syntax tree
	Symbol parseSource ();
};

#endif
