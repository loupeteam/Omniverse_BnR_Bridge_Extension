
/* global error status value definitions and a function to read the global error status value */


#ifndef _AR_ERRNO_H_
#define _AR_ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* AR error codes corresponding to POSIX Error codes */
#define AR_EPERM				1			/* Not owner */
#define AR_ENOENT				2			/* No such file or directory */
#define AR_ESRCH				3			/* No such process */
#define AR_EINTR				4			/* Interrupted system call */
#define AR_EIO					5			/* I/O error */
#define AR_ENXIO				6			/* No such device or address */
#define AR_E2BIG				7			/* Arg list too long */
#define AR_ENOEXEC				8			/* Exec format error */
#define AR_EBADF				9			/* Bad file number */
#define AR_ECHILD				10			/* No children */
#define AR_EAGAIN				11			/* No more processes */
#define AR_ENOMEM				12			/* Not enough core */
#define AR_EACCESS				13			/* Permission denied */
#define AR_EFAULT				14			/* Bad address */
#define AR_ENOTEMPTY			15			/* Directory not empty */
#define AR_EBUSY				16			/* Mount device busy */
#define AR_EEXIST				17			/* File exists */
#define AR_EXDEV				18			/* Cross-device link */
#define AR_ENODEV				19			/* No such device */
#define AR_ENOTDIR				20			/* Not a directory*/
#define AR_EISDIR				21			/* Is a directory */
#define AR_EINVAL				22			/* Invalid argument */
#define AR_ENFILE				23			/* File table overflow */
#define AR_EMFILE				24			/* Too many open files */
#define AR_ENOTTY				25			/* Not a typewriter */
#define AR_ENAMETOOLONG			26			/* File name too long */
#define AR_EFBIG				27			/* File too large */
#define AR_ENOSPC				28			/* No space left on device */
#define AR_ESPIPE				29			/* Illegal seek */
#define AR_EROFS				30			/* Read-only file system */
#define AR_EMLINK				31			/* Too many links */
#define AR_EPIPE				32			/* Broken pipe */
#define AR_EDEADLK				33			/* Resource deadlock avoided */
#define AR_ENOLCK				34			/* No locks available */
#define AR_ENOTSUP				35			/* Unsupported value */
#define AR_EMSGSIZE				36			/* Message size */

/* ANSI math software */
#define AR_EDOM					37			/* Argument too large */
#define AR_ERANGE				38			/* Result too large */

/* ipc/network software */

/* argument errors */
#define AR_EDESTADDRREQ			40			/* Destination address required */
#define AR_EPROTOTYPE			41			/* Protocol wrong type for socket */
#define AR_ENOPROTOOPT			42			/* Protocol not available */
#define AR_EPROTONOSUPPORT		43			/* Protocol not supported */
#define AR_ESOCKTNOSUPPORT		44			/* Socket type not supported */
#define AR_EOPNOTSUPP			45			/* Operation not supported on socket */
#define AR_EPFNOSUPPORT			46			/* Protocol family not supported */
#define AR_EAFNOSUPPORT			47			/* Addr family not supported */
#define AR_EADDRINUSE			48			/* Address already in use */
#define AR_EADDRNOTAVAIL		49			/* Can't assign requested address */
#define AR_ENOTSOCK				50			/* Socket operation on non-socket */

/* operational errors */
#define AR_ENETUNREACH			51			/* Network is unreachable */
#define AR_ENETRESET			52			/* Network dropped connection on reset*/
#define AR_ECONNABORTED			53			/* Software caused connection abort */
#define AR_ECONNRESET			54			/* Connection reset by peer */
#define AR_ENOBUFS				55			/* No buffer space available */
#define AR_EISCONN				56			/* Socket is already connected */
#define AR_ENOTCONN				57			/* Socket is not connected */
#define AR_ESHUTDOWN			58			/* Can't send after socket shutdown */
#define AR_ETOOMANYREFS			59			/* Too many references: can't splice */
#define AR_ETIMEDOUT			60			/* Connection timed out */
#define AR_ECONNREFUSED			61			/* Connection refused */
#define AR_ENETDOWN				62			/* Network is down */
#define AR_ETXTBSY				63			/* Text file busy */
#define AR_ELOOP				64			/* Too many levels of symbolic links */
#define AR_EHOSTUNREACH			65			/* No route to host */
#define AR_ENOTBLK				66			/* Block device required */
#define AR_EHOSTDOWN			67			/* Host is down */

/* non-blocking and interrupt i/o */
#define AR_EINPROGRESS			68			/* Operation now in progress */
#define AR_EALREADY				69			/* Operation already in progress */
#define AR_EWOULDBLOCK			70			/* Operation would block */

#define AR_ENOSYS				71			/* Function not implemented */

/* aio errors (should be under posix) */
#define AR_ECANCELED			72			/* Operation canceled */

/* specific STREAMS errno values */
#define AR_ENOSR           		74      	/* Insufficient memory */
#define AR_ENOSTR          		75      	/* STREAMS device required */
#define AR_EPROTO          		76      	/* Generic STREAMS error */
#define AR_EBADMSG         		77      	/* Invalid STREAMS message */
#define AR_ENODATA         		78      	/* Missing expected message data */
#define AR_ETIME           		79      	/* STREAMS timeout occurred */
#define AR_ENOMSG          		80      	/* Unexpected message type */ 

#define AR_EFPOS				81			/* File positioning error */
#define AR_EILSEQ				82			/* Illegal byte sequence. */

/* error codes used by network stack */
#define AR_EDQUOT          		83   		/* routing error */

#define AR_EIDRM				84			/* Identifier removed */
#define AR_EOVERFLOW			85			/* Value too large for data type */
#define AR_EMULTIHOP			86			/* Reserved */
#define AR_ENOLINK				87			/* Reserved */
#define AR_ESTALE				88			/* Reserved */

#define AR_EFIREWALL       		1006 		/* the communication was blocked by the AR firewall. This is not a POSIX error code */

#define AR_EUNKNOWN				0xFFFFFF

/* Get the global error status value of the calling task.
 * For ARsim only socket errors are currently handled properly.  */
int arerrnoGet(void);

#ifdef __cplusplus
}
#endif

#endif
