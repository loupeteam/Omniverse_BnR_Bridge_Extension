// Configure Macros for gdtoaimp
// We should use autoconf.sh to generate this file but I could not get it to work
// Kept saying autoconf v0.0.0 is less than 2.5.0
// Instead I was able to manually determine these by running qnan.c in ARSim
// I was able to get the decimal values because %x was not printing correctly. 
// So I wrote the decimal value in a comment above the hex definition

#ifndef QNAN_H_INCLUDED
#define QNAN_H_INCLUDED

// 4290772992
#define f_QNAN 0xFFC00000 
// 0
#define d_QNAN0 0x00000000
// 4294443008
#define d_QNAN1 0xFFF80000

// 0
#define ld_QNAN0 0x00000000
// 3221225472
#define ld_QNAN1 0xC0000000
// 65535
#define ld_QNAN2 0xFFFF
// 0
#define ld_QNAN3 0x00000000

// 0
#define ldus_QNAN0 0x0000
// 0
#define ldus_QNAN1 0x0000
// 0
#define ldus_QNAN2 0x0000
// 49152
#define ldus_QNAN3 0xC000
// 65535
#define ldus_QNAN4 0xFFFF

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* QNAN_H_INCLUDED */
