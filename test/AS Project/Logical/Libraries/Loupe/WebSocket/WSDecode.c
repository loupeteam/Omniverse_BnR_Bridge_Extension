/*
 * File: WSDecode.c
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
#include "internal.h"
#include "sha1.h"
#include <limits.h>

#ifdef __cplusplus
	};
#endif

// Websocket Frame
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |     Extended payload length continued, if payload len == 127  |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               | Masking-key, if MASK set to 1 |
// +-------------------------------+-------------------------------+
// | Masking-key (continued)       |          Payload Data         |
// +-------------------------------- - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+

//*******************************************************************
// Decode a WebSocket frame											*
//*******************************************************************

void wsDecode(struct wsDecode* t)
{
	
	if (t == 0) return;
	if (t->pFrame == 0) {
		t->status = WS_ERR_INVALID_INPUT;
		return;
	}
	
	// Get local copy of pFrame
	USINT *pFrame = (USINT*)t->pFrame;
	
	// FIN, RSV, opcode
	t->fin = (*pFrame & 0x80) >> 7;
	t->rsv = (*pFrame & 0x70) >> 4;
	t->opCode = (*pFrame & 0x0f);

	if( t->frameLength < 1 ) {
		// Partial frame
		// TODO: handle with error or output bit
		return;
	}
	
	pFrame++;
	
	// Mask
	t->mask = (*pFrame & 0x80) >> 7;
	
	// Payload Length
	unsigned long long ulPayloadLength = 0;
	UINT uiPayloadLength = 0;
	USINT usPayloadLength = (*pFrame & 0x7f);
	USINT i;
	
	t->status = 0; // This will be set if there is an issue
	
	if( usPayloadLength == 127 ){
		
		if(t->frameLength < (10 + (t->mask * 4))) {
			// Partial frame
			// TODO: handle with error or output bit
			t->partialHeader = 1;
			t->partialFrame = 1;
			return;
		}
		
		// 8 byte extended payload length
		// byte swap!
		pFrame++;
		for(i=0; i<8; i++){
			memcpy((USINT*)&ulPayloadLength + (7-i), pFrame + i, 1);
		}
		if (ulPayloadLength > ULONG_MAX) { // TODO: Confirm ULong is correct for this case (should be a UDINT)
			// TODO: This technically could be a valid payload length, just a number we can't support
			t->status = WS_ERR_PAYLOAD_LENGTH;
			return;
		}
		t->payloadLength = (UDINT)ulPayloadLength;
		pFrame += 8;
	}
	else if( usPayloadLength == 126 ){
		
		if(t->frameLength < (4 + (t->mask * 4))) {
			// Partial frame
			// TODO: handle with error or output bit
			t->partialHeader = 1;
			t->partialFrame = 1;
			return;
		}
		
		// 2 byte extended payload length
		// byte swap!
		pFrame++;
		for(i=0; i<2; i++){
			memcpy((USINT*)&uiPayloadLength + (1-i), pFrame + i, 1);
		}
		t->payloadLength = (UDINT)uiPayloadLength;
		pFrame += 2;
	}
	else{
		if(t->frameLength < (2 + (t->mask * 4))) {
			// Partial frame
			// TODO: handle with error or output bit
			t->partialHeader = 1;
			t->partialFrame = 1;
			return;
		}
		
		// no extended payload length
		pFrame++;
		t->payloadLength = (UDINT)usPayloadLength;
	}
			
	// Masking key
	if(t->mask) {
		memcpy(&(t->maskingKey[0]), pFrame, 4);
		pFrame += 4;
		
		// TODO: If we have a partial frame we are copying junk into the dest
		// TODO: Maybe copy what we can and then have some sort of reentrance
		
		// Copy Payload
		t->internal.mask.dest = t->pPayload;
		t->internal.mask.destSize = t->payloadSize;
		memcpy(&t->internal.mask.maskingKey, &t->maskingKey[0], sizeof(t->maskingKey));
		t->internal.mask.src = pFrame;
		t->internal.mask.srcLength = t->payloadLength;
		wsMask(&t->internal.mask);
		t->status = t->internal.mask.status;
		
	}
	else {
		// Copy Payload
		if(t->payloadSize < t->payloadLength || t->pPayload == 0) {
			// TODO: Maybe copy what we can and then have some sort of reentrance
			t->status = WS_ERR_BUFFER_FULL;
		}
		else {
			// TODO: If we have a partial frame we are copying junk into the dest
			memcpy(t->pPayload, pFrame, t->payloadLength);
		}
	}
	
	// pPayload data
	t->pPayload = (UDINT)pFrame;
	
	// Header length
	t->headerLength = t->pPayload - t->pFrame;
	
	// Frame length
	t->decodeLength = t->headerLength + t->payloadLength;
	
	// Set partial status
	t->partialFrame = t->decodeLength > t->frameLength;
	t->partialHeader = 0;
	

	
} // End Fn
