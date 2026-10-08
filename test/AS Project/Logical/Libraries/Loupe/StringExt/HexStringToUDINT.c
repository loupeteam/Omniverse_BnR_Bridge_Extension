/*
 * File: HexStringToUDINT.c
 * Copyright (c) 2026 Loupe
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
#include <stdlib.h>


/********************************************************************/
/* Parse a hexadecimal string into a UDINT							*/
/********************************************************************/

/* Unlike HexStringToDINT(), the full 32-bit range is representable and
	a parse failure is reported separately from the value, so a valid 0
	cannot be confused with bad input.
	
	Accepted prefixes (all optional, leading white space is ignored):
	
		0xFF	0XFF	C style
		16#FF	16#ff	IEC 61131-3 style
		$FF				Pascal style
		FF				bare
	
	Trailing white space is allowed; any other trailing character is an
	error, so a truncated or mistyped value is rejected rather than
	silently parsed up to the bad character. */


signed long HexStringToUDINT(unsigned long pHexStr, unsigned long pValue)
{


/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( 	(pHexStr == 0)
	||	(pValue == 0)
	){
		
	return STREXT_ERR_INVALID_INPUT;
		
} // Check for null pointer //


/************************************************/
/* Parse the string								*/
/************************************************/

const char*	p;
const char*	pEnd;

UDINT	Digits;


p=	(const char*)pHexStr;


/* Skip leading white space */

p+=	strspn( p, " \t\r\n\v\f" );


/* A sign is not valid for a bit string. strtoul() would silently negate
	a negative value, so reject it here. */

if( 	(*p == '-')
	||	(*p == '+')
	){
	
	return STREXT_ERR_INVALID_FORMAT;
	
}


/* Strip the prefix, if there is one */

if( 	(p[0] == '0')
	&&	((p[1] == 'x') || (p[1] == 'X'))
	){
	
	p+=	2;
	
}
else if( 	(p[0] == '1')
		&&	(p[1] == '6')
		&&	(p[2] == '#')
		){
	
	p+=	3;
	
}
else if( p[0] == '$' ){
	
	p+=	1;
	
}


/* Count the hex digits. Note that strtoul() would accept a second "0x"
	here, so the run of digits is measured rather than left to strtoul. */

Digits=	strspn( p, "0123456789abcdefABCDEF" );

if( Digits == 0 ) return STREXT_ERR_INVALID_FORMAT;


/* Anything after the digits other than white space is an error */

pEnd=	p + Digits;

pEnd+=	strspn( pEnd, " \t\r\n\v\f" );

if( *pEnd != '\0' ) return STREXT_ERR_INVALID_FORMAT;


/* Ignore leading zeros so that a zero padded literal such as 16#000000FF
	is not mistaken for an overflow */

while( 	(*p == '0')
	&&	(Digits > 1)
	){
	
	p++;
	Digits--;
	
}


/* More than 8 significant hex digits cannot fit in a UDINT */

if( Digits > 8 ) return STREXT_ERR_RANGE;


*(UDINT*)pValue=	(UDINT)strtoul( p, 0, 16 );

return 0;


} // End Fn //
