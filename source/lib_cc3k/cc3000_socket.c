/*
 * cc3000_socket.c
 *
 *  Created on: 09.09.2013
 *      Author: Johannes
 */

// ToDo: gethostbyname,getsockopt,

#include "cc3000_socket.h"
#include "cc3000_spi.h"
#include "cc3000_platform.h"
#include "cc3000_event_handler.h"
#include "cc3000_general.h"
#include "cc3000_hci.h"

#include <string.h>

typedef union
{
	uint8_t		uint8[4];
	uint16_t	uint16[2];
	int16_t		int16[2];
	uint32_t	uint32;
	int32_t		int32;
} long_t;

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
void cc3000_req_socket(uint8_t domain, uint8_t type, uint8_t protocol)
{
	cc3000_hci_start_command(HCI_CMND_SOCKET, 3*4);
	cc3000_hci_send_uint32(domain);
	cc3000_hci_send_uint32(type);
	cc3000_hci_send_uint32(protocol);
	cc3000_hci_end_command();
} // cc3000_req_socket

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
void cc3000_connect(uint8_t sd, uint16_t port, ip_addr_t addr)
{
	long_t addr32;

	addr32.uint8[0] = addr.ip[0];
	addr32.uint8[1] = addr.ip[1];
	addr32.uint8[2] = addr.ip[2];
	addr32.uint8[3] = addr.ip[3];

	cc3000_hci_start_command(HCI_CMND_CONNECT, 20);
    cc3000_hci_send_uint32(sd);
	cc3000_hci_send_uint32(0x00000008);		// magic
	cc3000_hci_send_uint32(8);				// addrlen
	cc3000_hci_send_uint16(AF_INET); 		// stSockAddr.sin_framily
	cc3000_hci_send_uint16(htons(port));	// stSockAddr.sin_port
	cc3000_hci_send_uint32(addr32.uint32);	// stSockAddr.sin_addr
	cc3000_hci_end_command();
} // cc3000_connect

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
void cc3000_req_accept(uint8_t sd)
{
	cc3000_hci_start_command(HCI_CMND_ACCEPT,4);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_end_command();
} // cc3000_req_accept

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
int32_t cc3000_get_accept(uint8_t sd, sockaddr *addr, socklen_t *addrlen)
{
	uint8_t evt_ret[16];

	// ToDO: make this cleaner and figure the return parameter out
	// need specify return parameters!!!
	cc3000_spi_get_rx_buffer_stream(evt_ret, 5, 16);

	addr->sa_data[5] = evt_ret[15];
	addr->sa_data[4] = evt_ret[14];
	addr->sa_data[3] = evt_ret[13];
	addr->sa_data[2] = evt_ret[12];
	addr->sa_data[1] = evt_ret[11];
	addr->sa_data[0] = evt_ret[10];
	addr->sa_family  = ((evt_ret[9]<<8)|evt_ret[8]);
	*addrlen		 = ASIC_ADDR_LEN;

	return	cc3000_spi_get_rx_buffer_int32(9);
} // cc3000_get_accept

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
void cc3000_listen(uint8_t sd)
{
	cc3000_hci_start_command(HCI_CMND_LISTEN,8);
	cc3000_hci_send_uint32(sd);			//socket
	cc3000_hci_send_uint32(0x00000001);	//backlog
	cc3000_hci_end_command();
} // cc3000_listen

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
void cc3000_bind(uint8_t sd, uint16_t port)
{
	cc3000_hci_start_command(HCI_CMND_BIND,20);
	cc3000_hci_send_uint32(sd);				//socket
	cc3000_hci_send_uint32(0x00000008);		//magic
	cc3000_hci_send_uint32(8);				//addrlen
	cc3000_hci_send_uint16(AF_INET);		//socket family
	cc3000_hci_send_uint16(htons(port));	//socket port
	cc3000_hci_send_uint32(0x00000000);		//IP
	cc3000_hci_end_command();
} // cc3000_bind

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
void cc3000_setsockopt(uint8_t sd, uint32_t level, uint32_t optname, const void *optval, socklen_t optlen)
{
	cc3000_hci_start_command(HCI_CMND_SETSOCKOPT,20+optlen);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_send_uint32(level);
	cc3000_hci_send_uint32(optname);
	cc3000_hci_send_uint32(0x00000008);
	cc3000_hci_send_uint32(optlen);

	if (2 == optlen) 
	{
		cc3000_hci_send_uint16(*(uint16_t*)optval);
	}
	else if (4 == optlen)
	{
		cc3000_hci_send_uint32(*(uint32_t*)optval);
	}

	cc3000_hci_end_command();
} // cc3000_setsockopt

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
void cc3000_req_select(long nfds, cc3000_fd_set *readsds, cc3000_fd_set *writesds, cc3000_fd_set *exceptsds, struct timeval *timeout)
{
	unsigned long is_blocking;

	if( NULL == timeout)
	{
		is_blocking = 1; /* blocking , infinity timeout */
	}
	else
	{
		is_blocking = 0; /* no blocking, timeout */
	}

	cc3000_hci_start_command(HCI_CMND_BSD_SELECT,44);
	cc3000_hci_send_uint32(nfds);
	cc3000_hci_send_uint32(0x00000014);
	cc3000_hci_send_uint32(0x00000014);
	cc3000_hci_send_uint32(0x00000014);
	cc3000_hci_send_uint32(0x00000014);
	cc3000_hci_send_uint32(is_blocking);
	cc3000_hci_send_uint32((readsds)   ? *(unsigned long*)readsds   : 0);
	cc3000_hci_send_uint32((writesds)  ? *(unsigned long*)writesds  : 0);
	cc3000_hci_send_uint32((exceptsds) ? *(unsigned long*)exceptsds : 0);

	if(timeout)
	{
		if ( 0 == timeout->tv_sec && timeout->tv_usec < SELECT_TIMEOUT_MIN_MICRO_SECONDS)
		{
			timeout->tv_usec = SELECT_TIMEOUT_MIN_MICRO_SECONDS;
		}

		cc3000_hci_send_uint32(timeout->tv_sec);
		cc3000_hci_send_uint32(timeout->tv_usec);
	}
	else
	{
		cc3000_hci_send_uint32(0);
		cc3000_hci_send_uint32(0);
	}

	cc3000_hci_end_command();
} // cc3000_req_select

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
int8_t cc3000_get_select(cc3000_fd_set *readsds, cc3000_fd_set *writesds, cc3000_fd_set *exceptsds)
{
	int32_t  status;
	uint32_t read;
	uint32_t write;
	uint32_t except;

	status = cc3000_spi_get_rx_buffer_int32(5);
	read   = cc3000_spi_get_rx_buffer_uint32(9);
	write  = cc3000_spi_get_rx_buffer_uint32(13);
	except = cc3000_spi_get_rx_buffer_uint32(17);
	
	// Update actually read FD
	if (status >= 0)
	{
		if (readsds)
		{
			memcpy(readsds, &read, sizeof(read));
		}
		if (writesds)
		{
			memcpy(writesds, &write, sizeof(write));
		}
		if (exceptsds)
		{
			memcpy(exceptsds, &except, sizeof(except));
		}

		return(status);
	}
	else
	{
		return(-1);
	}
} // cc3000_get_select

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
int8_t cc3000_send(uint8_t sd, uint8_t *buf, uint16_t len, uint8_t flags)
{
	uint16_t count;

	if (0 == cc3000_state.bFreeBuffers)
	{
		#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
		debug_str(PSTR("Send: No free buffers!\n\r"));
		#endif
		cc3000_state.uiNoBuffers++;

		return -2; // no free buffers;
	}

	cc3000_hci_start_data(HCI_CMND_SEND, 16, len);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_send_uint32(16-4);
	cc3000_hci_send_uint32(len);
	cc3000_hci_send_uint32(flags);

	if (len > cc3000_state.uiMaxSendBytes)
		len = cc3000_state.uiMaxSendBytes;

	if (len > 0)
	{
		for (count = 0; count < len; count++)
		{
			cc3000_hci_send_uint8(buf[count]);
		}
	}

	cc3000_hci_end_data();

	return 1;
} // cc3000_send

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
int8_t cc3000_send_circ_buffer(uint8_t sd, uint8_t *buf, txint_t mask, txint_t head, txint_t tail)
{
	txint_t len	= (tail - head) & mask;
	
	if (len > CLIENT_TRNS_SIZE)
	{
		len		= CLIENT_TRNS_SIZE;
		tail	= (head + CLIENT_TRNS_SIZE) & mask;
	}

	if (0 == cc3000_state.bFreeBuffers)
	{
		#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
		debug_str(PSTR("Send: No free buffers!\n\r"));
		#endif
		cc3000_state.uiNoBuffers++;
		
		return -2; // no free buffers;
	}

	cc3000_hci_start_data(HCI_CMND_SEND, 16, len);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_send_uint32(16-4);
	cc3000_hci_send_uint32(len);
	cc3000_hci_send_uint32(0);		// flags

	if (len > 0)
	{
		for (; head != tail; head++, head &= mask)
			cc3000_hci_send_uint8(buf[head]);
	}

	cc3000_hci_end_data();

	return 1;
} // cc3000_send_circ_buffer

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
void cc3000_req_recv(uint8_t sd, uint16_t len, uint8_t flags)
{
	cc3000_hci_start_command(HCI_CMND_RECV, 3*4);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_send_uint32(len);
	cc3000_hci_send_uint32(flags);
	cc3000_hci_end_command();
} // cc3000_req_recv

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
int16_t cc3000_get_recv_bytes(void)
{
	uint16_t num_bytes;

	// In return, we get
	// int32_t SocketDescriptor (8 bits will do us) - 5
	// int32_t NumberOfBytes	(16 bits will do us)
	// int32_t Flags

	num_bytes = cc3000_spi_get_rx_buffer_uint16(6);

	return num_bytes;
} // cc3000_get_recv_bytes

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
void cc3000_get_recv_data(uint8_t *buf, uint8_t len)
{
	uint8_t 	args_length;

	args_length = cc3000_spi_rx_buffer[2];
	cc3000_spi_get_rx_buffer_stream(buf,args_length+5,len);
} // cc3000_get_recv_data

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
void cc3000_socket_close(uint8_t sd)
{
	cc3000_hci_start_command(HCI_CMND_CLOSE_SOCKET, 4);
	cc3000_hci_send_uint32(sd);
	cc3000_hci_end_command();
} // cc3000_socket_close

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
int8_t cc3000_socket_mdns_advertise(uint8_t mdns_enabled, uint8_t *service_name)
{
	uint8_t service_name_length;
	uint8_t count;

	service_name_length = strlen((const char *)service_name);
	if (service_name_length > MDNS_DEVICE_SERVICE_MAX_LENGTH)
	{
		return -1;	// SOC_ERROR
	}

	cc3000_hci_start_command(HCI_CMND_MDNS_ADVERTISE, 12 + service_name_length);
	cc3000_hci_send_uint32(mdns_enabled);
	cc3000_hci_send_uint32(8);	// magic
	cc3000_hci_send_uint32(service_name_length);

	for (count = 0; count < service_name_length; count++)
	{
		cc3000_hci_send_uint8(service_name[count]);
	}

	cc3000_hci_end_command();

	//ToDO: do we return 1 or 0?
	return 0;
} // cc3000_socket_mdns_advertise
