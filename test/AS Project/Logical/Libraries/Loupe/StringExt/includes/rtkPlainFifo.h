
/* RTK Plain FIFO
 * Rtk plain FIFO is a simple ringbuffer which does not use any locking.
 * It is possible to have one task reading and one task writing to the FIFO without requiring any further locking.
 * If multiple tasks read or if multiple tasks write to the FIFO, then external locking is required.			
 */

#ifndef _RTK_PLAIN_FIFO_H_
#define _RTK_PLAIN_FIFO_H_

#include <rtkBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef RTK_H RTK_PLAIN_FIFO_H;

/* Creates a FIFO with the provided name (lpszFifoName must be unique or NULL) */
/* the FIFO provides space for ulFifoMaxEntries with ulFifoEntrySize.
 * A handle to the FIFO is returned as output-parameter (hFifo - must not be NULL) */
EXIMPORT RTK_ERROR
RtkCreatePlainFifo(LPSTRING lpszFifoName, ULONG ulFifoEntrySize, ULONG ulFifoMaxEntries, RTK_PLAIN_FIFO_H* hFifo);

/* Deletes a plain FIFO*/
EXIMPORT RTK_ERROR RtkDeletePlainFifo(RTK_PLAIN_FIFO_H hFifo);

/* Writes data to the FIFO.
 * The size of the data is the size provided with ulFifoEntrySize when creating the FIFO. If the FIFO is full (it only
 * contains entries which were not yet read), then this function returns ERR_RTK_FIFO_WRITE */
EXIMPORT RTK_ERROR RtkWritePlainFifo(RTK_PLAIN_FIFO_H hFifo, LPVOID lpDataSource);

/* Copies data from the FIFO to lpDataDestination (must not be NULL).
 * The size of the copied data is the size provided with ulFifoEntrySize when creating the  FIFO.
 * Returns ERR_RTK_FIFO_READ if the FIFO is empty */
EXIMPORT RTK_ERROR RtkReadPlainFifo(RTK_PLAIN_FIFO_H hFifo, LPVOID lpDataDestination);

/* Deletes all data contained in the FIFO
 * This function requires external locking if another task calls RtkReadPlainFifo */
EXIMPORT RTK_ERROR RtkClearPlainFifo(RTK_PLAIN_FIFO_H hFifo);

/* Obtains information about the FIFO
 * 
 * hFifo.............. Handle to the FIFO for which the information shall be retrieved
 * lpulFifoFree....... Output-parameter - can be NULL. Returns the amount of free FIFO entries
 * lpulFifoUsed....... Output-parameter - can be NULL. Returns the amount of occupied FIFO entries
 * lpulFifoEntrySize.. Output-parameter - can be NULL. Returns the size of one FIFO entry in byte */
EXIMPORT RTK_ERROR
RtkGetPlainFifoInfo(RTK_PLAIN_FIFO_H hFifo, LPULONG lpulFifoFree, LPULONG lpulFifoUsed, LPULONG lpulFifoEntrySize);

#ifdef __cplusplus
}
#endif

#endif
