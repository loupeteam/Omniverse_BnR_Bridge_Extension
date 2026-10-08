/*
 * File: varGetValueReal.c
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
// Get the value of a variable as a REAL 							*
//*******************************************************************

unsigned short varGetRealValue(unsigned long pVariable, unsigned long pValue)
{


//***********************************************
// Check for invalid inputs						*
//***********************************************

if( pVariable == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;
if( pValue == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;

//***********************************************
// Check address								*
//***********************************************

varVariable_typ	*ipVariable = (varVariable_typ*)pVariable;
UINT status;

if( ipVariable->address == 0 ){
	
	status = varGetInfo(pVariable);
	
	if(status != 0) return status;
		
} // Get info if necessary //


//*******************************************************
// Get value of variable and return to a user variable  *
//*******************************************************

// Clear the Value string. It will be overwritten below
switch( ipVariable->dataType ){
	
	
	//*******************************************************************************
	// BOOL																			*
	//*******************************************************************************
	
	case VAR_TYPE_BOOL:
		
		if( *(BOOL*)(ipVariable->address) == 0 ){
			
			*(REAL*)pValue = 0;
			
		}
		else if( *(BOOL*)(ipVariable->address) == 1 ){
			
			*(REAL*)pValue = 1;			

		}
		else{
			
			// Invalid BOOL value.
			// Use convention of non-zero == TRUE for now
			*(REAL*)pValue = 1;			

		}
		
		break;
	
	
	//*******************************************************************************
	// Integer types																*
	//*******************************************************************************
	
	case VAR_TYPE_SINT:
		
		*(REAL*)pValue = (REAL)*(SINT*)(ipVariable->address);
		
		break;
	
	
	case VAR_TYPE_INT:
		
		*(REAL*)pValue = (REAL)*(INT*)(ipVariable->address);
				
		break;
	
	
	case VAR_TYPE_DINT:
	case VAR_TYPE_TIME:
		
		*(REAL*)pValue = (REAL)*(DINT*)(ipVariable->address);
				
		break;
	
	
	case VAR_TYPE_BYTE:
	case VAR_TYPE_USINT:
		
		*(REAL*)pValue = (REAL)*(USINT*)(ipVariable->address);
				
		break;
	
	
	case VAR_TYPE_WORD:
	case VAR_TYPE_UINT:
		
		*(REAL*)pValue = (REAL)*(UINT*)(ipVariable->address);
				
		break;
	
	case VAR_TYPE_DWORD:
	case VAR_TYPE_UDINT:
	case VAR_TYPE_DATE_AND_TIME:
		
		*(REAL*)pValue = (REAL)*(UDINT*)(ipVariable->address);
		
		break;
	
	
	//*******************************************************************************
	// REAL																			*
	//*******************************************************************************
	
	case VAR_TYPE_REAL:
		
		*(REAL*)pValue = (REAL)*(REAL*)(ipVariable->address);
				
		break;
	
	case VAR_TYPE_LREAL: // maybe do something here //
			*(LREAL*)pValue = (REAL)*(REAL*)(ipVariable->address);
			break;
		
	//*******************************************************************************
	// STRING types																	*
	//*******************************************************************************
	
	case VAR_TYPE_STRING:

		*(REAL*)pValue = brsatof(ipVariable->address);
					
		break;
		
		case VAR_TYPE_WSTRING:
		{ // Add curly bois to create a scope in which we can declare variables
			// We do not have a brwatof function
			// So instead convert to string then use brsatof
			char tempString[25]; // Max number of characters of a double is 24
			wstring2string((UDINT)tempString, ipVariable->address, sizeof(tempString));
	
			*(REAL*)pValue = brsatof((UDINT)tempString); 
	
			break;
		}
		
	
	//*******************************************************************************
	// Unsupported and Invalid types												*
	//*******************************************************************************
	
	case VAR_TYPE_STRUCT:
	case VAR_TYPE_ULINT:
	case VAR_TYPE_DATE:
	case VAR_TYPE_ARRAY_OF_STRUCT:
	case VAR_TYPE_TIME_OF_DAY:
	case VAR_TYPE_LWORD:
	case VAR_TYPE_LINT: return VAR_ERR_UNSUPPORTEDTYPE; break;
	case VAR_TYPE_UNDEFINED: return VAR_ERR_PV_NOT_FOUND; break;

	default: return VAR_ERR_INVALIDTYPE; break;	
	
	
} // switch(DataType) //


return 0;


} // End Fn //
