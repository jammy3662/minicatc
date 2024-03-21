#include "language.h"

struct Primitive {char* name; Typeid type;};

arr <Tuple> langTypes;
Trie <char, int> langTypeTable;

const
Primitive langtypes [] =
{
	{"void", VOID},
	{"let", AUTO}, {"auto", AUTO},
	{"var", VAR},
	{"char", CHAR}, {"byte", CHAR},
	{"int", INT},
	{"float", FLOAT},
	{"double", DOUBLE},
};

const
Primitive keywords [] =
{
	{"local", AUTO},
	{"auto", AUTO},
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
	
	{"typeof", TYPEOF},
	{"nameof", NAMEOF},
	{"countof", COUNTOF},
	{"fieldsof", FIELDSOF},
	
	{"struct", STRUCT},
	{"union", UNION},
	{"enum", ENUM},
	{"module", MODULE},
	
	{"if", IF}, {"else", ELSE},
	{"while", WHILE}, {"switch", SWITCH},
	{"case", CASE}, {"do", DO},
	{"break", BREAK}, {"default", DEFAULT},
	{"continue", CONTINUE}, {"for", FOR},
	{"return", RETURN}, {"end", END},
};

LanguageInit::LanguageInit ()
{
	language.defs = (SymbolTable*) malloc (sizeof( SymbolTable ));
	
	range (Primitive, type, langtypes,
{
	language.defs;
})
}

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
