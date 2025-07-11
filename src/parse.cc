#include "parse.h"
#include "symbol.h"
#include "expression.h"

#include "cext/trie.h"

#include <stack>

#define is ==

typedef TokenID ID;

namespace CatLang
{

Scope ParseSource();

Scope ParseSource (char* path)
{
	FILE* fp = fopen (path, "r");
	
	if (not fp)
	{
		fprintf (stderr, "Failed to open file at '%s'\n", path);
		return Scope();
	}
	
	return ParseSource (fp);
}

Scope ParseSource (FILE* sourceFile)
{
	if (not sourceFile) fprintf (stderr, "Failed to read a null file pointer\n");
	
	SetSource (sourceFile);
	
	return ParseSource ();
}

Symbol* ParseStatement (Scope*);

Scope ParseSource ()
{
	Scope compile_unit;
	
	Symbol* next_symbol;
	
	do next_symbol = ParseStatement (&compile_unit);
	while ((Symbol*) -1 != next_symbol);
	
	return compile_unit;
}

// ----------------------------------------------------------------

bool IsType (Symbol* symbol)
{
	return
	Symbol::TUPLE <= symbol->type <=
	Symbol::ENUM;
}

bool IsObject (Symbol* symbol)
{
	return
	Symbol::VARIABLE <= symbol->type <=
	Symbol::MODULE;
}

std::vector <char*> LastComment;

struct Tag
{
	Token token;
	
	enum Category
	{
		#define keyword(__) __ = Keyword:: ## __
		
		END_INPUT = -1,
		
		NEW_NAME,
		
		PLACEHOLDER,
		OBJECT,
		OPERATOR,
		TYPE,
		
		STAR,
		ACCESS,
		SEPARATOR, // () {} [] ... ; //
		// ---- KEYWORDS	---- //
		// meta //
		keyword (INCLUDE),
		keyword (INLINE),
		keyword (SIZEOF), keyword (COUNTOF),
		keyword (NAMEOF), keyword (TYPEOF),
		// storage //
		keyword (LOCAL),
		keyword (STATIC),
		keyword (CONST),
		// alu //
		keyword (SIGNED), keyword (UNSIGNED),
		keyword (COMPLEX), keyword (IMAGINARY),
		// flow //
		keyword (BREAK), keyword (CONTINUE), keyword (RETURN),
		// declaration //
		keyword (STRUCT), keyword (UNION),
		keyword (MODULE), keyword (ENUM),
		// loop //
		keyword (WHILE), keyword (FOR),
		// selection //
		keyword (IF), keyword (ELSE), keyword (SWITCH),
		keyword (CASE), keyword (DEFAULT),
		
		#undef keyword
	}
	category;
	
	union {
	
	enum Type
	{
		BUILTIN,
		STRUCT = Symbol::STRUCT,
		UNION = Symbol::UNION,
		ENUM = Symbol::ENUM,
	}
	type;
	
	enum Object
	{
		VARIABLE = Symbol::VARIABLE,
		EXPRESSION = Symbol::EXPRESSION,
		FUNCTION = Symbol::FUNCTION,
		MODULE = Symbol::MODULE
	}
	object;
	
	};
	
	Symbol* reference;
};

Tag ParseNameTag (Scope*, Token);

Tag ParseTag (Scope* scope)
{
	Tag tag;
	
	Scanner scanner;
	
	Token& token = tag.token;
	
	get_token:
	token = scanner.get();
	
	if (IsOperator (token))
	tag.category = tag.OPERATOR;
	return tag;
	
	typedef TokenID I;
	switch (token.id) {
	
	case I::COM_LINE:
	case I::COM_BLOCK:
	
	LastComment.push_back (token.str);
	goto get_token;
	
	case I::AT:
	case I::DOLLAR:
	case I::HASH:
	// unused tokens //
	Log ("Ignoring unused character", token.loc, Error::Level::NOTE, scope);
	goto get_token;
	
	case I::END_FILE:
	case I::ELLIPSES:
	case I::SEMI:
	
	case I::BRACE_L: case I::BRACE_R:
	case I::BRACKET_L: case I::BRACKET_R:
	case I::PAREN_L: case I::PAREN_R:
	
	tag.category = tag.SEPARATOR;
	return tag;
		
	case I::INT_CONST:
	case I::FLOAT_CONST:
	case I::CHAR_CONST:
	case I::STR_CONST:
	
	tag.category = tag.OBJECT;
	return tag;
	
	case I::STAR:
	
	tag.category = tag.STAR;
	return tag;
	
	case I::UNDERSCORE:
	
	tag.category = tag.PLACEHOLDER;
	return tag;
	
	case I::DOT:
	
	tag.category = tag.ACCESS;
	return tag;
		
	case I::NAME:
	
	return ParseNameTag (scope, token);
	}
}

Tag ParseSymbolTag (Scope*, Token);

Tag ParseNameTag (Scope* scope, Token token)
{
	Tag tag;
	
	int IsntKeyword;
	Keyword keyword;
	
	keyword = keywords.find (token.str, &IsntKeyword);
	
	if (not IsntKeyword)
	{
		tag.category = (Tag::Category) keyword;
		return tag;
	}
	
	return ParseSymbolTag (scope, token);
}

Tag ParseSymbolTag (Scope* scope, Token token)
{
	Tag tag;
	
	Symbol* symbol;
		
	Scanner scanner;
	Token next = scanner.get();
	
	lookup:
	symbol = lookup (token.str, scope);
	
	if (symbol == 0)
		tag.category = tag.NEW_NAME;
		
	
	next = scanner.get();
	
	if (next.id == '.') {
		token = scanner.get();
		
		goto lookup; }
	else
		scanner.unget (next);
	
	// TODO: determine between type and object
	if (IsObject (symbol))
		tag.category = tag.OBJECT,
		tag.object = (Tag::Object) symbol->type;
	
	else
	if (IsType (symbol))
		tag.category = tag.TYPE,
		tag.type = (Tag::Type) symbol->type;
	
	tag.reference = symbol;
	return tag;
}

// parse a statement based on its first symbol
typedef Symbol* (ParseStatementFunc) (Tag, Scope*);
ParseStatementFunc
ParseLabelStatement,
ParseParenthesesStatement,
ParseExpressionStatement,
ParseScopeStatement;

Symbol* ParseVariableOrFunctionStatement
(Tag type, Tag name, Scope*);
Symbol* ParseKeywordStatement (Tag, Scope*);

// parses one statement into the symbol table
Symbol* ParseStatement (Scope* scope)
{
	// if the current scope has been closed, move to the outer scope
	// (outermost scope can only be closed with end of file)
	if (scope->closed and scope->parent != 0)
	scope = scope->parent;

	Tag tag;

	next_tag:
	tag = ParseTag (scope);

	switch (tag.category)
	{
		case tag.END_INPUT:
		return (Symbol*) -1; // stop parsing, end of file
		
		case Tag::NEW_NAME:
		fast idx = scope->Expressions.size();
		scope->Gotos.insert (std::pair<string, fast>
			(tag.token.str, idx));
		return (Symbol*) 0;
		
		case Tag::PLACEHOLDER:
		case Tag::OBJECT:
		case Tag::OPERATOR:
		case Tag::STAR:
		return ParseExpressionStatement (tag, scope);
		
		case Tag::TYPE:
		Tag next = ParseTag (scope);
		if (next.category == Tag::NEW_NAME)
			return ParseVariableOrFunctionStatement (tag, next, scope);
		
		case Tag::ACCESS:
		Log ("Use . after a name to access a field (ignoring)", tag.token.loc, Error::Level::NOTE, scope);
		goto next_tag;
		
		case Tag::SEPARATOR:
		switch (tag.token.id)
		{
			case TokenID::BRACE_L:
			return ParseScopeStatement (tag, scope);
			
			case TokenID::BRACKET_L:
			return ParseExpressionStatement (tag, scope);
			
			case TokenID::PAREN_L:
			return ParseScopeStatement (tag, scope);
			
			case TokenID::BRACE_R:
			Log ("'}' should be paired with an '{', otherwise use '...'", tag.token.loc, Error::Level::WARNING, scope);
			goto next_tag;
			
			case TokenID::BRACKET_R:
			Log ("']' has no matching '['", tag.token.loc, Error::Level::WARNING, scope);
			goto next_tag;
		
			case TokenID::PAREN_R:
			Log ("')' has no matching '('", tag.token.loc, Error::Level::WARNING, scope);
			goto next_tag;
			
			default:
			return ParseKeywordStatement (tag, scope);
		}
	}

	return (Symbol*) -2;

	// *snippet for later, probably in ParseExpressionStatement* //
	// no statement begins with these
	Log ("'..' denotes a range - place between two values:	1..9", tag.token.loc, Error::Level::WARNING, scope);
}

Symbol* ParseVariableOrFunctionStatement (Tag tag, Scope* scope)
{
	
}

// statements beginning with a keyword (not representing a type)
Symbol* ParseKeywordStatement (Tag tag, Symbol* scope)
{
	// TODO: could be a STRUCT, UNION, ENUM, MODULE, declaration specifier,
	// or control structure (if / switch / etc)
	switch (tag.category)
	{
		typedef Keyword ID;
		
		case ID::INCLUDE:
			
			return ParseIncludeStatement (scope);
		
		case ID::STRUCT:
		case ID::UNION:
		case ID::ENUM:
		case ID::MODULE:
			
			return ParseStructStatement (id, scope);
		
		case ID::LOCAL:
		case ID::STATIC:
		case ID::CONST:
		case ID::INLINE:
		case ID::SIGNED:
		case ID::UNSIGNED:
		case ID::COMPLEX:
		case ID::IMAGINARY:
			
			return ParseVariableOrFunctionStatement (token,scope);
		
		case ID::IF:
		case ID::ELSE:
		case ID::DO:
		case ID::WHILE:
		case ID::FOR:
		case ID::SWITCH:
			
			return ParseControlStatement (token,scope);
		
		case ID::RETURN:
			
			// TODO: implement return keyword in context of the syntax table
			return ParseReturnStatement (scope);
		
		case ID::SIZEOF:
		case ID::TYPEOF:
		case ID::COUNTOF:
		case ID::NAMEOF:
		case ID::FIELDSOF:
		
			return ParseExpressionStatement (token,scope);
		
		case ID::BREAK:
			
			if (Symbol::Type::IF > scope->type or
					Symbol::Type::SWITCH < scope->type)
				Log ("Break used outside of loop", token.loc, Error::Level::WARNING, scope);
			
			return ParseFlowStatement (token, scope);
		
		case ID::CONTINUE:
			
			if (Symbol::Type::IF > scope->type or
					Symbol::Type::SWITCH < scope->type)
				Log ("Continue used outside of loop", token.loc, Error::Level::WARNING, scope);
				return (Symbol*) 0;
				
			return ParseFlowStatement (token, scope);
		
		case ID::CASE:
			
			if (Symbol::Type::SWITCH != scope->type)
				Log ("Ignoring case used outside of switch", token.loc, Error::Level::WARNING, scope);
				return (Symbol*) 0;
			
			return ParseCaseStatement (token, scope);
			
		case ID::DEFAULT:
			
			if (Symbol::Type::SWITCH != scope->type)
				Log ("Ignoring default used outside of switch", token.loc, Error::Level::WARNING, scope);
				return (Symbol*) 0;
			
			return ParseCaseStatement (token, scope);
	}
}

// -------------------------------- //
// -------------------------------- //

Symbol* ParseLabelStatement (Token token, Symbol* scope)
{
	// statement starting with a name or keyword
	// could be a VARIABLE, FUNCTION, TYPE NAME, or KEYWORD
	int IsNotKeyword;
	Keyword keyword = (Keyword) keywords.find (token.str, &IsNotKeyword);
	
	if (IsNotKeyword) return ParseKeywordStatement (token, scope, keyword);
	
	Symbol* ref = lookup (token.str, scope);
	
	switch (ref->type)
	{
		typedef Symbol::Type ST;
		
		case ST::PLACEHOLDER:
		case ST::VARIABLE:
		case ST::EXPRESSION:
			return ParseExpressionStatement (token, scope);
		
		default:
			return ParseVariableOrFunctionStatement (token, scope);
	}
}

Symbol* ParseParenthesesStatement (Token token, Reference scope)
{
	// TODO: could be a TUPLE or EXPRESSION
	// aka: VARIABLE definition, FUNCTION definition, or EXPRESSION
	
}

Symbol* ParseScopeStatement (Token token, Symbol* scope)
{
	// TODO: parse a stack frame / anonymous namespace
	
}

Symbol* ParseExpressionStatement (Token token, Reference scope)
{
	
}


ParseStatementFunc ParseVariableOrFunctionStatement;

Symbol* ParseStructStatement (Keyword kind, Symbol* scope);
Symbol* ParseIncludeStatement (Symbol* scope);
Symbol* ParseReturnStatement (Symbol* scope);
Symbol* ParseFlowStatement (Token, Symbol* scope);
Symbol* ParseControlStatement (Token, Symbol* scope);
Symbol* ParseCaseStatement (Token, Symbol* scope);



ParseStatementFunc
ParseVariableStatement,
ParseFunctionStatement;

Symbol* ParseIncludeStatement (Symbol* scope)
{
	Scanner scanner = {};
	
	Token token = scanner.get();
	
	if (not TokenIs (TokenID::NAME, token))
		Log ("Expected symbol name after include", token.loc, Error::Level::WARNING, scope);
	
	return (Symbol*) 0;
}

Symbol* ParseFlowStatement (Symbol* scope)
{
	
}

Symbol* ParseReturnStatement (Symbol* scope)
{
	Scanner scanner = {};
	Token token = scanner.get ();
	
	Return returnobj;
	returnobj.type = Symbol::Type::RETURN; 
	
	Expression* expr = (Expression*) ParseExpression (token, scope);
	
	if (expr == 0)
	{
		Log ("Expected expression after return", token.loc, Error::Level::WARNING, scope);
		return (Symbol*) 0;
	}
	
	returnobj.value = expr;
}

}
