/*
 * File: WSConnect.c
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
//#include "jsonAux.h"
#include "sha1.h"
#include "internal.h"

#ifdef __cplusplus
	};
#endif

//*******************************************************************
// Negotiate a new WebSocket connection								*
//*******************************************************************

void wsConnect(struct wsConnect* t)
{

	// Check inputs
	if (t == 0) return;
	if (t->pInputMessage == 0 || t->pOutputMessage == 0 || t->outputMessageSize < WS_STRLEN_CONNECTMESSAGE+1) {
		t->status = WS_ERR_INVALID_INPUT;
		return;
	}
	
	// Internal variables
	char *pKey;
	UDINT msgLen = strlen(t->pInputMessage);
	STRING accept[320];
	SHA_CTX sha;
	USINT digest[20];
	
	// TODO: Maybe check if this is even a HTTP message
	if((msgLen >= 3 && memcmp((void*)t->pInputMessage, "GET", 3) != 0)
	|| msgLen < 3 && memcmp((void*)t->pInputMessage, "GET", msgLen) != 0) {
		t->status = WS_ERR_INVALID_HTTP_MESSAGE;
		return;
	}
	
	// Check for end of HTTP header
	pKey = strstr((char*)t->pInputMessage, "\r\n\r\n");
	if (pKey == 0) {
		t->status = WS_ERR_PARTIAL_HTTP_MESSAGE;
		return;
	}
	
	// Get key out of receive data
	pKey = strstr((char*)t->pInputMessage, "Sec-WebSocket-Key:");
	if (pKey == 0) {
		t->status = WS_ERR_KEY_NOT_FOUND;
		return;
	}
	pKey = pKey + WS_STRLEN_KEYHEADER; // TODO: this const should be derived from above
	pKey = skip(pKey);
	pKey = strtok(pKey, "\r\n");
	if (pKey == 0) {
		t->status = WS_ERR_KEY_NOT_FOUND;
		return;
	}
	
	// Generate accept from key
	strcpy(accept, pKey); // start with key
	strcat(accept, WS_SOC_MAGICSTRING); // cat magic string
				
	// Compute SHA-1 digest
	SHAInit(&sha);
	SHAUpdate(&sha, (unsigned char*)accept, strlen(accept) );
	SHAFinal(digest, &sha);
				
	// Take base64-encoding of digest 
	memset(accept, 0, sizeof(accept)); // bug in httpEncodeBase64() requires that you clear accept first

	t->internal.encodeBase64.enable = 1;
	t->internal.encodeBase64.pSrc = (UDINT)&digest;
	t->internal.encodeBase64.srcLen = sizeof(digest);
	t->internal.encodeBase64.pDest = (UDINT)&accept;
	t->internal.encodeBase64.destSize = sizeof(accept);
				
	httpEncodeBase64(&t->internal.encodeBase64);
	
	if (t->internal.encodeBase64.status != 0) {
		t->status = t->internal.encodeBase64.status;
		return;
	}
		
	// Have accept. Generate response.
	strcpy((char*)t->pOutputMessage, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ");
	strcat((char*)t->pOutputMessage, accept);
	strcat((char*)t->pOutputMessage, "\r\n\r\n");
				
	t->outputMessageLength= strlen((char*)t->pOutputMessage);
	t->status = 0;
	
} // End Fn
