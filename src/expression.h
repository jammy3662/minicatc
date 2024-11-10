#ifndef EXPRESSION_DOT_H
#define EXPRESSION_DOT_H

#include "symbol.h"
#include "words.h"

// continuing from "symbol.h"
namespace CatLang
{
	struct Literal: Symbol
	{
		union
		{
			long long int i, Int;
			double f, Float;
			char* str,* Str;
			char c, Char;
		};
		
		// a literal could be a compound object
		// with many values
		enum { COMPLEX=0, INT, FLOAT, STR, CHAR, }
		integralType;
		
		Reference dataType;
	};
	
	struct Expression: Symbol
	{
		fast opcode; // refers to word id
		
		enum { INFIX = (fast)0, POSTFIX, PREFIX }
		position;
		
		union{ Symbol* leftSide; Symbol* operand; };
		union{ Symbol* rightSide; };
		
		fast priority ();
		
		inline fast isUnary () { return !!(position); }
	};
	
	bool operator > (Expression, Expression);
	
	bool isPrefixOperator (Word),
	     isInfixOperator (Word),
			 isPostfixOperator (Word),
			 isOperator (Word);
}

#endif
