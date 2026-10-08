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

// General Fns
plcbit isDone(varGetAllVars_typ* t) 			{ return t->Status == 0; }
plcbit isBusy(varGetAllVars_typ* t)				{ return t->Status == 65535; }
plcbit isError(varGetAllVars_typ* t) 			{ return !isBusy(t) && !isDone(t); }
plcbit handleErrors(varGetAllVars_typ* t) {
	return 0;
}



// Variable Fns
plcbit getNextVariable(varGetAllVars_typ* t) {
	UDINT nextVarIndex;
	UINT xListStatus = 0;
	nextVarIndex = t->Internal.iVar+1;
	xListStatus = PV_xlist(t->Internal.iVar, (UDINT)&nextVarIndex, &t->Internal.xList);
	t->Internal.iVar = nextVarIndex;
	
	t->Status = xListStatus == ERR_BUR_NOENTRY ? 0 : xListStatus == 0 ? 65535 : xListStatus;
	return t->Status == 65535;
}
plcbit variableIsValid(varGetAllVars_typ* t) {
	return t->Variable.dataType != VAR_TYPE_UNDEFINED;
}
plcbit variableIsStructure(varGetAllVars_typ* t) {
	return t->Variable.dataType == VAR_TYPE_STRUCT || t->Variable.dataType == VAR_TYPE_ARRAY_OF_STRUCT;
}
plcbit variableIsArray(varGetAllVars_typ* t) {
	return t->Variable.dataType == VAR_TYPE_ARRAY_OF_STRUCT || t->Variable.dimension > 1;
}
plcbit variableIsPrimitive(varGetAllVars_typ* t) {
	return !variableIsStructure(t) && variableIsValid(t);
}
plcbit checkValidVar(varGetAllVars_typ* t) {
	t->Variable.name[0] = '\0';
	varRefresh(&t->Variable);
	
	if(t->Internal.MoName[0] != '\0') {
		strcpy(t->Variable.name, t->Internal.MoName);
		strcat(t->Variable.name, ":");
	}
	strcat(t->Variable.name, t->Internal.xList.name);
	
	varGetInfo(&t->Variable);
	varGetValue(&t->Variable);
	
	return variableIsValid(t);
}

plcbit setCheckGlobal(varGetAllVars_typ* t) 	{ return t->Internal.checkGlobal = 1;}
plcbit resetCheckGlobal(varGetAllVars_typ* t) 	{ return t->Internal.checkGlobal = 0;}
plcbit checkGlobal(varGetAllVars_typ* t) 		{ return t->Internal.checkGlobal;}

plcbit setIsGlobal(varGetAllVars_typ* t) 	{ return t->Internal.isGlobal = 1;}
plcbit resetIsGlobal(varGetAllVars_typ* t) 	{ return t->Internal.isGlobal = 0;}
plcbit isGlobal(varGetAllVars_typ* t) 		{ return t->Internal.isGlobal;}

plcbit isUniqueVar(varGetAllVars_typ* t) {
	if(t->Internal.numUniqueVars == 0) return 1;
	
	for (unsigned long i = 0; i < t->Internal.numUniqueVars-1; i++) {
		if(strcmp(t->Internal.uniqueVars[i], t->Variable.name) == 0) return 0;
	}
	return 1;
}
plcbit setAsUniqueVar(varGetAllVars_typ* t) {
	if(t->Internal.numUniqueVars < sizeof(t->Internal.uniqueVars)/sizeof(t->Internal.uniqueVars[0])) {
		strcpy(t->Internal.uniqueVars[t->Internal.numUniqueVars], t->Variable.name);
		t->Internal.numUniqueVars++;
	}
}



// Task Fns
plcbit getNextTask(varGetAllVars_typ* t) {
	UINT moStatus = 0;
	
	t->Internal.moList.enable = 1;
	//	t->Internal.moList.first = t->Internal.NextVariable;
	t->Internal.moList.pAppMoName = (UDINT)t->Internal.AppMoName;
	t->Internal.moList.pMoName = (UDINT)t->Internal.MoName;
	t->Internal.moList.sizeAppMoName = sizeof(t->Internal.AppMoName);
	t->Internal.moList.sizeMoName = sizeof(t->Internal.MoName);
	
	slMoList(&t->Internal.moList);
	
	t->Internal.moList.first = 0; // Always reset this. This should be set true when reading a new variable
	
	// TODO check for errors
	//if(t->Internal.moList.status != 0) break; // Break if error
	return t->Internal.moList.status == 0;
}
plcbit outOfTasks(varGetAllVars_typ* t) 		{ return t->Internal.moList.status == ERR_BUR_NOENTRY; }
plcbit checkValidTask(varGetAllVars_typ* t) {
	// 0x11 = Object of a cyclic resource 
	// MoName will be empty if its a cyclic not added by User
	return t->Internal.moList.moType != 0x11 || t->Internal.MoName[0] == '\0';
}



// Member Fns
plcbit atTopLevel(varGetAllVars_typ* t) 		{ return t->Internal.iDeep == 0; }
plcbit goDownALevel(varGetAllVars_typ* t) {
	if(t->Internal.iDeep == 0) // Name browser will update its own name if we are digging deeper on an existing variable
		strcpy(t->Internal.Browser.VariableName, t->Variable.name);
	else
		t->Internal.Browser.MemberIndex = t->Internal.iMember[t->Internal.iDeep]-1;
	
	t->Internal.iDeep++;
	strcpy(t->Internal.Deep[t->Internal.iDeep], t->Variable.name);
	
}
plcbit goUpALevel(varGetAllVars_typ* t) {
	t->Internal.Browser.Back = 1;
	strcpy(t->Internal.Deep[t->Internal.iDeep], "");
	t->Internal.iMember[t->Internal.iDeep] = 0;
	t->Internal.iDeep--;
}

plcbit getMembers(varGetAllVars_typ* t) {
	if(t->Internal.Browser.Back)
		variableBrowser(&t->Internal.Browser); // If we command a back, it takes 2 cycles to update (this may be fixed in the future)
	
	variableBrowser(&t->Internal.Browser);
}
plcbit incrementMember(varGetAllVars_typ* t) {
	t->Internal.iMember[t->Internal.iDeep]++;
}
plcbit outOfMembers(varGetAllVars_typ* t) {
	return !(t->Internal.iMember[t->Internal.iDeep] < t->Internal.Browser.NumberMembers);
}
plcbit outputMember(varGetAllVars_typ* t) {
	memcpy(&t->Variable, &t->Internal.Browser.MemberInfo[t->Internal.iMember[t->Internal.iDeep]], sizeof(t->Variable));
}
plcbit memberIsStructure(varGetAllVars_typ* t) {
	UDINT datatype = t->Internal.Browser.MemberInfo[t->Internal.iMember[t->Internal.iDeep]].dataType;
	return datatype == VAR_TYPE_STRUCT || datatype == VAR_TYPE_ARRAY_OF_STRUCT;
}
plcbit memberIsArray(varGetAllVars_typ* t) {
	varVariable_typ* var =  &t->Internal.Browser.MemberInfo[t->Internal.iMember[t->Internal.iDeep]];
	return var->dataType == VAR_TYPE_ARRAY_OF_STRUCT || var->dimension > 1;
}
plcbit memberIsPrimitive(varGetAllVars_typ* t) {
	return !memberIsStructure(t);
}

