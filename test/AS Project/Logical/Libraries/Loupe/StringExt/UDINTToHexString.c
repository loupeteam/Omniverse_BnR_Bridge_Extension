/*
 * File: UDINTToHexString.c
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


/********************************************************************/
/* Format a UDINT as a hexadecimal string							*/
/********************************************************************/

/* Prefix selects the notation, see STREXT_HEXPREFIX_enum:
	
		STREXT_HEXPREFIX_NONE		FF
		STREXT_HEXPREFIX_0X			0xFF
		STREXT_HEXPREFIX_IEC		16#FF
		STREXT_HEXPREFIX_DOLLAR		$FF
	
	NumDigits zero pads the value to that width (1..8). Pass 0 for the
	shortest representation. Pass the width of the source type - 2 for a
	BYTE, 4 for a WORD, 8 for a DWORD - to keep a column of values
	aligned and to show how wide the underlying variable is.
	
	Size is the size of the destination buffer including the terminator.
	Nothing is written unless the whole string fits. */


signed long UDINTToHexString(unsigned long Value, unsigned long pString, unsigned long Size, unsigned char NumDigits, unsigned char Prefix)
{


/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( 	(pString == 0)
	||	(Size == 0)
	){
		
	return STREXT_ERR_INVALID_INPUT;
		
} // Check for null pointer //


/************************************************/
/* Build the string								*/
/************************************************/

char	hexchar[16]=	{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

const char*	pPrefix;

char	Digits[8];

UDINT	PrefixLength, DigitCount, Length, i;

UDINT	Remaining;


switch( Prefix ){
	
	case STREXT_HEXPREFIX_0X:		pPrefix=	"0x";	break;
	case STREXT_HEXPREFIX_IEC:		pPrefix=	"16#";	break;
	case STREXT_HEXPREFIX_DOLLAR:	pPrefix=	"$";	break;
	case STREXT_HEXPREFIX_NONE:		pPrefix=	"";		break;
	
	default: return STREXT_ERR_INVALID_INPUT; break;
	
}

PrefixLength=	strlen( pPrefix );


/* Write the digits least significant first, then reverse them below */

DigitCount=	0;
Remaining=	Value;

do{
	
	Digits[DigitCount]=	hexchar[ Remaining & 0x0F ];
	
	DigitCount++;
	
	Remaining=	Remaining >> 4;
	
} while( (Remaining != 0) && (DigitCount < 8) );


/* Pad out to NumDigits if a fixed width was asked for */

if( NumDigits > 8 ) NumDigits=	8;

while( DigitCount < NumDigits ){
	
	Digits[DigitCount]=	'0';
	
	DigitCount++;
	
}


/* Write nothing at all unless the whole string fits */

Length=	PrefixLength + DigitCount;

if( Length + 1 > Size ) return STREXT_ERR_BUFFER_TOO_SMALL;


memcpy( (char*)pString, pPrefix, PrefixLength );

for(i=0; i<DigitCount; i++){
	
	*(char*)(pString + PrefixLength + i)=	Digits[DigitCount - 1 - i];
	
}

*(char*)(pString + Length)=	'\0';

return (signed long)Length;


} // End Fn //
