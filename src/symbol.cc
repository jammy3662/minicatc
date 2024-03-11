#include "symbol.h"

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
			enums.insert (s.enm->name, *s.enm);
			break;
		
		case Symbol::VAR:
			vars.insert (s.var.name, s.var);
			break;
		
		case Symbol::VALUE:
			expressions.insert (s.value.name, s.value);
			break;
		
		case Symbol::FUNC:
			functions.insert (s.function->name, *s.function);
			break;
		
		case Symbol::SECTION:
			sections.insert (s.section->name, s.section);
			break;
	}
}

Symbol getSymbol (Section* scope)
{
	Symbol s;
	
	if (scope != 0x0)	scope->insert(s);
	return s;
}

Program getProgram ()
{
	Program ret;
	
	// there is no outer scope at the global level
	Symbol s = {0};
	
	while (s.symbol != Symbol::END)
	{
		s = getSymbol (& ret.global);
	}
	
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
