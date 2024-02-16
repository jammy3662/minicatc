#include "words.h"

#include "stdlib.h"
#include <ctype.h>

Word getnumber (char first, FILE* f)
{
	Word w;
	w.id = Word::INT;
	w.str.append (first);
	
	char c = fgetc (f);
	
	char hasPrefix = 0, hasDecimal = 0, hasExponent = 0, hasNegativeInExponent = 0; 
	
	// special case for a prefix
	// in the second character
	switch (c)
	{
		case 'x': case 'o': case 'b':
			w.str.append (c);
			c = fgetc (f);
			hasPrefix = 1;
			goto readone;
		default:
			break;
	}
	
readone:
	
	if (c == '_' || isdigit (c))
	{
		w.str.append (c);
		c = fgetc (f);
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
		c = fgetc (f);
		goto readone;
	}
	if (c == '-' && hasExponent && !hasNegativeInExponent && !hasPrefix)
	{
		hasNegativeInExponent = 1;
		w.str.append (c);
		c = fgetc (f);
		goto readone;
	}
	if (c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F' || c == 'l' || c == 'L')
	{
		w.str.append (c);
		return w;
	}
	
	// found a non-number character,
	// put it back and end the word
	ungetc (c, f);
	w.str.shrink();
	return w;
}

Word getstring (char delim, FILE* f)
{
	Word w;
	w.id = Word::STR;
	
	char c = fgetc (f);
	
	char escapedNow = 0;

lookforclose:
	while (c != delim)
	{
		if (c == '\\' && !escapedNow)
			escapedNow = 1;
		else
			escapedNow = 0;
		w.str.append (c);
		c = fgetc (f);
	}
	if (escapedNow)
	{
		w.str.append (c);
		c = fgetc (f);
		goto lookforclose;
	}
	
	return w;
}

Word getalpha (char first, FILE* f)
{
	Word w;
	w.id = Word::TXT;
	w.str.append (first);
	
	char c = fgetc (f);
	
	while (c == '_' || isalpha (c) || isdigit (c))
	{
		w.str.append (c);
		c = fgetc (f);
	}
	
	ungetc (c, f);
	
	w.str.shrink();
	return w;
}

Word getdoubleop (char op, FILE* f)
{
	Word w;
	w.id = op;
	
	char next = fgetc (f);
	w.str.append (op);
	
	if (op == next)
		w.str.append (op);
	else
		ungetc (next, f);
	
	w.str.shrink();
	return w;
}

Word gettripleop (char op, FILE* f)
{
	Word w;
	w.id = op;
	
	char next = fgetc (f), follow = fgetc (f);
	w.str.append (op);
	
	if (op == next == follow)
		w.str.append (op),
		w.str.append (op);
	else
		ungetc (follow, f),
		ungetc (next, f);
	
	w.str.shrink();
	return w;
}

Word getword (FILE* stream)
{
	Word w;
	w.id = -1;
	
	char c = fgetc (stream);
	
	while (isblank(c) || iscntrl(c))
		c = fgetc (stream);
	
	if (c == '"' || c == '\'')
	{
		return getstring (c, stream);
	}
	if (c == '_' ||	isalpha (c))
	{
		return getalpha (c, stream);
	}
	if (isdigit (c))
	{
		return getnumber (c, stream);
	}

	w.id = c;
	w.str.append (c);
	
	char* doubleOperators = "+=-/&|";
	char* tripleOperators = ".<>";
	for (char* op = doubleOperators; *op !=0; ++op)
	{
		if (*op == c)
			return getdoubleop (c, stream);
	}
	for (char* op = tripleOperators; *op !=0; ++op)
	{
		if (*op == c)
			return gettripleop (c, stream);
	}
	
	w.str.shrink ();
	return w;
}
