
/* RTK memory functions */

#ifndef _RTK_MEMORY_H_
#define _RTK_MEMORY_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory access flags */
typedef unsigned long RTK_MEMORY_ACCESS;
#define RTK_MEM_NO_READ 0x00000000       /* ignored */
#define RTK_MEM_NO_WRITE 0x00000000      /* ignored */
#define RTK_MEM_NOT_EXECUTE 0x00000000   /* ignored */
#define RTK_MEM_READ_ALL 0x00000001      /* ignored */
#define RTK_MEM_WRITE_ALL 0x00000002     /* ignored */
#define RTK_MEM_EXECUTE_ALL 0x00000004   /* ignored */
#define RTK_MEM_READ_LOCAL 0x00000008    /* ignored */
#define RTK_MEM_WRITE_LOCAL 0x00000010   /* ignored */
#define RTK_MEM_EXECUTE_LOCAL 0x00000020 /* ignored */
#define RTK_MEM_NO_INIT 0x00000000       /* no memory initialization */
#define RTK_MEM_INITZERO 0x00008000      /* memory initialization with 0 */

/* Allocates heap memory.
 * Depending on the AccessFlags, the memory will be initialized with 0 (RTK_MEM_INITZERO) or it will be not initialized
 * (RTK_MEM_NO_INIT). After the function call lpMemoryAddress (which must not be NULL) contains the pointer to the
 * allocated memory */
EXIMPORT RTK_ERROR RtkAllocateMemory(ULONG ulByteLength, RTK_MEMORY_ACCESS AccessFlags, LPVOID* lpMemoryAddress);

/* Frees memory allocated with RtkAllocateMemory.
 * lpMemoryaddress is the pointer to the memory to be freed */
EXIMPORT RTK_ERROR RtkFreeMemory(LPVOID lpMemoryAddress, ULONG ulUnused);

/* Retrieves information about the DRAM
 * lpulMemSize................. Output-parameter - can be NULL. Returns the overall RAM size in byte.
 * lpulFreeMemSize............. Output-parameter - can be NULL. Returns the available/free RAM size in byte.
 * lpulBiggestFreeBlockSize.... Output-parameter - can be NULL. Returns the  biggest available/free consecutive
 *                                                              RAM block in byte. */
EXIMPORT RTK_ERROR RtkGetMemoryInfo(LPULONG lpulMemSize, LPULONG lpulFreeMemSize, LPULONG lpulBiggestFreeBlockSize);

#ifdef __cplusplus
}
#endif

#endif
