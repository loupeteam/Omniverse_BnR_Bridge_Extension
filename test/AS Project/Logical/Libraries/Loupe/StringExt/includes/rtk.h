
/* This API provides abstractions for OS-functionalities (semaphores, tasks, timers, ...).
 * Instead of using native Windows or VxWorks functions, you should use this API.
 * All functions returning RTK_ERROR return ERR_OK on success and other error codes on failure */

#ifndef _RTK_H_
#define _RTK_H_

#include <rtkBase.h>
#include <rtkTask.h>
#include <rtkCriticalSection.h>
#include <rtkSemaphore.h>
#include <rtkMemory.h>
#include <rtkTimer.h>
#include <rtkFifo.h>
#include <rtkPlainFifo.h>
#include <rtkRtc.h>
#include <rtkObject.h>
#include <rtkMisc.h>

#if AR_CORE
#include <private/rtkP.h>
#endif

#endif
