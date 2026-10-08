/*
 * File: datbufAppendToBuffer.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of DataBuffer, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "DataBuffer.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>


/********************************************************************/
/* Append data to a data buffer 									*/
/********************************************************************/


// Check null pointer		DONE and TESTED DFB 20140624
// Check initialized		DONE and TESTED DFB 20140624
// Buffer full				DONE not tested
// Buffer not full			DONE not tested


unsigned short datbufAppendToBuffer(unsigned long pBuffer, unsigned long pData, unsigned long dataLength)
{

// Check for invalid inputs

if( (pBuffer == 0) || (pData == 0) ) return DATBUF_ERR_INVALIDINPUT;
datbufBuffer_typ *buffer = (datbufBuffer_typ*)pBuffer;
if( buffer->pData == 0 ) return DATBUF_ERR_NOTINITIALIZED;


// Add data to buffer 

BOOL bufferFull = 0;
UDINT copyLength = 0;

if( (buffer->currentLength + dataLength) > buffer->maxLength ){
	bufferFull = 1;
	copyLength = buffer->maxLength - buffer->currentLength; // CHECK THIS!
}
else{
	bufferFull = 0;
	copyLength = dataLength;
}

memcpy( (void*)(buffer->pData + buffer->currentLength), (void*)pData, copyLength );
buffer->currentLength += copyLength;

if( bufferFull ){
	return DATBUF_ERR_BUFFERFULL;
}
else{
	return 0;
}


} // End Fn //
