#include <bur/plctypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

	#include "StringExt.h"

#ifdef __cplusplus
};
#endif

unsigned long bur_heap_size = 0xFFFF; // Set heap size to maximum possible value (65535) to allow for dynamic memory allocation in the StringExt library.