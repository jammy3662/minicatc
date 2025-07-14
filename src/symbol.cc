#include "symbol.h"
#include "token.h"

#include <stdlib.h>

#include <stdexcept>
using namespace std; // go to hell c++

namespace CatLang {

typedef TokenID ID;

Symbol* lookup (char* name, Symbol* start)
{
	Scope* scope = (Scope*) start;
	
	short NumMatches = scope->Namespace.count (name);
	
	until (NumMatches > 0)
	{
		scope = (Scope*) scope->parent;
		if (scope == 0) return (Symbol*) 0;
		
		NumMatches = scope->Namespace.count (name);
	}
	
	auto index = scope->Namespace.upper_bound (name) -> second;
	
	return & ((Scope*)scope)->Fields [index];
}

Error Log (Error err, Symbol* scope)
{
	scope->errors.push_back (err);
	return err;
}

Error Log (char* msg, Token t, Error::Level level, Symbol* scope)
{
	Error err;
	err.message = msg;
	err.severity = level;
	err.token = t;
	
	return Log (err, scope);
}

Error Log (char* msg, Token t, Error::Level level, Error::Code code, Symbol* scope)
{
	Error err;
	err.message = msg;
	err.severity = level;
	err.code = code;
	err.token = t;
	
	return Log (err, scope);
}

}
