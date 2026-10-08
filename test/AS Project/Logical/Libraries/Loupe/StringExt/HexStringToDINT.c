/*
 * File: HexStringToDINT.c
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
	#include <stdlib.h>

#ifdef __cplusplus
	};
#endif

//---------------------------------
// Converts a Hex String to a DINT
//---------------------------------

signed long HexStringToDINT(unsigned long pHexStr)
{
	return strtol((char*)pHexStr, 0, 16);
}
