#include "parse.h"
#include "symbol.h"
#include "expression.h"

#include "cext/trie.h"

#include <stack>
#include <sstream>

#define is ==
#define isnt !=

typedef TokenID ID;

namespace CatLang
{

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

Scope ParseSource();
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
	
	while ((Symbol*) -1 isnt ParseStatement (&compile_unit));
	
	return compile_unit;
}

// -------------------------------- //

bool IsType (Symbol* symbol)
{
	return
	Symbol::TUPLE <= symbol->type and
	Symbol::ENUM >= symbol->type;
}

bool IsObject (Symbol* symbol)
{
	return
	Symbol::VARIABLE <= symbol->type and
	Symbol::MODULE >= symbol->type;
}

std::vector <char*> LastComment;

enum Keyword
{
	// meta
	INCLUDE,
	INLINE,
	SIZEOF, COUNTOF,
	NAMEOF, TYPEOF,
	// storage //
	LOCAL,
	STATIC,
	CONST,
	// alu //
	SIGNED, UNSIGNED,
	COMPLEX, IMAGINARY,
	// flow //
	BREAK, CONTINUE, RETURN, GOTO,
	// declaration //
	STRUCT, UNION,
	MODULE, ENUM,
	// loop //
	WHILE, DO_WHILE, FOR,
	// selection //
	IF, ELSE, SWITCH,
	CASE, DEFAULT,
};

Trie <char, fast> keywords;

struct Tag
{
	Token token;
	
	enum Category
	{
		END_INPUT = -1,
		
		NEW_NAME,
		
		PLACEHOLDER,
		OBJECT,
		OPERATOR,
		TYPE,
		
		STAR,
		ACCESS,
		SEPARATOR, // () {} [] ... ; //
		KEYWORD,
	}
	category;
	
	Type type;
	
	enum Object
	{
		VARIABLE = Symbol::VARIABLE,
		EXPRESSION = Symbol::EXPRESSION,
		FUNCTION = Symbol::FUNCTION,
		MODULE = Symbol::MODULE
	}
	object;
	
	Keyword keyword;
	
	Array <string> path;
	Symbol* reference;
	bool nested;
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
	case I::HASH: {
	// unused tokens //
	std::stringstream msg;
	msg << "Ignoring unused character '"<<token.str.ptr<<"'\n";
	Log ((char*) msg.str().c_str(), token, Error::Level::NOTE, scope);
	goto get_token; }
	
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
	
	default:
	return tag;
	}
}

Tag ParseSymbolTag (Scope*, Token);

Tag ParseNameTag (Scope* scope, Token token)
{
	Tag tag;
	
	int IsntKeyword;
	Keyword keyword;
	
	keyword = (Keyword) keywords.find (token.str, &IsntKeyword);
	
	if (not IsntKeyword)
	{
		tag.category = Tag::KEYWORD;
		tag.keyword = keyword;
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
	
	bool unresolved = false;
	
	lookup:
	tag.path.push_back (token.str.ptr);
	if (unresolved) goto check_for_next;
	
	symbol = lookup (token.str, scope);
	
	if (symbol is 0) {
		tag.category = tag.NEW_NAME;
		unresolved = true;
		std::stringstream msg;
		msg <<	"'"<<tag.token.str.ptr<<"' not found in '"<<scope->name<<"'\n";
		Log ((char*) msg.str().c_str(), tag.token, Error::Level::ERROR, scope);
		goto check_for_next; }
	
	tag.reference = symbol;
	
	if (IsObject (symbol))
		tag.category = tag.OBJECT,
		tag.object = (Tag::Object) symbol->type;
	
	else
	if (IsType (symbol))
	{
		tag.category = tag.TYPE;
		Type type;
		type.datatype = Type::STRUCTURED;
		type.complex = (Tuple*) symbol;
		tag.type = type;
	}
	else
	{
		fprintf (stderr, "Invalid named symbol\n");
	}
	
	check_for_next:
	next = scanner.get();
	
	if (next.id is '.')
	{
		tag.nested = true;
		token = scanner.get();
		if (tag.category is Tag::OBJECT
		and tag.object < Tag::FUNCTION)
		{
			auto type = ((Object*) symbol)->datatype;
			if (type.datatype isnt Type::STRUCTURED);
				//Log (""
		}
		scope = (Scope*) symbol;
		goto lookup; }
	else
		scanner.unget (next);
	
	tag.reference = symbol;
	return tag;
}

Symbol* ParseVariableOrFunctionStatement (Tag type, Tag name, Scope*);
Symbol* ParseKeywordStatement (Tag, Scope*);

Symbol* ParseScopeStatement (Tag, Scope*);
Symbol* ParseExpressionStatement (Tag, Scope*);
Symbol* ParseTupleStatement (Tag, Scope*);

// parses one statement into the symbol table
Symbol* ParseStatement (Scope* scope)
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
		return (Symbol*) -1; // stop parsing, end of file
		
		case Tag::NEW_NAME: {
		
		scope->Gotos.insert
		(
			std::pair<string, fast> (
			tag.token.str.ptr,
			scope->Expressions.size() )
		);
		
		return (Symbol*) 0; }
		
		case Tag::PLACEHOLDER:
		case Tag::OBJECT:
		case Tag::OPERATOR:
		case Tag::STAR:
		return ParseExpressionStatement (tag, scope);
		
		case Tag::TYPE:
		next = ParseTag (scope);
		if (next.category == Tag::NEW_NAME)
			return ParseVariableOrFunctionStatement (tag, next, scope);
		
		case Tag::ACCESS:
		Log ("Use . after a name to access a field (ignoring)", tag.token, Error::Level::NOTE, scope);
		goto next_tag;
		
		case Tag::SEPARATOR:
		switch (tag.token.id)
		{
			case TokenID::BRACE_L:
			return ParseScopeStatement (tag, scope);
			
			case TokenID::BRACKET_L:
			return ParseExpressionStatement (tag, scope);
			
			case TokenID::PAREN_L:
			return ParseTupleStatement (tag, scope);
			
			case TokenID::BRACE_R:
			
			if (not scope->braced)
				Log ("'}' should be paired with an '{', otherwise use '...'", tag.token, Error::Level::WARNING, scope);
			scope->closed = true;
			
			
			case TokenID::BRACKET_R:
			Log ("']' has no matching '['", tag.token, Error::Level::WARNING, scope);
			goto next_tag;
		
			case TokenID::PAREN_R:
			Log ("')' has no matching '('", tag.token, Error::Level::WARNING, scope);
			goto next_tag;
			
			default: break;
		}
		
		case Tag::KEYWORD:
		return ParseKeywordStatement (tag, scope);
	}

	// *snippet for later, probably in ParseExpressionStatement* //
	Log ("'..' denotes a range - place between two values:	1..9", tag.token, Error::Level::WARNING, scope);
}

Symbol* ParseVariableOrFunctionStatement (Tag tag, Scope* scope)
{
	// TODO
}

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
	switch (tag.keyword)
	{
		typedef Keyword ID;
		
		case Keyword::INCLUDE:
			
		return ParseIncludeStatement (scope);
		
		case Keyword::STRUCT:
		case Keyword::UNION:
		case Keyword::ENUM:
		case Keyword::MODULE:
		
		return ParseStructStatement (tag, scope);
		
		case Keyword::LOCAL:
		case Keyword::STATIC:
		case Keyword::CONST:
		case Keyword::INLINE:
		case Keyword::SIGNED:
		case Keyword::UNSIGNED:
		case Keyword::COMPLEX:
		case Keyword::IMAGINARY:
		
		return ParseVariableOrFunctionStatement (tag, ParseTag (scope), scope);
		
		case Keyword::IF:
		case Keyword::ELSE:
		case Keyword::DO_WHILE:
		case Keyword::WHILE:
		case Keyword::FOR:
		case Keyword::SWITCH:
		
		return ParseSelectionStatement (tag, scope);
		
		case Keyword::RETURN:
		
		// TODO: implement return keyword in context of the syntax table
		return ParseReturnStatement (scope);
		
		case Keyword::SIZEOF:
		case Keyword::TYPEOF:
		case Keyword::COUNTOF:
		case Keyword::NAMEOF:
		
		return ParseExpressionStatement (tag, scope);
		
		case Keyword::GOTO:
		return ParseJumpStatement (tag, scope);
		
		case Keyword::BREAK:
			
		if (Symbol::Kind::IF >= scope->type or
				Symbol::Kind::SWITCH <= scope->type)
			Log ("Break used outside of loop", tag.token, Error::Level::WARNING, scope);
			
		return ParseJumpStatement (tag, scope);
		
		case Keyword::CONTINUE:
			
		if (Symbol::Kind::IF >= scope->type or
				Symbol::Kind::SWITCH <= scope->type) {
			Log ("Continue used outside of loop", tag.token, Error::Level::WARNING, scope);
			return (Symbol*) 0; }
				
		return ParseJumpStatement (tag, scope);
		
		case Keyword::DEFAULT:
			
		if (scope->type isnt Symbol::Kind::SWITCH) {
			Log ("Ignoring default used outside of switch", tag.token, Error::Level::WARNING, scope);
			return (Symbol*) 0; }
			
		return ParseCaseStatement (tag, scope);
		
		case Keyword::CASE:
			
		if (Symbol::Kind::SWITCH != scope->type) {
			Log ("Ignoring case used outside of switch", tag.token, Error::Level::WARNING, scope);
			return (Symbol*) 0; }
			
		return ParseCaseStatement (tag, scope);
	}
}

// -------------------------------- //
// -------------------------------- //

Symbol* ParseTupleStatement (Tag tag, Scope* scope)
{
	// TODO: could be a TUPLE or EXPRESSION
	// aka: VARIABLE definition, FUNCTION definition, or EXPRESSION
	
}

Symbol* ParseScopeStatement (Tag tag, Scope* scope)
{
	// TODO: parse a stack frame / anonymous namespace
	
}

Symbol* ParseExpressionStatement (Tag tag, Scope* scope)
{
	// TODO
}

// --------------------------------

Symbol* ParseIncludeStatement (Scope* scope)
{
	Scanner scanner = {};
	
	Token token = scanner.get();
	
	if (not TokenIs (TokenID::NAME, token))
		// TODO: add details to this err //
		Log ("Expected symbol name after include", token, Error::Level::WARNING, scope);
	
	return (Symbol*) 0;
}

Symbol* ParseJumpStatement (Tag tag, Scope* scope)
{
	// TODO
}

Symbol* ParseReturnStatement (Scope* scope)
{
	Scanner scanner = {};
	Token token = scanner.get ();
	
	Expression* expr
//	 = (Expression*) ParseExpression (token, scope)
	;
	
	if (expr == 0)
	{
		Log ("Expected expression after return", token, Error::Level::WARNING, scope);
		return (Symbol*) 0;
	}
	
	scope->Return.Initializer = expr;
}

}
