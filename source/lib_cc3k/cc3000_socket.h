/*
 * cc3000_socket.h
 *
 *  Created on: 09.09.2013
 *      Author: Johannes
 */
#ifndef CC3000_SOCKET_H_
#define CC3000_SOCKET_H_

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_Socket_API
//! @{
//
//*****************************************************************************

// Socket domains / families
#define  AF_INET                2
#define  AF_INET6               23

// Socket type
#define  SOCK_STREAM            1
#define  SOCK_DGRAM             2
#define  SOCK_RAW               3           //!< Raw sockets allow new IPv4 protocols to be implemented in user space.
											//!< A raw socket receives or sends the raw datagram not including link level headers
#define  SOCK_RDM               4
#define  SOCK_SEQPACKET         5

// Socket protocol
#define IPPROTO_IP              0           //!< Dummy for IP
#define IPPROTO_ICMP            1           //!< Control Message Protocol
#define IPPROTO_IPV4            IPPROTO_IP  //!< IP inside IP
#define IPPROTO_TCP             6           //!< TCP
#define IPPROTO_UDP             17          //!< User Datagram Protocol
#define IPPROTO_IPV6            41          //!< IPv6 in IPv6
#define IPPROTO_NONE            59          //!< No next header
#define IPPROTO_RAW             255         //!< Raw IP packet

#define htons(a) a >> 8 | a << 8

// Socket options - must find better define names for these. SOCK_ON is not representative of what it does!
#define  SOL_SOCKET                0xFFFF	//!< Socket level
#define  SOCKOPT_RECV_NONBLOCK         	0	//!< Recv non block mode, set SOCK_ON or SOCK_OFF (default block mode)
#define  SOCKOPT_RECV_TIMEOUT			1	//!< Optname to configure recv and recvfromtimeout
#define  SOCKOPT_ACCEPT_NONBLOCK		2	//!< Accept non block mode, set SOCK_ON or SOCK_OFF (default block mode)
#define  SOCK_ON                		0	//!< Socket non-blocking mode	is enabled
#define  SOCK_OFF               		1	//!< Socket blocking mode is enabled

//! Returned in the send (0x1003) and recv (0x1004) events' Number of Bytes field
//!
//! Type Opcode Arg Stat Socket NumberBytes Flags(recv only)
//!  04  0X 10  XX   00   Byte  C7 FF FF FF 00 00 00 00
#define ERROR_SOCKET_INACTIVE			(-57)

struct in_addr
{
    uint32_t s_addr;                 //!< Load with inet_aton()
};

struct sockaddr_in
{
    int16_t				sin_family;   //!< Socket family, e.g. AF_INET, AF_INET6
    uint16_t			sin_port;     //!< Port, e.g. htons(3490)
    struct in_addr  	sin_addr;     //!< Address, see struct in_addr, above
	int8_t				sin_zero[8];  //!< Zero this if you want to, see 
									  //!< http://silviocesare.wordpress.com/2007/10/22/setting-sin_zero-to-0-in-struct-sockaddr_in/
};

struct socket_accept_params_t
{
    int32_t            	sd;
    int32_t            	status;
    struct sockaddr_in	socket_address;
};

typedef struct _sockaddr_t
{
    uint16_t    sa_family;
    uint8_t     sa_data[14];
} sockaddr;

typedef struct _bsd_accept_return_t
{
    int32_t				iSocketDescriptor;
    int32_t				iStatus;
    sockaddr			tSocketAddress;

} tBsdReturnParams;

typedef struct socket_select_params
{
    int32_t				status;
	uint32_t			read_sd;
	uint32_t			write_sd;
	uint32_t			ex_sd;
} tBsdSelectRecvParams;

typedef struct _bsd_getsockopt_return_t
{
	uint8_t				ucOptValue[4];
	int8_t				iStatus;
} tBsdGetSockOptReturnParams;

typedef struct _bsd_gethostbyname_return_t
{
    int32_t				retVal;
    int32_t				outputAddress;
} tBsdGethostbynameParams;

typedef struct _ip_addr_t
{
	uint8_t ip[4];
} ip_addr_t;

typedef uint32_t time_t;
typedef uint32_t suseconds_t;

typedef struct timeval timeval;

struct timeval
{
    time_t         tv_sec;                  /*!< seconds */
    suseconds_t    tv_usec;                 /*!< microseconds */
};

// The cc3000_fd_set member is required to be an array of longs.
typedef int32_t __fd_mask;
#define __cc3000_FD_SETSIZE      32
// It's easier to assume 8-bit bytes than to get CHAR_BIT.
#define __NFDBITS               (8 * sizeof (__fd_mask))
#define __FDELT(d)              ((d) / __NFDBITS)
#define __FDMASK(d)             ((__fd_mask) 1 << ((d) % __NFDBITS))

//! cc3000_fd_set for select and pselect.
typedef struct
{
    __fd_mask fds_bits[__cc3000_FD_SETSIZE / __NFDBITS];
	#define __FDS_BITS(set)        ((set)->fds_bits)
} cc3000_fd_set;

// We don't use `memset' because this would require a prototype and
//   the array isn't too big.
#define __cc3000_FD_ZERO(set)                                               \
  do																		\
  {																			\
    unsigned int __i;                                                       \
    cc3000_fd_set *__arr = (set);                                           \
    for (__i = 0; __i < sizeof (cc3000_fd_set) / sizeof (__fd_mask); ++__i) \
      __FDS_BITS (__arr)[__i] = 0;                                          \
  } while (0)


#define __cc3000_FD_SET(d, set)       (__FDS_BITS (set)[__FDELT (d)] |= __FDMASK (d))
#define __cc3000_FD_CLR(d, set)       (__FDS_BITS (set)[__FDELT (d)] &= ~__FDMASK (d))
#define __cc3000_FD_ISSET(d, set)     (__FDS_BITS (set)[__FDELT (d)] & __FDMASK (d))

// Access macros for 'cc3000_fd_set'.
#define cc3000_FD_SET(fd, fdsetp)      __cc3000_FD_SET (fd, fdsetp)
#define cc3000_FD_CLR(fd, fdsetp)      __cc3000_FD_CLR (fd, fdsetp)
#define cc3000_FD_ISSET(fd, fdsetp)    __cc3000_FD_ISSET (fd, fdsetp)
#define cc3000_FD_ZERO(fdsetp)         __cc3000_FD_ZERO (fdsetp)

typedef uint32_t socklen_t;

#define MDNS_DEVICE_SERVICE_MAX_LENGTH 32
#define SOCKET_STATUS_ACTIVE           0
#define SOCKET_STATUS_INACTIVE         1

/* Init socket_active_status = 'all ones': init all sockets with SOCKET_STATUS_INACTIVE.
   Will be changed by 'set_socket_active_status' upon 'connect' and 'accept' calls */
#define SOCKET_STATUS_INIT_VAL  0xFFFF
#define M_IS_VALID_SD(sd) ((0 <= (sd)) && ((sd) <= 7))
#define M_IS_VALID_STATUS(status) (((status) == SOCKET_STATUS_ACTIVE)||((status) == SOCKET_STATUS_INACTIVE))

#define ASIC_ADDR_LEN          8
#define SELECT_TIMEOUT_MIN_MICRO_SECONDS  5000

//*****************************************************************************
//
//! cc3000_req_socket
//!
//!  @param  	domain 		Only AF_INET is supported
//!	 @param		type		Socket type
//!  @param		protocol	Socket protocol (TCP or UDP etc.)
//!
//!  @return  	none
//!
//!  @brief  	Request a socket
//!
//
//*****************************************************************************
extern void cc3000_req_socket(uint8_t domain, uint8_t type, uint8_t protocol);

//*****************************************************************************
//
//! cc3000_connect
//!
//!  @param  	sd 			Socket handle
//!	 @param		port		Port to connect to
//!  @param		addr		Address of the server
//!
//!  @return  	none
//!
//!  @brief  	Connect to a socket
//!
//
//*****************************************************************************
extern void cc3000_connect(uint8_t sd, uint16_t port, ip_addr_t addr);

//*****************************************************************************
//
//! cc3000_req_accept
//!
//!  @param  	sd 			Socket handle
//!
//!  @return  	none
//!
//!  @brief  	Request an accept for a connection on a socket
//!
//
//*****************************************************************************
extern void cc3000_req_accept(uint8_t sd);

//*****************************************************************************
//
//! cc3000_get_accept
//!
//!  @param		sd			Socket handle
//!	 @param		addr		Address of the peer socket
//!  @param		addrlen		Size of socket address
//!
//!  @return	For socket in blocking mode:
//!					On success, socket handle. on failure negative
//!				For socket in non-blocking mode:
//!					- On connection establishment, socket handle
//!					- On connection pending, SOC_IN_PROGRESS (-2)
//!					- On failure, SOC_ERROR	(-1)
//!
//!  @brief		Get the data of the connected client
//!
//
//*****************************************************************************
extern int32_t cc3000_get_accept(uint8_t sd, sockaddr *addr, socklen_t *addrlen);

//*****************************************************************************
//
//! cc3000_listen
//!
//!  @param  	sd 			Socket handle
//!
//!  @return  	none
//!
//!  @brief  	Listen for connections on a socket
//!
//
//*****************************************************************************
extern void cc3000_listen(uint8_t sd);

//*****************************************************************************
//
//! cc3000_bind
//!
//!  @param  	sd 			Socket handle
//!  @param  	port 		Socket port
//!
//!  @return  	none
//!
//!  @brief  	Assign a name to a socket
//!
//
//*****************************************************************************
extern void cc3000_bind(uint8_t sd, uint16_t port);

//*****************************************************************************
//
//! cc3000_setsockopt
//!
//!  @param		sd          Socket handle
//!  @param		level       Defines the protocol level for this option
//!  @param		optname     Defines the option name to Interrogate
//!  @param		optval      Specifies a value for the option
//!  @param		optlen      Specifies the length of the option value
//!
//!  @return	none
//!
//!  @brief  Set socket options
//
//*****************************************************************************
extern void cc3000_setsockopt(uint8_t sd, uint32_t level, uint32_t optname, const void *optval, socklen_t optlen);

//*****************************************************************************
//
//! cc3000_req_select
//!
//!  @param		nfds       The highest-numbered file descriptor in any of the
//!                        three sets, plus 1.
//!  @param		writesds   Socket descriptors list for write monitoring
//!  @param		readsds    Socket descriptors list for read monitoring
//!  @param		exceptsds  Socket descriptors list for exception monitoring
//!  @param		timeout    Is an upper bound on the amount of time elapsed
//!                        before select() returns. Null means infinity
//!                        timeout. The minimum timeout is 5 milliseconds,
//!                        less than 5 milliseconds will be set
//!                        automatically to 5 milliseconds.
//!
//!  @return	none
//!
//!  @brief  Request monitoring of the socket
//
//*****************************************************************************
extern void cc3000_req_select(long nfds, cc3000_fd_set *readsds, cc3000_fd_set *writesds, cc3000_fd_set *exceptsds, struct timeval *timeout);

//*****************************************************************************
//
//! cc3000_get_select
//!
//!  @param   	writesds   Socket descriptors list for write monitoring
//!  @param   	readsds    Socket descriptors list for read monitoring
//!  @param   	exceptsds  Socket descriptors list for exception monitoring
//!
//!  return		On success, cc3000_get_select() returns the number of file descriptors
//!             contained in the three returned descriptor sets (that is, the
//!             total number of bits that are set in readfds, writefds,
//!             exceptfds) which may be zero if the timeout expires before
//!             anything interesting  happens.
//!             On error, -1 is returned.
//!
//!  @brief  Get status of the socket
//
//*****************************************************************************
extern int8_t cc3000_get_select(cc3000_fd_set *readsds, cc3000_fd_set *writesds, cc3000_fd_set *exceptsds);

//*****************************************************************************
//
//!  cc3000_send
//!
//!  @param sd       Socket handle
//!  @param buf      Points to a buffer containing the message to be sent
//!  @param len      Message size in bytes
//!  @param flags    On this version, this parameter is not supported
//!
//!  @return         If executed returns 1, or -2 if an
//!                  error occurred
//!
//!  @brief          Write data to TCP socket.
//!                  This function is used to transmit a message to another
//!                  socket.
//!
//
//*****************************************************************************
extern int8_t cc3000_send(uint8_t sd, uint8_t *buf, uint16_t len, uint8_t flags);

//*****************************************************************************
//
//!  cc3000_send_circ_buffer
//!
//!  @param sd       Socket handle
//!  @param buf      Points to a buffer containing the message to be sent.
//!					 Buffer length must be a multiple of two.
//!  @param mask     Length of buffer - 1
//!  @param head     Head of data
//!  @param tail     Tail of data
//!
//!  @return         If executed returns 1, or -2 if an
//!                  error occurred
//!
//!  @brief          Write data to TCP socket.
//!                  This function is used to transmit a message to another
//!                  socket. The buffer is a circular buffer.
//!
//
//*****************************************************************************
extern int8_t cc3000_send_circ_buffer(uint8_t sd, uint8_t *buf, txint_t mask, txint_t head, txint_t tail);

//*****************************************************************************
//
//!  cc3000_req_recv
//!
//!  @param  	sd      Socket handle
//!  @param 	buf     Points to the buffer where the message should be stored
//!  @param  	len     Specifies the length in bytes of the buffer pointed to
//!                     by the buffer argument.
//!  @param 	flags   Specifies the type of message reception.
//!                     On this version, this parameter is not supported.
//!
//!  @return         none
//!
//!  @brief          Function requests a message from a connection-mode socket
//!
//
//*****************************************************************************
extern void cc3000_req_recv(uint8_t sd, uint16_t len, uint8_t flags);

//*****************************************************************************
//
//!  cc3000_get_recv_bytes
//!
//!  @param  	none
//!
//!  @return    Returns number of bytes received
//!
//!  @brief     Gets the number of bytes received
//!
//
//*****************************************************************************
extern int16_t cc3000_get_recv_bytes(void);

//*****************************************************************************
//
//!  cc3000_get_recv_data
//!
//!  @param  	buf		Pointer to receive buffer
//!
//!  @param  	len		Number of bytes to receive - given by cc3000_get_recv_bytes
//!
//!  @return    none
//!
//!  @brief     Gets the received data and stores it in buffer
//!
//
//*****************************************************************************
extern void cc3000_get_recv_data(uint8_t *buf, uint8_t len);

//*****************************************************************************
//
//! cc3000_socket_close
//!
//!  @param  sd    Socket handle.
//!
//!  @return  none
//!
//!  @brief  The socket function closes a created socket.
//
//*****************************************************************************
extern void cc3000_socket_close(uint8_t sd);

//*****************************************************************************
//
//!  mdnsAdvertiser
//!
//!  @param[in] mdnsEnabled         Flag to enable/disable the mDNS feature
//!  @param[in] deviceServiceName   Service name as part of the published
//!                                 canonical domain name
//!
//!
//!  @return	On success, zero is returned, return SOC_ERROR if socket was not
//!				opened successfully, or if an error occurred.
//!
//!  @brief		Set CC3000 in mDNS advertiser mode in order to advertise itself.
//!				NOTE: This routine has not been tested
//
//*****************************************************************************
extern int8_t cc3000_socket_mdns_advertise(uint8_t mdns_enabled, uint8_t *service_name);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_SOCKET_H_ */
