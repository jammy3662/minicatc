#include <stack>
#include <sstream>

#include "parse.h"
#include "symbol.h"
#include "expression.h"

#include "cext/trie.h"

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
	
	ParseBlockStatement (compile_unit, false, true);
	
	return (Symbol*) compile_unit;
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
		
		UNRESOLVED,
		NEW_NAME,
		PLACEHOLDER,
		
		VARIABLE,
		EXPRESSION,
		TYPE,
		FUNCTION,
		MARKER,
		
		OPERATOR,
		STAR, // pointer or multiplication
		TILDE, // reference or complement
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
	string path;
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
	
	case I::TILDE:
	tag.category = Tag::TILDE;
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
	
	bool not_found = false;
	bool not_container = false;
	
	lookup:
	tag.token = token;
	tag.tokens.push_back (token);
	
	if (not_container) not_found = true;
		
	if (not_found) goto check_for_dot;
	
	symbol = lookup (token.str, scope);
	
	if (symbol is 0)
	{
		tag.category = tag.NEW_NAME;
		not_found = true;
		
		goto check_for_dot;
	}
	
	tag.reference = symbol;
	
	switch (symbol->kind)
	{
		case Symbol::FUNCTION:
		tag.category = Tag::FUNCTION;
		break;
		
		case Symbol::VARIABLE:
		tag.category = Tag::VARIABLE;
		not_container = ((Object*)symbol)->datatype.data isnt Type::STRUCTURED;
		scope = ((Object*)symbol)->datatype.definition;
		break;
		
		case Symbol::EXPRESSION:
		tag.category = Tag::EXPRESSION;
		not_container = ((Object*)symbol)->datatype.data isnt Type::STRUCTURED;
		scope = ((Object*)symbol)->datatype.definition;
		break;
		
		case Symbol::TUPLE:
		case Symbol::STRUCT:
		case Symbol::UNION:
		case Symbol::ENUM:
		case Symbol::MODULE:
		tag.category = Tag::TYPE,
		tag.type.data = Type::STRUCTURED,
		tag.type.definition = (Scope*) symbol;	
		break;
		
		case Symbol::MARKER:
		tag.category = Tag::MARKER;
		not_container = true;
		break;
		
		default:
		fprintf (stderr, "Invalid named symbol: %s\n", symbol->name);
	}
	
	check_for_dot:
	next = scanner.get();
	
	if (next.kind isnt '.')
	{
		scanner.unget ();
		
		tag.path.pop_back ();
		
		if (not_found and not_container)
			
			tag.category = Tag::UNRESOLVED,
			Log	(Error::ERROR, tag.token, scope,
		       tag.token.str," can't be in ",tag.path,", which is not a type or structure with fields");
		
		return tag;
	}

	tag.nested = true;
	tag.path = tag.path + tag.str + '.';
	token = scanner.get();
	
	// look up the next name in the symbol just found
	scope = (Scope*) symbol;
	goto lookup;
}

Symbol* ParseMarker (Tag, Scope*);
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
		Log (Error::NOTE, tag.token, scope, "Try goto ",tag.token.str," to jump to marker ",tag.token.str," (the name of the marker does nothing by itself)");
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
		switch (tag.separator)
		{
			case Tag::SEMI:
			return (Symbol*) 0;
			
			case Tag::BRACE_L:
			return ParseBlockStatement (scope, true);
			
			case Tag::BRACKET_L:
			return ParseExpressionStatement (tag, scope);
			
			case Tag::PAREN_L:
			return ParseTupleStatement (scope);
			
			case Tag::BRACE_R:
			
			if (not scope->braced)
				Log (Error::WARNING, tag.token, scope, "} should be paired with an {, otherwise use ...");
			scope->closed = true;
			return (Symbol*) 0;
			
			case Tag::ELLIPSES:
			scope->closed = true;
			return (Symbol*) 0;
			
			case Tag::BRACKET_R:
			
			Log (Error::WARNING, tag.token, scope,
		    	 "] has no matching [");
			goto next_tag;
			
			case Tag::PAREN_R:
			
			Log (Error::WARNING, tag.token, scope,
			     ") has no matching (");
			goto next_tag;
			
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

Symbol* ParseMarker (Tag tag, Scope* scope)
{
	if (not tag.nested)
	{
		Scanner scanner;
		Token next = scanner.get();
		
		if (next.kind is ':')
		{
			Marker* go_to = new Marker;
			go_to->target_index =			
				scope->Insert (go_to);
			
			return go_to;
		}
		else scanner.unget ();
	}
	
	return (Symbol*) 0;
}


// TODO: ParseTuple implementation
Symbol* ParseTuple (Scope* scope);

Symbol* ParseTupleStatement (Scope* scope)
{
	// TODO: could be a TUPLE or EXPRESSION
	// aka: VARIABLE definition, FUNCTION definition, or EXPRESSION
	
}

Symbol* ParseBlockStatement (Scope* parent, bool braced, bool embedded)
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
		ParseStatement (scope);
	
	return scope;
}

// TODO: ParseType implementation
Type ParseType (Tag, Scope*);

// TODO: ParseVariable and ParseFunction implementations
Symbol* ParseVariable (char* name, Type datatype, bool initialize, Scope*);
Symbol* ParseFunction (char* name, Type return_type, Scope*);

Symbol* ParseTypedStatement (Tag tag, Scope* scope)
{
	Type datatype = ParseType (tag, scope);
	
	if (datatype.data is Type::INVALID)
		return (Symbol*) 0;
	
	Scanner scanner;
	Token next_token;
	
	tag = ParseTag (scope);
	
	Symbol* parent = scope;
	
	switch (tag.category)
	{
		case Tag::PLACEHOLDER:
		case Tag::NEW_NAME:
		
		if (tag.nested)
			parent = tag.reference;
		
		next_token = scanner.get ();
		switch (next_token.kind)
		{
			case TokenID::PAREN_L:
			return ParseFunction (tag.token.str, datatype, scope);
			
			case TokenID::EQUAL:
			return ParseVariable (tag.token.str, datatype, true, scope);
			
			case TokenID::SEMI:
			return ParseVariable (tag.token.str, datatype, false, scope);
			
			default:
			scanner.unget ();
			Log	(Error::WARNING, next_token, scope, tag.token.str," is not an initializer expression, parameter list, or end of statement",tag.token.str);
			return (Symbol*) 0;
		}
		
		case Tag::VARIABLE:
		case Tag::EXPRESSION:
		// type cast, if type followed by object
		return ParseExpressionStatement (tag, scope);
		
		default:
		Log (Error::WARNING, tag.token, scope, tag.token.str," cannot have a type of ",datatype.print());
		return (Symbol*) 0;
	}
}

// TODO: ParseExpression implementation
Expression* ParseExpression (Tag, Scope*);

Symbol* ParseExpressionStatement (Tag tag, Scope* scope)
{
	// TODO:
	// this fn should only need to call
	// ParseExpression one time and return
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

Type ParseType (Tag tag, Scope* scope)
{
	Type type;
	
	enum { TYPE, KEYWORD, POINTER }
	last_parsed;
	
	bool type_parsed = false;
	
	string wrong_tag = " is not a datatype or qualifier";
	
	while (true)
	{
		switch (tag.category)
		{
		case Tag::KEYWORD:
			last_parsed = KEYWORD;
			
			if (tag.keytag.category isnt Tag::Keytag::QUALIFIER)
			{
				Log (Error::WARNING, tag.token, scope,
				tag.token.str, wrong_tag);
				
				if (type_parsed) return type;
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
					type.indirection |= 1 << (2*type.indirection_ct+1);
				else
					type.CONST = true;
				break;
				
				default:
				// unreachable (should be)
				// TODO: qualifier enum to remove this ambiguity
				fprintf (stderr, "%s keyword incorrectly passed as qualifier\n", tag.token.str);
			}
		break;
			
		case Tag::TYPE:
			
			if (type_parsed)
				Log (Error::WARNING, tag.token, scope,
				tag.token.str," overrides previously specified type of ", type.print_data());
			else
				type_parsed = true,
				last_parsed = TYPE;
			
			type = tag.type;
			
		break;
			
		case Tag::STAR:
		case Tag::TILDE:
			
			if (not type_parsed)
			{
				Log (Error::WARNING, tag.token, scope,
				"Can't specify pointer before underlying datatype");
				break;
			}
			last_parsed = POINTER;
			if (tag.category is Tag::TILDE)
				type.indirection |= 1 << (2*type.indirection_ct+0);
			type.indirection_ct++;
			
		break;
			
		default:
			Log	(Error::WARNING, tag.token, scope,
			tag.token.str, wrong_tag);
			
			if (type_parsed) return type;
		}
		tag = ParseTag (scope);
	}
	
	return type;
}

// TODO: ParseCaseStatement implementation
Symbol* ParseCaseStatement (Tag tag, Scope* scope)
{
	if (scope->kind isnt Symbol::SWITCH) {
		
		Log (Error::WARNING, tag.token, scope,
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
	
	auto expression = (Expression*) ParseExpression (tag, scope);
	
	if (expression is 0)
	{
		Log (Error::Level::WARNING, tag.token, scope, tag.token.str," is not a return expression");
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
			parent = (Scope*) next.reference;
		
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
