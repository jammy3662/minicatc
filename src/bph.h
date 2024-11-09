#ifndef BPH_DOT_H
#define BPH_DOT_H

#include "ints.h"

// binary partition heap
// smallest partition based on word alignment
template <fast word_size = 4>
class BPH
{
	least8* buffer;
	fast nbytes;
	void*	root;
	
	friend struct Bin;
	
	public:
	void construct (fast byteTotal);
	
	template <typename T>
	fast request (fast count);
	
	template <typename T>
	T& access (fast ptr);
	
	template <typename T>
	void release (fast ptr);
};

#endif
