/*
 * cc3000_state_machine.h
 *
 *  Created on: 11.29.2014
 *      Author: David Harmon
 */

#ifndef CC3000_STATE_MACHINE_H_
#define CC3000_STATE_MACHINE_H_

#include <avr/pgmspace.h>

#include "cc3000_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_State_Machine_API
//! @{
//
//*****************************************************************************

#undef STATE_IDLE
//! States for the state machine. They are grouped into steps that perform
//! specific actions. The last step in each group sets the state to STATE_IDLE.
typedef enum
{
	STATE_IDLE,

	//! Steps to reset connection to AP
	STATE_RESET_CONNECTION,
	STATE_CMND_SIMPLE_LINK_START_REPLY,
	STATE_CMND_READ_BUFFER_SIZE,
	STATE_CMND_READ_BUFFER_SIZE_REPLY,
	STATE_NETAPP_SET_DEBUG_LEVEL,
	STATE_NETAPP_SET_DEBUG_LEVEL_REPLY,
	#if (CC3000_SM_STATE & CC3000_SM_STATE_SP)
	STATE_CMND_READ_SP_VERSION,
	STATE_CMND_READ_SP_VERSION_REPLY,
	#endif // (CC3000_SM_STATE & CC3000_SM_STATE_SP)
	#if (CC3000_SM_STATE & CC3000_SM_STATE_MAC)
	STATE_CMND_USER_GET_MAC,
	STATE_CMND_USER_GET_MAC_REPLY,
	#endif // (CC3000_SM_STATE & CC3000_SM_STATE_MAC)
	#if (CC3000_SM_STATE & CC3000_SM_STATE_EVENT_MASK)
	STATE_CMND_EVENT_MASK,
	STATE_CMND_EVENT_MASK_REPLY,
	#endif // (CC3000_SM_STATE & CC3000_SM_STATE_EVENT_MASK)
	#if (CC3000_SM_STATE & CC3000_SM_STATE_SCAN)
	STATE_CMND_WLAN_IOCTL_SET_SCANPARAM,
	STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_REPLY,
	STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_DELAY,
	STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS,
	STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS_REPLY,
	#endif // (CC3000_SM_STATE & CC3000_SM_STATE_SCAN)
	STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY,
	STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY_REPLY,
	STATE_CMND_WLAN_IOCTL_DEL_PROFILE,
	STATE_CMND_WLAN_IOCTL_DEL_PROFILE_REPLY,
	STATE_NETAPP_SET_TIMERS,
	STATE_NETAPP_SET_TIMERS_REPLY,
	STATE_CMND_WLAN_CONNECT,
	STATE_CMND_WLAN_CONNECT_REPLY,

	//! Steps to ping DHCP gateway
	#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
	STATE_NETAPP_PING_SEND,
	STATE_NETAPP_PING_SEND_REPLY,
	#endif

	//! Steps to request IP config
	#if (CC3000_SM_STATE & CC3000_SM_STATE_IPCONFIG)
	STATE_NETAPP_IPCONFIG,
	STATE_NETAPP_IPCONFIG_REPLY,
	#endif

	//! Steps to configure socket to listen to clients
	STATE_CREATE_SERVER_SOCKET,
	STATE_CMND_SOCKET_REPLY,
	STATE_CMND_SETSOCKOPT,
	STATE_CMND_SETSOCKOPT_REPLY,
	STATE_CMND_BIND,
	STATE_CMND_BIND_REPLY,
	STATE_CMND_LISTEN,
	STATE_CMND_LISTEN_REPLY,

	//! Steps to accept a client connection
	STATE_CMND_ACCEPT,
	STATE_CMND_ACCEPT_REPLY,

	//! Steps to write to all connected clients
	STATE_WRITE_ALL_CLIENTS,
	STATE_CMND_BSD_SELECT_WRITE,
	STATE_CMND_BSD_SELECT_WRITE_REPLY,
	STATE_CMND_SEND_REPLY,
	STATE_CMND_BSD_SELECT_CLOSE,
	STATE_CMND_BSD_SELECT_CLOSE_REPLY,

	//! Steps to write to a single client
	STATE_WRITE_CLIENT,
	STATE_CMND_SELECT_WRITE,
	STATE_CMND_SELECT_WRITE_REPLY,
	STATE_CMND_WRITE_SEND_REPLY,
	STATE_CMND_SELECT_CLOSE,
	STATE_CMND_SELECT_CLOSE_REPLY,

	//! Steps to read messages from connected clients
	STATE_READ_ALL_CLIENTS,
	STATE_CMND_BSD_SELECT_READ,
	STATE_CMND_BSD_SELECT_READ_REPLY,
	STATE_EVNT_RECV,
	STATE_DATA_RECV
} State_t;

//! Structure to hold client messages
typedef struct
{
	uint8_t		abBuffer[CLIENT_RECV_SIZE];
	rxint_t		xHead;
	rxint_t		xTail;
} tcp_rx_t;

//! Structure to hold TX buffer for console client
typedef struct
{
	uint8_t		abBuffer[CONSOLE_CLIENT_TRNS_SIZE];
	txint_t		xHead;
	txint_t		xTail;
	txint_t		xPost;
	uint8_t		bClientIndex;
} tcp_tx_t;

//! Current machine state
extern State_t g_State;

//! The last command operation code received by the event handler.
extern uint16_t g_wCommandCompleted;

//! Array to hold socket descriptors of connected clients
extern int8_t g_abClients[MAX_WLAN_CLIENTS];

//! WiFi client that is a command console. Only one client at a time can be
//! a console. This client will not receive the normal NMEA sentences.
//! This is an index into g_abClients.
extern uint8_t g_bCommandConsole;

//! Number of times select to write a socket can fail before closing socket
//! Zero means no maximum
extern uint8_t g_bMaxWriteFail;

//! Array to hold client messages
extern tcp_rx_t g_aTcp_rx[MAX_WLAN_CLIENTS];

//! Circular TX buffer for console client
extern tcp_tx_t g_console_tx;

//*****************************************************************************
//
//! cc3000_state_machine
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	State machine to handle the CC3000 commands. It should be
//!				called periodically from the main program loop
//!
//
//*****************************************************************************
extern void cc3000_state_machine(void);

//*****************************************************************************
//
//! cc3000_sm_set_state
//!
//!  @param  	offset	Offset into the array abState
//!
//!  @return  	none
//!
//!  @brief  	Sets the offset into the abState array. The states are
//!				executed until a STATE_IDLE state is encountered.
//!
//
//*****************************************************************************
extern void cc3000_sm_set_state(uint8_t offset);

//*****************************************************************************
//
//! cc3000_sm_get_state
//!
//!  @param  	none
//!
//!  @return  	Offset into the abState array.
//!
//!  @brief  	Returns the current offset into the abState array.
//!
//
//*****************************************************************************
extern uint8_t cc3000_sm_get_state(void);

//*****************************************************************************
//
//! cc3000_sm_socket_close
//!
//!  @param  	bSocket	Socket number to close
//!
//!  @return  	none
//!
//!  @brief  	Closes the passed socket number.
//!
//
//*****************************************************************************
extern void cc3000_sm_socket_close(uint8_t bSocket);

//*****************************************************************************
//
//! cc3000_sm_accept_client
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Accepts new clients. Will only attempt to accept a new client
//!				after waiting a minimum amount of time between attempts.
//!
//
//*****************************************************************************
extern void cc3000_sm_accept_client(void);

//*****************************************************************************
//
//! cc3000_sm_client_connected
//!
//!  @param  	none
//!
//!  @return  	Number of connected clients
//!
//!  @brief  	Returns the number of connected clients
//!
//
//*****************************************************************************
extern uint8_t cc3000_sm_client_connected(void);

//*****************************************************************************
//
//! cc3000_sm_write_all_clients
//!
//!  @param  	buffer	Pointer to data buffer
//!				len		Length of data
//!
//!  @return  	Number of bytes written
//!
//!  @brief  	Adds data to tcp_tx_buffer. Function should be called with null
//!				parameters occasionally to ensure that all data has been sent.
//!
//
//*****************************************************************************
extern txint_t cc3000_sm_write_all_clients(const uint8_t* buffer, txint_t len);

//*****************************************************************************
//
//! cc3000_sm_write_client
//!
//!  @param		bClientIndex	Index into g_abClients
//!
//!  @return  	none
//!
//!  @brief  	Writes the global tcp_tx buffer to the specified client
//!
//
//*****************************************************************************
extern void cc3000_sm_write_client(uint8_t bClientIndex);

//*****************************************************************************
//
//! cc3000_sm_write_client_no_delay
//!
//!  @param		bClientIndex	Index into g_abClients
//!
//!  @return  	none
//!
//!  @brief  	Writes the global console_tx buffer to the specified client
//!				without waiting for a delay between writes
//!
//
//*****************************************************************************
extern void cc3000_sm_write_client_no_delay(uint8_t bClientIndex);

//*****************************************************************************
//
//! cc3000_sm_get_client_message
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Reads message from first available client that has a message.
//!				Client messages are stored in the array g_aTcp_rx.
//!
//
//*****************************************************************************
extern void cc3000_sm_get_client_message(void);

//*****************************************************************************
//
//! cc3000_sm_process_client_message
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Stores client message into its return buffer
//!
//
//*****************************************************************************
extern void cc3000_sm_process_client_message(void);

//*****************************************************************************
//
//! cc3000_sm_reply_timeout
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Handles timeouts waiting for a reply from the CC3000. The
//!				CC3000 is cycled and connection to the AP is reset.
//!
//
//*****************************************************************************
extern void cc3000_sm_reply_timeout(void);

//*****************************************************************************
//
//! cc3000_sm_clear_buffers
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Clear transmit and receive buffers.
//!
//
//*****************************************************************************
extern void cc3000_sm_clear_buffers(void);

//*****************************************************************************
//
//! cc3000_sm_ping_gateway
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Ping the DHCP gateway.
//!
//
//*****************************************************************************
extern void cc3000_sm_ping_gateway(void);

//*****************************************************************************
//
//! cc3000_sm_get_ipconfig
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Get current IP configuration.
//!
//
//*****************************************************************************
extern void cc3000_sm_get_ipconfig(void);

//*****************************************************************************
//
//! cc3000_sm_check_state_change
//!
//!  @param  	wCommand	Command to wait for
//!
//!  @return  	0: No change. 1: Changed to wCommand
//!
//!  @brief  	Check if the CC3000 returned the event passed in wCommand.
//!				If a time out occurs waiting for the change then the CC3000 is
//!				cycled and connection to the AP is reset.
//!
//
//*****************************************************************************
extern char cc3000_sm_check_state_change(uint16_t wCommand);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // CC3000_STATE_MACHINE_H_
