#include "bph.h"

#include <stdlib.h>

struct Bin
{
	least depth;  
	bool empty;
	Bin *left, *right;
};

template <>
void BPH<>::construct (fast total)
{
	buffer = (least8*) malloc (total);
	nbytes = total;
	
	Bin* bin = (Bin*) malloc (1 * sizeof(Bin));
	bin->depth = 0;
	bin->empty = true;
	
	root = bin;
}

template <>
template <typename T>
fast BPH<>::request (fast count)
{
	fast res;
	
	fast bsize = count * sizeof(T);
	
	return res;
}

template <>
template <typename T>
T& BPH<>::access (fast ptr)
{
	void* item = (void*) (buffer + ptr);
	return *(T*) item;
}
