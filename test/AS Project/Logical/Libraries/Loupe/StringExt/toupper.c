/*
 * File: toupper.c
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

// Converts a string to upper case

// DONE and TESTED DFB 20150715

unsigned long ToUpper (unsigned long pString)
{

	if(pString == 0) return 0;
	
	for(; *(char*)pString; pString++)
		if(('a' <= *(char*)pString) && (*(char*)pString <= 'z'))
			*(char*)pString = 'A' + (*(char*)pString - 'a');

	return 0;
	
}
