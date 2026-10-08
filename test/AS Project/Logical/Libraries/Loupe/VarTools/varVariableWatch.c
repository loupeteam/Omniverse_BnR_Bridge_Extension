/*
 * File: varVariableWatch.c
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
// Watch a PV from the HMI											*
//*******************************************************************

void varVariableWatch(struct varVariableWatch* inst)
{

	
	//***********************************************
	// Check for invalid inputs						*
	//***********************************************

	if( inst->pVariable == 0){
		inst->Status = VAR_ERR_INVALIDINPUT;
		return;
	}

	
	//***********************************************
	// Check address and Info						*
	//***********************************************

	varVariable_typ	*ipVariable = (varVariable_typ*)inst->pVariable;
	UINT status;

	// Get info if necessary //
	if( ipVariable->address == 0 || strcmp(inst->iName, ipVariable->name)!=0 ){

		//Get the variable info if we don't have it
		status = varGetInfo((UDINT)ipVariable);

		strcpy(inst->iName, ipVariable->name);
		
		if(status != 0){ 
			inst->Status = status;
			return;
		}

		//Read the current value if it is a basic datatype to avoid overwriting data
		if(ipVariable->dataType != VAR_TYPE_ARRAY_OF_STRUCT && ipVariable->dataType != VAR_TYPE_STRUCT){

			varGetValue((UDINT)ipVariable);
		
			strcpy(inst->iValue, ipVariable->value);
		}
		
	}

	if(ipVariable->dataType != VAR_TYPE_ARRAY_OF_STRUCT && ipVariable->dataType != VAR_TYPE_STRUCT){
		
		//If the user changed the value set it on the PV
		if( strcmp(inst->iValue, ipVariable->value)!=0 ){
			//Set the value the user typed in
			inst->Status = varSetValue((UDINT)ipVariable);
		}
		else{
			inst->Status = varGetValue((UDINT)ipVariable);
		}

		//Save the current value
		strcpy(inst->iValue,ipVariable->value);

	}

} // End Fn //
