/*
 * File: varRefresh.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of VarTools, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "varToolsInternal.h"
	#include "VarTools.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>


//*******************************************************************
// Force refresh of variable info in next getValue call				*
//*******************************************************************


unsigned short varRefresh(unsigned long pVariable)
{

// check for invalid inputs
if( pVariable == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;

varVariable_typ* ipVariable = (varVariable_typ*) pVariable;

// forces varGetInfo on next getValue call for this pv
memset((void*) &ipVariable->address, 0, sizeof(ipVariable->address));

return 0;
	
} // End Fn
