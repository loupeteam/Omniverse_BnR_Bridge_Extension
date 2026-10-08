// Configure Macros for gdtoaimp
// We should use autoconf.sh to generate this file but I could not get it to work
// Kept saying autoconf v0.0.0 is less than 2.5.0
// Instead I was able to manually determine these by running arithchk.c in ARSim

#ifndef ARITH_H_INCLUDED
#define ARITH_H_INCLUDED

#include "../internal.h"

#define IEEE_8087
#define Arith_Kind_ASL 1

// initial private memory pool size - when exceeded, malloc
// default is 2304 bytes
// max required memory for a single thread of dtoa is 7400 bytes
// we choose 16384 bytes because it is the nearest power of 2 greater than 2x the max required memory for a single thread of dtoa
// this allows 2 threads of dtoa without need for mallocs
#define PRIVATE_MEM 16384

#define ENABLE_MULTITHREAD
#ifdef ENABLE_MULTITHREAD

#define MULTIPLE_THREADS 


// We likely want to use tlsf here but I dont see how as we can define FREE currently
// Will the heap_redirect take care of this for us ?
//#define MALLOC tlsf_malloc
//#define FREE tlsf_free

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* ARITH_H_INCLUDED */
