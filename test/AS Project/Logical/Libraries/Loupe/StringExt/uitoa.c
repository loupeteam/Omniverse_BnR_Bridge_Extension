/*
 * File: uitoa.c
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
/* Converts a UDINT to a string 									*/
/* This allows values greater than the DINT maximum					*/
/********************************************************************/


/* Check for null pointer		DONE and TESTED DFB 20120202 */

/* 0 input						DONE and TESTED DFB 20120202 */

/* Other input					DONE and TESTED DFB 20120202 */

/* Return value					DONE and TESTED DFB 20120202 */
	

unsigned long uitoa(unsigned long Value, unsigned long pString)
{


/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( pString == 0 ){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Convert UDINT to a string					*/
/************************************************/


/* Handle 0 as an edge case */

if( Value == 0 ){
	
	strcpy( (char*)pString, "0" );
	return 1;
	
}


UDINT	StringLength;
USINT	i;

STRING	TempString[10+1];

memset( TempString, 0, sizeof(TempString) );


/* Step through digits and write string backwards */

for( i = 0 ; Value > 0; i++ ){
	
	TempString[i]=	(Value % 10) + '0';
	Value=	Value / 10;
	
} // Loop through digits //


/* Reverse TempString and copy to pString */

StringLength=	strlen( TempString );

for( i=0; i<StringLength; i++ ){
	
	*(char*)(pString + i)=	TempString[(StringLength - 1) - i];
	
} // Reverse string //


/* Terminate pString and return string length */

*(char*)(pString + StringLength)=	0;

return strlen( (char*)pString );


} // End Fn //
