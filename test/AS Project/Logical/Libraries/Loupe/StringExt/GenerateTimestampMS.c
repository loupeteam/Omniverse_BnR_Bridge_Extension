/*
 * File: GenerateTimestampMS.c
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


//-----------------------------------------------------------------------------
// Generate a time stamp string from a DTStructure variable
//-----------------------------------------------------------------------------

// Check for invalid inputs			DONE and TESTED SC 20180214
// Generate string					DONE and TESTED SC 20180214

unsigned short GenerateTimestampMS(unsigned long pDTStruct, unsigned long pTimestamp, unsigned long TimestampLength)
{
	
	
/************************************************/
/* Check for invalid inputs						*/
/************************************************/

if( 	(pTimestamp == 0)
	||	(TimestampLength == 0)
	||	(pDTStruct == 0)
	){
		
	return 0;
		
} // Check for null pointer //


/************************************************/
/* Generate String 								*/
/************************************************/

/* 'YYYY-MM-DD HH:MM:SS.SSS' */

STRING			TempString[6];
//DTStructure*	pDTStruct = ((DTStructure*) pDTStructure);


/* Clear String */

strcpy( (char*)pTimestamp, "" );


/* Year */

brsitoa(((DTStructure*) pDTStruct)->year, (UDINT)TempString);

strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, "-", TimestampLength - strlen((char*)pTimestamp) );


/* Month */

brsitoa( ((DTStructure*) pDTStruct)->month, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->month < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, "-", TimestampLength - strlen((char*)pTimestamp) );


/* Day */

brsitoa( ((DTStructure*) pDTStruct)->day, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->day < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, " ", TimestampLength - strlen((char*)pTimestamp) );


/* Hour */

brsitoa( ((DTStructure*) pDTStruct)->hour, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->hour < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, ":", TimestampLength - strlen((char*)pTimestamp) );


/* Minute */

brsitoa( ((DTStructure*) pDTStruct)->minute, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->minute < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, ":", TimestampLength - strlen((char*)pTimestamp) );


/* Second */

brsitoa( ((DTStructure*) pDTStruct)->second, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->second < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );
strncat( (char*)pTimestamp, ".", TimestampLength - strlen((char*)pTimestamp) );

/* MiliSecond */

brsitoa( ((DTStructure*) pDTStruct)->millisec, (UDINT)TempString );

if( ((DTStructure*) pDTStruct)->millisec < 100 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
if( ((DTStructure*) pDTStruct)->millisec < 10 ) strncat( (char*)pTimestamp, "0" , TimestampLength - strlen((char*)pTimestamp) ); 
strncat( (char*)pTimestamp, TempString, TimestampLength - strlen((char*)pTimestamp) );

return strlen( (char*)pTimestamp );


} // End Fn //


//-----------------------------------------------------------------------------
// Generate a time stamp string from a DTStructure - YYYYMMDD_HHMMSS_SSS
//-----------------------------------------------------------------------------

unsigned short GenerateTimestampMS_1(unsigned long pDTStruct, unsigned long pTimestamp, unsigned long TimestampLength)
{
	
	// Check for invalid inputs - tested
	if( (pTimestamp == 0) || (TimestampLength == 0)	|| (pDTStruct == 0)) return 0;

	// 'YYYYMMDD_HHMMSS_SSS' - 19 characters - tested
	STRING tempString[6+1];
	
	// Clear string
	memset((void*)pTimestamp, 0, TimestampLength);

	// Year - YY
	brsitoa(((DTStructure*)pDTStruct)->year, (UDINT)tempString);
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	
	// Month - MM
	brsitoa(((DTStructure*)pDTStruct)->month, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->month < 10) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	
	// Day - DD_
	brsitoa(((DTStructure*)pDTStruct)->day, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->day < 10 ) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, "_", TimestampLength - strlen((char*)pTimestamp));

	// Hour - HH
	brsitoa(((DTStructure*)pDTStruct)->hour, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->hour < 10) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	
	// Minute - MM
	brsitoa(((DTStructure*)pDTStruct)->minute, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->minute < 10) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	
	// Second  - SS_
	brsitoa(((DTStructure*)pDTStruct)->second, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->second < 10) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, "_", TimestampLength - strlen((char*)pTimestamp));

	// MilliSecond - SSS
	brsitoa(((DTStructure*)pDTStruct)->millisec, (UDINT)tempString);
	if (((DTStructure*)pDTStruct)->millisec < 100) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	if (((DTStructure*)pDTStruct)->millisec < 10) strncat((char*)pTimestamp, "0", TimestampLength - strlen((char*)pTimestamp));
	strncat((char*)pTimestamp, tempString, TimestampLength - strlen((char*)pTimestamp));
	
	return strlen((char*)pTimestamp);

} // End Fn //
