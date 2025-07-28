#include <sstream>

#include "symbol.h"
#include "token.h"

#include <stdlib.h>

using namespace std; // go to hell c++

namespace CatLang {

typedef TokenID ID;

bool Type::indirection_const (fast idx)
{
	return indirection & (1 << (2*idx + 1));
}

bool Type::indirection_ref (fast idx)
{
	return indirection & (1 << (2*idx + 0));
}

void Type::indirection_set_const (bool constness)
{
	fast idx = indirection_ct - 1;
	byte const_bit = (1 && constness) << (2*idx + 1);
	
	// unset bits
	indirection &= ~const_bit;
	// set bits according to input values
	indirection |= const_bit;
}

void Type::indirection_set_ref (bool refness)
{
	fast idx = indirection_ct - 1;
	byte ref_bit = (1 && refness) << (2*idx +	0);
	
	// unset bits
	indirection &= ~ref_bit;
	// set bits according to input values
	indirection |= ref_bit;
}

string Type::print_data ()
{
	std::stringstream stream;
	
	string signed_str = SIGNED ? "signed " : "unsigned ";
	string ric_str = (RIC is 0) ? "" :	(RIC & 2) ? "complex " : "imaginary ";
	
	switch (data)
	{
		case CHAR:
		stream <<	signed_str << "char "; break;
		case BYTE:
		stream << signed_str <<	"byte "; break;
		case SHORT:
		stream << signed_str <<	"short int"; break;
		case INT:
		stream << signed_str <<	"int "; break;
		case LONG:
		stream << signed_str << ((LONG)?"long ":"")<<"long int "; break;
		case FLOAT:
		stream <<	ric_str << ((LONG)?"long ":"")<<((DOUBLE)?"double ":"float "); break;
		
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

Type Type::invalid ()
{
	Type type;
	type.data = Type::NONE;
}

inline
bool Type::is_valid ()
{
	return data isnt NONE;
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
