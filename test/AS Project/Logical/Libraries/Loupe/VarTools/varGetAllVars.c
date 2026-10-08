/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Library: VarInfo
 * File: varGetValue.c
 * Author: davidblackburn
 * Created: June 02, 2014
 ********************************************************************
 * Implementation of library VarTools
 ********************************************************************/

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "varToolsInternal.h"
#include "VarTools.h"
#include "varGetAllVarsInternal.h"

#ifdef __cplusplus
	};
#endif


#include <string.h>


//*******************************************************************
// Get All variables on PLC				 							*
//*******************************************************************


// Fix: Seems to skip some variables
void varGetAllVars(varGetAllVars_typ* t)
{
	// Check for invalid input
	if (t == 0) return;

	
	if(t->AcknowledgeError && isError(t)) {
		t->Status = 0;
	}
	
	if(t->Execute && isDone(t)) {
		t->Internal.iVar = 0xFFFF;
		t->Status = 65535;
		t->Internal.NextVariable = 1;
		t->Internal.numUniqueVars = 0;
	}
	
	t->Internal.validVar = 0;
	while(!t->Internal.validVar && isBusy(t)) {
		if(t->Internal.NextVariable) {
			
			getNextVariable(t);
			
			// Set rest of system bits that need to know next var
			t->Internal.moList.first = t->Internal.NextVariable; // moList needs to know to start back at begining
			setCheckGlobal(t); // Every new variable we need to check for global
			resetIsGlobal(t);
			
			t->Internal.NextVariable = 0;
			
			if(isError(t) || isDone(t)) continue;
		}
		
		if(checkGlobal(t)) {
			// Check if variable has global
			varRefresh(&t->Variable); // TODO: move this to getNextVariable
			strcpy(t->Variable.name, t->Internal.xList.name); // TODO: move this to getNextVariable
			
			t->Internal.validVar = checkValidVar(t);
			resetCheckGlobal(t);
			
			// getNextVariable returns all variables without a task perfix. So you can get duplicate variables (ex multiple Configurations)
			// So we record all varables found so we can see if we already checked this one
			// Because global variables cant exist in mulitple tasks we dont need to keep track of those
			if(isUniqueVar(t)) {
				setAsUniqueVar(t);
			} else {
				t->Internal.NextVariable = 1; // Skip this var because we already say this variable
				continue;
			}
			
			if(!t->Internal.validVar) {	
				continue;
			}
			
			setIsGlobal(t);
			
			if(variableIsStructure(t) && t->ExpandStructs || variableIsArray(t) && !t->CondenseArrays) {
				goDownALevel(t);
			}
			else if(variableIsArray(t) && t->CondenseArrays) {
				// tODO: Condense array to just [i]
				strcat(t->Variable.name, "[");
				strcat(t->Variable.name, "]");
			}
			
			if(!variableIsPrimitive(t) && t->PrimitivesOnly) {
				t->Internal.validVar = 0;
			}
			
			if(atTopLevel(t))
				t->Internal.NextVariable = 1; // Set new varaible here becuase if something is global then its in each task as well but we just want to say global
			
		}
		else if(atTopLevel(t)) {
			if(isGlobal(t)) {
				// We can get here for global structures or global array of strucutres
				t->Internal.NextVariable = 1; // Set new varaible here becuase if something is global then its in each task as well but we just want to say global
				continue;
			}
			
			getNextTask(t);
			
			if(isError(t)) continue;
			
			if(outOfTasks(t)) {
				t->Internal.NextVariable = 1;
				continue;
			}
			
			if(checkValidTask(t)) continue;
			
			t->Internal.validVar = checkValidVar(t);
			
			if(!t->Internal.validVar) continue;
			
			if(variableIsStructure(t) && t->ExpandStructs || variableIsArray(t) && !t->CondenseArrays) {
				goDownALevel(t);
			}
			else if(variableIsArray(t) && t->CondenseArrays) {
				// tODO: Condense array to just [i]
				strcat(t->Variable.name, "[");
				strcat(t->Variable.name, "]");
			}
			
			if(!variableIsPrimitive(t) && t->PrimitivesOnly) {
				t->Internal.validVar = 0;
			}
			
		}
		else {
			getMembers(t);
			
			while(outOfMembers(t) && !atTopLevel(t)) {
				goUpALevel(t);
				getMembers(t);
			}
			
			if(atTopLevel(t)) continue;
			
			outputMember(t);
			
			incrementMember(t); // Increment member her so if we go down a level we come back to the next member
			
			if(variableIsStructure(t)) {
				goDownALevel(t); // So next pass we will be inside the current variable 
			}
			
			t->Internal.validVar = !t->PrimitivesOnly || variableIsPrimitive(t);
		}
	}
	
	if(isError(t)) {
		handleErrors(t);	
	}
	
	
	if(t->Status == 0) {
		memset(&t->Variable, 0, sizeof(t->Variable));	
	}
	
	return;
}

