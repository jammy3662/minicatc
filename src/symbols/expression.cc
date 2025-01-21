#include "expression.h"

typedef Token::ID ID;

namespace CatLang
{
	bool isPrefixOperator (Token t)
	{
		switch (t.id)
		{
			case (Token::PLUS):
			case (Token::MINUS):
			case (Token::PLUSx2): // increment
			case (Token::MINUSx2): // decrement
			case (Token::TILDE): // complement
			case (Token::BANG): // not
			case (Token::AND): // address of
			case (Token::STAR): // value at
			case (Token::PAREN): // type cast
				return true;
			
			default: return false;
		}
	}

	bool isPostfixOperator (Token t)
	{
		switch (t.id)
		{
			case (Token::PLUSx2):
			case (Token::MINUSx2):
			case (Token::PAREN): // function call
			case (Token::BRACKET): // subscript
			case (Token::ARROW): // legacy member access
				return true;
			
			default: return false;
		}
	}

	bool isInfixOperator (Token t)
	{
		switch (t.id)
		{
			case (Token::PLUS):
			case (Token::MINUS):
			case (Token::STAR): // multiply
			case (Token::SLASH): // divide
			case (Token::MOD): // modulo
			case (Token::AND): // and
			case (Token::OR):	// or
			case (Token::POWER):	// xor
			case (Token::LEFTx2): // left shift
			case (Token::RIGHTx2): // right shift
			case (Token::LEFTx3): // left rotate
			case (Token::RIGHTx3): // right rotate
			case (Token::ANDx2): // logical and
			case (Token::ORx2): // logical or
			case (Token::BANG): // not
			case (Token::LEFT): // less than
			case (Token::RIGHT): // more than
			case (Token::EQUAL):
				return true;
			
			default: return false;
		}
	}

	// a terminal operator is not included in any expression
	bool isTerminalOperator (Token t)
	{
		switch (t.id)
		{
			case (EOF): // end of file
			case (Token::PAREN_R): // END group
			case (Token::BRACKET_R): // END list
			case (Token::BRACE_R): // END frame (scoped expression)
			case (Token::SEMI): // END (general)
				return true;
			
			default: return false;
		}
	}

	bool isOperator (Token t)
	{
		return
		(
			isInfixOperator(t) or
			isPrefixOperator (t) or
			isPostfixOperator (t) or
			isTerminalOperator (t)
		);
	}
	
	Symbol* parseLiteralToken (Token literal, Symbol* scope)
	{
		Symbol* res = (Symbol*) SymbolHeap.create <Literal>();
		
		Literal& l = * (Literal*) res;
		
		l.type = Symbol::LITERAL;
		
		switch (literal.id)
		{
			case (ID::INT_LIT):
				l.Int = atoll (literal.str);
				l.integralType = Literal::INT;
			break;
			
			case (ID::FLOAT_LIT):
				l.Float = atof (literal.str);
				l.integralType = Literal::FLOAT;
			break;
			
			case (ID::STR_LIT):
				l.Str = literal.str;
				l.integralType = Literal::STR;
			break;
			
			case (ID::CHAR_LIT):
				l.Str = literal.str;
				l.integralType = Literal::CH_STR;
			break;
			
			default: break;
		}
		
		return res;
	}

	Expression* parseExpression (Symbol* scope)
	{
		Scanner scanner = {};
		Token first = scanner.get();
		
		Expression* expr = new Expression;
		Expression* subexpr = new Expression;
		
		// prefix operator
		if (isOperator (first))
		{
			if (!isPrefixOperator ())
			{
				expr->error.code = 1;
				expr->error.message = "Infix operator needs a left operand."; 
			}
			
			return parsePrefixExpression (scope);
			
			expr->position = Expression::PREFIX;
			expr->opcode = first.id;
			
			subexpr = parseExpression (expr);
			
			expr->leftSide = subexpr;
			
			if (subexpr->priority() < expr->priority())
			{
				expr->leftSide = subexpr->leftSide;
				subexpr->leftSide = expr;
				
				if (subexpr->incomplete)
				{
					
				}
				
				// unary expression becomes the subexpression,
				// binary expression is root / main
				std::swap (expr, subexpr);
			}
			
			return expr;
		}
		
		expr->leftSide = parseOperand (expr);
		
		if (not expr->leftSide->incomplete)
		{
			// empty expression
			expr->incomplete = true;
			scanner.undoAfter = true;
			return expr;
		}

		Token next = scanner.get();
		
		if (isOperator (next))
			expr->opcode = next.id;
		else
			expr->opcode = Token::STAR;
		
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
				
				sub->type = Symbol::EXPRESSION;
				sub->position = Expression::POSTFIX;
				sub->opcode = next.id;
				sub->leftSide = outer;
				
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
			if (expr->priority() > sub->priority())
			{
				std::swap (expr->rightSide, sub->leftSide);
			}
			return expr;
		}
		
		scanner.unget (next);
		return expr;
	}

}
