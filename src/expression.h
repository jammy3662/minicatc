#ifndef EXPRESSION_DOT_H
#define EXPRESSION_DOT_H

#include "token.h"

namespace CatLang {

struct OperatorType
{
	bool prefix: 1;
	bool infix: 1;
	bool postfix: 1;
};

// an operator may have multiple positions
// example, ++ is both prefix & postfix
OperatorType TypeOfOp (Token);

bool IsOperator (Token);

}

#endif
