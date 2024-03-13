#include "symbol.h"

Symbol Value::get (char* name);
Symbol Enum::get (char* name);
Symbol Section::get (char* name);

// TODO
Symbol Value::get (char* name)
{
	Symbol res = {0};
	
	if (isTmp)
	{
		fprintf (stderr, "%s is part of a temporary object and can't be modified\n", name);
		return res;
	}
	
	if (isBuiltin (type.id))
	{
		fprintf (stderr, "%s doesn't store data\n", type.printf());
		return res;
	}
	
	return variable;
}

Symbol Enum::get (char* name)
{
	Symbol res = {0};
	
	int err = 0;
	Value* value;
	
	value = values.find (name, err);
	if (err) return res;
	
	res.symbol = Symbol::VALUE,
	res.ref = value;
	
	return res;
}

Symbol Section::get (char* name)
{
	Symbol res = {0};
	
	res = index.find (name, err);
	if (err)	res.symbol = Symbol::NONE;
	
	return res;
}

static
Program* program;

Program getProgram ();
Symbol getSymbol (Section* scope);

char* Type::printf ()
{
	if (name != 0) return name;
	
	arr <char> str = {0};
	
	str.append(0);
	return str;
}

Symbol* Section::insert (Symbol s)
{
	symbols.append (s);
	
	// do pointer math after append call
	// reallocation might move the data
	return symbols.ptr + symbols.count - 1;
}



Symbol Section::lookup (char** name, int* err)
{
	Symbol res = {0};
	Section* scope = this;
	
	char** loc = name;
	
	res = scope->get (*loc);

	if (no res.symbol)
		res = program->global.get (*loc);

	if (no res.symbol)
	{
		fprintf (stderr, "Can't find %s in %s\n", *loc, this->name);
		return res;
	}
	
	loc++; // already tested the first name above
	
	for (; *loc != 0; ++loc)
	{
		res = scope->get (*loc);
		if (no res.symbol)
		{
			fprintf (stderr, "%s is a %s and does not store data\n", *loc, this->name);
			return res;
		}
		
		int t = res->symbol;
		
		if (t == Symbol::SECTION)
		{
			if (res->section->isStatic)
				scope = res->section;
			else
			{
				fprintf (stderr, "%s is a type, not a variable\n", *loc);
				*err = 2;
				return res;
			}
		}
		else
		if (t == Symbol::VAR)
		{
			if (not isBuiltin (res->type.id))
			
		}
		else
		if (t == Symbol::VALUE)
		
		if (not (t == Symbol::VAR || t == Symbol::VALUE
		    || t == Symbol::SECTION))
		{
			*err = 2;
			fprintf (stderr, "%s does not store data\n", *loc);
			return res;
		}
		
		
	}
	
notfound:
	*err = 1;
	fprintf (stderr, "Unknown name '%s' in '%s'\n", *loc, this->name);
	return res;
}

Symbol getSymbol (Section* scope)
{
	Symbol s;
	
	Word w;
	w = getword ();
	
	
	if (scope != 0x0)	scope->insert(s);
	return s;
}

Section getSection (Section* parent = 0)
{
	Section* sc = malloc (sizeof (Section));
	{ Section tmp = {0}; *sc = tmp; } // zero-init
	
	Symbol s = {0};
	
	// populate the symbol tree one by one
	// this may branch into recursive calls
	while (s.symbol != Symbol::END)
	{	
		s = getSymbol (& sc);
	}
	
	if (parent != 0)
	{
		Symbol* parent->insert (*sc);
		free (sc);
	
	return sc;
}

Program getProgram ()
{
	Program res = {};
	program = & res;
	
	res.global = getSection();
	
	global = &res.global;
	
	res.global = getSection();
	res.name = res.global.name;
	
	return res;
}


/*

int getName (Name* nameRef, Word* first = 0x0)
{
	Word w;
	if (first) w = *first;
	else w = getword();
	
	arr<Word> buf;
	
	
}

int getTuple (Word open_paren, Type** tp)
{
	Word w;
	
	arr <Type> sig;
	sig.allocate (2);

	w = getword();
	
	nexttype:
	
	if (w.id != ')') goto nexttype;
	
	if (tp) *tp = sig.ptr;
	
	return 0;
}

int getType (Type* tp)
{
	Word w = getword();
	
	arr<Type> sig;
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

int getTypeid (Type* fields, Type* aType)
{
	aType->id = 0;
	const Type zerotype = {0};
	
	// if not present, add to type list
	// and store its index
	if (!typetable.find (fields, zerotype, aType))
	{
		aType->id = types.count + Types::tpOff;
		typetable.insert (fields, zerotype, *aType);
		types.append (fields);
	}
	
	return 0;
}

int getFields (Type t, Type** aFields)
{
	Type::ID idx = t.id - Types::tpOff;  // indices start at 0,
	                                // but values start after integral enum constants
	*aFields = types [idx];
	
	return 0;
}

*/
