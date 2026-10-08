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
plcbit isDone(varGetAllVars_typ* t);
plcbit isBusy(varGetAllVars_typ* t);
plcbit isError(varGetAllVars_typ* t);
plcbit handleErrors(varGetAllVars_typ* t);



// Variable Fns
plcbit getNextVariable(varGetAllVars_typ* t);
plcbit checkValidVar(varGetAllVars_typ* t);
plcbit variableIsValid(varGetAllVars_typ* t);
plcbit variableIsPrimitive(varGetAllVars_typ* t);
plcbit variableIsStructure(varGetAllVars_typ* t);
plcbit variableIsArray(varGetAllVars_typ* t);

plcbit setCheckGlobal(varGetAllVars_typ* t);
plcbit resetCheckGlobal(varGetAllVars_typ* t);
plcbit checkGlobal(varGetAllVars_typ* t);
plcbit setIsGlobal(varGetAllVars_typ* t);
plcbit resetIsGlobal(varGetAllVars_typ* t);
plcbit isGlobal(varGetAllVars_typ* t);

plcbit isUniqueVar(varGetAllVars_typ* t);
plcbit setAsUniqueVar(varGetAllVars_typ* t);



// Task Fns
plcbit getNextTask(varGetAllVars_typ* t);
plcbit outOfTasks(varGetAllVars_typ* t);
plcbit checkValidTask(varGetAllVars_typ* t);



// Member Fns
plcbit atTopLevel(varGetAllVars_typ* t);
plcbit goDownALevel(varGetAllVars_typ* t);
plcbit goUpALevel(varGetAllVars_typ* t);

plcbit getMembers(varGetAllVars_typ* t);
plcbit incrementMember(varGetAllVars_typ* t);
plcbit outOfMembers(varGetAllVars_typ* t);
plcbit outputMember(varGetAllVars_typ* t);
plcbit memberIsPrimitive(varGetAllVars_typ* t);
plcbit memberIsStructure(varGetAllVars_typ* t);
plcbit memberIsArray(varGetAllVars_typ* t);

