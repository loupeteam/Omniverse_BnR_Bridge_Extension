
/* Functions to read general information about RTK objects */

#ifndef _RTK_OBJECT_H_
#define _RTK_OBJECT_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Objecttypes */
typedef enum
{
	RTK_OBJ_INVALID = 0,
	RTK_OBJ_TASK,
	RTK_OBJ_SEMAPHORE,
	RTK_OBJ_CRITICAL_SECTION,
	RTK_OBJ_TIMER,
	RTK_OBJ_INTERRUPT,
	RTK_OBJ_EXCEPTION,
	RTK_OBJ_PLAINFIFO,
	RTK_OBJ_FIFO,
	RTK_OBJ_MEMORY,
	RTK_OBJ_SHARED_MEMORY,
	RTK_OBJ_DEBUGGER,
	RTK_OBJ_LOG,
	RTK_OBJ_RTC,
	RTK_OBJ_MISC,
	RTK_OBJ_TASKTIMER /* internal */
} RTK_OBJ_TYPE;

typedef RTK_OBJ_TYPE* LPRTK_OBJ_TYPE;

/* Searches an RTK object with the provided name (lpszObjectName).
 * Returns the object (hObject - must not be NULL) and the type of the object (lpObjectType - can be NULL) */
EXIMPORT RTK_ERROR RtkGetIdent(LPSTRING lpszObjectName, RTK_H* hObject, LPRTK_OBJ_TYPE lpObjectType);

/* Returns information about the RTK-Object.
 * 
 * hObject............. Handle of the RTK Object
 * ulMaxNameLength..... Length of the provided lpszObjectName buffer
 * lpObjectType........ Output-parameter - can be NULL. Returns the type of the RTK-object.
 * lpszObjectName...... Output-parameter - can be NULL. The function call writes the RTK-object name to this buffer */
EXIMPORT RTK_ERROR
RtkGetObjectInfo(RTK_H hObject, ULONG ulMaxNameLength, LPRTK_OBJ_TYPE lpObjectType, LPSTRING lpszObjectName);

#ifdef __cplusplus
}
#endif

#endif
