/*
 * File: lstrip.c
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
/* Strips the leading characters from a string 						*/
/********************************************************************/


/* Check for null pointer			DONE and TESTED DFB 20120202 */

/* pChar == 0						DONE and TESTED DFB 20120202 */
	/* Some leading chars			DONE and TESTED DFB 20120202 */
	/* No leading chars		 		DONE and TESTED DFB 20120202 */
	/* All leading chars			DONE and TESTED DFB 20120202 */

/* pChar != 0						DONE and TESTED DFB 20120202 */
	/* Some leading chars			DONE and TESTED DFB 20120202 */
	/* No leading chars		 		DONE and TESTED DFB 20120202 */
	/* All leading chars			DONE and TESTED DFB 20120202 */


unsigned long lstrip(unsigned long pString, unsigned long pChars)
{
	
	
/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( pString == 0 ){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Remove leading characters					*/
/************************************************/

UDINT	StringLength, NumLeading, CopyLength;

StringLength=	strlen((char*)pString);
NumLeading=		0;


STRING	TempString[StringLength+1];


/* Get NumLeading; if pChars is null, default to strip whitespace */

if( pChars == 0 ){
	
	NumLeading=	strspn( (char*)pString, " \t\r\n\v\f" );
	
}
else{
	
	NumLeading=	strspn( (char*)pString, (char*)pChars );
	
}


/* Get CopyLength */

if( NumLeading <= StringLength ){

	CopyLength=	StringLength - NumLeading;

}
else{

	return 0;

}


/* Copy meat of string to TempString and back to pString (because of overlap) */

memset( TempString, 0, sizeof(TempString) );
strncpy( TempString, (char*)(pString+NumLeading), CopyLength );

strcpy( (char*)pString, TempString);


return CopyLength;
	

} // End Fn //
