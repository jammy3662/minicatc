#include "words.h"

#include "stdlib.h"

Word getword (FILE* stream)
{
	Word word = {0};
	int i = 0;
	int sz = 32;
	
	word.str = (char*) malloc (sz * sizeof (char));
	word.id = 0;
	
	#define append() word.str [i] = c; i++; sz++; if (!(i % sz)) {	sz *= 2; char* s = word.str; word.str = (char*) realloc (word.str, sz * sizeof (char));	if (!word.str) printf ("Failed realloc %u bytes\n", sz);} 
	
	static char c; // preserve the last character pulled from the previous call
	
	// skip whitespace and non-printable chars
	while (c < '!' || c > '~') c = fgetc (stream);
	
	if (c == -1)
	{
		word.id = -1;
		return word;
	}

getnumeric:
	if (c >= '0' && c <= '9' ||	(c == '_' && word.id != 0))
	{
		if (word.id == 0) word.id = Word::INT;
		if (c != '_')
			append ();
		
		if (word.id == Word::INT)
		{
			char n = fgetc (stream);
			
			switch (n)
			{ case 'x':
			  case 'o':
			  case 'b':
					word.id = Word::FINT; break;
			  case '.':
					word.id = Word::DEC; break;
			  default: break; }
			c = n;
			append ();
		}
		c = fgetc (stream);
		goto getnumeric;
	}
	else if (word.id == Word::INT || word.id == Word::FINT || word.id == Word::DEC)
		goto endword;

getalpha:
	if (c >= 'A' && c <= 'z' || c == '_')
	{
		word.id = Word::TXT;
		append ();
		c = fgetc (stream);
		goto getalpha;
	}
	else if (word.id == Word::TXT) goto endword;

	if ((c >= '!' && c <= '@') ||
	    (c >= '[' && c <= '`') ||
			(c >= '{' && c <= '~'))
		{
			append ();
			word.id = c;
			goto endword;
		}
		
	
endword:

	word.str = (char*) realloc (word.str, i+1 * sizeof (char));
	word.str [i] = 0;
	
	if (word.id < Word::INT) c = fgetc (stream);
	
	switch (word.id)
	{
		case Word::INT:
		case Word::FINT:
			word.value.l = strtol (word.str, 0x0, 0);
			break;
		case Word::DEC:
			word.value.d = strtod (word.str, 0x0);
			break;
		default:
			break;
	}
	
	return word;
}
