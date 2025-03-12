#include "parse.h"
#include "token.h"

typedef Token::ID ID;

namespace CatLang
{
	struct ParseError {};
	
	Reference parseLabel (Token token, Container* scope)
	{
		// TODO: check for language keywords
		// then determine what definition is referenced
	}
	
	Reference parseConstant (Token token, Container* scope)
	{
		
	}
	
	void parseSymbol (Container* scope)
	{
		Scanner scanner = {};
		
		Token token;
		
		readtoken:
		token = scanner.get();
		
		switch (tokenType (token))
		{
			case TokenType::NAME:
				parseLabel (token, scope);
				return;
			
			case TokenType::LITERAL:
				parseConstant (token, scope);
				return;
			
			case TokenType::COMMENT:
				// for now, ignore comments
				goto readtoken;
			
			case TokenType::PUNCTUATION:
				parsePunctuation	(token, scope);
				return;
			
			case TokenType::NONE:
				// end of file reached, stop parsing
				return;
		}
		
	}

	Container parseSource ()
	{
		Container root;
		
		parseSymbol (&root);
		
		return root;
	}
}
