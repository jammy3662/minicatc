#ifndef LIST_DOT_H
#define LIST_DOT_H

#include "ints.h"
#include <vector>

// List that stores data of arbitrary types.
// Inserts and retrieves by byte offset.
// Elements are guaranteed contiguous
struct List
{
	void* buffer; // the raw byte data
	fast bytes; // used bytes of data in buffer
	fast capacity; // total allocated bytes

	// Allocates element of specified type
	// and returns its offset
	template <typename T>
	void* create ();

	// Returns the offset in bytes of the inserted element 
	template <typename T>
	fast insert (T element);
	
	void* get (fast index);
	
	List();
};

// The implementation symbols need to be
// compiled in once, because of the template
#ifndef LIST_DOT_CC_DOT_H
#include "list.cc.h"
#endif

#endif
