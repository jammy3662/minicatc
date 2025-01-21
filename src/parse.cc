#include "parse.h"
#include "token.h"

typedef Token::ID ID;

namespace CatLang
{
	Symbol* parseLabel (Token token, Symbol* scope)
	{
		// TODO: check for language keywords
		// then determine what definition is referenced
	}
	
	void parseSymbol (Symbol* scope)
	{
		Scanner scanner = {};
		
		Token token;
		
		firsttoken:
		
		token = scanner.get();
		
		switch (typeOf (token))
		{
			case TokenType::LABEL:
				return parseLabel (token, scope);
			
			case TokenType::LITERAL:
				return parseLiteral (token, scope);
			
			case TokenType::COMMENT:
				// for now, ignore comments
				goto firsttoken;
			
			case TokenType::NONE:
				// end of file reached, stop parsing
				break;
			
			case TokenType::PUNCTUATION:
				return parsePunctuation	(token, scope);
		}
		
	}

	Symbol parseSource ()
	{
		Symbol root = {};
		
		Symbol*	next;
		do next = parseSymbol (&root); 
		until (next == NULL);
		
		return root;
	}
}
