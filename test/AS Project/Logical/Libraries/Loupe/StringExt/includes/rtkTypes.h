/* contains legacy type definitions. These types should only be used
 * for calling functions which require these types. */

#ifndef _RTK_TYPES_H_
#define _RTK_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _ULONG_DEFINED_ /* ULONG might also be defined by vxTypesOld.h */
#define _ULONG_DEFINED_
typedef uint32_t ULONG;
#endif

#ifndef _BYTE_DEFINED_ /* BYTE might already be defined from plctypes.h */
#define _BYTE_DEFINED_
typedef uint8_t BYTE;
#endif

#ifndef _WORD_DEFINED_ /* WORD might already be defined from plctypes.h */
#define _WORD_DEFINED_
typedef uint16_t WORD;
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned char BOOLEAN;
typedef char* LPSTRING;
typedef void* LPVOID;
typedef BYTE* LPBYTE;
typedef WORD* LPWORD;
typedef ULONG* LPULONG;

typedef struct
{
	WORD Year;        /* 0 up (for example 1999)		*/
	BYTE Month;       /* 1-12							*/
	BYTE Day;         /* 1-31							*/
	BYTE DayOfWeek;   /* 1=monday, ..., 7=sunday		*/
	BYTE Hour;        /* 0-23							*/
	BYTE Minute;      /* 0-59							*/
	BYTE Second;      /* 0-59							*/
	WORD Millisecond; /* 0-999							*/
	WORD Microsecond; /* 0-999							*/
} RTK_RTC, *LPRTK_RTC;

#ifdef __cplusplus
}
#endif

#endif
