
/* RTK timer functions
 * Timers can be used for cyclic actions and for single timeouts
 */

#ifndef _RTK_TIMER_H_
#define _RTK_TIMER_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef RTK_H RTK_TIMER_H;

/* Flags for RtkStartTimer */
typedef unsigned long RTK_TIMERFLAG;
#define RTK_TIMER_CYCLIC 0x00000000
#define RTK_TIMER_ONESHOT 0x00000001

/* Create a timer object with the provided name (lpszTimerName must be unique or NULL).
 * When the timer expires (to be started with RtkStartTimer), then timerFunction(timerContext) is called with task
 * priority wTimerPriorityand with stack size ulTimerSupervidorStackSize
 * The output-parameter hTimer (must not be NULL) returns a handle to the timer */
EXIMPORT RTK_ERROR RtkCreateTimer(LPSTRING lpszTimerName, WORD wTimerPriority, ULONG ulTimerSupervisorStackSize,
                                  LPRTK_CREATE_TASK_FKT timerFunction, void* timerContext, RTK_TIMER_H* hTimer);

/* Deletes a timer object */
EXIMPORT RTK_ERROR RtkDeleteTimer(RTK_TIMER_H hTimer);

/* Starts a cyclical timer (TimerFlage=RTK_TIMER_CYCLIC) with the provided cycle time (ulTimerCycleUsec)
 * or a single-shot timer (TimerFlags=RTK_TIMER_ONESHOT) with the provided delay (ulTimerCycleUsec) */
EXIMPORT RTK_ERROR RtkStartTimer(RTK_TIMER_H hTimer, ULONG ulTimerCycleUsec, RTK_TIMERFLAG TimerFlags);

/* Stops a timer started with RtkStartTimer */
EXIMPORT RTK_ERROR RtkStopTimer(RTK_TIMER_H hTimer);

#ifdef __cplusplus
}
#endif

#endif
