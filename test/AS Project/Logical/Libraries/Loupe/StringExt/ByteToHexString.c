/*
 * File: ByteToHexString.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of StringExt, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

	#include "StringExt.h"

#ifdef __cplusplus
};
#endif


#include <string.h>


/********************************************************************/
/* Converts bytes to a hex string									*/
/********************************************************************/


/* Check for invalid inputs			DONE and TESTED DFB 20120202 */

/* Convert to string				DONE and TESTED DFB 20120202 */

/* Return value						DONE and TESTED DFB 20120202 */


unsigned long ByteToHexString(unsigned long pByte, unsigned long NumBytes, unsigned long pString)
{


/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( 	(pByte == 0)
	||	(NumBytes == 0)
	||	(pString == 0)
	){
		
	return 0;
		
} // Check for invalid inputs //


/************************************************/
/* Convert bytes to hex string					*/
/************************************************/

char hexchar[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

UDINT	i;


/* Clear string */

memset( (char*)pString, 0, 2*NumBytes+1 );


/* Loop through bytes to write string */

for(i=0; i<NumBytes; i++){

	*(char*)(pString + 2*i)=		hexchar[ *(USINT*)(pByte + i) / 16 ];
	*(char*)(pString + 2*i + 1)=	hexchar[ *(USINT*)(pByte + i) & 0x0F ];

} // Loop through bytes //


return strlen( (char*)pString );


} // End Fn //
