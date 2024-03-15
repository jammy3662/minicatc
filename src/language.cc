#include "language.h"

Symbol language = {};

LanguageConstruction::LanguageConstruction ()
{
	language.kind = STRUCT;
	
	range (Primitive, type, types,
{
	Typesig* sig = (Typesig*) malloc (sizeof( Typesig ));
	sig->kind = TYPE;
	sig->name = type.name;
	sig->id = type.type;
	
	language.insert (type.name, sig);
})
	range (Primitive, word, keywords,
{
	Keyword* w = (Keyword*) malloc (sizeof( Keyword ));
	w->kind = KEYWORD;
	w->name = word.name;
	w->id = word.type;
	
	language.insert (word.name, w);
})
}
