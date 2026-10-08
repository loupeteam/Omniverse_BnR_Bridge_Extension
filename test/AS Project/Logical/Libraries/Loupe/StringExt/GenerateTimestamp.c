/*
 * File: GenerateTimestamp.c
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
/* Generate a time stamp string from a DATE_AND_TIME variable		*/
/********************************************************************/


/* Check for invalid inputs			DONE and TESTED DFB 20120328 */

/* Generate string					DONE and TESTED DFB 20120328 */


unsigned short GenerateTimestamp(plcdt DT1, unsigned long pTimestamp, unsigned long TimestampLength)
{
	
	
/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( 	(pTimestamp == 0)
	||	(TimestampLength == 0)
	){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Generate String 								*/
/************************************************/

/* 'YYYY-MM-DD HH:MM:SS' */

STRING			TempString[6];
DTStructure		DTStruct;

DT_TO_DTStructure( DT1, (UDINT)&DTStruct );


/* Clear String */

strcpy( (char*)pTimestamp, "" );


/* Year */

brsitoa(DTStruct.year, (UDINT)TempString);

strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, "-", TimestampLength - strlen((char*)pTimestamp) );


/* Month */

brsitoa( DTStruct.month, (UDINT)TempString );

if( DTStruct.month < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, "-", TimestampLength - strlen((char*)pTimestamp) );


/* Day */

brsitoa( DTStruct.day, (UDINT)TempString );

if( DTStruct.day < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, " ", TimestampLength - strlen((char*)pTimestamp) );


/* Hour */

brsitoa( DTStruct.hour, (UDINT)TempString );

if( DTStruct.hour < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, ":", TimestampLength - strlen((char*)pTimestamp) );


/* Minute */

brsitoa( DTStruct.minute, (UDINT)TempString );

if( DTStruct.minute < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, ":", TimestampLength - strlen((char*)pTimestamp) );


/* Second */

brsitoa( DTStruct.second, (UDINT)TempString );

if( DTStruct.second < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );


return strlen( (char*)pTimestamp );


} // End Fn //
