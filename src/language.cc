#include "language.h"

struct Keyword {char* name; Typeid type;};

const
Keyword keywordPairs [] =
{
	{"void", VOID_T},
	{"var", VAR_T},
	{"char", CHAR_T}, {"byte", CHAR_T},
	{"int", INT_T},
	{"float", FLOAT_T},
	{"double", DOUBLE_T},
	
	{"let", AUTO},
	{"auto", AUTO},
	{"local", LOCAL},
	{"static", STATIC},
	{"extern", EXTERN},
	{"register", REGISTER},
	{"const", CONST},
	{"volatile", VOLATILE},
	{"inline", INLINE},
	{"local", LOCAL},
	{"short", SHORT},
	{"long", LONG},
	{"signed", SIGNED},
	{"unsigned", UNSIGNED},

	{"include", INCLUDE},
	{"merge", SHORT},
	
	{"sizeof", SIZEOF},
	{"typeof", TYPEOF},
	{"nameof", NAMEOF},
	{"countof", COUNTOF},
	{"fieldsof", FIELDSOF},
	
	{"struct", STRUCT},
	{"union", UNION},
	{"enum", ENUM},
	{"module", MODULE},
	
	{"do", DO}, {"end", END},
	{"if", IF}, {"else", ELSE},
	{"while", WHILE}, {"switch", SWITCH},
	{"case", CASE}, {"default", DEFAULT},
	{"break", BREAK}, {"continue", CONTINUE},
	{"for", FOR},
	{"return", RETURN},
};

LanguageInit _;
LanguageInit::LanguageInit ()
{
	SymbolTable defs;
	
	const int ct = sizeof (keywordPairs) / sizeof (*keywordPairs);
	
	int i;
	for (i = 0; i < 7; ++i)
	{
		Keyword k = keywordPairs [i];
		defs.types.index.insert (k.name, (char)0, k.type);
	}
	for (; i < ct; ++i)
	{
		Keyword k = keywordPairs [i];
		defs.keywords.insert (k.name, (char)0, k.type);
	}
	
	language.defs = (SymbolTable*) malloc (sizeof( SymbolTable ));
	*language.defs = defs;
}

Word getwordF (Object* scope)
{
	Word res;
	res = getword();
	
	if (not isBuiltin (res.id))
{
		long newid;
		
		check:
		
		if (scope != 0x0)
	{
			int err;
			newid = scope->defs->keywords.find (res.str, (char)0, &err);
			
			if (!err)
		{
				res.id = newid;
				return res;
		}
			
			newid = scope->defs->types.index.find (res.str, (char)0, &err);
			
			if (!err) { res.id = newid; }
			else { scope = &language; goto check; }
	}
	else { scope = &language; goto check; }
}
	return res;
}
