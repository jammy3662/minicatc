#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#include "token.h"
#include "container.h"

const char* doubleOperators = ".<>+=-/&|";
const char* tripleOperators = ".<>";
const char* assignOperators = "+-*/%&^|";

FILE* source;

typedef TokenID ID;

long line, column;

array <Token> Scanner::source;

char fgetc_lc (FILE* f)
{
	char c = fgetc (f);
	
	if (c == '\n') line++;
	column++;
	
	return c;
}

#define fgetc(F) fgetc_lc (F)

void SetSource (FILE* fp)
{
	source = fp;
}

Token getnumber (char first)
{
	Token w;
	w.kind = ID::INT_CONST;
	
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
		w.kind = ID::FLOAT_CONST;
		hasDecimal = 1;
		str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == 'e' || c == 'E' &&	not hasExponent && not hasPrefix)
	{
		w.kind = ID::FLOAT_CONST;
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
	?	w.kind = ID::STR_CONST
	: w.kind = ID::CHAR_CONST;
	
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
	if (c == -1) { w.kind = ID(-1); return w; }
		
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
		w.kind = ID::UNDERSCORE;
		str.append (c);
		c = fgetc (source);
		goto update;
	}
	
nextletter:

	w.kind = ID::NAME;
	
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
	w.kind = ID::COM_LINE;
	
	array<char> str = {};
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { str.append(0); w.kind = ID(-1); return w; }
	}
	str.append (c);
	
	char n = fgetc (source);
	
	while (not (c != '\\' && n == '\n'))
	{
		if (n == -1) { str.append(0); w.kind = ID(-1); return w; }
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
	w.kind = ID::COM_BLOCK;
	
	array<char> str = {};
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { str.append(0); w.kind = ID(-1); return w; }
	}
	str.append (c);
	
	char n = fgetc (source);
	
	while (not (c == '*' && n == '/'))
	{
		if (n == -1) { str.append(0); w.kind = ID(-1); return w; }
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
	w.kind = ID(op);
	
	array<char> str = {};
	str.append (op);
	
	char next = fgetc (source);
	if (next == -1) { ungetc (next, source); goto finish; }
	
	if (op == next)
		str.append (op),
		w.kind = ID (w.kind + ID::x2);
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
	w.kind = ID(op);
	
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
		w.kind = ID (w.kind + ID::x3);
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
	w.kind = ID(EOF);
	
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
	(c == EOF) ? ungetc (c, source) :0; // keep reading EOF indefinitely
	
	w.loc.line = line;
	w.loc.column = column;
	
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

	w.kind = ID(c);
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

TokenType TypeOf (Token t)
{
	switch (t.kind)
	{
		case ID::NAME:
		case ID::UNDERSCORE:
			return TokenType::LABEL;
		
		case ID::INT_CONST:
		case ID::FLOAT_CONST:
		case ID::CHAR_CONST:
		case ID::STR_CONST:
			return TokenType::CONSTANT;
		
		case ID::COM_LINE:
		case ID::COM_BLOCK:
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
	while (token.kind == ID::COM_LINE || token.kind == ID::COM_BLOCK);

	return token;
}

Scanner::~Scanner ()
{
	if (undoAfter)
	
		// put tokens back in the same order as they started in
		for (fast i = source.count-1; i >= 0; --i)
		
			source.append (buffer [i]);
	
	buffer.clear ();
}

Token Scanner::get (bool discard_comments)
{
	Token t;
	
	if (source.count > 0) {
		
		--source.count;
		t = source [source.count];
	}
	else
		t = gettoken();
	
	buffer.append (t);
	return t;
}

Token Scanner::top (bool discardComments)
{	
	return buffer [buffer.count-1];
}

void Scanner::unget ()
{
	if (buffer.count < 1) return;
	
	buffer.count--;
	Token t = buffer [buffer.count];
	source.append (t);
}
