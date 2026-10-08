
/* AR select functions
   The syntax and behavior of these functions is equal to the POSIX IPv4 select function */


#ifndef _AR_SELECT_H_
#define _AR_SELECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct AR_FD_SET* AR_FD_SET_H;

/* Allocate internal resources for a set of file descriptors */
AR_FD_SET_H arFD_ALLOC(void);

/* Cleanup internal resources allocated with arFD_ALLOC */
void arFD_FREE(AR_FD_SET_H set);

/* Test if file descriptor "fd" is marked in the provided "set" of file descriptors.
 * Returns a non zero value if the file descriptor is marked in the set of file descriptors, otherwise 0 */
long arFD_ISSET(int fd, AR_FD_SET_H set);

/* Mark file descriptor "fd" in the "set" to be observed */
void arFD_SET(int fd, AR_FD_SET_H set);

/* Unmark file descriptor "fd" in the "set" */
void arFD_CLR(int fd, AR_FD_SET_H set);

/* Unmark an enire "set" of file descriptors */
void arFD_ZERO(AR_FD_SET_H set);

/* Get the maximum width to be set for arselect */
int arFD_SIZE(void);

typedef struct AR_TIMEVAL
{
	int32_t tv_sec;
	int32_t tv_usec;
} AR_TIMEVAL;

/* Pends until one of the observed file descriptors becomes active (ready to read, ready to write, or exception occurred).
 * The function pends until "pTimeOut" (NULL = pend forever).
 *
 * pReadFds, pWriteFds, pExceptFds contain the list of observed file descriptors. They have to be provided with an object allocated
 * with arFD_ALLOC or they can be NULL. All in the object marked file descriptors (marked with arFD_SET) will be observed.
 * After the function returns, these objects contain the information about the active file descriptors. This information can
 * be querried with arFD_ISSET.
 *
 * The "width" parameter can be used to limit the observed file descriptors to only observe file descriptors 0 to width-1
 * (to not observe them eventhough they are in pReadFds, pWriteFds, or pExceptFds). Usually the "width" parameter is not used
 * to limit the set of file descriptors and is thus simply set to the value of the observed file descriptor plus one or to arFD_SIZE.
 *
 * Returns the number of active file descriptors, 0 if timed out, -1 on error 
 *
 * Usage example:
 *   ARFD_SET_H fdSet = arFD_ALLOC();
 *   arFD_SET(fdToCheck, fdSet);
 *   arselect(arFD_SIZE(), fdSet, NULL, NULL, 0);
 *   isActive = arFD_ISSET(fdToCheck, fdSet);
 *   arFD_FREE(fdSet);
 */
int arselect(int width, AR_FD_SET_H pReadFds, AR_FD_SET_H pWriteFds, AR_FD_SET_H pExceptFds, AR_TIMEVAL* pTimeOut);

#ifdef __cplusplus
}
#endif

#endif
