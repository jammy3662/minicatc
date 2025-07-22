#include <stack>
#include <sstream>

#include "parse.h"
#include "symbol.h"
#include "expression.h"

#include "cext/trie.h"

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

template <typename... T>
Error Log (Error::Level lvl, Token token, Symbol* scope, T... diagnostics)
{
	std::stringstream msg;
	
	(msg << ... << diagnostics);
		
	auto str = msg.str();
	auto cstr = (char*) str.c_str();
	
	return Log (cstr, token, lvl, scope);
}

struct Tag
{
	Token token;
	
	char* str = token.str.ptr;
	
	enum Category
	{
		END_INPUT = -1,
		
		NEW_NAME,
		PLACEHOLDER,
		
		VARIABLE,
		EXPRESSION,
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
	
	enum { PAREN_L, PAREN_R, BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, ELLIPSES, SEMI } separator;
	
	struct Keytag
	{
		Keyword keyword;
		enum {QUALIFIER, JUMP, DECL, LOOP, SELECTION, INCLUDE, META} category;
	}
	keytag;
	
	Symbol* reference;
	
	
	bool nested;
	Array <Token> tokens;
	Array <string> path;
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
	
	typedef TokenID I;
	switch (token.kind) {
	
	case I::COM_LINE:
	case I::COM_BLOCK:
	
	LastComment.push_back (token.str);
	goto get_token;
	
	case I::AT:
	case I::DOLLAR:
	case I::HASH:
	// unused tokens //
	Log (Error::NOTE, token, scope, "Ignoring unused character ",tag.str);
	goto get_token;
	
	case I::END_FILE:
	tag.category = Tag::END_INPUT;
	return tag;
	
	case I::ELLIPSES:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::ELLIPSES;
	return tag;
	
	case I::SEMI:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::SEMI;
	return tag;
	
	case I::BRACE_L:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::BRACE_L;
	return tag;
	
	case I::BRACE_R:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::BRACE_R;
	return tag;
	
	case I::BRACKET_L:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::BRACKET_L;
	return tag;
	
	case I::BRACKET_R:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::BRACKET_R;
	return tag;
	
	case I::PAREN_L:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::PAREN_L;
	return tag;
	
	case I::PAREN_R:
	tag.category = Tag::SEPARATOR;
	tag.separator = Tag::PAREN_R;
	return tag;
		
	case I::INT_CONST:
	case I::FLOAT_CONST:
	case I::CHAR_CONST:
	case I::STR_CONST:
	
	tag.category = Tag::EXPRESSION;
	return tag;
	
	case I::STAR:
	
	tag.category = Tag::STAR;
	return tag;
	
	case I::UNDERSCORE:
	
	tag.category = Tag::PLACEHOLDER;
	return tag;
	
	case I::DOT:
	
	tag.category = Tag::ACCESS;
	return tag;
		
	case I::NAME:
	
	return ParseNameTag (scope, token);
	
	default: break;
	}
	
	if (IsOperator (token))
	tag.category = tag.OPERATOR;
	return tag;
	
	fprintf (stderr, "Failed parsing tag:	%s", token.str.ptr);
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
		tag.keytag.keyword = keyword;
		switch (keyword)
		{
			case LOCAL:
			case STATIC:
			case CONST:
			case INLINE:
			case SIGNED:
			case UNSIGNED:
			case COMPLEX:
			case IMAGINARY:
			tag.keytag.category = tag.keytag.QUALIFIER;
			break;
			
			case BREAK:
			case CONTINUE:
			case RETURN:
			case GOTO:
			tag.keytag.category = tag.keytag.JUMP;
			break;
			
			case STRUCT:
			case UNION:
			case ENUM:
			case MODULE:
			tag.keytag.category = tag.keytag.DECL;
			break;
			
			case WHILE:
			case DO:
			case FOR:
			tag.keytag.category = tag.keytag.LOOP;
			break;
			
			case IF:
			case ELSE:
			case SWITCH:
			case CASE:
			case DEFAULT:
			tag.keytag.category = tag.keytag.SELECTION;
			break;
			
			case INCLUDE:
			tag.keytag.category = tag.keytag.INCLUDE;
			break;
			
			case SIZEOF:
			case COUNTOF:
			case NAMEOF:
			case TYPEOF:
			tag.keytag.category = tag.keytag.META;
			break;
		}
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
		
		Log (Error::ERROR, tag.token, scope,
		     tag.str," not found in ",scope);
		goto check_for_next;
	}
	
	tag.reference = symbol;
	
	switch (symbol->kind)
	{
		case Symbol::FUNCTION:
		tag.category = Tag::FUNCTION;
		break;
		
		case Symbol::VARIABLE:
		tag.category = Tag::VARIABLE;
		break;
		
		case Symbol::EXPRESSION:
		tag.category = Tag::EXPRESSION;
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
	
	if (tag.category is Tag::VARIABLE or
	    tag.category is Tag::EXPRESSION)
	{
		auto obj = (Object*) symbol;
		
		if (obj->datatype.data is Type::STRUCTURED)
			symbol = obj->datatype.definition;
		
		else
		{
			Log (Error::ERROR, token, scope,
			     "Basic type ",token.str," doesn't have fields");
			
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
		
		case Tag::MARKER:
		Log (Error::NOTE, tag.token, scope, "Try goto ",tag.token.str," to jump execution (the name of the marker does nothing by itself)");
		return (Symbol*) 0;
		
		case Tag::NEW_NAME:
		return ParseMarker (tag, scope);
		
		case Tag::PLACEHOLDER:
		case Tag::VARIABLE:
		case Tag::EXPRESSION:
		case Tag::FUNCTION:
		case Tag::OPERATOR:
		case Tag::STAR:
		return ParseExpressionStatement (tag, scope);
		
		case Tag::TYPE:
		return ParseTypedStatement (tag, scope);
		
		case Tag::ACCESS:
		Log (Error::NOTE, tag.token, scope, "Use . after a name to access a field (ignoring)");
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
				Log (Error::WARNING, tag.token, scope, "'}' should be paired with an '{', otherwise use '...'");
			scope->closed = true;
			
			
			case TokenID::BRACKET_R:
			
			Log (Error::WARNING, tag.token, scope,
			     "] has no matching [");
			goto next_tag;
			
			case TokenID::PAREN_R:
			
			Log (Error::WARNING, tag.token, scope,
			     ") has no matching (");
			goto next_tag;
			
			default: break;
		}
		
		case Tag::KEYWORD:
		return ParseKeywordStatement (tag, scope);
	}

	// *snippet for later, probably in ParseExpressionStatement* //
	if (0)
	Log (Error::WARNING, tag.token, scope,
	     ".. denotes a range - place between two values:	1..9");
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
			
			scope->Insert (go_to);
			
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
		p->Insert (s);
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
	get_type_or_qualifier:
	
	switch (tag.category)
	{
		case Tag::TYPE:
		
		case Tag::KEYWORD:
		
		default:
		break;
	}
	
	Type type = tag.type;
	
	
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
	switch (tag.keytag.keyword)
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
		
	switch (tag.keytag.keyword)
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
	
	n->overlap = t.keytag.keyword is Keyword::UNION;
	n->symbolic = t.keytag.keyword isnt Keyword::MODULE;
	
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
