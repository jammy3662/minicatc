#include "cst.h"
#include "words.h"

#include <stdlib.h>

#include <stdexcept>
using namespace std; // go to hell c++

using namespace CST;

typedef Word::ID ID;

Symbol& Symbol::insert (Symbol s)
{
	s.parent = this;
	s.index = contents.size();
	contents.push_back (s);
	
	if (s.name != 0x0)
	{
		pair <string, fast>
			entry (s.name, s.index);
	
		nametable.insert (entry);
	}
	
	return contents.back();
}

Symbol& Symbol::generate (char* name)
{
	Symbol s = {};
	s.name = name;
	return insert (s);
}

Symbol* Symbol::find (char* name, fast expectType)
{
	if (nametable.count (name) < 1)
		return (Symbol*)(0);
	
	auto lookup = nametable.equal_range (name);
	
	if (not expectType)
		// return the first search result
		return &(contents [lookup.first->second]);
	
	// look for specific type of symbol
	for (auto i = lookup.first; i != lookup.second; ++i)
		if (i->second == expectType)
			return &(contents [i->second]);
	
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

inline bool isPrefixOperator (Word w)
{
	switch (w.id)
	{
		default: break;
		case (w.PLUS):
		case (w.MINUS):
		case (w.PLUSx2): // increment
		case (w.MINUSx2): // decrement
		case (w.TILDE): // complement
		case (w.BANG): // not
		case (w.AND): // address of
		case (w.STAR): // value at
		case (w.GROUP_L): // () enclosed expression
		return true;
	}
	return false;
}

inline bool isPostfixOperator (Word w)
{
	switch (w.id)
	{
		default: break;
		case (w.PLUSx2):
		case (w.MINUSx2):
		case (w.GROUP_L): // function call
		case (w.LIST_L): // subscript
		return true;
	}
	return false;
}

bool isInfixOperator (Word w)
{
	switch (w.id)
	{
		default: break;
		case (w.PLUS):
		case (w.MINUS):
		case (w.STAR): // multiply
		case (w.SLASH): // divide
		case (w.MOD): // modulo
		case (w.AND): // and
		case (w.OR):	// or
		case (w.POWER):	// xor
		case (w.LEFTx2): // left shift
		case (w.RIGHTx2): // right shift
		case (w.LEFTx3): // left rotate
		case (w.RIGHTx3): // right rotate
		case (w.ANDx2): // logical and
		case (w.ORx2): // logical or
		case (w.BANG): // not
		case (w.LEFT): // less than
		case (w.RIGHT): // more than
		case (w.EQUAL):
		return true;
	}
	return false;
}

inline bool isOperator (Word w)
{
	return (isInfixOperator(w) or
	        isPrefixOperator (w));
					// postfix operators are repeats
}

Symbol* parseLabel (Word word, Symbol* scope)
{
	Symbol& label = scope->generate();
	
	return &(label);
}

Symbol* parseLiteral (Word literal, Symbol* scope)
{
	Symbol& s = scope->generate();
	s.type = Symbol::VALUE;
	
	switch (literal.id)
	{
		default: break;
		case (ID::INT_LIT):
			s.data.literal.i = atoll (literal.str);
			break;
		case (ID::FLOAT_LIT):
			s.data.literal.f = atof (literal.str);
			break;
		case (ID::STR_LIT):
		case (ID::CHAR_LIT):
			s.data.literal.str = literal.str;
			break;
	}
	
	return &s;
}

// operator precedence
fast priority (Operation op)
{
	// NOTE: this implementation just copies
	// the c operator precedence rules
	// ( https://en.cppreference.com/w/c/language/operator_precedence )
	
	switch (op.opcode)
	{
		case (ID::GROUP):
			// prefix: enclosed expression: (1+1)
			// postfix: function call: fn (args, etc)
			// infix: type cast: (float) (1/4)
			switch (op.position) {
				case (op.PREFIX): return 0;
				case (op.POSTFIX): return 1;
				case (op.INFIX): return 2; }
		
		case (ID::LIST):
			// postfix: array subscript: array[index]
			// prefix: literal array: [item1, item2, etc]
			return (op.position == op.POSTFIX) ? 1 : 0;
		
		case (ID::PLUSx2):
		case (ID::MINUSx2):
			return (op.position == op.PREFIX) ? 1 : 2;
		
		case (ID::PLUS):
		case (ID::MINUS):
			return (op.position == op.PREFIX) ? 2 : 4;
		
		case (ID::BANG):
		case (ID::TILDE):
			return 2;
		
		case (ID::STAR):
			return (op.position == op.PREFIX) ? 2 : 3;
		case (ID::AND):
			return (op.position == op.PREFIX) ? 2 : 8;
		
		case (ID::SLASH):
		case (ID::MOD):
			return 3;
		
		case (ID::LEFTx2):
		case (ID::RIGHTx2):
			return 5;
		
		case (ID::LEFT):
		case (ID::RIGHT):
			return 6;
		
		case (ID::EQUALx2):
			return 7;
		
		case (ID::POWER): // xor
			return 9;
		
		case (ID::OR):
			return 10;
		
		case (ID::ANDx2):
			return 11;
			
		case (ID::ORx2):
			return 12;
		
		case (ID::EQUAL):
			return 14;
		
		case (ID::COMMA):
			return 15;
	}
	
	// if none of the above,
	// treat with "lowest" priority
	return ( (unsigned)(fast)(-1) );
}

bool operator > (Operation left, Operation right)
{
	return (priority (left) > priority (right));
}

inline bool operator < (Operation left, Operation right) { return !(left > right); }

Symbol* parseExpression (Symbol* scope)
{
	Scanner scanner = {};
	scanner.putback = true;
	Word first = scanner.get();
	
	Symbol& symbol = scope->generate();
	symbol.type = Symbol::OPERATION;
	
	Operation& expr = symbol.data.expr;
	expr.opcode = (0);
	
	Operation& leftSide = expr.leftSide->data.expr;
	Operation& rightSide = expr.rightSide->data.expr;
	
	if (isPrefixOperator (first))
	{
		expr.position = Operation::PREFIX;
		expr.opcode = first.id;
		
		Symbol* sub = expr.operand = parseExpression (&symbol);
		
		// apply the unary operation before
		// the subexpression, unless also unary
		if (sub->data.expr > expr)
		{
			std::swap (expr.operand, sub->data.expr.operand);
		}
		
		return &(symbol);
	}
	
	if (first.isLabel())
		expr.leftSide = parseLabel (first, &symbol);
	else
	if (first.isLiteral())
		expr.leftSide = parseLiteral (first, &symbol);
	
	if (expr.leftSide == (0))
	{
		// no operand and no prefix operator
		// empty expression
		scanner.unget (first);
		return &(symbol);
	}

	Word next = scanner.get();
	expr.opcode = next.id;
	
	if (isPostfixOperator (next))
	{
		expr.position = Operation::POSTFIX;
		
		// handle any trailing postfix operators
		// (something like num++--, contrived as it is)
		next = scanner.get();
		Symbol* outer = &symbol;
		while (isPostfixOperator (next))
		{
			Symbol& sub = symbol.generate ();
			sub.type = Symbol::OPERATION;
			
			sub.data.expr.position = Operation::POSTFIX;
			sub.data.expr.opcode = next.id;
			sub.data.expr.operand = outer;
			
			outer = &sub;
			next = scanner.get();
		}
		
		return &(symbol);
	}
	if (isInfixOperator (next))
	{
		expr.position = Operation::INFIX;
		
		Symbol* sub = parseExpression (&symbol);
		expr.rightSide = sub;
		
		// if the suboperation has lower priority,
		// swap the associativity
		if (sub->data.expr > expr)
		{
			std::swap (expr.rightSide, sub->data.expr.operand);
		}
		return &(symbol);
	}
	
	scanner.unget (next);
	return &(symbol);
}

Symbol* parseSymbol (Symbol* scope)
{
	Scanner scanner = {};
	
	Word first = scanner.top();
	
	if (first.id == ID(EOF)) return (Symbol*)(0);
	
	if (isOperator (first) or
	    first.isLiteral() )
	{
		return parseExpression (scope);
	}
	
	if (ID::GROUP_L == first.id)
	{
		//Word next = scanner.get();
		
		
	}
	
	Symbol* empty = &scope->generate();
	empty->type = (enum Symbol::type) 0;
	return empty;
}

Symbol CST::parseSource ()
{
	Symbol root = {};
	
	Symbol*	next;
	do next = parseSymbol (&root); 
	until ((Symbol*)0 == next);
	
	return root;
}
