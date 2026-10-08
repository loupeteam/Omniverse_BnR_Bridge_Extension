/*
 * File: jsonInternalWrapper.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of OMJSON, licensed under the MIT License.
 */

#include <bur/plctypes.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "OMJSON.h"
	#include <string.h>
	#include "jsonAux.h"

#ifdef __cplusplus
	};
#endif

//*******************************************************************
// Wrapper for internal json functions								*
//*******************************************************************

unsigned long jsonUnescape(unsigned long pDest, unsigned long pSrc, unsigned long maxLength, unsigned long reentry)
{
	return unescape_string((char*)pDest, (char*)pSrc, maxLength, (char**)reentry);
}

unsigned long jsonEscape(unsigned long pDest, unsigned long pSrc, unsigned long maxLength, unsigned long reentry)
{
	return stringify_string((char*)pDest, (char*)pSrc, maxLength, (char**)reentry);
}
