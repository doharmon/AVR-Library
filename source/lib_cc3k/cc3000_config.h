/*
 * cc3000_config.h
 *
 *  Created on: 12.14.2014
 *      Author: David Harmon
 */
#ifndef CC3000_CONFIG_H_
#define CC3000_CONFIG_H_

#ifndef F_CPU
#define	 F_CPU	16000000UL
#endif

#include <inttypes.h>

//*****************************************************************************
//
//! \addtogroup CC3000_Configuration
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for debug printing
//
//*****************************************************************************
#define CC3000_TRACE_LEVEL_NONE				0x0000
#define CC3000_TRACE_LEVEL_STARTUP			0x0001
#define CC3000_TRACE_LEVEL_UNSOL			0x0002
#define CC3000_TRACE_LEVEL_INFO				0x0004
#define CC3000_TRACE_LEVEL_DEBUG			0x0008
#define CC3000_TRACE_LEVEL_STATE			0x0010
#define CC3000_TRACE_LEVEL_STATE_VERBOSE	0x0020
#define CC3000_TRACE_LEVEL_FREE_BUFFERS		0x0040
#define CC3000_TRACE_LEVEL_DUMP_SPI_RX_BUFF	0x0080
#define CC3000_TRACE_LEVEL_ERROR			0x0100
#define CC3000_TRACE_LEVEL_ALL				0xFFFF
#define CC3000_TRACE_LEVEL_NOTHING			0x8000	// Use to enable the debug statements

//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_INFO | CC3000_TRACE_LEVEL_UNSOL)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_ALL & ~CC3000_TRACE_LEVEL_STATE_VERBOSE)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_FREE_BUFFERS | CC3000_TRACE_LEVEL_STATE)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_INFO | CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_ERROR | CC3000_TRACE_LEVEL_STARTUP)
#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_ERROR)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_NOTHING)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_ALL)
//#define CC3000_TRACE_LEVEL		(CC3000_TRACE_LEVEL_NONE)

//*****************************************************************************
//
// Defines to enable optional states of the state machine
//
//*****************************************************************************
#define CC3000_SM_STATE_NONE		0x0000	//!< Disable optional states
#define CC3000_SM_STATE_SCAN		0x0001	//!< Enable scanning APs
#define CC3000_SM_STATE_SP			0x0002	//!< Enable reading firmware version
#define CC3000_SM_STATE_MAC			0x0004	//!< Enable reading MAC
#define CC3000_SM_STATE_EVENT_MASK	0x0008	//!< Enable event mask
#define CC3000_SM_STATE_PING		0x0010	//!< Enable sending pings
#define CC3000_SM_STATE_IPCONFIG	0x0020	//!< Enable requesting IP config
#define CC3000_SM_STATE_ALL			0xFFFF	//!< Enable all optional states

//! Enabled optional states of the state machine
//#define CC3000_SM_STATE			(CC3000_SM_STATE_SP | CC3000_SM_STATE_MAC)
//#define CC3000_SM_STATE			(CC3000_SM_STATE_SCAN | CC3000_SM_STATE_SP | CC3000_SM_STATE_MAC)
//#define CC3000_SM_STATE			(CC3000_SM_STATE_IPCONFIG | CC3000_SM_STATE_PING | CC3000_SM_STATE_SP | CC3000_SM_STATE_MAC)
#define CC3000_SM_STATE			(CC3000_SM_STATE_SP | CC3000_SM_STATE_MAC)
//#define CC3000_SM_STATE			(CC3000_SM_STATE_ALL)
//#define CC3000_SM_STATE			(CC3000_SM_STATE_NONE)


//! Default port to listen to
#define CC3000_DEFAULT_PORT			(23)

//! Size of the transmit buffer for client messages. Must be a power of 2.
//#define CLIENT_TRNS_SIZE			2048
#define CLIENT_TRNS_SIZE			512
#define CLIENT_TRNS_MASK			(CLIENT_TRNS_SIZE-1)

//! Size of the transmit buffer for client console messages. Must be a power of 2.
#define CONSOLE_CLIENT_TRNS_SIZE	CLIENT_TRNS_SIZE
#define CONSOLE_CLIENT_TRNS_MASK	(CONSOLE_CLIENT_TRNS_SIZE-1)

//! Size of indexes into transmit buffers
//!
//! If CLIENT_TRNS_SIZE is 256 or less then it is uint8_t.
//! If greater than 256 then it is uint16_t.
#if CLIENT_TRNS_SIZE > 256
typedef uint16_t					txint_t;
#else
typedef uint8_t						txint_t;
#endif

//! Size of buffer to hold CC3000 client messages
#define CC3000_RECV_SIZE			64

//! Size of indexes into transmit buffers
//!
//! If CLIENT_TRNS_SIZE is 256 or less then it is uint8_t.
//! If greater than 256 then it is uint16_t.
#if CC3000_RECV_SIZE > 256
typedef uint16_t					rxint_t;
#else
typedef uint8_t						rxint_t;
#endif

//! Size of SPI data receive header and optional extra tail byte.
//! The maximum number of bytes that can be received from a CC3000
//! client is CC3000_RECV_SIZE - CC3000_RECV_HEADER
#define CC3000_RECV_HEADER			30

//! Size of the receive buffer for client messages. Must be a power of 2.
#define CLIENT_RECV_SIZE			CC3000_RECV_SIZE
#define CLIENT_RECV_MASK			(CLIENT_RECV_SIZE-1)

//! Assume 4 sockets available, 1 of which is used for the server, so at most 3
//! clients can be connected at once.
#define MAX_WLAN_CLIENTS			1

//! Delay in milliseconds to wait to scan APs
#define SCAN_APS_DELAY				(5000)

#define INIT_CC3000_MIN_WAIT_TIME	(2000)	//!< Min time to wait between attempts to initialize CC3000
#define WRITE_CLIENT_MIN_WAIT_TIME	(200)	//!< Min time to wait between writing to console client
#define WRITE_ALL_MIN_WAIT_TIME		(250)	//!< Min time to wait between writing to all clients
#define READ_CLIENT_MIN_WAIT_TIME	(300)	//!< Min time to wait between reading clients
#define ACCEPT_MIN_WAIT_TIME		(1000)	//!< Min time to wait between attempts to accept clients
#define PING_MIN_WAIT_TIME			(9000)	//!< Min time to wait between attempts to ping DCHP gateway
#define IPCONFIG_MIN_WAIT_TIME		(10000)	//!< Min time to wait between attempts to request IP config
#define MAX_SELECT_ATTEMPTS			(2)		//!< Max attempts to get a socket to write to
#define MAX_WRITE_FAIL				(25)	//!< Max times select to write a socket can fail before closing socket. Zero means no max.
#define DHCP_MAX_WAIT_TIME			(30000)	//!< Max time to wait to receive DHCP
#define PING_ATTEMPTS				(1)		//!< Number of echo requests to send
#define PING_SIZE					(32)	//!< Send buffer size which may be up to 1400 bytes
#define PING_TIMEOUT				(1000)	//!< Time to wait for a response, in milliseconds

#define MAX_EXTRA_REPLY_WAIT_TIME	(1125)	//!< Extra time to wait for states needing more time
#define MAX_REPLY_WAIT_TIME			(750)	//!< Maximum time in ms to wait for a reply

//! Delay in seconds for keep alive event from CC3000. Minimum delay is 20.
#define CC3000_KEEP_ALIVE_DELAY		(20)

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif // CC3000_CONFIG_H_
