/*
 * File: varGetValue.c
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
// Get the value of a variable as a string 							*
//*******************************************************************


// Check for null pointers		DONE and TESTED DFB 20140625

// Check for variable info		DONE and TESTED DFB 20140625

// Get Value					DONE and TESTED DFB 20141103
	
	// BOOL						DONE and TESTED DFB 20141103
			
	// SINT						DONE and TESTED DFB 20140625
	// INT						DONE and TESTED DFB 20140625
	// DINT						DONE and TESTED DFB 20140625
	// TIME						DONE and TESTED DFB 20140625
	// USINT					DONE and TESTED DFB 20140625
	// UINT						DONE and TESTED DFB 20140625
	// UDINT					DONE and TESTED DFB 20140625
	// DATE_AND_TIME			DONE and TESTED DFB 20140625

	// REAL						DONE and TESTED DFB 20140625

	// STRING					DONE and TESTED DFB 20140925
	
	// Unsupported type			DONE and TESTED DFB 20140625
	// Invalid type				DONE and TESTED DFB 20140625

	
unsigned short varGetValue(unsigned long pVariable)
{

	// Check for invalid input
	if (pVariable == 0) return (UINT)VAR_ERR_INVALIDINPUT;

	// Check address
	varVariable_typ	*ipVariable = (varVariable_typ*)pVariable;
	UINT status;

	if (ipVariable->address == 0) {
		// Get info if necessary
		status = varGetInfo(pVariable);
		if (status != 0) return status;
	}


	//*******************************************************
	// Get value of variable and copy to Variable.value		*
	//*******************************************************

	DINT	valueDint;
	UDINT	valueUdint;
	REAL	valueReal;


	// Clear the Value string. It will be overwritten below

	memset( ipVariable->value, 0, sizeof(ipVariable->value) );


	switch( ipVariable->dataType ){
	
	
		//*******************************************************************************
		// BOOL																			*
		//*******************************************************************************
	
		case VAR_TYPE_BOOL:
		
			if( *(BOOL*)(ipVariable->address) == 0 ){
			
				strcpy( ipVariable->value, "false" );
			
			}
			else if( *(BOOL*)(ipVariable->address) == 1 ){
			
				strcpy( ipVariable->value, "true" );
			
			}
			else{
			
				// Invalid BOOL value.
				// Use convention of non-zero == TRUE for now
				strcpy( ipVariable->value, "true" );
			
			}
		
			break;
	
	
		//*******************************************************************************
		// Integer types																*
		//*******************************************************************************
					
		case VAR_TYPE_SINT:
		
			valueDint=	*(SINT*)(ipVariable->address);
		
			brsitoa( valueDint, (UDINT)ipVariable->value );
		
			break;
	
	
		case VAR_TYPE_INT:
		
			valueDint=	*(INT*)(ipVariable->address);
		
			brsitoa( valueDint, (UDINT)ipVariable->value );
		
			break;
	
	
		case VAR_TYPE_DINT:
		case VAR_TYPE_TIME:
		
			valueDint=	*(DINT*)(ipVariable->address);
		
			brsitoa( valueDint, (UDINT)ipVariable->value );
		
			break;
	
		case VAR_TYPE_BYTE:
		case VAR_TYPE_USINT:
		
			valueDint=	*(USINT*)(ipVariable->address);
		
			brsitoa( valueDint, (UDINT)ipVariable->value );
		
			break;

		case VAR_TYPE_WORD:
		case VAR_TYPE_UINT:
		
			valueDint=	*(UINT*)(ipVariable->address);
		
			brsitoa( valueDint, (UDINT)ipVariable->value );
		
			break;
	
		case VAR_TYPE_DWORD:
		case VAR_TYPE_UDINT:
		case VAR_TYPE_DATE_AND_TIME:
		
			valueUdint=	*(UDINT*)(ipVariable->address);
		
			uitoa( valueUdint, (UDINT)ipVariable->value );
		
			break;
	
	
		//*******************************************************************************
		// REAL																			*
		//*******************************************************************************
	
		case VAR_TYPE_REAL:
		
			valueReal=	*(REAL*)(ipVariable->address);
		
			brsftoa( valueReal, (UDINT)ipVariable->value );
		
			break;
	
		//*******************************************************************************
		// LREAL																			*
		//*******************************************************************************
	
		case VAR_TYPE_LREAL:
		
			valueReal=	(REAL)*(LREAL*)(ipVariable->address);
		
			brsftoa( valueReal, (UDINT)ipVariable->value );
		
			break;
	
		//*******************************************************************************
		// STRING types																	*
		//*******************************************************************************
	
		case VAR_TYPE_STRING:
		
			strncpy(ipVariable->value, (char*)ipVariable->address, VAR_STRLEN_VALUE);
			ipVariable->value[VAR_STRLEN_VALUE] = '\0';
		
			break;
		
		case VAR_TYPE_WSTRING:
			
			wstring2string((UDINT)ipVariable->value, ipVariable->address, VAR_STRLEN_VALUE);
			
			break;
		
	
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
		case VAR_TYPE_UNDEFINED:  return VAR_ERR_PV_NOT_FOUND; break;

		default: return VAR_ERR_INVALIDTYPE; break;	
	
	
	} // switch(DataType) //


	return 0;


} // End Fn //
