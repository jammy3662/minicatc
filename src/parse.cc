#include "parse.h"
#include "symbol.h"
#include "expression.h"

#include "cext/trie.h"

typedef TokenID ID;

namespace CatLang
{
	Reference ParseSource();
	
	Reference ParseSource (char* path)
	{
		FILE* fp = fopen (path, "r");
		
		if (not fp)
		{
			fprintf (stderr, "Failed to open file at '%s'\n", path);
			return (Reference) {};
		}
		
		return ParseSource (fp);
	}
	
	Reference ParseSource (FILE* sourceFile)
	{
		if (not sourceFile) fprintf (stderr, "Failed to read a null file pointer\n");
		
		SetSource (sourceFile);
		
		return ParseSource ();
	}
	
	Symbol* ParseSymbol (Reference);
	
	Reference ParseSource ()
	{
		Scope container;
		
		Reference root = RefFrom (& container);
		root.Symbol = &container;
		
		while ((Symbol*) -1 != ParseSymbol (root));
		
		return root;
	}
	
	// ----------------------------------------------------------------
	
	struct ParseError {};
	
	bool DescribesType (Symbol s)
	{
		return (SymbolTypes::TUPLE <= s.SymbolType and
		        SymbolTypes::FUNCTION >= s.SymbolType);
	}
	
	bool DescribesType (Reference r)
	{
		return DescribesType (*r.Symbol);
	}
	
	// parse a statement based on its first symbol
	typedef Symbol* (ParseStatementFunc) (Token, Symbol*);
	ParseStatementFunc
	ParseLabelStatement,
	ParseParenthesesStatement,
	ParseExpressionStatement,
	ParseScopeStatement,
	ParseControlStatement,
	ParseEmptyStatement;
	
	// parses one statement into the symbol table
	Symbol* ParseSymbol (Symbol* scope)
	{
		Scanner scanner = {};
		
		Token token;
		
		readtoken:
		token = scanner.get();
		
		switch (TypeOf (token))
		{
			typedef TokenType TT;
			typedef TokenID ID;
			
			case TT::NONE:
				// end of file reached, stop parsing
				return (Symbol*) -1;
			
			case TT::COMMENT:
				// discard comments in isolation (use note keyword)
				// and get the next token
				goto readtoken;
			
			case TT::LABEL:
				
				return ParseLabelStatement (token,scope);
				
			case TT::CONSTANT:
				
				return ParseExpressionStatement (token,scope);
				
			case TT::PUNCTUATION:
				
				if (IsOperator (token))
					
					return ParseExpressionStatement (token,scope);
				
				switch (token.id)
				{
					// ignore unused characters
					case ID::HASH:
					case ID::DOLLAR:
					case ID::AT:
						Log ("Ignoring unused character", ErrorLevel::LEVEL_NOTE, scope);
						goto readtoken;
					
					// no statement begins with these
					case ID::TAIL:
						Log ("Tail (..) is not valid here", ErrorLevel::LEVEL_WARNING, scope);
						goto readtoken;
					
					case ID::ARROW:
						Log ("Arrow (->) is not valid here", ErrorLevel::LEVEL_WARNING, scope);
						goto readtoken;
					
					case ID::BRACE_R:
					
						if (SymbolTypes::VARIABLE <= scope->SymbolType and
						    SymbolTypes::TUPLE >= scope->SymbolType)
							 Log ("Ignoring unmatched }", ErrorLevel::LEVEL_WARNING, scope);
							 goto readtoken;
					
					case ID::BRACKET_R:
						
						if (not SymbolTypes::EXPRESSION == scope->SymbolType)
							Log ("Ignoring unmatched ]", ErrorLevel::LEVEL_WARNING, scope);
							goto readtoken;
					
					case ID::PAREN_R:
						
						if (not SymbolTypes::EXPRESSION == scope->SymbolType and
						    not SymbolTypes::TUPLE == scope->SymbolType)
							Log ("Ignoring unmatched )", ErrorLevel::LEVEL_WARNING, scope);
							goto readtoken;
					
					case ID::SEMI:
					case ID::ELLIPSES:
					
						// close the current statement or scopes
						((Scope*) scope)->open = false;
						return ParseEmptyStatement (token,scope);
					
					case ID::DOT:
					case ID::UNDERSCORE: // extraneous; underscore is not included in punctuation characters
						
						return ParseLabelStatement (token,scope);
					
					case ID::BRACE_L:
					
						return ParseScopeStatement (token,scope);
					
					case ID::PAREN_L:
					
						return ParseParenthesesStatement (token,scope);
					
					case ID::BRACKET_L:
						
						return ParseExpressionStatement (token,scope);
					
					default: break;	
				}
			
		}
		
		// if this part executes, something went really wrong
		// all possible token IDs should be covered by the switch above
		fprintf (stderr, "Unknown token (ID %i) at line %i col %i\n", token.id, token.loc.line, token.loc.column);
		return (Symbol*) 0;
	}
	
	Symbol* ParseEmptyStatement (Token token, Symbol* scope)
	{
		// unmatched closing symbol; ignore but warn
		Error err;
		err.severity = ErrorLevel::LEVEL_WARNING;
		err.loc = token.loc;
		
		const char* message = "Unmatched '%c', ignoring\n";
		err.message = (char*) malloc (strlen (message) * sizeof (char));
		sprintf (err.message, message, token.id);
		
		scope->errors.push_back (err);
		
		return (Symbol*) 0;
	}
	
	// statements beginning with a keyword (not representing a type)
	Symbol* ParseKeywordStatement (Token, Symbol* scope, KeywordID);
	ParseStatementFunc ParseVariableOrFunctionStatement;
	
	Symbol* ParseLabelStatement (Token token, Symbol* scope)
	{
		// statement starting with a name or keyword
		// could be a VARIABLE, FUNCTION, TYPE NAME, or KEYWORD
		int IsNotKeyword;
		KeywordID keywordid = (KeywordID) keywords.find (token.str, &IsNotKeyword);
		
		if (IsNotKeyword) return ParseKeywordStatement (token, scope, keywordid);
		
		Symbol* ref = lookup (token.str, scope);
		
		switch (ref->SymbolType)
		{
			typedef SymbolTypes ST;
			
			case ST::PLACEHOLDER:
			case ST::VARIABLE:
			case ST::EXPRESSION:
				return ParseExpressionStatement (token, scope);
			
			default:
				return ParseVariableOrFunctionStatement (token, scope);
		}
	}
	
	Symbol* ParseStructStatement (Token token, Symbol* scope, KeywordID struct_type);
	
	Symbol* ParseKeywordStatement (Token token, Symbol* scope, KeywordID id)
	{
		// TODO: could be a STRUCT, UNION, ENUM, MODULE, declaration specifier,
		// or control structure (if / switch / etc)
		switch (id)
		{
			typedef KeywordID ID;
			
			case ID::KW_STRUCT:
			case ID::KW_UNION:
			case ID::KW_ENUM:
			case ID::KW_MODULE:
				return ParseStructStatement (token, scope, id);
			
			case ID::KW_LOCAL:
			case ID::KW_STATIC:
			case ID::KW_CONST:
			case ID::KW_INLINE:
			case ID::KW_MATH_SIGNED:
			case ID::KW_MATH_UNSIGNED:
			case ID::KW_MATH_COMPLEX:
			case ID::KW_MATH_IMAGINARY:
				return ParseVariableOrFunctionStatement (token, scope);
			
			case ID::KW_IF:
			case ID::KW_ELSE:
			case ID::KW_DO:
			case ID::KW_WHILE:
			case ID::KW_FOR:
			case ID::KW_SWITCH:
				return ParseControlStatement (token, scope);
			
			case ID::KW_RETURN:
				// TODO: implement return keyword in context of the syntax table
			
			default:
				// TODO: control flow keyword being used outside a control structure
				break;
		}
	}
	
	ParseStatementFunc
	ParseVariableStatement,
	ParseFunctionStatement;
	
	Symbol* ParseVariableOrFunctionStatement (Token token, Reference scope)
	{
		
	}
	
	Symbol* ParseParenthesesStatement (Token token, Reference scope)
	{
		// TODO: could be a TUPLE or EXPRESSION
		
	}
	
	Symbol* ParseScopeStatement (Token token, Reference scope)
	{
		// TODO: parse a stack frame / anonymous namespace
		
	}
	
	Symbol* ParseExpressionStatement (Token token, Reference scope)
	{
		
	}
}
