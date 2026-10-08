
/* AR socket functions
   The syntax and behavior of these functions is equal to POSIX IPv4 sockets */


#ifndef _AR_SOCKET_H_
#define _AR_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <arInternetTypes.h>			

/* IOCTL used for arioctl */
#define AR_FIONREAD				1			/* get num chars available to read */
#define AR_FIONBIO				16			/* set/unset socket non-blocking */

/* Types used for arsocket */
#define AR_SOCK_STREAM			1			/* stream socket */
#define AR_SOCK_DGRAM			2			/* datagram socket */
#define AR_SOCK_RAW				3			/* raw-protocol interface */

/* Options for getsockopt/setsockopt - TCP level. */
#define AR_TCP_NODELAY			1			/* don't delay send to coalesce packets */
#define AR_TCP_KEEPIDLE     	4			/* int; Send first keepalive probe when the connections been isdl this time (in seconds) */
#define AR_TCP_KEEPINTVL		5			/* int; Interval (in seconds) between keepalives */
#define AR_TCP_KEEPCNT			6			/* int; Maximum number of keepalives before dropping the connection */

/* Options for getsockopt/setsockopt - flags per-socket. */
#define AR_SO_REUSEADDR			0x0004		/* allow local address reuse */
#define AR_SO_KEEPALIVE			0x0008		/* keep connections alive */
#define AR_SO_BROADCAST			0x0020		/* permit sending of broadcast msgs */
#define AR_SO_LINGER			0x0080		/* linger on close if data present */
#define AR_SO_OOBINLINE			0x0100		/* leave received OOB data in line */
#define AR_SO_REUSEPORT			0x0200		/* allow local address & port reuse */

/* Options for getsockopt/setsockopt - additional options */
#define AR_SO_SNDBUF			0x1001		/* send buffer size */
#define AR_SO_RCVBUF			0x1002		/* receive buffer size */
#define AR_SO_ERROR				0x1007		/* get error status and clear */
#define AR_SO_BINDTODEVICE 		0x1010		/* bind to device */

/* shutodwn options used for arshutdown */
#define AR_SHUT_RD				0			/* shutdown incoming network connection on socket */
#define AR_SHUT_WR				1			/* shutdown outgoing network connection on socket */
#define AR_SHUT_RDWR			2			/* shutdown incoming and outgoing network connection on socket */

/* Level number for getsockopt/setsockopt to apply to socket itself. */
#define AR_SOL_SOCKET			0xffff		/* options for socket level */

/* used for the "flags" parameter of the following functions:
 * arsendto arrecvfrom, arsend, arrecv */
#define AR_MSG_OOB				0x1			/* process out-of-band data */
#define AR_MSG_PEEK				0x2			/* peek at incoming message */
#define AR_MSG_DONTROUTE		0x4			/* send without using routing tables */

/* Options for getsockopt() and setsockopt() at the IP level.
 * First word of the comment is the data type; bool is stored in int */
#define	AR_IP_TOS				3 			/* int; IP type of service and preced. */
#define	AR_IP_TTL				4 			/* int; IP time to live */

#define	AR_IP_MULTICAST_IF		9 			/* u_char; set/get IP multicast i/f  */
#define	AR_IP_MULTICAST_TTL		10			/* u_char; set/get IP multicast ttl */
#define	AR_IP_MULTICAST_LOOP	11			/* u_char; set/get IP multicast loopback */
#define	AR_IP_ADD_MEMBERSHIP	12			/* AR_IP_MREQ; add an IP group membership */
#define	AR_IP_DROP_MEMBERSHIP	13			/* AR_IP_MREQ; drop an IP group membership */

/* Argument structure for AR_IP_ADD_MEMBERSHIP and AR_IP_DROP_MEMBERSHIP. */
typedef struct AR_IP_MREQ
{
	AR_IN_ADDR imr_multiaddr;	/* IP multicast address of group */
	AR_IN_ADDR imr_interface;	/* local IP address of interface */
}AR_IP_MREQ;

/* Structure used for AR_SO_LINGER. */
typedef struct AR_LINGER
{
	int32_t l_onoff;  			/* option on/off */
	int32_t l_linger; 			/* linger time */
} AR_LINGER;

/* IPv4 address struct.
 * Required for the "addr" parameter of the following functions:
 * arsentto, arrecvfrom, arbind, arconnect, araccept, argetsockname */
typedef struct AR_SOCKADDR_IN
{
	int16_t sin_family;
	uint16_t sin_port;
	AR_IN_ADDR sin_addr;
	char sin_zero[8];
} AR_SOCKADDR_IN;

/* Open a new socket. Sockets are usually created using the domain AR_AF_INET and the protocol AR_IPPROTO_IP.
 * The type parameter specifies whether the socket is raw (AR_SOCK_RAW), streaming (AR_SOCK_STREAM) or
 * datagram (AR_SOCK_DGRAM).
 * Returns the socket descriptor on success, -1 on error */
int arsocket(int domain, int type, int protocol);

/* Send a datagram message (with content "buf" and length "bufLen") via socket "s" to a peer.
 * The peer is identified by "addrTo" which takes a pointer to a AR_SOCKADDR_IN structure. "addrLen"
 * is the size of the provided structure. "flags" is the bitwise OR of AR_MSG_xxx parameters.
 * Returns the number of byte sent, or -1 on error */
int arsendto(int s, const char* buf, int bufLen, int flags, void* addrTo, int addrLen);

/* Receive a datagram message via socket "s" from a peer. Information about the peer is available
 * after the function call via the "addrFrom" (pointer to a AR_SOCKADDR_IN) and the "addrLen"
 * (size of addrFrom type) parameters.  "flags" is the bitwise OR of AR_MSG_xxx parameters.
 * Returns the number of byte received, or -1 on error */
int arrecvfrom(int s, char* buf, int bufLen, int flags, void* addrFrom, int* addrLen);

/* Performs an I/O control function for socket "s"
 * Possible commands "cmd" are AR_FIOxxx
 * Returns 0 on success, else error */
int arioctl(int s, int cmd, int arg);

/* Close the socket "s" 
 * Returns 0 on success, else error */
int arclose(int s);

/* Send data (with content "buf" and length "bufLen") to a previously established connection-based
 * socket "s" (streaming, sequential packets). "flags" is the bitwise OR of AR_MSG_xxx parameters.
 * Returns the number of byte sent, or -1 on error */
int arsend(int s, const char* buf, int bufLen, int flags);

/* Receive data from a connection-based (stream) socket "s".
 * The function has to be provided a buffer "buf" and the buffer length "bufLen". The received
 * data is copied to this buffer. 
 * "flags" is the bitwise OR of AR_MSG_xxx parameters.
 * Returns the number of byte received, or -1 on error */ 
int arrecv(int s, char* buf, int bufLen, int flags);

/* Binds socket "s" to an address ("addrName" as pointer to AR_SOCKADDR_IN and "addrLen" as its length).
 * When a socket is bound to an address, then then only messages sent/received on this
 * address (network interface, port, ...) are handled by this socket. 
 * Returns 0 on success, or -1 on error */
int arbind(int s, void* addrName, int addrLen);

/* Initiate a connection via socket "s" to a peer ("addrName" as pointer to AR_SOCKADDR_IN and "addrLen" as its length).
 * Returns 0 on success, or -1 on error */
int arconnect(int s, void* addrName, int addrLen);

/* Accept an incoming connection (which was initiated by the peer with arconnect).
 * "s" is a socket listening for incoming connections (arlisten).
 * "addr" has to contain a pointer to AR_SOCKADDR_IN and addrLen has to contain its length.
 * After the function call "addr" contains information about the peer which initiated the connection.
 * Returns a new socket descriptor created for the new connection, or -1 on error */
int araccept(int s, void* addr, int* addrLen);

/* Listen for incoming connections on socket "s". The maximum number of pending incoming connections
 * (connections not yet handled by araccept) is set by "backlog".
 * Returns 0 on success, or -1 on error */
int arlisten(int s, int backlog);

/* Shutdown a network connection on socket "s". "how" specifies which kind of traffic is shut down:
 * incoming (AR_SHUT_RD), outgoing (AR_SHUT_WR) or both (AR_SHUT_BOTH).
 * Returns 0 on success, or -1 on error */
int arshutdown(int s, int how);

/* Get the address to which the socket "s" is bound.
 * "addr" has to contain a pointer to AR_SOCKADDR_IN and addrLen has to contain its length.
 * After the function call "addr" contains information about the address.
 * Returns 0 on success, or -1 on error
 */
int argetsockname(int s, void* addrName, int* addrLen);

/* Set an option for socket "s". 
 * "optName" specifies the kind of option to be set (AR_IP_xxx)
 * "optVal" contains the data with length "optLen" required for that kind of option
 * "level" contains AR_SOL_SOCKET or AR_IPPROTO_xxx
 * Returns 0 on success, or -1 on error */
int arsetsockopt(int s, int level, int optName, const void* optVal, int optLen);

/* Get an option value associated with socket "s". 
 * "optName" specifies the kind of option to be read (AR_IP_xxx)
 * "level" contains AR_SOL_SOCKET or AR_IPPROTO_xxx
 * After the call, "optVal" contains the the data for the read option (with length "optLen").
 * Returns 0 on success, or -1 on error  */
int argetsockopt(int s, int level, int optName, void* optVal, int* optLen);

#ifdef __cplusplus
}
#endif

#endif
