#ifndef INTS_H
#define INTS_H

#include <stdint.h>
#include <limits.h>

#define ____(a, b) a ## b
#define ___(a, b) ____(a, b)

#define fastN(N) ___(___(int_fast, N),_t)
#define leastN(N) ___(___(int_least, N),_t)

#define fastN_t(N) typedef fastN(N) fast ## N;
#define leastN_t(N) typedef leastN(N) least ## N;

#define fast_t() typedef ___(___(int_fast, WORD_BIT),_t) fast;
#define least_t() typedef ___(___(int_least, WORD_BIT),_t) least;

fastN_t(8) fastN_t(16) fastN_t(32) fastN_t(64)
leastN_t(8) leastN_t(16) leastN_t(32) leastN_t(64)

fast_t()
least_t()

#undef fastN
#undef leastN
#undef fastN_t
#undef leastN_t
#undef fast_t
#undef least_t

#define until(...) while( ! (__VA_ARGS__) )

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8 byte;

#endif
