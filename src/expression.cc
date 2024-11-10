#include "expression.h"

typedef Word::ID ID;

namespace CatLang
{
	fast Expression::priority()
	{
		// NOTE: this implementation just copies
		// the c operator precedence rules
		// ( https://en.cppreference.com/w/c/language/operator_precedence )
		
		switch (opcode)
		{
			case (ID::GROUP):
				// prefix: enclosed expression: (1+1)
				// postfix: function call: fn (args, etc)
				// infix: type cast: (float) (1/4)
				switch (position) {
					case (PREFIX): return 0;
					case (POSTFIX): return 1;
					case (INFIX): return 2; }
			
			case (ID::LIST):
				// postfix: array subscript: array[index]
				// prefix: literal array: [item1, item2, etc]
				return (position == POSTFIX) ? 1 : 0;
			
			case (ID::PLUSx2):
			case (ID::MINUSx2):
				return (position == PREFIX) ? 1 : 2;
			
			case (ID::PLUS):
			case (ID::MINUS):
				return (position == PREFIX) ? 2 : 4;
			
			case (ID::BANG):
			case (ID::TILDE):
				return 2;
			
			case (ID::STAR):
				return (position == PREFIX) ? 2 : 3;
			case (ID::AND):
				return (position == PREFIX) ? 2 : 8;
			
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
	
	bool operator > (Expression left, Expression right)
	{
		return (left.priority() < right.priority());
	}
	
	bool isPrefixOperator (Word w)
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

	bool isPostfixOperator (Word w)
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

	bool isOperator (Word w)
	{
		return (isInfixOperator(w) or
						isPrefixOperator (w));
						// postfix operators are repeats
	}
}
