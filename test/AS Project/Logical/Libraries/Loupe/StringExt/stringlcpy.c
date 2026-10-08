#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "StringExt.h"
#ifdef __cplusplus
	};
#endif

#include "string.h"

/*
 * Copy string pSrc to buffer dst of size dSize.  At most dSize-1
 * chars will be copied.  Always NUL terminates (unless dSize == 0).
 * Returns strlen(pSrc); if retval >= dSize, truncation occurred.
 */
unsigned long stringlcpy(unsigned long pDest, const unsigned long pSrc, unsigned long dSize)
{
	// We have access to this funtion but it does not appear in autocomplete or in AS help
	// This is a wrapper
	return strlcpy((char*) pDest, (char*) pSrc, dSize);
}
