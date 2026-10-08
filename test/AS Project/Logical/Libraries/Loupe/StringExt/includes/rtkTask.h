
/* RTK task functions */

#ifndef _RTK_TASK_H_
#define _RTK_TASK_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef RTK_H RTK_TASK_H;
typedef RTK_H RTK_TASK_TIMER_H;

#define RTK_TASKIDENT_ITSELF 0x00000000
#define RTK_MAX_TASK_PRIORITY 255
#define RTK_MIN_TASK_PRIORITY 1
#define RTK_IDLE_TASK_PRIORITY 0x1000
#define RTK_MAX_TASK_VALUES 16

/* Task flags (create/start) */
typedef unsigned long RTK_TASKFLAG;
#define RTK_TASK_RESUMED 0x00000000 /* task starts immediately*/
#define RTK_TASK_SUSPENDED 0x00000001 /* task does not start immediately (to be started with RtkTaskResume) */
#define RTK_TASK_SYSTEM 0x00000000 /* ignored */
#define RTK_TASK_DRIVER 0x00000002 /* ignored */
#define RTK_TASK_APPLICATION 0x00000004 /* ignored */

/* Task event flags */
typedef unsigned long RTK_EVENTFLAG;
#define RTK_EVENT_ANY 0x00000000 /* wait until any of the wanted events are set */
#define RTK_EVENT_ALL 0x00000001 /* wait until all wanted events are set */
#define RTK_EVENT_NO_DELETE 0x00000002 /* ignored */

/* Task state flags (taskInfo) */
typedef unsigned long RTK_TASKSTATEFLAG;
#define RTK_TASKSTATE_READY 0x00000000     /* Task is not waiting for any resource other than the CPU. */
#define RTK_TASKSTATE_SUSPENDED 0x00000001 /* Task is unavailable for execution. */
#define RTK_TASKSTATE_PENDED 0x00000002    /* Task is blocked due to the unavailability of some resource. */
#define RTK_TASKSTATE_DELAYED 0x00000004   /* Task is asleep for some duration. */
/* RTK_TASKSTATE_DELAYED + RTK_TASKSTATE_SUSPENDED     Task is both delayed and suspended */
/* RTK_TASKSTATE_PENDED  + RTK_TASKSTATE_SUSPENDED     Task is both pended and suspended */

/* Creates a task running function taskFunction(taskFunctionParameter).
 * The task has the name lpszTaskName (must be unique or NULL), the priority wTaskPriority (1.. lowest, 255.. highest),
 * and the stack size ulTaskUserStackSize+ulTaskSupervisorStackSize in bytes. This combined stack size has to be 
 * greater or equal to 0x1000 and less than or equal to 0x100000, otherwise the error ERR_RTK_NO_STACK will be returned.
 * The TaskFlags can be used to determine the initial task state (resumed/suspended).
 * The function returns a handle to the created task (hTask) */
EXIMPORT RTK_ERROR RtkCreateTask(LPSTRING lpszTaskName, WORD wTaskPriority, ULONG ulTaskSupervisorStackSize,
                                 ULONG ulTaskUserStackSize, RTK_TASKFLAG TaskFlags, LPRTK_CREATE_TASK_FKT taskFunction,
                                 void* taskFunctionParameter, RTK_TASK_H* hTask);

/* Deletes the task.
 * Stops execution and frees stack of the task. */
EXIMPORT RTK_ERROR RtkDeleteTask(RTK_TASK_H hTask);

/* Suspends the task.
 * A suspended task stops its execution until it is resumed (RtkResumeTask) */
EXIMPORT RTK_ERROR RtkSuspendTask(RTK_TASK_H hTask);

/* Resumes the task. */
EXIMPORT RTK_ERROR RtkResumeTask(RTK_TASK_H hTask);

/* Delays the calling task for the specified time in µs. */
EXIMPORT void RtkSleepTaskUsec(ULONG ulSleepUsec);

/* Sends an event to a task (hTask).
 * Each bit of wTaskEvent is one event of the task - sending multiple events is possible (logical OR of event-bits).
 * Sending one event-bit multiple times (e.g. by calling this function twice) has no effect if the event was not yet
 * read with RtkReceiveTaskEvent. */
EXIMPORT RTK_ERROR RtkSendTaskEvent(RTK_TASK_H hTask, WORD wTaskEvents);

/* Waits for one (EventFlags=RTK_EVENT_ANY) or all (EventFlag=RTK_EVENT_ALL) of the requested
 * Events and returns these events. Resets all returned events of the task.
 *
 * hTask............. Handle of the task for which the events shall be returned
 * wWantedEvents..... Bit-mask of the events which shall be returned
 * EventFlags........ With EventFlags=RTK_EVENT_ANY this function returns if any of the events specified with
 *                    wWantedEvents is received. With EventFlags=RTK_EVENT_ALL this function only returns if all of the
 *                    events specified with wWantedEvents is received
 * ulTimeoutUsec..... If the specified timeout is over before the events are received, the function returns with an
 *                    error and lpwReceivedEvent is not valid. If some of the requested events were already received by
 *                    the task, then these events are not reset.
 * lpwReceivedEvent.. This output-parameter contains the received events after the function call.
 *                    Parameter must not be NULL. */
EXIMPORT RTK_ERROR RtkReceiveTaskEvent(RTK_TASK_H hTask, WORD wWantedEvents, RTK_EVENTFLAG EventFlags,
                                       ULONG ulTimeoutUsec, LPWORD lpwReceivedEvents);

/* Cyclically sends events to a task (hTask). The events will be sent every ulDelayUsec µs.
 * Each bit of wTaskEvent is one event of the task - sending multiple events is possible (logical OR of event-bits)
 * Sending one event-bit multiple times has no effect (e.g. if the timer expires and the event is re-sent before the
 * event was read with RtkReceiveTaskEvent).
 * Returns a timer handle (hTaskTimer - must not be NULL) which can be used to stop sending the events
 * (RtkCancelTaskTimer) */
EXIMPORT RTK_ERROR
RtkSendTaskEventCyclic(RTK_TASK_H hTask, WORD wTaskEvents, ULONG ulDelayUsec, RTK_TASK_TIMER_H* hTaskTimer);

/* Sends events once to a task (hTask). The events will be sent after ulDelayUsec µs.
 * Each bit of wTaskEvent is one event of the task - sending multiple events is possible (logical OR of event-bits).
 * Sending one event-bit multiple times (e.g. by calling this function twice) has no effect if the event was not yet
 * read with RtkReceiveTaskEvent.
 * Returns a timer handle (hTaskTimer - must not be NULL) which can be used to stop sending the event
 * (RtkCancelTaskTimer) */
EXIMPORT RTK_ERROR
RtkSendTaskEventAfter(RTK_TASK_H hTask, WORD wTaskEvents, ULONG ulDelayUsec, RTK_TASK_TIMER_H* hTaskTimer);

/* Cancels the provided timer (hTaskTimer).
 * This results in canceling sending the events initiated with RtkSendtaskEventCyclic() or RtkSendTaskEventAfter() */
EXIMPORT RTK_ERROR RtkCancelTaskTimer(RTK_TASK_TIMER_H hTaskTimer);

/* Writes to one of the 16 available local variables of the task (hTask).
 * byValueIndex (0-15) addresses the variables, ulValue is the values to be written */
EXIMPORT RTK_ERROR RtkSetTaskValue(RTK_TASK_H hTask, BYTE byValueIndex, ULONG ulValue);

/* Reads to one of the 16 available local variables of the task (hTask).
 * byValueIndex (0-15) addresses the variables, lpulValue contains the value of the variable after the function call */
EXIMPORT RTK_ERROR RtkGetTaskValue(RTK_TASK_H hTask, BYTE byValueIndex, LPULONG lpulValue);

/* Sets the task priority to wTaskPriority (1.. lowest, 255.. highest) */
EXIMPORT RTK_ERROR RtkSetTaskPriority(RTK_TASK_H hTask, WORD wTaskPriority);

/* Retrieves the task priority.
 * lpwTaskPriority contains the priority of the task after the function call */
EXIMPORT RTK_ERROR RtkGetTaskPriority(RTK_TASK_H hTask, LPWORD lpwTaskPriority);

/* Returns task information
 *
* hTask................ Handle of the task for which the information shall be returned
* lpTaskStateFlags..... Output-parameter - can be NULL. Returns the task state (RTK_TASKSTATE_READY,
*                       RTK_TASKSTATE_SUSPENDED, RTK_TASKSTATE_PENDED, RTK_TASKSTATE_DELAYED)
* lpwTaskPriority....... Output-parameter - can be NULL. Returns the task priority
* lpulUnused1........... unused - can be NULL
* lpulUnused2........... unused - can be NULL
* lpulFreeStack......... Output-parameter - can be NULL. Returns the free stack size
* lpulInternalTaskIdent. Output-parameter - can be NULL. Returns an OS-specific task ident */
EXIMPORT RTK_ERROR RtkGetTaskInfo(RTK_TASK_H hTask, RTK_TASKSTATEFLAG* lpTaskStateFlags, LPWORD lpwTaskPriority,
                                  LPULONG lpulUnused1, LPULONG lpulUnused2, LPULONG lpulFreeStack,
                                  LPULONG lpulInternalTaskIdent);

/* Get the handle for the currently running task */
EXIMPORT RTK_TASK_H RtkGetCurrentTaskIdent();

/* Returns TRUE if the task is ready.
 * Returns FALSE if the task is not ready or if 'hTask' is invalid. */
EXIMPORT BOOLEAN RtkIsTaskReady(RTK_TASK_H hTask);

/* Returns TRUE if the task is suspended.
 * Returns FALSE if the task is not suspended or if 'hTask' is invalid. */
EXIMPORT BOOLEAN RtkIsTaskSuspended(RTK_TASK_H hTask);

#ifdef __cplusplus
}
#endif

#endif
