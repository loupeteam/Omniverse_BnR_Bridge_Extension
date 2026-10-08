/*
 * File: jsonInternalSetAddError.c
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

#ifdef __cplusplus
	};
#endif


//***********************************************************************
// Internal: Set error status on a jsonAddVariableToCache FUB instance	*
//***********************************************************************

unsigned short jsonInternalSetAddError(unsigned short ErrorID, struct jsonAddVariableToCache* t)
{
	
	if( t == 0 || t->pCache == 0 ) return JSON_ERR_INVALIDINPUT;
	
	// Set FUB outputs
	t->Status = ErrorID;
	t->iVariable = 65535;
	
	// Clear internal structure data
	t->internal.iStructLevel = 0;
	memset( &(t->internal.structLevel), 0, sizeof(t->internal.structLevel) );
	
	// Undo caching from this call -- NEEDS WORK!
	jsonCache_typ* pCache = (jsonCache_typ*)(t->pCache);
	
	// Reset cache info -- NEEDS WORK!
	
	// Reset variable list info
	if( pCache->iVariable <= JSON_MAI_CACHEVAR ){
		memset( &(pCache->variable[pCache->iVariable]), 0, sizeof(pCache->variable[pCache->iVariable]) );
	}
	
	return 0;

} // End Fn //
