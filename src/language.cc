#include "language.h"

/*

//
// From an older model of the symbol structure
// TODO: update along with newer model
//

Symbol language = {};

LanguageConstruction::LanguageConstruction ()
{
	language.kind = STRUCT;
	
	range (Primitive, type, types,
{
	Symbol insert;
	insert.kind = TYPE;
	insert.name = type.name;
	
	Typesig* sig = (Typesig*) malloc (sizeof( Typesig ));
	sig->id = type.type;
	
	insert.data = sig;
	language.insert (insert.name, insert);
})
	range (Primitive, word, keywords,
{
	Symbol insert;
	insert.kind = KEYWORD;
	insert.name = word.name;
	
	Keyword* w = (Keyword*) malloc (sizeof( Keyword ));
	w->id = word.type;
	
	insert.data = w;
	language.insert (insert.name, insert);
})
}

*/
