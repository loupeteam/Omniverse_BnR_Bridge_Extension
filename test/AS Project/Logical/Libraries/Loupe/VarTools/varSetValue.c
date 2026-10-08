/*
 * File: varSetValue.c
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
#include <ctype.h>


//*******************************************************************
// Set the value of a variable from a string 						*
//*******************************************************************


// Check for null pointer		DONE and TESTED DFB 20140625

// Check for variable info		DONE and TESTED DFB 20140625

// Set Variable					DONE and TESTED DFB 20141103
	
	// BOOL						DONE and TESTED DFB 20141103
		// 0					DONE and TESTED DFB 20140625
		// 1					DONE and TESTED DFB 20140625
		// false				DONE and TESTED DFB 20141103
		// true					DONE and TESTED DFB 20141103
		// Invalid value		DONE and TESTED DFB 20141103
		
	// SINT						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// INT						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// DINT						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// TIME						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// USINT					DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// UINT						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// UDINT					DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625
	// DATE_AND_TIME			DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625

	// REAL						DONE and TESTED DFB 20140625
		// Valid value			DONE and TESTED DFB 20140625
		// Invalid value		DONE and TESTED DFB 20140625

	// STRING					DONE and TESTED DFB 20140625
	
	// Unsupported type			DONE and TESTED DFB 20140625
	// Invalid type				DONE and TESTED DFB 20140625
	

unsigned short varSetValue(unsigned long pVariable)
{


	//***********************************************
	// Check for invalid inputs						*
	//***********************************************

	if( pVariable == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;


	//***********************************************
	// Check address								*
	//***********************************************

	varVariable_typ	*ipVariable = (varVariable_typ*)pVariable;
	UINT status;

	if( ipVariable->address == 0 ){

		// varGetInfo will clear the current 'value'
		// Store it temporarily and restore it afterward
		
		STRING tempValue[VAR_STRLEN_VALUE+1];
		strcpy(tempValue, ipVariable->value);
	
		status = varGetInfo(pVariable);
	
		strcpy(ipVariable->value, tempValue);
	
		if(status != 0) return status;
		
	} // Get info if necessary //


	//***********************************************
	// Set value of variable to Variable.value		*
	//***********************************************

	DINT	ValueDint;
	UDINT	ValueUdint, i, ValueStringLength;
	REAL	ValueReal;


	switch( ipVariable->dataType ){
	
	
		//*******************************************************************************
		// BOOL																			*
		//*******************************************************************************
	
		case VAR_TYPE_BOOL:
		
			if(	strcmp( ipVariable->value, "0" ) == 0 ){
		
				*(BOOL*)(ipVariable->address)=	0;
		
			} // "0" //
		
			else if( strcmp( ipVariable->value, "1" ) == 0 ){
		
				*(BOOL*)(ipVariable->address)=	1;
		
			} // "1" //
		
			else{
			
			
				// Convert String to lower case
			
				ValueStringLength=	strlen( ipVariable->value );
			
				for( i=0; i<ValueStringLength; i++ ){
			
					ipVariable->value[i]=	tolower( ipVariable->value[i] );
			
				} // convert to lower case //
			
			
				// Check string
			
				if( strcmp( ipVariable->value, "false" ) == 0 ){
		
					*(BOOL*)(ipVariable->address)=	0;
		
				}
				else if( strcmp( ipVariable->value, "true" ) == 0 ){
		
					*(BOOL*)(ipVariable->address)=	1;
		
				}
				else{
		
					// Invalid BOOL value
					// Use convention of non-zero == TRUE for now		
					*(BOOL*)(ipVariable->address)=	1;
			
				}
			
			
			} // Other string //

			break;
	
	
		//*******************************************************************************
		// Integer types																*
		//*******************************************************************************
	
		case VAR_TYPE_SINT:
		
			ValueDint=	brsatoi( (UDINT)ipVariable->value );
		
			if( 	(ValueDint > VAR_MAX_SINT)
				||	(ValueDint < VAR_MIN_SINT)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(SINT*)(ipVariable->address)=	(SINT)ValueDint;
				
			}
			
			break;
	
	
		case VAR_TYPE_INT:
		
			ValueDint=	brsatoi( (UDINT)ipVariable->value );
		
			if( 	(ValueDint > VAR_MAX_INT)
				||	(ValueDint < VAR_MIN_INT)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(INT*)(ipVariable->address)=	(INT)ValueDint;
				
			}
			
			break;
	
	
		case VAR_TYPE_DINT:
		case VAR_TYPE_TIME:
		
			ValueDint=	brsatoi( (UDINT)ipVariable->value );
		
			if( 	(ValueDint > VAR_MAX_DINT)
			||	(ValueDint < VAR_MIN_DINT)
			){
			
			return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
			*(DINT*)(ipVariable->address)=	(DINT)ValueDint;
				
			}
			
			break;
	
	
		case VAR_TYPE_BYTE:
		case VAR_TYPE_USINT:
		
			ValueDint=	brsatoi( (UDINT)ipVariable->value );
		
			if( 	(ValueDint > VAR_MAX_USINT)
				||	(ValueDint < VAR_MIN_USINT)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(USINT*)(ipVariable->address)=	(USINT)ValueDint;
				
			}
			
			break;
	
		
		case VAR_TYPE_WORD:
		case VAR_TYPE_UINT:
		
			ValueDint=	brsatoi( (UDINT)ipVariable->value );
		
			if( 	(ValueDint > VAR_MAX_UINT)
				||	(ValueDint < VAR_MIN_UINT)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(UINT*)(ipVariable->address)=	(UINT)ValueDint;
				
			}
			
			break;
	
		
		case VAR_TYPE_DWORD:
		case VAR_TYPE_UDINT:
		case VAR_TYPE_DATE_AND_TIME:
		
			ValueUdint=	atoui( (UDINT)ipVariable->value );
		
			if( 	(ValueUdint > VAR_MAX_UDINT)
			||	(ValueUdint < VAR_MIN_UDINT)
			){
			
			return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
			*(UDINT*)(ipVariable->address)=	(UDINT)ValueUdint;
				
			}
			
			break;
	
	
		//*******************************************************************************
		// REAL																			*
		//*******************************************************************************
	
		case VAR_TYPE_REAL:
		
			ValueReal=	brsatof( (UDINT)ipVariable->value );
		
			if( 	(ValueReal > VAR_MAX_REAL)
				||	(ValueReal < VAR_MIN_REAL)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(REAL*)(ipVariable->address)=	ValueReal;
				
			}
			
			break;
		
		//*******************************************************************************
		// LREAL																			*
		//*******************************************************************************
		case VAR_TYPE_LREAL: // Lets only support up to a real //
			ValueReal=	brsatof( (UDINT)ipVariable->value );
		
			if( 	(ValueReal > VAR_MAX_REAL)
				||	(ValueReal < VAR_MIN_REAL)
			){
			
				return VAR_ERR_INVALIDVALUE;
		
			}
			else{
			
				*(LREAL*)(ipVariable->address)=	ValueReal;
				
			}
			
			break;

		//*******************************************************************************
		// STRING																		*
		//*******************************************************************************
	
		case VAR_TYPE_STRING: // TODO: This can be optimized
		
			memset( (char*)ipVariable->address, 0, ipVariable->length );
		
			strncpy( (char*)ipVariable->address, ipVariable->value, ipVariable->length-1 );
	
			break;
		
		case VAR_TYPE_WSTRING: 
			
			string2wstring( (UDINT)ipVariable->address, (UDINT)ipVariable->value, ipVariable->length-1 );
		
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

		default: return VAR_ERR_INVALIDTYPE; break;	
	
	
	} // switch(DataType) //


	return 0;


} // End Fn //
