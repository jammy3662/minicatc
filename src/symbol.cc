#include "symbol.h"

#include "trie.h"
#include "container.h"



struct Symbol
{
	int type;
	int idx;
};

// user defined operator behaviors
// aka 'operator overloading'
arr <Operator> operations;

struct Scope
{
	Trie <char, Typeid> types;
	
	Trie <char, Var> objects;
	Trie <char, Func> functions;
};

static
Symbol global;
Symbol* scope = &global;

typedef char** Name;

arr <Typeid*>
types;

// identifies a type signature with a number
Trie <Typeid, Typeid>
typetable;

int getName (Name* nameRef, Word* first = 0x0)
{
	Word w;
	if (first) w = *first;
	else w = getword();
	
	arr<Word> buf;
	
	
}

int getTuple (Word open_paren, Typeid** tp)
{
	Word w;
	
	arr <Typeid> sig;
	sig.allocate (2);

	w = getword();
	
	nexttype:
	
	if (w.id != ')') goto nexttype;
	
	if (tp) *tp = sig.ptr;
	
	return 0;
}

int getType (Typeid* tp)
{
	Word w = getword();
	
	arr<Typeid> sig;
	sig.allocate (2);
	
	if (w.id == '(')
		return getTuple (w, &sig.ptr) | getTypeid (sig, tp);
	
	if (w.id == WordID::TXT)
	{
		Name name;
		
		int err = getName (&name);
		if (err != 0) return err;
		
		
	}
	
	return 1;
}

int getTypeid (Typeid* fields, Typeid* aType)
{
	aType->id = 0;
	const Typeid zerotype = {0};
	
	// if not present, add to type list
	// and store its index
	if (!typetable.find (fields, zerotype, aType))
	{
		aType->id = types.count + Builtin::tpOff;
		typetable.insert (fields, zerotype, *aType);
		types.append (fields);
	}
	
	return 0;
}

int getFields (Typeid t, Typeid** aFields)
{
	Typeid::ID idx = t.id - Builtin::tpOff;  // indices start at 0,
	                                // but values start after integral enum constants
	*aFields = types [idx];
	
	return 0;
}
