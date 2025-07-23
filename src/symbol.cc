#include <sstream>

#include "symbol.h"
#include "token.h"

#include <stdlib.h>

using namespace std; // go to hell c++

namespace CatLang {

typedef TokenID ID;

string Type::print_data ()
{
	std::stringstream stream;
	
	string signed_str = SIGNED ? "signed " : "unsigned ";
	string ric_str = (RIC is 0) ? "" :	(RIC & 2) ? "complex " : "imaginary ";
	
	switch (data)
	{
		case BYTE:
		stream << signed_str <<	"byte "; break;
		case SHORT:
		stream << signed_str <<	"short int"; break;
		case INT:
		stream << signed_str <<	"int "; break;
		case LONG:
		stream << signed_str <<	"long int "; break;
		case LONG_L:
		stream << signed_str <<	"long long int "; break;
		
		case FLOAT:
		stream <<	ric_str << "float "; break;
		case DOUBLE:
		stream << ric_str << "double "; break;
		case DOUBLE_L:
		stream << ric_str << "long double"; break;
		
		case VOID:
		stream << "void "; break;
		
		case STRUCTURED:
		stream <<	definition->name <<	' ';
	}
	
	auto str = stream.str();
	return str;
}

string Type::print ()
{
	std::stringstream stream;
	
	if (LOCAL) stream << "local ";
	if (INLINE) stream << "inline ";
	if (STATIC) stream << "static ";
	if (CONST) stream <<	"const ";
	
	stream <<	print_data ();
	
	for (int i = 0; i <	indirection_ct; ++i)
	{
		bool const_indirection = indirection & (1 << (2*i+0));
		bool ref_indirection = indirection & (1 << (2*i+1));
		
		(ref_indirection) ?	stream << "~ " : stream << "* ";
		if (const_indirection) stream << "const ";
	}
	
	auto str = stream.str();
	return str;
}

Symbol* lookup (char* name, Symbol* start)
{
	Scope* scope = (Scope*) start;
	
	short NumMatches = scope->Tags.count (name);
	
	until (NumMatches > 0)
	{
		scope = (Scope*) scope->parent;
		if (scope == 0) return (Symbol*) 0;
		
		NumMatches = scope->Tags.count (name);
	}
	
	return scope->Fields
		[scope->Tags.upper_bound (name) -> second];
}

int Tuple::Insert (Symbol* symbol, char* name)
{
	int idx = Fields.size();
	Fields.push_back (symbol);
	
	if (name isnt 0)
		symbol->name = name,
		Tags.insert (std::make_pair (name, idx));
	
	return idx;
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
