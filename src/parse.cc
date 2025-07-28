#include <stack>
#include <sstream>

#include "parse.h"
#include "symbol.h"
#include "tag.h"

typedef TokenID ID;

namespace CatLang
{

Symbol* ParseSource (char* path)
{
	FILE* fp = fopen (path, "r");
	
	if (not fp)
	{
		fprintf (stderr, "Failed to open file at '%s'\n", path);
		return (Symbol*) 0;
	}
	
	return ParseSource (fp);
}

Symbol* ParseSource();
Symbol* ParseSource (FILE* sourceFile)
{
	if (not sourceFile) fprintf (stderr, "Failed to read a null file pointer\n");
	
	SetSource (sourceFile);
	
	return ParseSource ();
}

Symbol* ParseBlockStatement (Scope*, bool braced = false, bool embedded = false);

Symbol* ParseSource ()
{
	Scope* compile_unit = new Scope;
	
	Scope::parse (compile_unit, true, false);
	
	return (Symbol*) compile_unit;
}

// -------------------------------- //

Symbol* ParseKeywordStatement (Tag keyword, Scope*);
Symbol* ParseParenthesizedStatement (Scope*);

// parses one statement into the symbol table
Symbol* Symbol::parse (Scope* scope)
{
	// if the current scope has been closed, move to the outer scope
	// (outermost scope can only be closed with end of file)
	if (scope->closed and scope->parent != 0)
	scope = scope->parent;

	Tag tag, next;

	next_tag:
	tag = ParseTag (scope);

	switch (tag.category)
	{
		case tag.END_INPUT:
		scope->closed = true;
		return last_symbol; // stop parsing, end of file
		
		case Tag::MARKER:
		scope->Log (Error::NOTE, tag.token, "Try goto ",tag.token.str," to jump to marker ",tag.token.str," (the name of the marker does nothing by itself)");
		return (Symbol*) 0;
		
		case Tag::NEW_NAME:
		return Marker::parse (scope, &tag);
		
		case Tag::PLACEHOLDER:
		case Tag::VARIABLE:
		case Tag::EXPRESSION:
		case Tag::FUNCTION:
		case Tag::OPERATOR:
		case Tag::STAR:
		return Expression::parse (scope, false, &tag);
		
		case Tag::TYPE:
		return Object::parse (scope, &tag);
		
		case Tag::ACCESS:
		scope->Log (Error::NOTE, tag.token, "Use . after a name to access a field (ignoring)");
		goto next_tag;
		
		case Tag::SEPARATOR:
		switch (tag.separator)
		{
			case Tag::SEMI:
			// empty statement
			return (Symbol*) 0;
			
			case Tag::BRACE_L:
			return Scope::parse (scope, false, true);
			
			case Tag::BRACKET_L:
			return Expression::parse (scope, false, &tag);
			
			case Tag::PAREN_L:
			return ParseParenthesizedStatement (scope);
			
			case Tag::BRACE_R:
			
			if (not scope->braced)
				scope->Log (Error::WARNING, tag.token, "} doesn't match a {, use ... instead");
			scope->closed = true;
			return (Symbol*) 0;
			
			case Tag::ELLIPSES:
			
			if (scope->braced)
				scope->Log (Error::WARNING, tag.token, "... shouldn't be used to close {, use } instead");
			scope->closed = true;
			return (Symbol*) 0;
			
			case Tag::BRACKET_R:
			
			scope->Log (Error::WARNING, tag.token,
		    	 "] has no matching [");
			goto next_tag;
			
			case Tag::PAREN_R:
			
			scope->Log (Error::WARNING, tag.token,
			     ") has no matching (");
			goto next_tag;
			
		}
		
		case Tag::KEYWORD:
		return ParseKeywordStatement (tag, scope);
	}

	// *snippet for later, probably in ParseExpressionStatement* //
	if (0)
	scope->Log (Error::WARNING, tag.token,
	     ".. denotes a range - place between two values:	1..9");
}

// -------------------------------- //

Symbol* ParseParenthesizedStatement (Scope* scope)
{
	// TODO: could be a TUPLE or EXPRESSION
	// aka: VARIABLE definition, FUNCTION definition, or EXPRESSION
	Tag tag;
	
	get_tag:
	tag = ParseTag (scope);
	
	Tuple* tuple;
	
	switch (tag.category)
	{
	case Tag::VARIABLE:
	case Tag::EXPRESSION:
	case Tag::FUNCTION:
	
		return Expression::parse (scope, true, &tag);
		
	case Tag::TYPE:
	
		return Tuple::parse (scope, &tag);
		
	case Tag::UNRESOLVED:
		// TODO
	
	case Tag::KEYWORD:
	
		if (tag.keytag.category isnt Tag::Keytag::QUALIFIER)
			return Tuple::parse (scope, &tag);
	
	default:
		scope->Log (Error::WARNING, tag.token,
			tag.token.str," is a(n) ",tag.kind(),", but should be a type or object");
			
		tag = ParseTag (scope);
		if (tag.token.kind isnt COMMA)
		ReplaceTag (tag);
		
		goto get_tag;
	}
}

Marker* Marker::parse (Scope* scope, Tag* name)
{
	Marker* marker;
	Tag tag;
	
	if (name is 0)
		tag = ParseTag (scope);
	else
		tag = *name;
	
	if (not tag.nested)
	{
		Tag next = ParseTag (scope);
		
		if (next.token.kind is COLON)
		{
			marker = new Marker;
			marker->target_index =			
				scope->Insert (marker);
		}
		
		// can't define a marker in a different scope
		else ReplaceTag (next);
	}
	
	return marker;
}

Object* Object::parse (Scope* scope, Tag* first_tag)
{
	Type type = Type::parse (scope, first_tag);
	
	if (not type.is_valid())
		return (Object*) 0;
	
	Tag identifier, next;
	
	get_tag:
	identifier = ParseTag (scope);
	
	Symbol* parent = scope;
	
	switch (identifier.category)
	{
		case Tag::PLACEHOLDER:
		case Tag::NEW_NAME:
		
		if (identifier.nested)
			parent = identifier.scope;
		
		next = ParseTag (scope);
		
		switch (next.token.kind)
		{
			case TokenID::PAREN_L:
			Function::parse (scope);
			return (Object*) 0;
			
			case TokenID::EQUAL:
			return Variable::parse (scope, type, &identifier, true);
			
			default:
			return Variable::parse (scope, type, &identifier, false);
		}
		
		case Tag::VARIABLE:
		case Tag::EXPRESSION:
		// type cast: type followed by object
		return Expression::parse (scope, false, &type, &identifier);
		
		default:
		ReplaceTag (identifier);
		return (Object*) 0;
	}
}

Scope* Scope::parse (Scope* parent, bool embedded, bool braced)
{
	Scope* scope;
	
	if (not embedded)
	{
		// create a new scope symbol and add it to the parent
		scope = new Scope;
		scope->kind = Scope::STACKFRAME;
		parent->Insert (scope);
	}
	else
		// if embedded, edit the existing scope symbol directly
		scope = parent;
	
	scope->braced = braced;
	
	while (not scope->closed)
		Symbol::parse (scope);
	
	return scope;
}

Expression* Expression::parse (Scope* scope, bool parenthesized, Tag* first_tag)
{
	// TODO: implementation
}

// keyword statements //
Symbol* ParseIncludeStatement (Scope*);
Symbol* ParseReturnStatement (Scope*);

Symbol* ParseStructStatement (Tag, Scope*);
Symbol* ParseSelectionStatement (Tag, Scope*);
Symbol* ParseCaseStatement (Tag, Scope*);
Symbol* ParseJumpStatement (Tag, Scope*);

// statements beginning with a keyword (not representing a type)
Symbol* ParseKeywordStatement (Tag tag, Scope* scope)
{
	// could be a STRUCT, UNION, ENUM, MODULE, declaration specifier,
	// or control structure (if / switch / etc)
	switch (tag.keytag.keyword)
	{
		typedef Keyword ID;
		
		case Keyword::INCLUDE:
			
		return ParseIncludeStatement (scope);
		
		case Keyword::STRUCT:
		case Keyword::UNION:
		case Keyword::ENUM:
		case Keyword::MODULE:
		
		return Struct::parse (scope, &tag);
		
		case Keyword::LOCAL:
		case Keyword::STATIC:
		case Keyword::CONST:
		case Keyword::INLINE:
		case Keyword::SIGNED:
		case Keyword::UNSIGNED:
		case Keyword::COMPLEX:
		case Keyword::IMAGINARY:
		
		return Object::parse (scope, &tag);
		
		case Keyword::IF:
		case Keyword::ELSE:
		case Keyword::DO:
		case Keyword::WHILE:
		case Keyword::FOR:
		case Keyword::SWITCH:
		
		return ParseSelectionStatement (tag, scope);
		
		case Keyword::RETURN:
		
		return ParseReturnStatement (scope);
		
		case Keyword::SIZEOF:
		case Keyword::TYPEOF:
		case Keyword::COUNTOF:
		case Keyword::NAMEOF:
		
		return Expression::parse (scope, false, &tag);
		
		case Keyword::GOTO:
		case Keyword::BREAK:
		case Keyword::CONTINUE:
				
		return ParseJumpStatement (tag, scope);
		
		case Keyword::DEFAULT:
		case Keyword::CASE:
		
		return ParseCaseStatement (tag, scope);
	}
}

// -------------------------------- //

Tuple* Tuple::parse (Scope* scope, Tag* first_tag)
{
	Tuple* tuple;
	
	Tag tag;
	
	if (first_tag is 0)
		tag = ParseTag (scope);
	else
		tag = *first_tag;
	
	
}

Type Type::parse (Scope* scope, Tag* first_tag)
{	
	Type type;
	type.data = NONE;
	
	Tag tag;
	
	if (first_tag is 0)
		tag = ParseTag (scope);
	else
		tag = *first_tag;
	
	enum { NUL, TYPE, KEYWORD, POINTER, OTHER }
	last_parsed = NUL;
	
	for (; last_parsed isnt OTHER; tag = ParseTag (scope))
	{
		switch (tag.category)
		{
		case Tag::UNRESOLVED:
		// TODO
			
		case Tag::KEYWORD:
		
			last_parsed = KEYWORD;
			
			if (tag.keytag.category isnt Tag::Keytag::QUALIFIER)
			{	
				if (type.data isnt NONE) return type;
				
				last_parsed = OTHER;
				scope->Log (Error::WARNING, tag.token,
				tag.token.str," is a ",tag.kind(),", but should be a type or qualifier");
				break;
			}
			
			switch (tag.keytag.keyword)
			{
				case Keyword::STATIC:
				type.STATIC = true;
				break;
				
				case Keyword::INLINE:
				type.INLINE = true;
				break;
				
				case Keyword::LOCAL:
				type.LOCAL = true;
				break;
				
				case Keyword::CONST:
				
				if (last_parsed is POINTER)
					type.indirection_set_const (true);
				else
					type.CONST = true;
				break;
				
				case Keyword::SIGNED:
				
				if (type.data isnt NONE and
				    not between (type.data, BIT, LONG))
				
					scope->Log (Error::WARNING, tag.token, type.print_data()," cannot be signed - only int types can be signed or unsigned");
				
				else
					type.data = INT,
					type.SIGNED = true;
				break;
				
				case Keyword::UNSIGNED:
				
				if (type.data isnt NONE and
				    not between (type.data, BIT, LONG))
				
					scope->Log (Error::WARNING, tag.token, type.print_data()," cannot be unsigned - only int types can be signed or unsigned");
				
				else
					type.data = INT,
					type.SIGNED = false;
				break;
				
				case Keyword::COMPLEX:
				
				if (type.data isnt NONE and type.data isnt FLOAT)
				
					scope->Log (Error::WARNING, tag.token, type.print_data()," cannot be complex - only float types can be complex");
				
				else
					type.data = FLOAT,
					type.RIC = RIC::COMPLEX;
				
				case Keyword::IMAGINARY:
				
				if (type.data isnt NONE and type.data isnt FLOAT)
				
					scope->Log (Error::WARNING, tag.token, type.print_data()," cannot be imaginary - only float types can be imaginary");
				
				else
					type.data = FLOAT,
					type.RIC = RIC::IMAGINARY;
				
				default:
				// unreachable (should be)
				// TODO: qualifier enum to remove this ambiguity
				fprintf (stderr, "%s keyword incorrectly passed as qualifier\n", tag.token.str.ptr);
			}
		break;
			
		case Tag::TYPE:
			
			last_parsed = TYPE;
			
			if (type.data isnt NONE)
				scope->Log (Error::WARNING, tag.token,
				tag.token.str," overrides previously specified type of ", type.print_data());
				
			last_parsed = TYPE;
			type.data = tag.type.data;
			
		break;
			
		case Tag::STAR:
		case Tag::TILDE:
			
			if (type.data is NONE)
			{
				scope->Log (Error::WARNING, tag.token,
				"Can't specify pointer before underlying type");
				break;
			}
			last_parsed = POINTER;
			
			type.indirection_ct++;
			
			if (tag.category is Tag::TILDE)
				type.indirection_set_ref (true);
			
		break;
		
		case Tag::SEPARATOR:
		
		if (tag.separator is tag.PAREN_L)
		{
			// function signature as type
			type.parameters = Tuple::parse (scope);
			return type;
		}
			
		default:
			last_parsed = OTHER;
			
			if (type.data isnt NONE) return type;
			
			scope->Log (Error::WARNING, tag.token,
						tag.token.str," is a ",tag.kind(),", but should be a type or qualifier");
		break;
		}
	}
	
	scope->Log (Error::ERROR, tag.token, "Type of declaration not specified");
	return type;
}

Variable* Variable::parse (Scope* scope, Type type, Tag* identifier, bool initialize)
{
	Variable* variable = new Variable;
	
	variable->datatype = type;
	
	Scope* owner = scope;
	
	if (identifier->nested)
		owner = (Scope*) identifier->scope;
	
	if (initialize)
	{
		Tag next = ParseTag (scope);
		variable->Initializer = Expression::parse (scope, false, &next);
	}
	
	owner->Insert (variable, identifier->token.str);
	return variable;
}

Variable* Variable::parse (Scope* scope, Tag* first_tag)
{
	
}

Symbol* ParseFunction (Tag name, Type return_type, Scope* scope)
{
	Scope* function = new Scope;
	scope->kind = Symbol::FUNCTION;
	
	Scope* owner = scope;
	
	if (name.nested)
		owner = (Scope*) name.scope;
	
	function->Parameters = Tuple::parse (scope);
	bool braced = false;
	bool EMBEDDED	= true;
	
	Scanner scanner;
	if (scanner.get().kind is '{')
		braced = true;
	else
		scanner.unget();
	
	ParseBlockStatement (function, braced, EMBEDDED);
}

// TODO: ParseCaseStatement implementation
Symbol* ParseCaseStatement (Tag tag, Scope* scope)
{
	if (scope->kind isnt Symbol::SWITCH) {
		
		scope->Log (Error::WARNING, tag.token,
		     "Ignoring ",tag.token.str," used outside of switch");
	}
	
	Scanner scanner;
	Token next;
	
	// TODO: look for a case expression followed by ;
	switch (tag.keytag.keyword)
	{
		case Keyword::DEFAULT:
		
		if (scanner.get().kind isnt TokenID::SEMI)
			scanner.unget();
		
		case Keyword::CASE:
		
		
		default:
		fprintf (stderr, "Non-case keyword passed to ParseCaseStatement\n");
		
		// TODO: what struct represents a switch
		// with any number of cases
	}
}

// TODO: ParseJumpStatement implementation
Symbol* ParseJumpStatement (Tag tag, Scope* scope)
{
	Tag next;
		
	switch (tag.keytag.keyword)
	{
		case Keyword::BREAK:
		
		case Keyword::CONTINUE:
		
		case Keyword::GOTO:
		// TODO: look for a name tag for a goto marker
		next = ParseTag (scope);
		if (next.category is Tag::KEYWORD and
		    not next.nested);
		
		default: break;
	}
	
	{
		Symbol::Kind upper_bound = Symbol::FOR;
		// continue can only be in a loop
		// break can be in a loop OR switch
		if (tag.keytag.keyword is Keyword::BREAK)
			upper_bound = (Symbol::Kind) (upper_bound	+ 1);
		
		if (not between (scope->kind, Symbol::WHILE, upper_bound))
		{
			std::stringstream msg;
			msg << tag.str <<" used outside of loop";
			auto stream = msg.str();
			auto str = (char*) stream.c_str();
			Log (str, tag.token, Error::Level::WARNING, scope);
		}
	}
}

// TODO: ParseIncludeStatement implementation
Symbol* ParseIncludeStatement (Scope* scope)
{
	Scanner scanner = {};
	
	Token token = scanner.get();
	
	if (not TokenIs (TokenID::NAME, token)) {
		// TODO: add details to this err //
		std::stringstream msg;
		msg << token.str <<" cannot be included in "<< scope->name;
		auto stream = msg.str();
		auto str = (char*) stream.c_str();
		Log (str, token, Error::Level::WARNING, scope);
	}
	return (Symbol*) 0;
}

Symbol* ParseReturnStatement (Scope* scope)
{
	Tag tag = ParseTag (scope);
	
	auto expression = (Expression*) Expression::parse (scope, false, &tag);
	
	if (expression is 0)
	{
		scope->Log (Error::Level::WARNING, tag.token, tag.token.str," is not a return expression");
		return (Symbol*) 0;
	}
	
	Variable* return_symbol = new Variable;
	return_symbol->kind = Symbol::RETURN;
	return_symbol->datatype = expression->datatype;
	return_symbol->Initializer = expression;
	scope->Insert (return_symbol);
	return return_symbol;
}

// TODO: ParseStructStatement implementation
Symbol* ParseStructStatement (Tag tag, Scope* scope)
{
	Scope* newscope = new Scope;
	newscope->symbolic = true;
	
	if (tag.keytag.keyword is Keyword::UNION)
		newscope->overlap = true,
		newscope->kind = Symbol::UNION;
	else
	if (tag.keytag.keyword is Keyword::MODULE)
		newscope->symbolic = false,
		newscope->kind = Symbol::MODULE;
	
	// TODO: parsing enum is significantly different
	// from parsing a struct, union, or module;
	// address here by calling a unique enum parsing fn
	
	Scope* parent = scope;
	
	Tag next = ParseTag (scope);
	
	char* name = 0;
	
	// look for name tag (optional)
	if (next.category is Tag::NEW_NAME)
	{
		if (next.nested)
			parent = (Scope*) next.scope;
		
		name = next.token.str;
		next = ParseTag (scope);
	}
	
	// look for open brace (optional)
	bool braced = next.token.kind is '{';
	bool embedded = true;
	
	ParseBlockStatement (newscope, braced, embedded);
	
	parent->Insert (newscope, name);
	return newscope;
}

Symbol* ParseInstanceStatement (Tag t, Tag n, Scope* s)
{
	// TODO //
}

Symbol* ParseSelectionStatement (Tag t, Scope* s)
{
	// TODO //
}

}
