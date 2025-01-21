#ifndef LIST_DOT_CC_DOT_H
#define LIST_DOT_CC_DOT_H

#include "list.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

List::List ()
{
	bytes = 0;
	capacity = 64 * sizeof (fast);
	
	buffer = malloc (capacity);
}

template <typename T>
void* List::create ()
{
	T _;
	return (void*)
	(
		(byte*)(buffer) + insert (_)
	);
}

template <typename T>
fast List::insert (T item)
{
	unsigned long itembytes; // size in bytes of new item
	fast dst; // new item's offset
	
	itembytes = sizeof (item);
	dst = bytes;
	
	bytes += itembytes;
	
	// resize buffer if needed
	if (bytes >= capacity)
	{
		capacity = capacity * 2;
		buffer = realloc (buffer, capacity);
	}
	
	if (buffer == NULL)
	{ fprintf (stderr, "Realloc failed in List::insert (T item)\n"); }
	
	memcpy ((byte*)(buffer)+dst, &item, bytes);
	
	return dst;
}

void* List::get (fast off)
{
	return (void*) ((byte*)(buffer)+off);
}

#endif
