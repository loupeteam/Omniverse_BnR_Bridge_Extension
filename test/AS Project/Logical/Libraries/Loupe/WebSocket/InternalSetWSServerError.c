/*
 * File: InternalSetWSServerError.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of WebSocket, licensed under the MIT License.
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "WebSocket.h"
#include <string.h>
#include <ctype.h> // For isspace
#include "internal.h"
	
#ifdef __cplusplus
	};
#endif

/************************************************************************/
/* Internal: Set error status on a jsonWebSocketServer FUB instance 	*/
/************************************************************************/

char *skip(char *in) {while (in && isspace(*in)) in++; return in;}

unsigned short internalSetWSConnectionError(struct WSConnectionManager_typ* t, signed long errorID, char* errorString)
{

	if( t == 0 ) return WS_ERR_INVALID_INPUT;
	
	t->out.error = 1;
	t->out.errorID = errorID;
	
	if(errorString) {
		stringlcpy(t->out.errorString, (UDINT)errorString, sizeof(t->out.errorString));
	}
	else {
		internalGetErrorMsg((char*)&t->out.errorString, errorID, sizeof(t->out.errorString));
	}
	
	return 0;

}

unsigned short internalSetWSStreamError(struct WSStream_typ* t, signed long errorID, char* errorString)
{
	if( t == 0 ) return WS_ERR_INVALID_INPUT;
	
	t->out.error = 1;
	t->out.errorID = errorID;
	
	if(errorString) {
		stringlcpy(t->out.errorString, errorString, sizeof(t->out.errorString));
	}
	else {
		internalGetErrorMsg((char*)&t->out.errorString, errorID, sizeof(t->out.errorString));
	}
	
	return 0;
	
}

unsigned short internalGetErrorMsg(char* errorString, signed long errorID, unsigned long errorStringSize)
{
	// No arg checks because this is an internal function
	switch( errorID ){
		// It is developers res
		case WS_ERR_INVALID_INPUT: stringlcpy((UDINT)errorString, (UDINT)&"Invalid FUB inputs.", errorStringSize); break;
		case WS_ERR_NOT_IMPLEMENTED: stringlcpy((UDINT)errorString, (UDINT)&"Feature not yet implemented.", errorStringSize); break;
		case WS_ERR_PAYLOAD_LENGTH : stringlcpy((UDINT)errorString, (UDINT)&"Invalid payload length.", errorStringSize); break;
		case WS_ERR_MEM_ALLOC: stringlcpy((UDINT)errorString, (UDINT)&"Error allocating memory for buffers. Check BufferSize.", errorStringSize); break;
		case WS_ERR_NO_LICENSE: stringlcpy((UDINT)errorString, (UDINT)&"No license located on TG.", errorStringSize); break;
		case WS_ERR_KEY_NOT_FOUND: stringlcpy((UDINT)errorString, (UDINT)&"WebSocket key not found. Invalid upgrade request.", errorStringSize); break;
		case WS_ERR_PARTIAL_HTTP_MESSAGE: stringlcpy((UDINT)errorString, (UDINT)&"Partial HTTP message received.", errorStringSize); break;
		case WS_ERR_INVALID_HTTP_MESSAGE: stringlcpy((UDINT)errorString, (UDINT)&"Invalid HTTP message received.", errorStringSize); break;
		case WS_ERR_BUFFER_FULL: stringlcpy((UDINT)errorString, (UDINT)&"Buffer full.", errorStringSize); break;
		
		default: stringlcpy((UDINT)errorString, (UDINT)&"Unknown error.", errorStringSize); break;
		
	} // switch(ErrorID) //
	
	return 0;
}
