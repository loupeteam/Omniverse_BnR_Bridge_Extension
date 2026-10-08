/*
 * File: stringConvert.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of StringExt, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "StringExt.h"
#ifdef __cplusplus
	};
#endif

#include "string.h"

/*
 * 
 */
unsigned long string2wstring(unsigned long pDest, const unsigned long pSrc, unsigned long dSize)
{
	char *dst = (char*) pDest;
	char *src = (char*) pSrc;
	char *d = dst;
	const char *s = src;
	unsigned long n = dSize;

	n = dSize;

	if ((n % 2) == 1)
		n--; // n must be an even number
	if (n == 0)
		return(strlen(s));
	while (*s != '\0') {
		if (n != 2) {
			*d++ = *s;
			*d++ = '\0';
			n -= 2;
		}
		s++;
	}
	*d++ = '\0';
	*d = '\0';

	return(s - src);	/* count does not include NUL */
}

unsigned long wstring2string(unsigned long pDest, const unsigned long pSrc, unsigned long dSize)
{
	char *dst = (char*) pDest;
	char *src = (char*) pSrc;
	char *d = dst;
	const UINT *s = (UINT*)src;
	unsigned long n = dSize;
	
	n = dSize;
	
	if (n == 0)
		return(brwcslen((UINT*)src));
	while (*s != '\0') {
		if (n != 1) {
			if(*s > 0xFF) {
				// Character not supported by Strings
				*d++ = STREXT_INVALID_CHAR;
			}
			else {
				*d++ = *((char*)s);
			}
			n--;
		}
		s++;
	}
	*d = '\0';

	return(((UDINT)s - (UDINT)src)/2);	/* count does not include NUL */
}

unsigned short char2wchar(unsigned char c) {
	return ((unsigned short)c);
}

unsigned char wchar2char(unsigned short c) {
	if(c > 0xFF) {
		// Character not supported by Strings
		return STREXT_INVALID_CHAR;
	}
	else {
		return ((char)c);
	}
}
