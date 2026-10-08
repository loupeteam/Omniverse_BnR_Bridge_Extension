/*
 * File: varGetInfo.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of VarTools, licensed under the MIT License.
 * 
 */

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

	#include "varToolsInternal.h"
	#include "VarTools.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>


//*******************************************************************
// Get variable information 										*
//*******************************************************************

// Check for null pointers		DONE and TESTED DFB 20140612

// Clear old junk				DONE and TESTED DFB 20140612

// Get Address					DONE and TESTED DFB 20140612
	// No error					DONE and TESTED DFB 20140612
	// Error					DONE and TESTED DFB 20140612

// Get Info						DONE and TESTED DFB 20140612
	// Check for enums			DONE and TESTED DFB 20140612
	// No error					DONE and TESTED DFB 20140612
	// Error					DONE and TESTED DFB 20140612


unsigned short varGetInfo(unsigned long pVariable)
{


//***********************************************
// Check for invalid inputs						*
//***********************************************

if( pVariable == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;


//***********************************************
// Get variable info							*
//***********************************************

varVariable_typ *ipVariable = (varVariable_typ*)pVariable;
UINT	status;
UDINT	dataLength;
STRING	elementName[VAR_STRLEN_NAME+1];


// Clear information to get rid of potential junk

memset( &(ipVariable->value), 0, sizeof(*ipVariable) - sizeof(ipVariable->name) );


// Get address

status=	PV_xgetadr(	ipVariable->name, 
					&(ipVariable->address), 
					&dataLength );


if( status == 0 ){

	
	// Have address. Get other info
	
	status=	PV_ninfo(	ipVariable->name,
						&(ipVariable->dataType),
						&(ipVariable->length),
						&(ipVariable->dimension) );
						
	if( status == 0 ){
		
	
		// Check for enums. If the structure has no elements, it is an enum, 
		// and should be treated as a DINT
		
		if( ipVariable->dataType == VAR_TYPE_STRUCT ){
			
			status=	PV_item( ipVariable->name, 0, elementName );
			if( status == VAR_ERR_PVITEM_ENUM )	ipVariable->dataType=	VAR_TYPE_DINT;
			
		} // if struct //
		
		
		return 0;
	
		
	} // Found var info //
	
	else{
	
		
		// Error getting variable information
		
		return status;
	
		
	} // Error getting var info //
	
} // Found address //

else {

	// Error getting address
	// Assume the variable is undefined
	ipVariable->dataType = VAR_TYPE_UNDEFINED;
	strcpy(ipVariable->value, "undefined");
	// Set a dummy address so we don't try to get info repeatedly
	ipVariable->address = (UDINT) &(ipVariable->value);
	return status;

}

} // End Fn
