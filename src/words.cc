#include "stdlib.h"
#include <ctype.h>
#include <string.h>

#include "words.h"

const char* doubleOperators = "+=-/&|;";
const char* tripleOperators = ".<>";

FILE* source;

Word::Word ()
{
	multiple = 1;
}

Word getnumber (char first)
{
	Word w;
	w.id = Word::INT;
	w.str.append (first);
	
	char c = fgetc (source);
	
	char hasPrefix = 0, hasDecimal = 0, hasExponent = 0, hasNegativeInExponent = 0; 
	
	// special case for a prefix
	// in the second character
	switch (c)
	{
		case 'x': case 'o': case 'b':
			w.str.append (c);
			c = fgetc (source);
			hasPrefix = 1;
			goto readone;
		default:
			break;
	}
	
readone:
	
	if (c == '_' || isdigit (c))
	{
		w.str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == '.' && !hasDecimal && !hasPrefix)
	{
		w.id = Word::FLOAT;
		hasDecimal = 1;
		goto readone;
	}
	if (c == 'e' || c == 'E' &&	!hasExponent && !hasPrefix)
	{
		w.id = Word::FLOAT;
		hasExponent = 1;
		w.str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == '-' && hasExponent && !hasNegativeInExponent && !hasPrefix)
	{
		hasNegativeInExponent = 1;
		w.str.append (c);
		c = fgetc (source);
		goto readone;
	}
	if (c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F')
	{
		w.str.append (c);
		return w;
	}
	
	// found a non-number character,
	// put it back and end the wordt
	ungetc (c, source);
	w.str.append (0);
	w.str.shrink();
	return w;
}

Word getstring (char delim)
{
	Word w;
	w.id = Word::STR;
	
	char escaped = 0;
	
	char c = 0;
	goto nextc;
	
addc:
	w.str.append (c);
nextc:
	if (c == '\\' && !escaped)
		escaped = 1;
	else escaped = 0;
	
	c = fgetc (source);
	if (c == -1) { w.id = -1; return w; }
		
	if (c != delim || escaped) goto addc;
	
	w.str.append (0);
	w.str.shrink();
	return w;
}

Word getalpha (char first)
{
	Word w;
	w.id = Word::TXT;
	w.str.append (first);
	
	char c = fgetc (source);
	
	while (c == '_' || isalpha (c) || isdigit (c))
	{
		w.str.append (c);
		c = fgetc (source);
	}
	
	ungetc (c, source);
	
	w.str.append (0);
	w.str.shrink();
	return w;
}

Word getlinecom ()
{
	Word w;
	w.id = Word::COM_LINE;
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { w.str.append(0); w.id = -1; return w; }
	}
	w.str.append (c);
	
	char n = fgetc (source);
	
	while (!(c != '\\' && n == '\n'))
	{
		if (n == -1) { w.str.append(0); w.id = -1; return w; }
		w.str.append (n);
		c = n;
		n = fgetc (source);
	}
	
	// remove trailing empty space
	char* last = w.str.buf + w.str.count-1;
	while (isblank(*last) || iscntrl(*last))
		last--, w.str.count--;
	
	w.str.append(0);
	w.str.shrink();
	return w;
}

Word getblockcom ()
{
	Word w;
	w.id = Word::COM_BLOCK;
	
	char c = ' ';
	while (isblank(c) || iscntrl(c))
	{
		c = fgetc (source);
		if (c == -1) { w.str.append(0); w.id = -1; return w; }
	}
	w.str.append (c);
	
	char n = fgetc (source);
	
	while (!(c == '*' && n == '/'))
	{
		if (n == -1) { w.str.append(0); w.id = -1; return w; }
		w.str.append (n);
		c = n;
		n = fgetc (source);
	}
	
	w.str.count -= 1; // remove the trailing '*' from '*/'
	// remove trailing empty space
	char* last = w.str.buf + w.str.count-1;
	while (isblank(*last) || iscntrl(*last))
		last--, w.str.count--;
	
	w.str.append(0);
	w.str.shrink();
	return w;
}

Word get2 (char op)
{
	Word w;
	w.id = op;
	w.str.append (op);
	
	char next = fgetc (source);
	if (next == -1) { ungetc (next, source); w.str.append (0); w.str.shrink(); return w; }
	
	if (op == next)
		w.str.append (op),
		w.multiple = 2;
	else
		ungetc (next, source);
	
	w.str.append (0);
	w.str.shrink();
	return w;
}

Word get3 (char op)
{
	Word w;
	w.id = op;
	w.str.append (op);
	
	char next = fgetc (source);
	if (next == -1) { ungetc (next, source); w.str.append (0); w.str.shrink(); return w; }
	char follow = fgetc (source);
	if (follow == -1) { ungetc (follow, source); w.str.append (0); w.str.shrink(); return w; }
	
	if (op == next == follow)
		w.str.append (op),
		w.str.append (op),
		w.multiple = 3;
	else
		ungetc (follow, source),
		ungetc (next, source);
	
	w.str.append (0);
	w.str.shrink();
	return w;
}

Word getword ()
{
	Word w;
	w.id = -1;
	
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

	w.id = c;
	w.str.append (c);
	
	if (c == '/')
	{
		char n = fgetc (source);
		if (n == '/') return getlinecom ();
		if (n == '*') return getblockcom ();
		else ungetc (n, source);
	}	
{
	const char *cc;
		cc = strchr (doubleOperators, c);
	if (cc) return get2 (*cc);
		cc = strchr (tripleOperators, c);
	if (cc) return get3 (*cc);
}
	w.str.append (0);
	w.str.shrink ();
	return w;
}
