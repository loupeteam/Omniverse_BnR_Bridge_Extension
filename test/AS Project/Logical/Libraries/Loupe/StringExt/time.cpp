/*
 * File: time.cpp
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

//TODO: These time functions don't seem to be supported in AS6
//		1. Figure out if we can use them
//		2. Check if AS4 somehow, since they seem to at least build there
#ifndef __arm__	
#define __XSI_VISIBLE 1
#endif	
#include "time.h"


unsigned long stringfTime(unsigned long dest, unsigned long destSize, unsigned long format, DATE_AND_TIME time) {
	if(!dest || !destSize || !format) return 0; // Return if null pointers or if destSize is 0
	tm t = {};
#ifndef __arm__	
	gmtime_r((time_t*)&time, &t);
#endif	
	return strftime((char*)dest, destSize, (char*)format, &t);
}

DATE_AND_TIME stringpTime(unsigned long src, unsigned long format) {
	if(!src || !format) return 0; // Return if null pointers
	tm t = {};
#ifndef __arm__	
	strptime((char*)src, (char*)format, &t);
#endif	
	return mktime(&t);
}
