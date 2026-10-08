/*
 * File: rstrip.c
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
/* Strips the trailing characters from a string 					*/
/********************************************************************/


/* Check for null pointer			DONE and TESTED DFB 20120202 */

/* pChar == 0						DONE and TESTED DFB 20120202 */
	/* Some trailing chars			DONE and TESTED DFB 20120202 */
	/* No trailing chars		 	DONE and TESTED DFB 20120202 */
	/* All trailing chars			DONE and TESTED DFB 20120202 */

/* pChar != 0						DONE and TESTED DFB 20120202 */
	/* Some trailing chars			DONE and TESTED DFB 20120202 */
	/* No trailing chars		 	DONE and TESTED DFB 20120202 */
	/* All trailing chars			DONE and TESTED DFB 20120202 */


unsigned long rstrip(unsigned long pString, unsigned long pChars)
{
	
	
/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( pString == 0 ){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Remove trailing characters					*/
/************************************************/

UDINT	StringLength, NumTrailing, CopyLength;
DINT	i;

StringLength=	strlen((char*)pString);
NumTrailing=	0;


STRING	TempString[StringLength+1];


/* Get NumTrailing; if pChars is null, default to strip whitespace */

for(i=(StringLength - 1); i>=0; i--){

	
	if( pChars == 0 ){

		if( strspn( (char*)(pString + i), " \t\r\n\v\f" ) == 0 ){
			break;
		}
		
	}
	
	else{

		if( strspn( (char*)(pString + i), (char*)pChars ) == 0 ){
			break;
		}
		
	}
	
	
} // Loop to find NumTrailing //


NumTrailing=	(StringLength - 1) - i;
	

/* Get CopyLength */

if( NumTrailing <= StringLength ){

	CopyLength=	StringLength - NumTrailing;

}
else{

	return 0;

}


/* Copy meat of string to TempString and back to pString (because of overlap) */

memset( TempString, 0, sizeof(TempString) );
strncpy( TempString, (char*)pString, CopyLength );

strcpy( (char*)pString, TempString);


return CopyLength;
	

} // End Fn //
