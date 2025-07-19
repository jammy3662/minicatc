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

Symbol* ParseBlockStatement (Scope*, bool braced = false, bool embedded = false);

Scope ParseSource ()
{
	Scope compile_unit;
	
	ParseBlockStatement (&compile_unit, false, true);
	
	return compile_unit;
}

// -------------------------------- //

std::vector <char*> LastComment;

Trie <char, fast> keywords;

struct Tag
{
	Token token;
	
	char* str = token.str.ptr;
	
	enum Category
	{
		END_INPUT = -1,
		
		NEW_NAME,
		PLACEHOLDER,
		
		OBJECT,
		TYPE,
		FUNCTION,
		MARKER,
		
		OPERATOR,
		STAR,
		ACCESS,
		SEPARATOR, // () {} [] ... ; //
		KEYWORD,
	}
	category;
	
	Type type;
	
	enum ObjectKind
	{
		VARIABLE = Symbol::VARIABLE,
		EXPRESSION = Symbol::EXPRESSION,
		MODULE = Symbol::MODULE
	}
	object_kind;
	
	Keyword keyword;
	
	Array <Token> tokens;
	Array <string> path;
	Symbol* reference;
	bool nested;
};

Array <Tag> tags;

Tag ParseTagLocal (Scope*);

Tag ParseTag (Scope* s)
{
	if (tags.size() > 0)
	{
		Tag t = tags.back();
		tags.pop_back();
		return t;
	}
	
	return ParseTagLocal (s);
}

void ReplaceTag (Tag t)
{
	tags.push_back (t);
}

Tag ParseNameTag (Scope*, Token);

Tag ParseTagLocal (Scope* scope)
{
	Tag tag;
	
	Scanner scanner;
	
	Token& token = tag.token;
	
	get_token:
	token = scanner.get();
	tag.tokens.push_back (token);
	
	if (IsOperator (token))
	tag.category = tag.OPERATOR;
	return tag;
	
	typedef TokenID I;
	switch (token.kind) {
	
	case I::COM_LINE:
	case I::COM_BLOCK:
	
	LastComment.push_back (token.str);
	goto get_token;
	
	case I::AT:
	case I::DOLLAR:
	case I::HASH: {
	// unused tokens //
	std::stringstream msg;
	msg << "Ignoring unused character '"<<tag.str<<"'";
	auto stream = msg.str();
	auto str = (char*) stream.c_str();
	Log (str, token, Error::Level::NOTE, scope);
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
	tag.tokens.push_back (token);
	tag.path.push_back (tag.str);
	if (unresolved) goto check_for_next;
	
	symbol = lookup (token.str, scope);
	
	if (symbol is 0)
	{
		tag.category = tag.NEW_NAME;
		unresolved = true;
		
		std::stringstream msg;
		msg <<	"'"<<tag.str<<"' not found in '"<<scope->name<<"'";
		auto stream = msg.str();
		auto str = (char*) stream.c_str();
		Log (str, tag.token, Error::Level::ERROR, scope);
		
		goto check_for_next;
	}
	
	tag.reference = symbol;
	
	switch (symbol->kind)
	{
		case Symbol::FUNCTION:
		tag.category = Tag::FUNCTION;
		break;
		
		case Symbol::VARIABLE:
		case Symbol::EXPRESSION:
		tag.category = Tag::OBJECT;
		tag.object_kind = (Tag::ObjectKind) symbol->kind;
		break;
		
		case Symbol::TUPLE:
		case Symbol::STRUCT:
		case Symbol::UNION:
		case Symbol::ENUM:
		case Symbol::MODULE:
		tag.category = Tag::TYPE,
		tag.type.data = Type::STRUCTURED,
		tag.type.definition = (Tuple*) symbol;	
		break;
		
		case Symbol::GOTO:
		tag.category = Tag::MARKER;
		break;
		
		default:
		fprintf (stderr, "Invalid named symbol\n");
	}
	
	check_for_next:
	next = scanner.get();
	
	if (next.kind isnt '.')
	{
		scanner.unget ();
		return tag;
	}

	tag.nested = true;
	token = scanner.get();
	
	if (tag.category is Tag::OBJECT)
	{
		auto obj = (Object*) symbol;
		
		if (obj->datatype.data is Type::STRUCTURED)
			symbol = obj->datatype.definition;
		
		else
		{
			// TODO: err simple type cannot have members
			
			unresolved = true; // (can't look up names in here, even though the symbol exists)
		}
	}
	
	// look up the next name in the symbol just found
	scope = (Scope*) symbol;
	goto lookup;
}

Symbol* ParseMarker (Tag&, Scope*);
Symbol* ParseTypedStatement (Tag, Scope*);
Symbol* ParseKeywordStatement (Tag, Scope*);

//Symbol* ParseBlockStatement (Scope*, bool braced = false);
Symbol* ParseExpressionStatement (Tag, Scope*);
Symbol* ParseTupleStatement (Scope*);

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
		return last_symbol; // stop parsing, end of file
		
		case Tag::NEW_NAME:
		return ParseMarker (tag, scope);
		
		case Tag::PLACEHOLDER:
		case Tag::OBJECT:
		case Tag::OPERATOR:
		case Tag::STAR:
		return ParseExpressionStatement (tag, scope);
		
		case Tag::TYPE:
		return ParseTypedStatement (tag, scope);
		
		case Tag::ACCESS:
		Log ("Use . after a name to access a field (ignoring)", tag.token, Error::Level::NOTE, scope);
		goto next_tag;
		
		case Tag::SEPARATOR:
		switch (tag.token.kind)
		{
			case TokenID::BRACE_L:
			return ParseBlockStatement (scope, true);
			
			case TokenID::BRACKET_L:
			return ParseExpressionStatement (tag, scope);
			
			case TokenID::PAREN_L:
			return ParseTupleStatement (scope);
			
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

// -------------------------------- //

Symbol* ParseMarker (Tag& tag, Scope* scope)
{
	if (not tag.nested)
	{
		Scanner scanner;
		Token next = scanner.get();
		
		if (next.kind is ':')
		{
			Goto* go_to = new Goto;
			go_to->kind = Symbol::GOTO;
			
			fast target_index = scope->Members.size();
			go_to->target_index = target_index;
			
			scope->Tags.insert (std::make_pair (tag.str, go_to));
			
			return go_to;
		}
		else scanner.unget ();
	}
	
	return (Symbol*) 0;
}

Symbol* ParseTupleStatement (Scope* scope)
{
	// TODO: could be a TUPLE or EXPRESSION
	// aka: VARIABLE definition, FUNCTION definition, or EXPRESSION
	
}

Symbol* ParseBlockStatement (Scope* p, bool braced, bool embedded)
{
	Scope* s;
	
	if (not embedded)
	{
		// create a new scope symbol and add it to the parent
		s = new Scope;
		p->Members.push_back (s);
	}
	else
		// if embedded, edit the existing scope symbol directly
		s = p;
	
	s->braced = braced;
	
	while (not s->closed)
		ParseStatement (s);
	
	return s;
}

Symbol* ParseTypedStatement (Tag tag, Scope* scope)
{
	// TODO
}

Symbol* ParseExpressionStatement (Tag tag, Scope* scope)
{
	// TODO
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
		
		return ParseTypedStatement (tag, scope);
		
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
		
		return ParseExpressionStatement (tag, scope);
		
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

Symbol* ParseCaseStatement (Tag t, Scope* s)
{
	if (s->kind isnt Symbol::Kind::SWITCH) {
		
		std::stringstream msg;
		msg <<	"Ignoring "<<t.token.str<<" used outside of switch";
		auto stream = msg.str();
		auto str = (char*) stream.c_str();
		Log (str, t.token, Error::Level::WARNING, s); }
	
	// TODO: look for an expression for case or ; for default
}

Symbol* ParseJumpStatement (Tag tag, Scope* scope)
{
	Tag next;
		
	switch (tag.keyword)
	{
		case Keyword::BREAK:
		
		case Keyword::CONTINUE:
		
		case Keyword::GOTO:
		// TODO: look for a name tag for a goto marker
		next = ParseTag (scope);
		if (next.category is Tag::KEYWORD and
		    not next.nested);
				
		case Keyword::RETURN:
		
		
		default: break;
	}
	
	{
		Symbol::Kind upper_bound = Symbol::FOR;
		// continue can only be in a loop
		// break can be in a loop OR switch
		if (tag.keyword is Keyword::BREAK)
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
	Scanner scanner = {};
	Token token = scanner.get ();
	
	Expression* expr
//	 = (Expression*) ParseExpression (token, scope)
	;
	
	if (expr is 0)
	{
		Log ("Expected expression after return", token, Error::Level::WARNING, scope);
		return (Symbol*) 0;
	}
	
	//scope->Return.Initializer = expr;
}

Symbol* ParseStructStatement (Tag t, Scope* s)
{
	Scope* n = new Scope;
	
	n->overlap = t.keyword is Keyword::UNION;
	n->symbolic = t.keyword isnt Keyword::MODULE;
	
	Tag next = ParseTag (s);
	
	// TODO: look for name tag
	if (next.category is Tag::NEW_NAME and
	    not next.nested)
	
	// TODO: look for open brace (optional)
	if (next.token.kind is '{')
		n->braced = true;
	
	ParseBlockStatement (n, true, true);
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
