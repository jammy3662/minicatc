#include "parse.h"
#include "words.h"
#include "expression.h"

typedef Word::ID ID;

namespace CatLang
{
	Symbol* parseLabel (Word word, Symbol* scope)
	{
		Symbol* label = new Symbol;
		scope->insert (label);
		
		return label;
	}

	Symbol* parseLiteral (Word literal, Symbol* scope)
	{
		Literal* res = new Literal;
		scope->insert (res);
		
		res->type = Symbol::VALUE;
		
		switch (literal.id)
		{
			default: break;
			case (ID::INT_LIT):
				res->Int = atoll (literal.str);
				break;
			case (ID::FLOAT_LIT):
				res->Float = atof (literal.str);
				break;
			case (ID::STR_LIT):
			case (ID::CHAR_LIT):
				res->Str = literal.str;
				break;
		}
		
		return res;
	}

	Expression* parseExpression (Symbol* scope)
	{
		Scanner scanner = {};
		scanner.putback = true;
		Word first = scanner.get();
		
		Expression* expr = new Expression;
		scope->insert (expr);
		
		expr->type = Symbol::OPERATION;
		expr->opcode = 0;
		
		if (isPrefixOperator (first))
		{
			expr->position = Expression::PREFIX;
			expr->opcode = first.id;
			
			Expression* sub = parseExpression (expr);
			expr->operand = sub;
			
			// apply the unary operation before
			// the subexpression, unless also unary
			if (-expr->priority() > -sub->priority())
			{
				std::swap (expr->rightSide, sub->leftSide);
			}
			
			return expr;
		}
		
		if (first.isLabel())
			expr->leftSide = parseLabel (first, expr);
		else
		if (first.isLiteral())
			expr->leftSide = parseLiteral (first, expr);
		
		if (expr->leftSide == (0))
		{
			// no operand and no prefix operator
			// empty expression
			scanner.unget (first);
			return expr;
		}

		Word next = scanner.get();
		expr->opcode = next.id;
		
		if (isPostfixOperator (next))
		{
			expr->position = Expression::POSTFIX;
			
			// handle any trailing postfix operators
			// (something like num++--, contrived as it is)
			next = scanner.get();
			Symbol* outer = expr;
			while (isPostfixOperator (next))
			{
				Expression* sub = new Expression;
				outer->insert (sub);
				
				sub->type = Symbol::OPERATION;
				sub->position = Expression::POSTFIX;
				sub->opcode = next.id;
				sub->operand = outer;
				
				outer = sub;
				next = scanner.get();
			}
			
			return expr;
		}
		if (isInfixOperator (next))
		{
			expr->position = Expression::INFIX;
			
			Expression* sub = parseExpression (expr);
			expr->rightSide = sub;
			
			// if the suboperation has lower priority,
			// swap the associativity
			if (-expr->priority() > -sub->priority())
			{
				std::swap (expr->rightSide, sub->leftSide);
			}
			return expr;
		}
		
		scanner.unget (next);
		return expr;
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
		
		Symbol* empty = new Symbol;
		scope->insert (empty);
		empty->type = (enum Symbol::type) 0;
		return empty;
	}

	Symbol parseSource ()
	{
		Symbol root = {};
		
		Symbol*	next;
		do next = parseSymbol (&root); 
		until ((Symbol*)0 == next);
		
		return root;
	}
}
