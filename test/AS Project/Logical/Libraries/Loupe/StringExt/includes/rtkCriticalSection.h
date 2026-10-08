
/* RTK critical section functions */

#ifndef _RTK_CRITICAL_SECTION_H_
#define _RTK_CRITICAL_SECTION_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef RTK_H RTK_CRIT_SEC_H;

/**********************************************************************************************/
/* Critical Section                                                                           */
/* A critical sections can be used to mutually exclude access to critical code sections which */
/* process shared data that might be simultaniously modified by other tasks as well.          */
/* RTK critical sections support priority inheritance to avoid priority inversion.            */
/**********************************************************************************************/

/* Creates a critical section object with the provided name (lpszCriticalSectionName must be unique or NULL).
 * The output-parameter hCriticalSection (must not be NULL) returns a handle to the critical section */
EXIMPORT RTK_ERROR RtkCreateCriticalSection(LPSTRING lpszCriticalSectionName, RTK_CRIT_SEC_H* hCriticalSection);

/* Deletes a critical section object */
EXIMPORT RTK_ERROR RtkDeleteCriticalSection(RTK_CRIT_SEC_H hCriticalSection);

/* Enter a critical section.
 * This function has to be called before accessing the data which needs mutual exclusion.
 * If no other task is in the critical section (entered and did not yet leave), then this function returns immediately.
 * If another task is in the critical section, then the current task yields until no other
 * task is in the critical section. */
EXIMPORT RTK_ERROR RtkEnterCriticalSection(RTK_CRIT_SEC_H hCriticalSection);

/* Leave a critical section.
 * This function has to be called after accessing the data which needs mutual exclusion. */
EXIMPORT RTK_ERROR RtkLeaveCriticalSection(RTK_CRIT_SEC_H hCriticalSection);

#ifdef __cplusplus
}
#endif

#endif
