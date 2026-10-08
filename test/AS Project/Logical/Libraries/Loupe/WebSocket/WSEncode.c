/*
 * File: WSEncode.c
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
//#include <stdio.h>
//#include "jsonAux.h"
#include "sha1.h"

#ifdef __cplusplus
	};
#endif

//srand48(clock_ms()); // Seed random numbers

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
// Encode data into a WebSocket frame 								*
//*******************************************************************

void wsEncode(struct wsEncode* t)
{

	if (t == 0) return;
	if (t->pFrame == 0) {
		t->status = WS_ERR_INVALID_INPUT;
		return;
	}

	// Get local copy of pFrame
	USINT *pFrame = (USINT*)t->pFrame;
	
	// FIN, RSV, opcode
	if ( (t->rsv > 7) || (t->opCode > 15) ) {
		t->status = WS_ERR_INVALID_INPUT;
		return;
	}

	*pFrame = ((USINT)t->fin)*128 + t->rsv*64 + t->opCode;
	pFrame++;
			


	// Payload Length
	USINT i;
	
	if (t->payloadLength < 126) {
		*pFrame = (USINT)t->payloadLength + t->mask*128; // MASK = 0, payload len  = payloadLength
		pFrame++;
	} else if (t->payloadLength < 65536) {
		*pFrame = 126 + t->mask*128; // MASK = 0, payload len  = 126, use 2 byte extended payload length
		pFrame++;
		// byte swap
		for(i=0; i<2; i++){
			memcpy(pFrame + i, (USINT*)&t->payloadLength + (1-i), 1);
		}
		pFrame += 2;
	} else {
		*pFrame = 127 + t->mask*128; // MASK = 0, payload len  = 127, use 8 byte extended payload length
		pFrame++;
				
		// Clear top 4 bytes then byte swap
		memset(pFrame, 0, 4);
		pFrame += 4;
				
		for(i=0; i<4; i++){
			memcpy(pFrame + i, (USINT*)&t->payloadLength + (3-i), 1);
		}
		pFrame += 4;		
	}
	
	// Payload
	t->frameLength = ((UDINT)pFrame - t->pFrame) + t->payloadLength + (((USINT)t->mask)*4);
	
	if (t->frameLength <= t->frameSize) {
		if(t->mask) {
			// Generate key and mask payload
			t->internal.mask.src = t->pPayload;
			t->internal.mask.dest = pFrame+4;
			t->internal.mask.srcLength = t->payloadLength;
			t->internal.mask.destSize = t->frameSize; // This is not accurate but it is handled above so we are good 
			wsMask(&t->internal.mask);
			
			// Write masking key
			memcpy(pFrame, t->internal.mask.maskingKey, sizeof(t->internal.mask.maskingKey));
			pFrame += 4;
			
			// Check errors
			t->status = t->internal.mask.status;

		}
		else {
			// Write payload
			memcpy(pFrame, (char*)t->pPayload, t->payloadLength);
		}
		t->status = 0;
	} else {
		t->status = WS_ERR_PAYLOAD_LENGTH;
		return;
	}
	
} // End Fn
