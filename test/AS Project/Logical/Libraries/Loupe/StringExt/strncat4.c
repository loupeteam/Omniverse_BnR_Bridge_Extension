/*
 * File: strncat4.c
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
/* Combine strings pSource1 through pSource4 into pDest, 			*/
/* with ' / ' between source strings 								*/
/********************************************************************/


/* Check for invalid inputs				DONE and TESTED DFB 20140213 */
/* Cat strings							DONE and TESTED DFB 20140213 */
	/* Check for null or empty string	DONE and TESTED DFB 20140213 */
	/* Check length of strncat			DONE and TESTED DFB 20140213 */
/* Return length of Dest string			DONE and TESTED DFB 20140213 */


unsigned short strncat4(	unsigned long pDest, 
							unsigned long pSource1, 
							unsigned long pSource2, 
							unsigned long pSource3, 
							unsigned long pSource4, 
							unsigned long MaxLength)
{


/************************************/
/* Check for invalid inputs			*/
/************************************/

if(		(pDest == 0)
	||	(MaxLength == 0)
	){
		
	return 0;
	
}


/************************************/
/* Cat strings 						*/
/************************************/

STRING	TempString[MaxLength + 1];

memset(TempString, 0, sizeof(TempString) );


if( pSource1 != 0 ){
	
	if( strcmp((char*)pSource1, "") != 0 ){
		
		strncpy( TempString, (char*)pSource1, MaxLength );
		
	} // not empty string //
	
} // valid pointer //


if( pSource2 != 0 ){
	
	if( strcmp((char*)pSource2, "") != 0 ){
		
		strncat( TempString, " / ", MaxLength - strlen(TempString) );
		strncat( TempString, (char*)pSource2, MaxLength - strlen(TempString) );
		
	} // not empty string //
	
} // valid pointer //


if( pSource3 != 0 ){
	
	if( strcmp((char*)pSource3, "") != 0 ){
		
		strncat( TempString, " / ", MaxLength - strlen(TempString) );
		strncat( TempString, (char*)pSource3, MaxLength - strlen(TempString) );
		
	} // not empty string //
	
} // valid pointer //


if( pSource4 != 0 ){
	
	if( strcmp((char*)pSource4, "") != 0 ){
		
		strncat( TempString, " / ", MaxLength - strlen(TempString) );
		strncat( TempString, (char*)pSource4, MaxLength - strlen(TempString) );
		
	} // not empty string //
	
} // valid pointer //


strcpy( (char*)pDest, TempString );

return strlen((char*)pDest);


} // End Fn //
