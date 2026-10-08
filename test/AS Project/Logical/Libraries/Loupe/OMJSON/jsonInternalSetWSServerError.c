/*
 * File: jsonInternalSetWSServerError.c
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


/************************************************************************/
/* Internal: Set error status on a jsonWebSocketServer FUB instance 	*/
/************************************************************************/

unsigned short jsonInternalSetWSServerError(unsigned short ErrorID, struct jsonWebSocketServer* t)
{

	if( t == 0 ) return JSON_ERR_INVALIDINPUT;
	
	t->Error = 1;
	t->ErrorID = ErrorID;
	
	switch( ErrorID ){
		
		case JSON_ERR_INVALIDINPUT: strcpy(t->ErrorString, "Invalid FUB inputs."); break;
		case JSON_ERR_MEMALLOC: strcpy(t->ErrorString, "Error allocating memory for buffers. Check BufferSize."); break;
		case JSON_ERR_MAXLEVELREACHED: strcpy(t->ErrorString, "Maximum structure depth reached. Increase JSON_MAI_STRUCTLEVEL."); break;
		case JSON_ERR_PARSE: strcpy(t->ErrorString, "Error parsing request."); break;
		case JSON_ERR_HUH: strcpy(t->ErrorString, "THAT'S NOT POSSIBLE!"); break;
		case JSON_ERR_CACHEFULLITEMS: strcpy(t->ErrorString, "Cache is full of items. Increase JSON_MAI_CACHEITEM."); break;
		case JSON_ERR_CACHEFULLVARS: strcpy(t->ErrorString, "Cache is full of variables. Increase JSON_MAI_CACHEVAR."); break;
		case JSON_ERR_CACHEEMPTY: strcpy(t->ErrorString, "Cache is empty. That doesn't seem right..."); break;
		case JSON_ERR_MAXITERATIONS: strcpy(t->ErrorString, "Maximum iterations reached. Increase MaxIterations input."); break;
		case JSON_ERR_WS_KEYNOTFOUND: strcpy(t->ErrorString, "WS_KEYNOTFOUND"); break;
		case JSON_ERR_WS_FRAGMENT: strcpy(t->ErrorString, "WebSocket fragment received"); break;
		case JSON_ERR_WS_RSV: strcpy(t->ErrorString, "Invalid WebSocket RSV received"); break;
		case JSON_ERR_WS_OPCODE: strcpy(t->ErrorString, "Invalid WebSocket OpCode received"); break;
		case JSON_ERR_WS_MASK: strcpy(t->ErrorString, "Unmasked WebSocket frame received"); break;
		case JSON_ERR_WS_PAYLOADLENGTH: strcpy(t->ErrorString, "WebSocket frame too large"); break;
		case JSON_ERR_TCP_FRAGMENT: strcpy(t->ErrorString, "TCP_FRAGMENT"); break;
		case JSON_ERR_TCP_BAD_PACKET: strcpy(t->ErrorString, "BAD_PACKET"); break;
		case JSON_ERR_NO_LICENSE: strcpy(t->ErrorString, "No TG license detected"); break;
		case JSON_ERR_WS_MAX_CLIENTS: strcpy(t->ErrorString, "Max number of clients reached"); break;
		case JSON_ERR_WS_INVALID_MAX_CLIENTS: strcpy(t->ErrorString, "Invalid input provided for maxClients"); break;
		default: strcpy(t->ErrorString, "Unknown error."); break;
		
	} // switch(ErrorID) //
	
	return 0;

} // End Fn //
