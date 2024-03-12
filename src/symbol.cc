#include "symbol.h"

Section* global;

Symbol getSymbol (Section* scope);

void Section::insert (Symbol s)
{
switch (s.symbol)
	{
		case Symbol::HEADER:
			this->name = s.header;
		
		case Symbol::TYPE:
			types.val (s.type.fields);
			break;
		
		case Symbol::TYPEDEF:
			typedefs.insert (s.typenm.name, s.typenm.type);
			break;
		
		case Symbol::ENUM:
			enums.insert (s.enm.name, s.enm);
			break;
		
		case Symbol::VAR:
			vars.insert (s.var.name, s.var);
			break;
		
		case Symbol::VALUE:
			expressions.insert (s.value.name, s.value);
			break;
		
		case Symbol::FUNC:
			functions.insert (s.function.name, s.function);
			break;
		
		case Symbol::SECTION:
			sections.insert (s.section->name, s.section);
			break;
		
		case Symbol::END:
			/* finalize section; nothing to insert */
			break;
		
		default:
			fprintf (stderr, "Found an unknown symbol \t %p \n", &s);
			break;
	}
}

Symbol Section::get (char* name, int* err)
{
	Symbol res;
	
	Section* scope = this;

search:
	Typeid tp = scope->typedefs.find (name, err);
	res.typenm.name = tp.name;
	res.typenm.type = tp;
	if (no *err) return res;
	
	res.enm = scope->enums.find (name, err);
	if (no *err) return res;
	
	res.var = scope->vars.find (name, err);
	if (no *err) return res;
	
	res.value = scope->expressions.find (name, err);
	if (no *err) return res;
	
	res.function = scope->functions.find (name, err);
	if (no *err) return res;
	
	res.section = scope->sections.find (name, err);
	if (no *err) return res;
	
	// look in global namespace if not found locally
	if (scope == this) scope = global;
	goto search;
	
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

Section getSection ()
{
	Section sc = {};
	Symbol s = {};
	
	// populate the symbol tree one by one
	// this may branch into recursive calls
	while (s.symbol != Symbol::END)
	{	
		s = getSymbol (& sc);
	}
	return sc;
}

Program getProgram ()
{
	Program ret;
	global = &ret.global;
	
	ret.global = getSection();
	
	
	
	return ret;
}


/*

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

*/
