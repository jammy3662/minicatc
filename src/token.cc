#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#include "token.h"
#include "container.h"

const char* doubleOperators = ".<>+=-/&|";
const char* tripleOperators = ".<>";
const char* assignOperators = "+-*/%&^|";

FILE* source;

typedef Token::ID ID;

Token getnumber (char first)
{
	Token w;
	w.id = w.INT_LIT;
	
	array<char> str = {};
	str.append (first);
	
	char c = fgetc (source);
	
	char hasPrefix = 0, hasDecimal = 0, hasExponent = 0, hasNegativeInExponent = 0; 
	
	// special case for a prefix
	// in the second character
	switch (c)
	{
		case 'x': case 'o': case 'b':
			str.append (c);
			c = fgetc (source);
			hasPrefix = 1;
			goto readone;
		default:
			break;
	}
	
readone:
	
	if (c == '_' || isdigit (c))
	{
		str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == '.' && not hasDecimal && not hasPrefix)
	{
		w.id = w.FLOAT_LIT;
		hasDecimal = 1;
		str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == 'e' || c == 'E' &&	not hasExponent && not hasPrefix)
	{
		w.id = w.FLOAT_LIT;
		hasExponent = 1;
		str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == '-' && hasExponent && not hasNegativeInExponent && not hasPrefix)
	{
		hasNegativeInExponent = 1;
		str.append (c);
		c = fgetc (source);
		goto readone;
	}
	
	if (not (c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F'))
	{
		// found a non-number character, non-suffix
		// put it back and end the token
		ungetc (c, source);
	}
	
	str.append (0);
	str.shrink();
	w.str = str;
	return w;
}

Token getstring (char delim)
{
	Token w;
	
	(delim == '"')
	?	w.id = w.STR_LIT
	: w.id = w.CHAR_LIT;
	
	array<char> str = {};
	
	char escaped = 0;
	
	char c = 0;
	goto nextc;
	
addc:
	str.append (c);
nextc:
	if (c == '\\' && not escaped)
		escaped = 1;
	else escaped = 0;
	
	c = fgetc (source);
	if (c == -1) { w.id = ID(-1); return w; }
		
	if (c != delim || escaped) goto addc;
	
	str.append (0);
	str.shrink();
	w.str = str;
	return w;
}

Token getalpha (char first)
{
	Token w;
	
	array<char> str = {};
	
	char c = first;
	
	// single underscore is treated as a special name
	// essentially ignores the symbol outside of its declaration
	if (c == '_')
	{
		w.id = w.PLACEHOLDER;
		str.append (c);
		c = fgetc (source);
		goto update;
	}
	
nextletter:

	w.id = w.LABEL;
	
update:

	str.append (c);
	
	if (c == '_' || isalpha (c) || isdigit (c))
	{
		c = fgetc (source);
		goto nextletter;
	}
	
	str.count--;
	ungetc (c, source);
	
	str.append (0);
	str.shrink();
	w.str = str;
	return w;
}

Token getlinecom ()
{
	Token w;
	w.id = w.COM_LINE;
	
	array<char> str = {};
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { str.append(0); w.id = ID(-1); return w; }
	}
	str.append (c);
	
	char n = fgetc (source);
	
	while (not (c != '\\' && n == '\n'))
	{
		if (n == -1) { str.append(0); w.id = ID(-1); return w; }
		str.append (n);
		c = n;
		n = fgetc (source);
	}
	
	// remove trailing empty space
	char* last = str.ptr + str.count-1;
	while (isblank(*last) || iscntrl(*last))
		last--, str.count--;
	
	str.append(0);
	str.shrink();
	w.str = str;
	return w;
}

Token getblockcom ()
{
	Token w;
	w.id = w.COM_BLOCK;
	
	array<char> str = {};
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { str.append(0); w.id = ID(-1); return w; }
	}
	str.append (c);
	
	char n = fgetc (source);
	
	while (not (c == '*' && n == '/'))
	{
		if (n == -1) { str.append(0); w.id = ID(-1); return w; }
		str.append (n);
		c = n;
		n = fgetc (source);
	}
	
	str.count -= 1; // remove the trailing '*' from '*/'
	// remove trailing empty space
	char* last = str.ptr + str.count-1;
	while (isblank(*last) || iscntrl(*last))
		last--, str.count--;
	
	str.append(0);
	str.shrink();
	w.str = str;
	return w;
}

int get2 (char op, Token* _token_)
{
	Token w;
	w.id = ID(op);
	
	array<char> str = {};
	str.append (op);
	
	char next = fgetc (source);
	if (next == -1) { ungetc (next, source); goto finish; }
	
	if (op == next)
		str.append (op),
		w.id = ID (w.id + Token::x2);
	else
	{
		ungetc (next, source);
		return 0;
	}
	
finish:
	str.append (0);
	str.shrink();
	w.str = str;
	*_token_ = w;
	return 1;
}

int get3 (char op, Token* _token_)
{
	Token w;
	w.id = ID(op);
	
	array<char> str = {};
	str.append (op);
	
	char next, follow;
	
	next = fgetc (source);
	if (next == -1) { ungetc (next, source); goto finish; }
	follow = fgetc (source);
	if (follow == -1) { ungetc (follow, source); goto finish; }
	
	if (op == next == follow)
		str.append (op),
		str.append (op),
		w.id = ID (w.id + Token::x3);
	else
	{
		ungetc (follow, source),
		ungetc (next, source);
		return 0;
	}

finish:
	str.append (0);
	str.shrink();
	w.str = str;
	*_token_ = w;
	return 1;
}

array <Token> buf = {};

Token gettoken ()
{
	Token w;
	w.id = ID(EOF);
	
	// if tokens were put back earlier, use those first
	if (buf.count > 0)
{
	w = buf [buf.count - 1];
	buf.count--;
	if (buf.count % 8 == 0) buf.shrink();
	return w;
}	
	
	array<char> str = {};
	
	if (source == NULL)
	{
		fprintf (stderr, "File didn't open\n");
		return w;
	}
	
	char c = fgetc (source);
	
	while (isblank(c) || iscntrl(c))
		c = fgetc (source);
	
	if (c == '"' || c == '\'')
	{
		return getstring (c);
	}
	if (c == '_' ||	isalpha (c))
	{
		return getalpha (c);
	}
	if (isdigit (c))
	{
		return getnumber (c);
	}

	w.id = ID(c);
	str.append (c);
	
	if (c == '/')
	{
		char n = fgetc (source);
		if (n == '/') return getlinecom ();
		if (n == '*') return getblockcom ();
		else ungetc (n, source);
	}	

	multiop:
	{
		Token test;
		
		const char *op;
			op = strchr (tripleOperators, c);
		if (op && get3 (*op, &test)) return test;
		
			op = strchr (doubleOperators, c);
		if (op && get2 (*op, &test)) return test;
	}

	str.append (0);
	str.shrink ();
	w.str = str;
	return w;
}

void puttokenback (Token w)
{
	buf.append (w);
}

TokenType typeOf (Token t)
{
	switch (t.id)
	{
		case Token::LABEL:
		case Token::PLACEHOLDER:
			return TokenType::LABEL;
		
		case Token::INT_LIT:
		case Token::FLOAT_LIT:
		case Token::CHAR_LIT:
		case Token::STR_LIT:
			return TokenType::LITERAL;
		
		case Token::COM_LINE:
		case Token::COM_BLOCK:
			return TokenType::COMMENT;
		
		case (EOF):
			return TokenType::NONE;
		
		default:
			return TokenType::PUNCTUATION;
	}
}

// get non-comment tokens
Token gettokenc ()
{
	Token token;
	do
	{
		token = gettoken ();
	}
	while (token.id == token.COM_LINE || token.id == token.COM_BLOCK);

	return token;
}

Scanner::~Scanner ()
{
	if (undoAfter)
	{
		// put tokens back in the same order as they started in
		for (fast i = buffer.count-1; i >= 0; --i)
		{
			puttokenback (buffer [i]);
		}
	}
	
	buffer.clear ();
}

Token Scanner::get (bool discardComments)
{
	fast available = buffer.count-consumed;
	
	if (available < 1)
	{
		Token result = gettoken ();
		
		if (discardComments)
			while (typeOf (result) == TokenType::COMMENT)
				result = gettoken ();
		
		buffer.append (result);
		consumed++;
		
		return result;
	}
	
	fast top = available-1;
	Token result = buffer [top];
	
	if (discardComments)
	 while (typeOf (result) == TokenType::COMMENT)
		consumed++,
		top--,
		result = buffer [top];
	
	return result;
}

Token Scanner::top (bool discardComments)
{
	Token result = {};
	
	fast top = (buffer.count-1) - consumed;
	if (top < 0) return (Token){};
	
	for (fast idx = buffer.count - 1; idx > 0; --idx)
	{
		if (typeOf (buffer [idx]) != TokenType::COMMENT) return buffer [idx];
	}
	
	// if token is from file directly,
	// take it and copy it back to peek
	result = gettokenc();
	buffer.append (result);
	
	return result;
}

void Scanner::unget (Token w)
{
	buffer.append (w);
}
