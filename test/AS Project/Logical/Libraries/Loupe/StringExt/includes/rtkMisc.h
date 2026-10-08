
/* Miscellaneous RTK functions */

#ifndef _RTK_MISC_H_
#define _RTK_MISC_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reads the current value of a microsecond counter.
 * Can be used for time measurements.
 * Overflow from 0xFFFFFFFF to 0x00000000 */
EXIMPORT ULONG RtkReadUsecCounter(void);

/* Returns the configured system tick */
EXIMPORT ULONG RtkGetSystemTick();

#ifdef __cplusplus
}
#endif

#endif
