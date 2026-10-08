/*
 * File: atoui.c
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
/* Converts a string to a UDINT value								*/
/* This allows values greater than the DINT maximum					*/
/********************************************************************/


/* Check for null pointer		DONE and TESTED DFB 20120202 */

/* Convert string				DONE and TESTED DFB 20120202 */
	/* Ignore whitespace		DONE and TESTED DFB 20120202 */
	/* Valid character			DONE and TESTED DFB 20120202 */
	/* Invalid character		DONE and TESTED DFB 20120202 */
	
/* Check range					DONE and TESTED DFB 20120202 */
	

unsigned long atoui(unsigned long pString)
{


/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( pString == 0 ){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Convert string to a UDINT					*/
/************************************************/

UDINT	i;

unsigned long long	Value;


Value=	0;


/* Loop through string, skipping white space and stopping when you encounter a non-numeric character */

for(	i = strspn( (char*)pString, " \t\r\n\v\f" ); 
		( *(char*)(pString + i) >= '0' ) && ( *(char*)(pString + i) <= '9' );
		i++
		){
	
	Value=	Value * 10 + ( *(char*)(pString + i) - '0' );
	
}


/* Check for overflow */

if( Value > STREXT_MAX_UDINT ){
	
	return STREXT_MAX_UDINT;
	
}
else{
	
	return (UDINT)Value;
	
}


} // End Fn //
