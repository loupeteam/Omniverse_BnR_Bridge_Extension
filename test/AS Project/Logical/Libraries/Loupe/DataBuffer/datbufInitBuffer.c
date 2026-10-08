/*
 * File: datbufInitBuffer.c
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
/* Initialize a data buffer 										*/
/********************************************************************/

// Check inputs			DONE and TESTED DFB 20140624
// Clear junk			DONE and TESTED DFB 20140624
// Allocate memory		DONE and TESTED DFB 20140624
	// Good				DONE and TESTED DFB 20140624
	// Bad				DONE and TESTED DFB 20140624
// Set maxLength		DONE and TESTED DFB 20140624
// ClearBuffer			DONE and TESTED DFB 20140624


unsigned short datbufInitBuffer(unsigned long pBuffer, unsigned long maxLength)
{


// Check for invalid inputs	

if( (pBuffer == 0) || (maxLength == 0) ) return DATBUF_ERR_INVALIDINPUT;


// Initialize buffer

datbufBuffer_typ *buffer = (datbufBuffer_typ*)pBuffer;

memset( buffer, 0, sizeof(datbufBuffer_typ) );

if( TMP_alloc( maxLength, (void **)&(buffer->pData) ) != 0 ) return DATBUF_ERR_MEMALLOC;
	
buffer->maxLength = maxLength;

return datbufClearBuffer(pBuffer);

	
} // End Fn //
