
/* Functions to set and read the real-time-clock */

#ifndef _RTK_RTC_H_
#define _RTK_RTC_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Set the real-time-clock to 'newDateTime'.
 * 'newDateTime' has to contain the localtime (not the UTC time).
 */
EXIMPORT RTK_ERROR RtkSetRealTimeClock(LPRTK_RTC newDateTime);

/*
 * Retrieves the current date and localtime.
 */
EXIMPORT void RtkGetRealTimeClock(LPRTK_RTC dateTime);

#ifdef __cplusplus
}
#endif

#endif
