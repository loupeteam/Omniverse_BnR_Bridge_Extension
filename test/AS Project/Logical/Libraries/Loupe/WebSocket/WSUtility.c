/*
 * File: WSUtility.c
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
#include <stdlib.h>
#include "internal.h"
	
#ifdef __cplusplus
	};
#endif

MaskingKey_typ emptyMaskingKey = {0};

unsigned long wsGetErrorMsg(unsigned long errorString, signed long errorID, unsigned long errorStringSize)
{
	
	if(!errorString) return WS_ERR_INVALID_INPUT;
	
	return internalGetErrorMsg((char*)errorString, errorID, errorStringSize);
}

//*******************************************************************
// Check if mode is a valid 										*
//*******************************************************************

plcbit wsModeIsValid(signed long mode)
{
	switch (mode)
	{
		case WS_MODE_SERVER:
		case WS_MODE_CLIENT:
			return 1;
			break;

		default:
			return 0;
			break;
	}
     
} // End Fn

void wsMask(struct wsMask* t)
{
	if(!t) return;
	
	if(t->src == 0){
		t->status = WS_ERR_INVALID_INPUT;
		return;
	}
	
	if(t->srcLength+1 > t->destSize || t->dest == 0) {
		t->status = WS_ERR_BUFFER_FULL;
		return;
	}
	
	UDINT i;
	
	// If blank masking key, populate
	if(memcmp(&t->maskingKey, &emptyMaskingKey, sizeof(t->maskingKey)) == 0) {
		srand(clock_ms());
		for (i = 0; i < sizeof(t->maskingKey)/sizeof(t->maskingKey[0]); i++) {
			t->maskingKey[i] = (unsigned short)rand(); // Dataloss is acceptable here
		}
	}
	
	// Mask / Unmask data

	char *payloadData = (char*)t->src;
	char *dest = (char*)t->dest;
	for (i = 0; i < t->srcLength; i++) {
		dest[i] = payloadData[i] ^ t->maskingKey[i % 4];
	}
	dest[i] = '\0';
	
	t->status = 0;
	t->destLength = t->srcLength;
}
