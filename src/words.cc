#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#include "words.h"
#include "container.h"

const char* doubleOperators = ".<>+=-/&|;";
const char* tripleOperators = ".<>";

FILE* source;

typedef Word::ID ID;

Word getnumber (char first)
{
	Word w;
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
		// put it back and end the word
		ungetc (c, source);
	}
	
	str.append (0);
	str.shrink();
	w.str = str;
	return w;
}

Word getstring (char delim)
{
	Word w;
	
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

Word getalpha (char first)
{
	Word w;
	
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

Word getlinecom ()
{
	Word w;
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

Word getblockcom ()
{
	Word w;
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

int get2 (char op, Word* _word_)
{
	Word w;
	w.id = ID(op);
	
	array<char> str = {};
	str.append (op);
	
	char next = fgetc (source);
	if (next == -1) { ungetc (next, source); goto finish; }
	
	if (op == next)
		str.append (op),
		w.id = ID (w.id | WordDOUBLEOPBit);
	else
	{
		ungetc (next, source);
		return 0;
	}
	
finish:
	str.append (0);
	str.shrink();
	w.str = str;
	*_word_ = w;
	return 1;
}

int get3 (char op, Word* _word_)
{
	Word w;
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
		w.id = ID (w.id | WordTRIPLEOPBit);
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
	*_word_ = w;
	return 1;
}

array <Word> buf = {};

Word getword ()
{
	Word w;
	w.id = ID(EOF);
	
	// if words were put back earlier, use those first
	if (buf.count > 0)
{
	w = buf [buf.count - 1];
	buf.count--;
	if (buf.count % 8 == 0) buf.shrink();
	return w;
}	
	
	array<char> str = {};
	
	if (source == 0x0)
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
		Word test;
		
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

void putwordback (Word w)
{
	buf.append (w);
}

bool Word::isSyntax ()
{
	return (id < INT_LIT);
}

bool Word::isLiteral ()
{
	return (id >= INT_LIT && id <= STR_LIT);
}

bool Word::isLabel ()
{
	return (id == LABEL);
}

bool Word::isEmpty ()
{
	return (id >= PLACEHOLDER && id <= COM_BLOCK);
}

bool Word::isInvalid ()
{
	return (id == ID(EOF));
}

// get non-comment words
Word getwordc ()
{
	Word word;
	do
	{
		word = getword ();
	}
	while (word.id == word.COM_LINE || word.id == word.COM_BLOCK);

	return word;
}

array<Word> Scanner::source = {};

Scanner::~Scanner ()
{
	if (putback)
	{
		// put tokens back in the same order as they started in
		for (int i = buffer.count-1; i >= 0; --i)
		{
			source.append (buffer [i]);
		}
	}
	
	buffer.clear ();
}

Word Scanner::get (bool discardComments)
{
	while (source.count > 0)
		if (not source [--source.count].isEmpty())
			return source [source.count];
	
	while (buffer.count > 0)
		if (not buffer [--buffer.count].isEmpty())
			return buffer [buffer.count];
	
	return getwordc ();
}

Word Scanner::top (bool discardComments)
{
	Word result = {};
	
	for (int idx = source.count - 1; idx > 0; --idx)
	{
		if (! source [idx].isEmpty()) return source [idx];
	}
	for (int idx = buffer.count - 1; idx > 0; --idx)
	{
		if (! buffer [idx].isEmpty()) return buffer [idx];
	}
	
	// if word is from file directly,
	// take it and copy it back to peek
	result = getwordc();
	source.append (result);
	
	return result;
}

void Scanner::unget (Word w)
{
	buffer.append (w);
}
