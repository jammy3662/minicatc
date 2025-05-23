#include "symbol.h"
#include "token.h"

#include <stdlib.h>

#include <stdexcept>
using namespace std; // go to hell c++

namespace CatLang {

typedef TokenID ID;

Reference RefFrom (Symbol* symbol)
{
	return {symbol, symbol->SymbolType, 0};
}

Symbol* lookup (char* name, Reference start)
{
	Scope* scope = (Scope*) start.Symbol;
	
	short NumMatches = scope->Namespace.count (name);
	
	while (NumMatches <= 0)
	{
		scope = (Scope*) scope->parent.Symbol;
		if (scope == 0) return scope;
		
		NumMatches = scope->Namespace.count (name);
	}
	
	auto index = scope->Namespace.upper_bound (name) -> second;
	
	return & scope->Locals [index];
}

Error Log (Error err, Symbol* scope)
{
	scope->errors.push_back (err);
}

Error Log (char* message, byte level, Symbol* scope)
{
	Error err;
	err.message = message;
	err.severity = level;
	
	return Log (err, scope);
}

Error Log (char* message, byte level, int2 code, Symbol* scope)
{
	Error err;
	err.message = message;
	err.severity = level;
	err.code = code;
	
	return Log (err, scope);
}

}
