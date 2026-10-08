/*
 * File: jsonInternalSetReadNoCacheError.c
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
// Internal: Set error status on a jsonReadVariableNoCache FUB instance	*
//***********************************************************************

unsigned short jsonInternalSetReadNoCacheError(unsigned short ErrorID, struct jsonReadVariableNoCache* t)
{

	if( t == 0 ) return JSON_ERR_INVALIDINPUT;
	
	// Set FUB outputs
	t->Status = ErrorID;
	t->pJSONObject = 0;
	t->JSONObjectLength = 0;
	
	// Clear internal structure data
	t->internal.iStructLevel = 0;
	memset( &(t->internal.structLevel), 0, sizeof(t->internal.structLevel) );
	
	return 0;

} // End Fn
