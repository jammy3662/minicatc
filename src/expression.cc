#include "expression.h"

typedef TokenID ID;

namespace CatLang
{
	OperatorType TypeOfOp (Token t)
	{
		OperatorType ot = {0};
		
		// prefix
		switch (t.id)
		{
			case ID::PLUS:
			case ID::MINUS:
			case ID::PLUSx2: // increment
			case ID::MINUSx2: // decrement
			case ID::TILDE: // complement
			case ID::BANG: // not
			case ID::AMP: // address of
			case ID::STAR: // value at
			case ID::PAREN: // type cast
				ot.prefix = true;
			
			default: break;
		}
		// postfix
		switch (t.id)
		{
			case ID::PLUSx2:
			case ID::MINUSx2:
			case ID::PAREN: // function call
			case ID::BRACKET: // subscript
			case ID::ARROW: // legacy member access
				ot.postfix = true;
			
			default: break;
		}
		// infix
		switch (t.id)
		{
			case ID::PLUS:
			case ID::MINUS:
			case ID::STAR: // multiply
			case ID::SLASH: // divide
			case ID::MOD: // modulo
			case ID::AMP: // and
			case ID::PIPE:	// or
			case ID::POWER:	// xor
			case ID::LEFTx2: // left shift
			case ID::RIGHTx2: // right shift
			case ID::LEFTx3: // left rotate
			case ID::RIGHTx3: // right rotate
			case ID::AMPx2: // logical and
			case ID::PIPEx2: // logical or
			case ID::BANG: // not
			case ID::LEFT: // less than
			case ID::RIGHT: // more than
			case ID::EQUAL:
			case ID::TAIL: // range
				ot.infix = true;
			
			default: break;
		}
		
		return ot;
	}
	
	bool IsPrefixOperator (Token t)
	{
		switch (t.id)
		{
			case ID::PLUS:
			case ID::MINUS:
			case ID::PLUSx2: // increment
			case ID::MINUSx2: // decrement
			case ID::TILDE: // complement
			case ID::BANG: // not
			case ID::AMP: // address of
			case ID::STAR: // value at
			case ID::PAREN: // type cast
			return true;
			
			default: return false;
		}
	}

	bool IsPostfixOperator (Token t)
	{
		switch (t.id)
		{
			case ID::PLUSx2:
			case ID::MINUSx2:
			case ID::PAREN: // function call
			case ID::BRACKET: // subscript
			case ID::ARROW: // legacy member access
			return true;
			
			default: return false;
		}
	}

	bool IsInfixOperator (Token t)
	{
		switch (t.id)
		{
			case ID::PLUS:
			case ID::MINUS:
			case ID::STAR: // multiply
			case ID::SLASH: // divide
			case ID::MOD: // modulo
			case ID::AMP: // and
			case ID::PIPE:	// or
			case ID::POWER:	// xor
			case ID::LEFTx2: // left shift
			case ID::RIGHTx2: // right shift
			case ID::LEFTx3: // left rotate
			case ID::RIGHTx3: // right rotate
			case ID::AMPx2: // logical and
			case ID::PIPEx2: // logical or
			case ID::BANG: // not
			case ID::LEFT: // less than
			case ID::RIGHT: // more than
			case ID::EQUAL:
			return true;
			
			default: return false;
		}
	}

	// a terminal operator is not included in any expression
	bool IsTerminalOperator (Token t)
	{
		switch (t.id)
		{
			case ID::END_FILE: // end of file
			case ID::PAREN_R: // END group
			case ID::BRACKET_R: // END list
			case ID::BRACE_R: // END frame (scoped expression)
			case ID::SEMI: // END (general)
			return true;
			
			default: return false;
		}
	}

	bool IsOperator (Token t)
	{
		auto type = TypeOfOp (t);
		return (type.infix or type.prefix or type.postfix);
	}
	
	
	/*
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
*/
/*
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

*/
}
