/*
 * File: variableBrowser.c
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

void variableBrowser(struct variableBrowser* inst)
{

	int iLevelIterator=0;
	int iReadIterator=0;
	STRING tempstring[4];
	STRING MemberName[VAR_STRLEN_NAME];

	
	//*******************************************************************
	// If the user selects a member, explore that member				*
	//*******************************************************************
	if(inst->MemberIndex != -1){

		//***************************************
		// Check for a valid index				*
		//***************************************
		
		if(inst->MemberIndex<50 && inst->MemberIndex<inst->iMemberIndex){
	
			strcpy(inst->iLevel[inst->iLevelIndex],inst->MemberName[inst->MemberIndex]);
			inst->iLevelIndex++;		

		}
		//***********************************************
		// If the selected index is greater max members	*
		// assume it is a large array					*
		//***********************************************
		else if(inst->MemberIndex < inst->info.dimension ){

			brsitoa(inst->MemberIndex,(UDINT)tempstring);
			strcat(inst->iLevel[inst->iLevelIndex],"[");				
			strcat(inst->iLevel[inst->iLevelIndex],tempstring);
			strcat(inst->iLevel[inst->iLevelIndex],"]");
			inst->iLevelIndex++;		

		}
		
		//Reset the member index to allow the user to select again
		inst->MemberIndex=-1;
		
	}
	
	
	//***********************************************
	// Build up the current variable from the levels*
	//***********************************************
	strcpy(inst->CurrentVariable,inst->VariableName);
	for(iLevelIterator=0;iLevelIterator<inst->iLevelIndex;iLevelIterator++){
		strcat(inst->CurrentVariable,inst->iLevel[iLevelIterator]);			
	}		

	//***********************************************
	// If the Current variable isn't what we have	*
	// data for look up the variable				*
	//***********************************************
	if(strcmp(inst->info.name,inst->CurrentVariable)!=0){
		
		//***********************************************
		// Reset the known data 						*
		//***********************************************
		memcpy(inst->info.name,inst->CurrentVariable,sizeof(inst->info.name));
		memset(&inst->MemberInfo,0,sizeof(inst->MemberInfo));
		memset(&inst->MemberName,0,sizeof(inst->MemberName));
		memset(&inst->MemberValue,0,sizeof(inst->MemberValue));
		
		//***********************************************
		// Get new info for the selected variable		*
		//***********************************************
		varGetInfo((UDINT)&inst->info);
		
		//***********************************************
		// If this is a structure get all the members	*
		//***********************************************
		inst->iMemberIndex=0;
		if(inst->info.dataType == VAR_TYPE_STRUCT){
			while(PV_item(inst->CurrentVariable,inst->iMemberIndex,MemberName) == 0 && inst->iMemberIndex < 50){
				strcpy(inst->MemberInfo[inst->iMemberIndex].name,inst->CurrentVariable);
				strcpy(inst->MemberName[inst->iMemberIndex],".");				
				strcat(inst->MemberName[inst->iMemberIndex],MemberName);
				strcat(inst->MemberInfo[inst->iMemberIndex].name,inst->MemberName[inst->iMemberIndex]);				
				inst->iMemberIndex++;
			}	
		}
		//***************************************************
		// If this is an array build up strings to select	*
		//***************************************************
		else if(inst->info.dimension > 1){
			while(inst->iMemberIndex < inst->info.dimension && inst->iMemberIndex < 50){

				strcpy(inst->MemberInfo[inst->iMemberIndex].name,inst->CurrentVariable);
				
				brsitoa(inst->iMemberIndex,(UDINT)tempstring);
				strcpy(inst->MemberName[inst->iMemberIndex],"[");				
				strcat(inst->MemberName[inst->iMemberIndex],tempstring);
				strcat(inst->MemberName[inst->iMemberIndex],"]");	
				
				strcat(inst->MemberInfo[inst->iMemberIndex].name,inst->MemberName[inst->iMemberIndex]);

				inst->iMemberIndex++;
			}			
		}
	}
	
	//***************************************************
	// Run a variable watch on all members				*
	//***************************************************
	for(iReadIterator=0;iReadIterator<inst->iMemberIndex;iReadIterator++){
		inst->iMemberWatch[iReadIterator].pVariable = (UDINT)&inst->MemberInfo[iReadIterator];
		varVariableWatch(&inst->iMemberWatch[iReadIterator]);	
		if(inst->iMemberWatch[iReadIterator].Status == 0){
			strcpy(inst->MemberValue[iReadIterator],((varVariable_typ*)(inst->iMemberWatch[iReadIterator].pVariable))->value);
		}
		else{
			strcpy(inst->MemberValue[iReadIterator],"");
		}
	}

	//***************************************************
	// Run a variable watch top level PV				*
	//***************************************************
	inst->iCurrentWatch.pVariable = (UDINT)&inst->info;
	varVariableWatch(&inst->iCurrentWatch);

	
	
	//***************************************************
	// Navigate up the PV if requested or the info isn't*
	// found											*
	//***************************************************
	if(inst->Back || inst->info.address == 0){	
		if(inst->iLevelIndex>0){
			inst->iLevelIndex--;
			memset(inst->iLevel[inst->iLevelIndex],0,sizeof(inst->iLevel[inst->iLevelIndex]));
		}
		inst->Back=0;
	}

	inst->NumberMembers= inst->iMemberIndex;
}
