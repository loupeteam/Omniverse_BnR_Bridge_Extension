/*
 * File: varGetVariableList.c
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
#include <stdlib.h>


//*******************************************************************
// Get a list of all local and global variables with a given name 	*
//*******************************************************************

// DONE passes first tests

unsigned short varGetVariableList(unsigned long pVariable, unsigned long List)
{
	
	//structure to look through data objects
	MO_List_typ item;

	//Local list of variables that were found
	STRING list[VAR_MAI_VARLIST+1][VAR_STRLEN_NAME+1];

	//Task:VariableName to look for
	STRING taskVar[VAR_STRLEN_NAME+1];	

	//Keep track of the objects that we've gone through
	//unsigned int objectIndex = 0;
	UINT objectIndex = 0;

	UINT index=0;
	BOOL duplicate=0;
		
	//Number of variables that have been found
	UINT numberVariables =0;

	//dummy variable for PV_xgetadr. Don't care what is in it.
	UDINT dat =0;
	
	//Check for valid inputs
	if( pVariable == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;
	if( List == 0 ) return (UINT)VAR_ERR_INVALIDINPUT;
	
	//Clear out any bad data
	memset((void*)List,0,(VAR_STRLEN_NAME+1)*(VAR_MAI_VARLIST+1));		
	memset(list,0,sizeof(list));
	
	//Look to see if there is a global variable
	//Denoted by gVarName
	strcpy(taskVar,"g");		
	strcat(taskVar,(char*)pVariable);
	//Check if variable exists
	if( PV_xgetadr(taskVar,&dat,&dat) == 0){
		//Check if we're full
		if( numberVariables >= (VAR_MAI_VARLIST+1)) {
			return VAR_ERR_TOO_MANY_ITEMS;//Not enough space
		}
		//Copy to list and count
		//strcpy( &list[numberVariables],taskVar);
		strcpy( list[numberVariables],taskVar);
		numberVariables++;				
	}
	
	
	//Look through all the objects
	while( MO_list(objectIndex, &objectIndex, &item) == 0){
		//If it is a cyclic object check if it has the variable
		if( item.type == 0x11){
			//create taskVar name task:VarName
			memcpy(&taskVar,&item.name,sizeof(item.name));		
			strcat(taskVar,":");
			strcat(taskVar,(char*)pVariable);
			
			//Check if variable exists
			if( PV_xgetadr(taskVar,&dat,&dat) == 0){
				//Check if we're full
				if( numberVariables >= (VAR_MAI_VARLIST+1)) {
					return VAR_ERR_TOO_MANY_ITEMS;//Not enough space
				}
				duplicate=0;				
				for(index=0;index<numberVariables;index++){
					if(strcmp(taskVar,list[index])==0){
						duplicate=1;
						break;
					}
				}
				if(!duplicate){
					strcpy( list[numberVariables],taskVar);
					numberVariables++;				
				}
			}
		}
	}
		
	//Sort the list that we found to ensure the order doesn't change arbitrarily.
	qsort(list,numberVariables,sizeof(list[0]),(int(*)(const void*,const void*))strcmp);
		
	//Copy list out for the user
	memcpy((void*)List,list,sizeof(list));	

	//Return the number of found variables to make it easy to add to the list outside the function.
	return numberVariables;
	
}
