/*
 * cc3000_state_machine.c
 *
 *  Created on: 11.29.2014
 *      Author: David Harmon
 */

#include "../lib_timer/Timer.h"

#include "cc3000_event_handler.h"
#include "cc3000_general.h"
#include "cc3000_hci.h"				// needed for hci_send_command to send HCI_CMND_SIMPLE_LINK_START
#include "cc3000_netapp.h"
#include "cc3000_nvmem.h"
#include "cc3000_socket.h"
#include "cc3000_spi.h"
#include "cc3000_state_machine.h"
#include "cc3000_wlan.h"

#ifndef NULL
#define NULL 0
#endif

// Current machine state
State_t g_State;

// The last operation code received by the event handler.
uint16_t g_wCommandCompleted;

// Array to hold socket descriptors of connected clients
#if MAX_WLAN_CLIENTS == 4
int8_t g_abClients[MAX_WLAN_CLIENTS] = { -1, -1, -1, -1 };
#elif MAX_WLAN_CLIENTS == 3
int8_t g_abClients[MAX_WLAN_CLIENTS] = { -1, -1, -1 };
#elif MAX_WLAN_CLIENTS == 2
int8_t g_abClients[MAX_WLAN_CLIENTS] = { -1, -1 };
#elif MAX_WLAN_CLIENTS == 1
int8_t g_abClients[MAX_WLAN_CLIENTS] = { -1 };
#endif

// Index into g_abClients
uint8_t g_bReadIndex	= 0;
uint8_t g_bWriteIndex	= 0;

// WiFi client that is a command console. Only one client at a time can be
// a console. This client will not receive the normal NMEA sentences.
// This is an index into g_abClients.
uint8_t g_bCommandConsole = -1;

// Attempts to select a socket to write. Use MAX_SELECT_ATTEMPTS in
// cc3000_config.h to set maximum attempts.
uint8_t g_bSelectAttempts;

// Number of times select to write a socket failed. Use MAX_WRITE_FAIL in
// cc3000_config.h to set maximum attempts. Zero means no maximum.
#if MAX_WLAN_CLIENTS == 4
uint8_t g_abWriteFail[MAX_WLAN_CLIENTS] = { 0, 0, 0, 0 };
#elif MAX_WLAN_CLIENTS == 3
uint8_t g_abWriteFail[MAX_WLAN_CLIENTS] = { 0, 0, 0 };
#elif MAX_WLAN_CLIENTS == 2
uint8_t g_abWriteFail[MAX_WLAN_CLIENTS] = { 0, 0 };
#elif MAX_WLAN_CLIENTS == 1
uint8_t g_abWriteFail[MAX_WLAN_CLIENTS] = { 0 };
#endif

///////////////////////////////////////////////////////////////////////////////
// Various variables for the command functions
///////////////////////////////////////////////////////////////////////////////

// Buffer for TCP transmit data. This is a circular buffer.
uint8_t			tcp_tx_buffer[CLIENT_TRNS_SIZE];
txint_t			tcp_tx_head	= 0;
txint_t			tcp_tx_tail	= 0;
txint_t			tcp_tx_post = 0;		// End of buffer if data added after
										// already attempting to transmit
										// buffer to client.

// Array to hold client messages
tcp_rx_t g_aTcp_rx[MAX_WLAN_CLIENTS];

// Circular TX buffer for console client
tcp_tx_t g_console_tx;

int8_t			socket;				// Server socket

#if (CC3000_SM_STATE & CC3000_SM_STATE_SCAN)
// Results from scanning for APs
ap_entry_t		result;
#endif

cc3000_fd_set	rwsock;			// Read/Write socket
sockaddr		clientaddr;		// Only used for cc3000_get_accept function call
socklen_t		addrlen;		// Only used for cc3000_get_accept function call

// Wait time for a reply
uint32_t		g_ulWaitTime;

// Delay to restart and delay to scan for APs
uint32_t		g_ulDelay;

// Time at start of the cc3000_state_machine function
uint32_t		g_ulMillis;

//*****************************************************************************
//
//! cc3000_state_machine
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	State machine to handle the CC3000 commands. It should be
//!				called periodically from the main program loop.
//!
//
//*****************************************************************************
void cc3000_state_machine(void)
{
	uint8_t		i;					// Used in for loop
	int16_t		iTemp = 0;			// Used for clientstatus and noBlocking
	timeval		timeout;
	uint32_t	ulMillis = millis();

	static uint32_t ulMillisDHCP = DHCP_MAX_WAIT_TIME;		// Delay to wait for getting IP address

	g_ulMillis = ulMillis;

	// Verify that the CC3000 is initialized
	if (!(cc3000_state.bInitStatus & CC3000_IS_HW_INIT))
	{
		// Only attempt to initialize the CC3000 after waiting min wait time
		if (g_ulDelay > ulMillis)
			return;

		g_ulDelay = ulMillis + INIT_CC3000_MIN_WAIT_TIME;

		cc3000_start(0);

		if (cc3000_state.bInitStatus & CC3000_IS_HW_INIT)
			cc3000_sm_set_state(STATE_RESET_CONNECTION);

		return;
	}
	
	// Check if exceeded max wait time to get IP address
	if (0 == cc3000_state.bDhcpComplete)
	{
		if (ulMillisDHCP < ulMillis)
		{
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_ERROR)
			debug_str(PSTR("Error waiting for DHCP connection\n\r"));
			#endif
			cc3000_stop();
			ulMillisDHCP	= ulMillis + DHCP_MAX_WAIT_TIME;
			g_ulDelay		= ulMillis + INIT_CC3000_MIN_WAIT_TIME;

			return;
		}
	}
	else
	{
		// Reset delay in case there is an unsolicited disconnect.
		// The AP may reconnect without needing to stop the CC3000.
		ulMillisDHCP = ulMillis + DHCP_MAX_WAIT_TIME;
	}

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE_VERBOSE)
	if (g_State != STATE_IDLE)
	{
		debug_str(PSTR("g_State: "));
		debug_int(g_State);
		debug_nl();
	}
	#endif

	switch (g_State)
	{
		//---------------------------------------------------------------------
		//	Idle
		//---------------------------------------------------------------------
		case STATE_IDLE:
			break;

		//---------------------------------------------------------------------
		//	STATE_RESET_CONNECTION
		//
		//	Entry point to reset the CC3000 connection to the AP as well as
		//	read in the CC3000 firmware version and MAC address and do some
		//	initial configuration of the CC3000.
		//
		//	cc3000_state.bConnected is set in the cc3000_event_handler
		//	routine when the unsolicited event HCI_EVNT_WLAN_UNSOL_CONNECT
		//	is received from the CC3000.
		//---------------------------------------------------------------------
		case STATE_RESET_CONNECTION:
			// The HCI_CMND_SIMPLE_LINK_START command was sent as part of
			// the CC3000 startup sequence. Send the command a second time.
			i = 0;
			cc3000_hci_send_command(HCI_CMND_SIMPLE_LINK_START, 1, &i);
			cc3000_sm_set_state(STATE_CMND_SIMPLE_LINK_START_REPLY);
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;
			
		case STATE_CMND_SIMPLE_LINK_START_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_SIMPLE_LINK_START))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_RESET_CONNECTION\n\r"));
			#endif
			ulMillisDHCP	= ulMillis + DHCP_MAX_WAIT_TIME;
			g_State			= STATE_CMND_READ_BUFFER_SIZE;
			break;

		case STATE_CMND_READ_BUFFER_SIZE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_READ_BUFFER_SIZE\n\r"));
			#endif
            cc3000_general_req_buffer_size();
			cc3000_sm_set_state(STATE_CMND_READ_BUFFER_SIZE_REPLY);
			// Extra time needed to get buffer size reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;

		// Get the CC3000 buffer size and number of buffers.
		case STATE_CMND_READ_BUFFER_SIZE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_READ_BUFFER_SIZE))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("HCI_CMND_READ_BUFFER_SIZE\n\r"));
			#endif
            cc3000_general_read_buffer_size();
			cc3000_sm_set_state(STATE_NETAPP_SET_DEBUG_LEVEL);
			break;

		// Turn off the CC3000 debug functionality
		case STATE_NETAPP_SET_DEBUG_LEVEL:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_SET_DEBUG_LEVEL\n\r"));
			#endif
			// Disable all CC3000 debug messages
            cc3000_netapp_set_debug_level(0);
			cc3000_sm_set_state(STATE_NETAPP_SET_DEBUG_LEVEL_REPLY);
			break;

		// Wait for the debug setting reply then set the AP scan parameters
		case STATE_NETAPP_SET_DEBUG_LEVEL_REPLY:
			if (!cc3000_sm_check_state_change(HCI_NETAPP_SET_DEBUG_LEVEL))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_SET_DEBUG_LEVEL_REPLY\n\r"));
			#endif

			// At one time the set scan params was not needed. Now, the
			// CC3000 will not connect to an AP without it.
			// This looks to be corrected.
//			cc3000_wlan_set_scan_params(4000, 0x1FFF);

			g_State++;
			break;

		#if (CC3000_SM_STATE & CC3000_SM_STATE_SP)
		// Request the firmware version
		case STATE_CMND_READ_SP_VERSION:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_READ_SP_VERSION\n\r"));
			#endif
			cc3000_nvmem_read_sp_version();
			cc3000_sm_set_state(STATE_CMND_READ_SP_VERSION_REPLY);
			// Extra time needed to get SP version
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;

		// Read in the firmware version
		case STATE_CMND_READ_SP_VERSION_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_READ_SP_VERSION))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_READ_SP_VERSION_REPLY\n\r"));
			#endif
			cc3000_state.abFirmware[0] = cc3000_spi_rx_buffer[7];	// Major
			cc3000_state.abFirmware[1] = cc3000_spi_rx_buffer[8];	// Minor

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_INFO)
			debug_str(PSTR("Firmware version: "));
			debug_int(cc3000_state.abFirmware[0]);
			debug_putc('.');
			debug_int(cc3000_state.abFirmware[1]);
			debug_nl();
			#endif

			g_State++;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_SP)

		#if (CC3000_SM_STATE & CC3000_SM_STATE_MAC)
		// Request the MAC address
		case STATE_CMND_USER_GET_MAC:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_USER_GET_MAC\n\r"));
			#endif
			cc3000_nvmem_get_mac_address();
			cc3000_sm_set_state(STATE_CMND_USER_GET_MAC_REPLY);
			break;

		// Read in the MAC address
		case STATE_CMND_USER_GET_MAC_REPLY:
			if (!cc3000_sm_check_state_change(HCI_DATA_NVMEM_READ))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_USER_GET_MAC_REPLY\n\r"));
			#endif
			for (i = 0; i < MAC_ADDR_LEN; i++)
			{
				cc3000_state.abMAC[i] = cc3000_spi_rx_buffer[29+i];
			}

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_INFO)
			debug_str(PSTR("MAC address:"));
			for (i = 0; i < MAC_ADDR_LEN; i++)
			{
				debug_int_hex(cc3000_state.abMAC[i]);
			}
			debug_nl();
			#endif
			g_State++;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_MAC)

		#if (CC3000_SM_STATE & CC3000_SM_STATE_EVENT_MASK)
		// Disable specific unsolicited events
		case STATE_CMND_EVENT_MASK:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_EVENT_MASK\n\r"));
			#endif
            // Disable specific unsolicited events.
			cc3000_wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE);
			cc3000_sm_set_state(STATE_CMND_EVENT_MASK_REPLY);
			break;

		// Wait for reply
		case STATE_CMND_EVENT_MASK_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_EVENT_MASK))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_EVENT_MASK_REPLY\n\r"));
			#endif
			g_State++;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_EVENT_MASK)

		#if (CC3000_SM_STATE & CC3000_SM_STATE_SCAN)
		// Set AP scan parameters
		case STATE_CMND_WLAN_IOCTL_SET_SCANPARAM:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_SET_SCANPARAM\n\r"));
			#endif
			cc3000_wlan_set_scan_params(4000, 0x1FFF);
			cc3000_sm_set_state(STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_REPLY);
			// Extra time needed to get reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;

		// Wait for reply
		case STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_WLAN_IOCTL_SET_SCANPARAM))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_REPLY\n\r"));
			#endif
			g_ulDelay = ulMillis + SCAN_APS_DELAY;
			g_State++;
			break;

		// Delay long enough for the CC3000 to scan for APs
		case STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_DELAY:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_SET_SCANPARAM_DELAY\n\r"));
			#endif
			if (g_ulDelay < ulMillis)
				g_State++;
			break;

		// Stop scanning for APs and request result
		case STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS\n\r"));
			#endif
			// Stop scanning
			cc3000_wlan_set_scan_params(0, 0x1FFF);
			cc3000_wlan_req_scan_result();
			cc3000_sm_set_state(STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS_REPLY);
			// Extra time needed to get reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;

		// Read in scan result. Go to previous state to request next result
		// if more are pending.
		case STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_WLAN_IOCTL_GET_SCAN_RESULTS))
				break;

			// If there is a long list of APs then DHCP may time out
			ulMillisDHCP	= ulMillis + DHCP_MAX_WAIT_TIME;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_GET_SCAN_RESULTS_REPLY\n\r"));
			#endif
			cc3000_wlan_get_scan_result(&result);
			// No more results if entry_count is zero
			if (0 == result.entry_count)
			{
				g_State++;
				break;
			}

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_INFO)
			if (result.entry_count < 10)
				debug_str(PSTR(" "));
			debug_int(result.entry_count);
			switch (result.scan_status) 
			{
				case 0: debug_str(PSTR(" Aged results ")); break;
				case 1: debug_str(PSTR(" Results valid")); break;
				case 2: debug_str(PSTR(" No result    ")); break;
			}
			// This entry valid:
			if (result.entry_valid)
			{
				debug_str(PSTR(" Valid    "));
			}
			else
			{
				debug_str(PSTR(" Not Valid"));
			}
			debug_str(PSTR(" RSSI: "));
			debug_int(result.rssi);
			switch (result.security)
			{
				case 0: debug_str(PSTR(" Open")); break;
				case 1: debug_str(PSTR(" WEP "));  break;
				case 2: debug_str(PSTR(" WPA "));  break;
				case 3: debug_str(PSTR(" WPA2")); break;
			}
			debug_str(PSTR(" SSID: "));
			debug_sram_str((const char *)result.ssid);
			debug_nl();
			#endif // (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_INFO)
			
			// There are more results until entry_count is zero
			g_State--;
			g_wCommandCompleted	= STATE_IDLE;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_SCAN)

		// Set connection policy to AP
		case STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY\n\r"));
			#endif
			cc3000_wlan_set_connection_policy(fast_connect_disabled, 
											  open_ap_auto_connect_disabled, 
											  use_profiles_disabled);
			cc3000_sm_set_state(STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY_REPLY);
			// Extra time needed for reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			break;

		// Wait for reply
		case STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY_REPLY\n\r"));
			#endif
			g_State = STATE_CMND_WLAN_IOCTL_DEL_PROFILE;
			break;

		// Delete all other connection profiles
		case STATE_CMND_WLAN_IOCTL_DEL_PROFILE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_DEL_PROFILE\n\r"));
			#endif
			// Delete all profiles
			cc3000_wlan_del_profile(255);
			cc3000_sm_set_state(STATE_CMND_WLAN_IOCTL_DEL_PROFILE_REPLY);
			break;

		// Wait for reply
		case STATE_CMND_WLAN_IOCTL_DEL_PROFILE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_WLAN_IOCTL_DEL_PROFILE))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_IOCTL_DEL_PROFILE_REPLY\n\r"));
			#endif
			g_State = STATE_NETAPP_SET_TIMERS;
			break;

		// Configure time out values
		case STATE_NETAPP_SET_TIMERS:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_SET_TIMERS\n\r"));
			#endif
			cc3000_netapp_timeout_values(14400, 3600, CC3000_KEEP_ALIVE_DELAY, 0);
			cc3000_sm_set_state(STATE_NETAPP_SET_TIMERS_REPLY);
			break;

		// Wait for reply
		case STATE_NETAPP_SET_TIMERS_REPLY:
			if (!cc3000_sm_check_state_change(HCI_NETAPP_SET_TIMERS))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_SET_TIMERS_REPLY\n\r"));
			#endif
			g_State = STATE_CMND_WLAN_CONNECT;
			break;

		// Request connection to AP
		case STATE_CMND_WLAN_CONNECT:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_CONNECT\n\r"));
			#endif
			cc3000_wlan_connect(cc3000_state.sSSID, WLAN_SEC_WPA2, cc3000_state.sPWD);
			cc3000_sm_set_state(STATE_CMND_WLAN_CONNECT_REPLY);
			// Extra time needed to get WLAN connect reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME * 10;
			break;

		// Wait for reply. The reply does not mean that the connection to the AP has
		// been made. It is the reply that the request to connect has been received.
		// The unsolicited event HCI_EVNT_WLAN_UNSOL_CONNECT is sent by the CC3000
		// when a connection to the AP has been made.
		case STATE_CMND_WLAN_CONNECT_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_WLAN_CONNECT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WLAN_CONNECT_REPLY\n\r"));
			#endif
			
			// Set max delay to wait to get IP address
			ulMillisDHCP = ulMillis + DHCP_MAX_WAIT_TIME;
			
			g_State = STATE_IDLE;
			break;

		#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
		//---------------------------------------------------------------------
		//	STATE_NETAPP_PING_SEND
		//
		//	Entry point to ping the DHCP gateway.
		//---------------------------------------------------------------------
		case STATE_NETAPP_PING_SEND:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_PING_SEND\n\r"));
			#endif

			cc3000_netapp_ping_send(*(uint32_t*)cc3000_state.abIPGateway, PING_ATTEMPTS, PING_SIZE, PING_TIMEOUT);
			cc3000_sm_set_state(STATE_NETAPP_PING_SEND_REPLY);
			break;

		// Wait for reply
		case STATE_NETAPP_PING_SEND_REPLY:
			if (!cc3000_sm_check_state_change(HCI_NETAPP_PING_SEND))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_PING_SEND_REPLY\n\r"));
			#endif

			g_State = STATE_IDLE;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_PING)

		#if (CC3000_SM_STATE & CC3000_SM_STATE_IPCONFIG)
		//---------------------------------------------------------------------
		//	STATE_NETAPP_IPCONFIG
		//
		//	Entry point to ping the DHCP gateway.
		//---------------------------------------------------------------------
		case STATE_NETAPP_IPCONFIG:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_IPCONFIG\n\r"));
			#endif

			cc3000_netapp_ipconfig();
			cc3000_sm_set_state(STATE_NETAPP_IPCONFIG_REPLY);
			break;

		// Wait for reply
		case STATE_NETAPP_IPCONFIG_REPLY:
			if (!cc3000_sm_check_state_change(HCI_NETAPP_IPCONFIG))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_NETAPP_IPCONFIG_REPLY\n\r"));
			#endif

			g_State = STATE_IDLE;
			break;
		#endif // (CC3000_SM_STATE & CC3000_SM_STATE_IPCONFIG)

		//---------------------------------------------------------------------
		//	STATE_CREATE_SERVER_SOCKET
		//
		//	Entry point to create the server socket and to start listening
		//	for client connections.
		//
		//	This entry point should be used after the CC3000 connects to the
		//	AP.
		//---------------------------------------------------------------------
		case STATE_CREATE_SERVER_SOCKET:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CREATE_SERVER_SOCKET\n\r"));
			#endif
			cc3000_req_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			cc3000_sm_set_state(STATE_CMND_SOCKET_REPLY);
			break;

		// Wait for reply from request to create the server socket
		case STATE_CMND_SOCKET_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_SOCKET))
				break;

			socket = (int8_t)cc3000_spi_rx_buffer[4];

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SOCKET_REPLY "));
			debug_str(PSTR("Socket: "));
			debug_int(socket);
			debug_nl();
			#endif

			g_State = STATE_CMND_SETSOCKOPT;
			break;

		// Set the socket options
		case STATE_CMND_SETSOCKOPT:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SETSOCKOPT\n\r"));
			#endif
			cc3000_setsockopt(socket, 
                              SOL_SOCKET, 
                              SOCKOPT_ACCEPT_NONBLOCK, 
                              &iTemp, 
                              sizeof(iTemp));
			cc3000_sm_set_state(STATE_CMND_SETSOCKOPT_REPLY);
			break;

		// Wait for reply
		case STATE_CMND_SETSOCKOPT_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_SETSOCKOPT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SETSOCKOPT_REPLY\n\r"));
			#endif

			g_State = STATE_CMND_BIND;
			break;

		// Request to bind the port to the socket
		case STATE_CMND_BIND:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BIND\n\r"));
			#endif

			cc3000_bind(socket, cc3000_state.wPort);
			cc3000_sm_set_state(STATE_CMND_BIND_REPLY);
			break;

		// Wait for reply
		case STATE_CMND_BIND_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_BIND))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BIND_REPLY\n\r"));
			#endif

			g_State = STATE_CMND_LISTEN;
			break;

		// Request to listen for client connections
		case STATE_CMND_LISTEN:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_LISTEN\n\r"));
			#endif
			cc3000_listen(socket);
			cc3000_sm_set_state(STATE_CMND_LISTEN_REPLY);
			break;

		// Wait for reply. The reply does not mean that a client has connected.
		// The reply means that the request to listen has been accepted.
		case STATE_CMND_LISTEN_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_LISTEN))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_LISTEN_REPLY\n\r"));
			#endif

			cc3000_state.bWlanStatus = 1;
			g_State = STATE_IDLE;
			break;

		//---------------------------------------------------------------------
		//	STATE_CMND_ACCEPT
		//
		//	Entry point to accept a client connection.
		//---------------------------------------------------------------------
		case STATE_CMND_ACCEPT:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_ACCEPT\n\r"));
			#endif
			cc3000_req_accept(socket);
			cc3000_sm_set_state(STATE_CMND_ACCEPT_REPLY);
			break;

		// Accept socket
		case STATE_CMND_ACCEPT_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_ACCEPT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_ACCEPT_REPLY\n\r"));
			#endif

			iTemp = cc3000_get_accept(socket, (sockaddr *) &clientaddr, &addrlen);

			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_INFO))
			if (iTemp >= 0)
			{
				debug_str(PSTR("Accept: "));
				debug_int(iTemp);
				debug_nl();
			}
			#endif

			if (iTemp >= 0)
			{
				#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_INFO))
				debug_str(PSTR("Client: "));
				debug_int(clientaddr.sa_data[5]);
				debug_putc('.');
				debug_int(clientaddr.sa_data[4]);
				debug_putc('.');
				debug_int(clientaddr.sa_data[3]);
				debug_putc('.');
				debug_int(clientaddr.sa_data[2]);
				debug_nl();
				#endif

				// See if socket is already in list. This could happen if the
				// socket was disconnected, such as a timeout.
				for (i = 0; i < MAX_WLAN_CLIENTS; i++)
					if (g_abClients[i] == iTemp)
					{
						#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_INFO))
						debug_str(PSTR("Client socket already in list\n\r"));
						#endif

						break;
					}

				// Save the client socket if one is available
				if (MAX_WLAN_CLIENTS == i)
					for (i = 0; i < MAX_WLAN_CLIENTS; i++)
						if (g_abClients[i] < 0)
						{
							g_abClients[i] = iTemp;
							break;
						}
				
				// Close the socket if there is no room
				if (MAX_WLAN_CLIENTS == i)
				{
					#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_INFO | CC3000_TRACE_LEVEL_ERROR))
					debug_str(PSTR("Max client connections exceeded. Closing socket: "));
					debug_int(iTemp);
					debug_nl();
					#endif
					cc3000_sm_socket_close(iTemp);
				}
			}
			g_State = STATE_IDLE;
			break;

		//---------------------------------------------------------------------
		//	STATE_WRITE_ALL_CLIENTS
		//
		//	Entry point to write the tcp_tx_buffer buffer to all clients.
		//	NOTE: The buffer is not written to a client if it is currently
		//	being used as a command console.
		//---------------------------------------------------------------------
		case STATE_WRITE_ALL_CLIENTS:
            // Check if anything to send
			if (tcp_tx_head == tcp_tx_tail)
			{
				g_State = STATE_IDLE;
				break;
			}
			
			if (((tcp_tx_tail - tcp_tx_head) & CLIENT_TRNS_MASK) > cc3000_state.uiMaxSendBytes)
			{
				tcp_tx_tail = (tcp_tx_head + cc3000_state.uiMaxSendBytes) & CLIENT_TRNS_MASK;
			}

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_WRITE_ALL_CLIENTS\n\r"));
			#endif

			// Write to any connected clients that are not being used as a command console
			for (; g_bWriteIndex < MAX_WLAN_CLIENTS; g_bWriteIndex++)
				if (g_abClients[g_bWriteIndex] >= 0 && g_bCommandConsole != g_bWriteIndex)
				{
					#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
					debug_str(PSTR("g_bWriteIndex: "));
					debug_int(g_bWriteIndex);
					debug_nl();
					#endif
					g_bSelectAttempts	= 0;
					g_State				= STATE_CMND_BSD_SELECT_WRITE;
					break;
				}

			if (g_bWriteIndex >= MAX_WLAN_CLIENTS)
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("No more clients to write to\n\r"));
				#endif
				g_bWriteIndex	= 0;
				g_State			= STATE_IDLE;
				
				tcp_tx_head = tcp_tx_tail;
				tcp_tx_tail = tcp_tx_post;
			}
			break;

		// Request if client can be written to
		case STATE_CMND_BSD_SELECT_WRITE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_WRITE\n\r"));
			#endif

			timeout.tv_sec  = 0;
			timeout.tv_usec = 500000;	//	500 milliseconds

			cc3000_FD_ZERO( &rwsock );

			// Add client socket ID to the write set
			cc3000_FD_SET(g_abClients[g_bWriteIndex], &rwsock);

			cc3000_req_select(g_abClients[g_bWriteIndex] + 1, 
                              NULL, 
                              &rwsock, 
                              NULL, 
                              &timeout);

			cc3000_sm_set_state(STATE_CMND_BSD_SELECT_WRITE_REPLY);
			// Extra time needed for select reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME << 2;
			break;
			
		// Wait for reply then write buffer to client
		case STATE_CMND_BSD_SELECT_WRITE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_BSD_SELECT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_WRITE_REPLY\n\r"));
			#endif

			if (cc3000_get_select(NULL, &rwsock, NULL) < 0)
			{
 				#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
				debug_str(PSTR("Select for write returned error\n\r"));
				#endif
				cc3000_state.uiSelectError++;
			}
			else if (cc3000_FD_ISSET(g_abClients[g_bWriteIndex], &rwsock))
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("tcp_tx_head: ")); debug_int(tcp_tx_head);
				debug_str(PSTR(" tcp_tx_tail: ")); debug_int(tcp_tx_tail);
				debug_str(PSTR(" length: ")); debug_int((tcp_tx_tail - tcp_tx_head) & CLIENT_TRNS_MASK);
				debug_nl();
				#endif
				g_abWriteFail[g_bWriteIndex] = 0;
				cc3000_send_circ_buffer(g_abClients[g_bWriteIndex], 
                                        tcp_tx_buffer, 
                                        CLIENT_TRNS_MASK, 
                                        tcp_tx_head, 
                                        tcp_tx_tail);
			}
			else
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("Select returned no sockets ready to write to\n\r"));
				#endif

				cc3000_state.uiSelectWriteFail++;
				
				if (MAX_WRITE_FAIL)
					if (++g_abWriteFail[g_bWriteIndex] > MAX_WRITE_FAIL)
					{
						#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
						debug_str(PSTR("Failed attempts to write exceeded max. Closing socket.\n\r"));
						#endif

						cc3000_state.uiSelectExceedMaxWriteFail++;
				
						// Close the socket
						cc3000_sm_set_state(STATE_CMND_BSD_SELECT_CLOSE);
						break;
					}
					
				// Attempt to write to this socket again.
				// Check if maximum attempts reached
				if (++g_bSelectAttempts < MAX_SELECT_ATTEMPTS)
				{
					// Retry select for this socket
					cc3000_sm_set_state(STATE_CMND_BSD_SELECT_WRITE);
					break;
				}
				
				// If we reach here then maximum retries exceeded.
				// Go to STATE_WRITE_ALL_CLIENTS state if more clients pending.
				g_bWriteIndex++;
				cc3000_sm_set_state(STATE_WRITE_ALL_CLIENTS);
				break;
			}
			
			cc3000_sm_set_state(STATE_CMND_SEND_REPLY);
			// Extra time needed for send reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME << 2;
			break;

		// Wait for reply that buffer was received to write to client.
		// Go to STATE_WRITE_ALL_CLIENTS state if more clients pending.
		case STATE_CMND_SEND_REPLY: 	
			if (!cc3000_sm_check_state_change(HCI_EVNT_SEND))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SEND_REPLY\n\r"));
			#endif

			// Note: If the socket is inactive, a second HCI_EVNT_SEND event will be sent that
			//		 has ERROR_SOCKET_INACTIVE (-57) in the number of bytes field. This second
			//		 event is handled by the cc3000_event_handler function where it closes the
			//		 socket.

			// Note: It appears that a second HCI_EVNT_SEND event will be sent that has a -1
			//       in the number of bytes field when there is an error writing to the socket.
			//       The socket never recovers. This second event is handled by the
			//       cc3000_event_handler function where it closes the socket.

			// The cc3000_event_handler function handles the above two errors.

			g_bWriteIndex++;
			cc3000_sm_set_state(STATE_WRITE_ALL_CLIENTS);
			break;

		// If exceeded maximum attempts to write to client then request to
		// close socket to client.
		case STATE_CMND_BSD_SELECT_CLOSE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_CLOSE\n\r"));
			#endif

			cc3000_socket_close(g_abClients[g_bWriteIndex]);
			g_abWriteFail[g_bWriteIndex]	= 0;
			g_abClients[g_bWriteIndex]		= -1;
			if (g_bCommandConsole == g_bWriteIndex)
				g_bCommandConsole = -1;
			cc3000_sm_set_state(STATE_CMND_BSD_SELECT_CLOSE_REPLY);
			break;

		// Wait for reply then loop to write to next client.
		case STATE_CMND_BSD_SELECT_CLOSE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_CLOSE_SOCKET))
				break;
		
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_CLOSE_REPLY\n\r"));
			#endif

			g_bWriteIndex++;
			cc3000_sm_set_state(STATE_WRITE_ALL_CLIENTS);
			break;

		//---------------------------------------------------------------------
		//	STATE_WRITE_CLIENT
		//
		//	Entry point to write the g_console_tx buffer to the client
		//	currently being used as a command console.
		//---------------------------------------------------------------------
		case STATE_WRITE_CLIENT:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_WRITE_CLIENT\n\r"));
			#endif

			// Check if anything to send
			if (g_console_tx.xHead == g_console_tx.xTail)
			{
				g_State = STATE_IDLE;
				break;
			}

			if (((g_console_tx.xTail - g_console_tx.xHead) & CONSOLE_CLIENT_TRNS_MASK) > cc3000_state.uiMaxSendBytes)
			{
				g_console_tx.xTail = (g_console_tx.xHead + cc3000_state.uiMaxSendBytes) & CONSOLE_CLIENT_TRNS_MASK;
			}

			// Verify client is valid
			if (g_abClients[g_console_tx.bClientIndex] >= 0)
			{
				g_bSelectAttempts	= 0;
				g_State				= STATE_CMND_SELECT_WRITE;
				break;
			}
			break;

		// Request if client can be written to
		case STATE_CMND_SELECT_WRITE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SELECT_WRITE\n\r"));
			#endif

			timeout.tv_sec  = 0;
			timeout.tv_usec = 500000;	//	500 milliseconds

			cc3000_FD_ZERO( &rwsock );

			// Add client socket ID to the write set
			cc3000_FD_SET(g_abClients[g_console_tx.bClientIndex], &rwsock);

			cc3000_req_select(g_abClients[g_console_tx.bClientIndex] + 1,
							  NULL,
							  &rwsock,
							  NULL,
							  &timeout);

			cc3000_sm_set_state(STATE_CMND_SELECT_WRITE_REPLY);
			// Extra time needed for select reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME << 2;
			break;
		
		// Wait for reply then start writing buffer.
		case STATE_CMND_SELECT_WRITE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_BSD_SELECT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SELECT_WRITE_REPLY\n\r"));
			#endif

			if (cc3000_get_select(NULL, &rwsock, NULL) < 0)
			{
				#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
				debug_str(PSTR("Select for write returned error\n\r"));
				#endif
				cc3000_state.uiSelectError++;
			}
			else if (cc3000_FD_ISSET(g_abClients[g_console_tx.bClientIndex], &rwsock))
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("g_console_tx.xHead: ")); debug_int(g_console_tx.xHead);
				debug_str(PSTR(" g_console_tx.xTail: ")); debug_int(g_console_tx.xTail);
				debug_str(PSTR(" length: ")); debug_int((g_console_tx.xTail - g_console_tx.xHead) & CONSOLE_CLIENT_TRNS_MASK);
				debug_nl();
				#endif
				g_abWriteFail[g_console_tx.bClientIndex] = 0;
				cc3000_send_circ_buffer(g_abClients[g_console_tx.bClientIndex],
										g_console_tx.abBuffer,
										CONSOLE_CLIENT_TRNS_MASK,
										g_console_tx.xHead,
										g_console_tx.xTail);
			}
			else
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("Select returned no sockets ready to write to\n\r"));
				#endif

				cc3000_state.uiSelectWriteFail++;
			
				if (MAX_WRITE_FAIL)
				if (++g_abWriteFail[g_console_tx.bClientIndex] > MAX_WRITE_FAIL)
				{
					#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
					debug_str(PSTR("Failed attempts to write exceeded max. Closing socket.\n\r"));
					#endif

					cc3000_state.uiSelectExceedMaxWriteFail++;
				
					// Close the socket
					cc3000_sm_set_state(STATE_CMND_SELECT_CLOSE);
					break;
				}
			
				// Attempt to write to this socket again.
				// Check if maximum attempts reached
				if (++g_bSelectAttempts < MAX_SELECT_ATTEMPTS)
				{
					// Retry select for this socket
					cc3000_sm_set_state(STATE_CMND_SELECT_WRITE);
					break;
				}
			}
		
			cc3000_sm_set_state(STATE_CMND_WRITE_SEND_REPLY);
			// Extra time needed for send reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME << 2;
			break;

		// Wait for reply that buffer was received to write to client.
		case STATE_CMND_WRITE_SEND_REPLY:
			if (!cc3000_sm_check_state_change(HCI_EVNT_SEND))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_WRITE_SEND_REPLY\n\r"));
			#endif

			// Note: If the socket is inactive, a second HCI_EVNT_SEND event will be sent that
			//		 has ERROR_SOCKET_INACTIVE (-57) in the number of bytes field. This second
			//		 event is handled by the cc3000_event_handler function where it closes the
			//		 socket.

			// Note: It appears that a second HCI_EVNT_SEND event will be sent that has a -1
			//       in the number of bytes field when there is an error writing to the socket.
			//       The socket never recovers. This second event is handled by the 
			//       cc3000_event_handler function where it closes the socket.

			// The cc3000_event_handler function handles the above two errors.

			// The global TX buffer can be larger than the CC3000 TX buffer. Adjust the head
			// according to the number of bytes sent.
//			if (cc3000_spi_get_rx_buffer_int32(5) > 0)
			if (cc3000_spi_get_rx_buffer_int32(7) > 0)
			{
//				g_console_tx.xHead += cc3000_spi_get_rx_buffer_int32(5);
				g_console_tx.xHead += cc3000_spi_get_rx_buffer_int32(7);
				g_console_tx.xHead &= CONSOLE_CLIENT_TRNS_MASK;
				g_console_tx.xTail  = g_console_tx.xPost;
			}

			cc3000_sm_set_state(STATE_IDLE);
			break;

		case STATE_CMND_SELECT_CLOSE:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SELECT_CLOSE\n\r"));
			#endif

			cc3000_socket_close(g_abClients[g_console_tx.bClientIndex]);
			g_abWriteFail[g_console_tx.bClientIndex]	= 0;
			g_abClients[g_console_tx.bClientIndex]	= -1;

			if (g_bCommandConsole == g_console_tx.bClientIndex)
				g_bCommandConsole = -1;

			cc3000_sm_set_state(STATE_CMND_SELECT_CLOSE_REPLY);
			break;

		case STATE_CMND_SELECT_CLOSE_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_CLOSE_SOCKET))
				break;
		
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_SELECT_CLOSE_REPLY\n\r"));
			#endif

			cc3000_sm_set_state(STATE_IDLE);
			break;

		//---------------------------------------------------------------------
		//	STATE_READ_ALL_CLIENTS
		//
		//	Entry point to read any data from all clients.
		//---------------------------------------------------------------------
		case STATE_READ_ALL_CLIENTS:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_READ_ALL_CLIENTS\n\r"));
			#endif

			for (; g_bReadIndex < MAX_WLAN_CLIENTS; g_bReadIndex++)
				if (g_abClients[g_bReadIndex] >= 0)
				{
					#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
					debug_str(PSTR("g_bReadIndex: "));
					debug_int(g_bReadIndex);
					debug_nl();
					#endif
					g_State = STATE_CMND_BSD_SELECT_READ;
					break;
				}

			if (g_bReadIndex >= MAX_WLAN_CLIENTS)
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("No more clients to read from\n\r"));
				#endif
				g_bReadIndex	= 0;
				g_State			= STATE_IDLE;
			}
			break;

		case STATE_CMND_BSD_SELECT_READ:
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_READ\n\r"));
			#endif

			// Read from each connected client
			timeout.tv_sec  = 0;
			timeout.tv_usec = 5000;	//	5 milliseconds

			cc3000_FD_ZERO( &rwsock );

			//Add client socket ID to the read set
			cc3000_FD_SET(g_abClients[g_bReadIndex], &rwsock);

			cc3000_req_select(g_abClients[g_bReadIndex] + 1, &rwsock, NULL, NULL, &timeout);

			cc3000_sm_set_state(STATE_CMND_BSD_SELECT_READ_REPLY);
			// Extra time needed for select reply
			g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME << 2;
			break;
			
		case STATE_CMND_BSD_SELECT_READ_REPLY:
			if (!cc3000_sm_check_state_change(HCI_CMND_BSD_SELECT))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_CMND_BSD_SELECT_READ_REPLY\n\r"));
			#endif

			if (cc3000_get_select(&rwsock, NULL, NULL) < 0)
			{
				#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
				debug_str(PSTR("Select for read returned error\n\r"));
				#endif
			}
			else if (cc3000_FD_ISSET(g_abClients[g_bReadIndex], &rwsock))
			{
				// we can read
				cc3000_req_recv(g_abClients[g_bReadIndex], CC3000_RECV_SIZE - CC3000_RECV_HEADER, 0);
				
				cc3000_sm_set_state(STATE_EVNT_RECV);
				break;
			}
			else
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("Select returned no sockets ready to read from\n\r"));
				#endif
			}

			// Check next client			
			g_bReadIndex++;
			cc3000_sm_set_state(STATE_READ_ALL_CLIENTS);
			break;

		case STATE_EVNT_RECV:
			if (!cc3000_sm_check_state_change(HCI_EVNT_RECV))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_EVNT_RECV\n\r"));
			#endif

			// Now we have the number of bytes
			iTemp = cc3000_spi_get_rx_buffer_uint16(9);
			if (iTemp)
			{
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
				debug_str(PSTR("Bytes that will be sent from CC3000: "));
				debug_int(iTemp);
				debug_nl();
				#endif
				
				cc3000_sm_set_state(STATE_DATA_RECV);
				// Extra time needed to get data. More time needed the larger
				// the maximum received data.
				g_ulWaitTime += MAX_EXTRA_REPLY_WAIT_TIME;
			}
			else
			{
				g_bReadIndex++;
				cc3000_sm_set_state(STATE_READ_ALL_CLIENTS);
			}
			break;
			
		case STATE_DATA_RECV:
			if (!cc3000_sm_check_state_change(HCI_DATA_RECV))
				break;

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STATE)
			debug_str(PSTR("STATE_DATA_RECV\n\r"));
			#endif

			// The data in cc3000_return_buffer is:
			//	0		1		2	3  4  5  6	7  8  9  10 11 12 13 14 15
			// ArgLen	PayLoadLen	SocketDesc	DataLength	MessgLength	DataLengthOfBytes MessgLengthOfBytes
			//	Byte	LoBy  HiBy	Lo By By Hi Lo By By Hi	Lo By By Hi By By ..... By By By By ..... By By
			//                      |													|				  |
			//						+---------------Argument Length of Bytes------------+				  |
			//						|																	  |
			//                      +-----------------------Payload Length of Bytes-----------------------+
			//
			// Data (offset 15) is all zeros
			// Message (offset 15 + data length) is data from client
			cc3000_sm_process_client_message();

			g_bReadIndex++;
			g_State = STATE_IDLE;
			break;

		// Default. Should never need this.
		#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
		default:
			if (g_State != STATE_IDLE)
			{
				debug_str(PSTR("Default State Machine: "));
				debug_int(g_State);
				debug_nl();
			}
			break;
		#endif
	}
} // cc3000_state_machine

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
void cc3000_sm_set_state(uint8_t state)
{
	g_State = state;

	// Needed in case the next state is waiting for a reply
	g_ulWaitTime = millis() + MAX_REPLY_WAIT_TIME;

	// Needed in case the state is reset to the same state
	g_wCommandCompleted = STATE_IDLE;
} // cc3000_sm_set_state

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
uint8_t cc3000_sm_get_state(void)
{
	return g_State;
} // cc3000_sm_get_state

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
void cc3000_sm_socket_close(uint8_t bSocket)
{
	uint8_t bClient;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_sm_socket_close: "));
	debug_int(bSocket);
	debug_nl();
	#endif

	// Mark client that is owner of socket as closed then close socket
	for (bClient = 0; bClient < MAX_WLAN_CLIENTS; bClient++)
		if (g_abClients[bClient] == bSocket)
		{
			g_abClients[bClient]	= -1;
			g_abWriteFail[bClient]	= 0;
			if (g_bCommandConsole == bClient)
				g_bCommandConsole = -1;
		}

	cc3000_socket_close(bSocket);
} // cc3000_sm_socket_close

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
void cc3000_sm_accept_client(void)
{
	static unsigned long ulMillis = 0;

	if (cc3000_state.bWlanStatus && STATE_IDLE == g_State && ulMillis < millis())
	{
		ulMillis = millis() + ACCEPT_MIN_WAIT_TIME;
		
		cc3000_sm_set_state(STATE_CMND_ACCEPT);
	}
} // cc3000_sm_accept_client

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
uint8_t cc3000_sm_client_connected(void)
{
	uint8_t	i;
	uint8_t	bRet = 0;
	
	for (i = 0; i < MAX_WLAN_CLIENTS; i++)
		if (g_abClients[i] >= 0)
			bRet++;
			
	return bRet;
} // cc3000_sm_client_connected

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
txint_t cc3000_sm_write_all_clients(const uint8_t* buffer, txint_t len)
{
	txint_t	count;
	txint_t space;

	static unsigned long ulMillis = 0;
	
	// Verify that there are clients to write to
	if (cc3000_sm_client_connected() == 0)
		return 0;

	if (len)
	{
		space = CLIENT_TRNS_MASK - ((tcp_tx_post - tcp_tx_head) & CLIENT_TRNS_MASK);

		if (len > space)
		{
			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
			debug_str(PSTR("Not enough space in WiFi TX buffer\n\r"));
			#endif

			cc3000_state.bOverflow = 1;
			cc3000_state.uiTxOverflow++;
			
			len = 0;
		}
	
		// Use tcp_tx_post as tail in case the state machine is currently
		// writing the buffer to clients
		for (count = 0; count < len; count++, tcp_tx_post++, tcp_tx_post &= CLIENT_TRNS_MASK)
		{
			tcp_tx_buffer[tcp_tx_post] = buffer[count];
		}
	}

	if (STATE_IDLE == g_State && ulMillis < millis())
	{
		// Set delay before next write
		ulMillis = millis() + WRITE_ALL_MIN_WAIT_TIME;

		tcp_tx_tail	= tcp_tx_post;
		cc3000_sm_set_state(STATE_WRITE_ALL_CLIENTS);
	}

	return len;
} // cc3000_sm_write_all_clients

//*****************************************************************************
//
//! cc3000_sm_write_client
//!
//!  @param		bClientIndex	Index into g_abClients
//!
//!  @return  	none
//!
//!  @brief  	Writes the global console_tx buffer to the specified client
//!
//
//*****************************************************************************
void cc3000_sm_write_client(uint8_t bClientIndex)
{
	static unsigned long ulMillis = 0;
	
	if (STATE_IDLE == g_State && ulMillis < millis())
	{
		// Set delay before next write
		ulMillis = millis() + WRITE_CLIENT_MIN_WAIT_TIME;
		
		// Verify that the buffer is not empty
		if (g_console_tx.xHead == g_console_tx.xTail)
			g_console_tx.xTail = g_console_tx.xPost;

		if (g_console_tx.xHead == g_console_tx.xTail)
			return;

		g_console_tx.bClientIndex = bClientIndex;

		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("Writing console buffer to client\n\r"));
		#endif

		cc3000_sm_set_state(STATE_WRITE_CLIENT);
	}
} // cc3000_sm_write_client

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
void cc3000_sm_write_client_no_delay(uint8_t bClientIndex)
{
	if (STATE_IDLE == g_State)
	{
		// Verify that the buffer is not empty
		if (g_console_tx.xHead == g_console_tx.xTail)
			g_console_tx.xTail = g_console_tx.xPost;

		if (g_console_tx.xHead == g_console_tx.xTail)
			return;

		g_console_tx.bClientIndex = bClientIndex;

		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("Writing console buffer to client\n\r"));
		#endif

		cc3000_sm_set_state(STATE_WRITE_CLIENT);
	}
} // cc3000_sm_write_client_no_delay

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
void cc3000_sm_get_client_message(void)
{
	static unsigned long ulMillis = 0;
	
	// Verify that there are clients to read from
	if (cc3000_sm_client_connected() == 0)
		return;

	if (STATE_IDLE == g_State && ulMillis < millis())
	{
		// Set delay before next read
		ulMillis = millis() + READ_CLIENT_MIN_WAIT_TIME;
		
		// Set state machine to start reading clients
		cc3000_sm_set_state(STATE_READ_ALL_CLIENTS);
	}
} // cc3000_sm_get_client_message

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
void cc3000_sm_process_client_message(void)
{
	rxint_t		xI;
	rxint_t		xSpace;
	rxint_t		xLen;
	uint8_t*	pbData;
	uint8_t*	pbBuffer;
	rxint_t		xHead;
	rxint_t		xTail;

	// The data in cc3000_return_buffer is:
	//	0		1		2	3  4  5  6	7  8  9  10 11 12 13 14 15
	// ArgLen	PayLoadLen	SocketDesc	DataLength	MessgLength	DataLengthOfBytes MessgLengthOfBytes
	//	Byte	LoBy  HiBy	Lo By By Hi Lo By By Hi	Lo By By Hi By By ..... By By By By ..... By By
	//                      |													|				  |
	//						+---------------Argument Length of Bytes------------+				  |
	//						|																	  |
	//                      +-----------------------Payload Length of Bytes-----------------------+
	//
	// Data (offset 15) is all zeros
	// Message (offset 15 + data length) is data from client

	xLen = cc3000_spi_get_rx_buffer_uint32(13);	// Message Length

	if (xLen)
	{
		// Set pbData to first byte of client message: offset = 17 + DataLength
		pbData		= cc3000_spi_rx_buffer + 17 + cc3000_spi_get_rx_buffer_uint32(9);
		pbBuffer	= g_aTcp_rx[g_bReadIndex].abBuffer;
		xHead		= g_aTcp_rx[g_bReadIndex].xHead;
		xTail		= g_aTcp_rx[g_bReadIndex].xTail;
		xSpace		= CLIENT_RECV_MASK - ((xTail - xHead) & CLIENT_RECV_MASK);

		if (xLen > xSpace)
		{
				#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
				debug_str(PSTR("Not enough space in WiFi RX buffer\n\r"));
				#endif

				cc3000_state.bOverflow = 1;
				cc3000_state.uiRxOverflow++;
			
				xLen = xSpace;

		}
		
		for (xI = 0; xI < xLen; xI++, xTail++, xTail &= CLIENT_RECV_MASK)
		{
			pbBuffer[xTail] = pbData[xI];
		}

		g_aTcp_rx[g_bReadIndex].xTail = xTail;
	}
} // cc3000_sm_process_client_message

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
void cc3000_sm_reply_timeout(void)
{
	#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
	debug_str(PSTR("!!!!!!!! State Machine Reply Timeout !!!!!!!! State: "));
	debug_int(g_State);
	debug_nl();
	#endif
				
	cc3000_stop();

	// Delay before trying to restart
	g_ulDelay	= millis() + INIT_CC3000_MIN_WAIT_TIME;

	g_State		= STATE_IDLE;
} // cc3000_sm_reply_timeout

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
void cc3000_sm_clear_buffers(void)
{
	tcp_tx_head = 0;
	tcp_tx_tail = 0;
	tcp_tx_post = 0;
	
	for (int i = 0; i < MAX_WLAN_CLIENTS; i++)
	{
		g_aTcp_rx[i].xHead = 0;
		g_aTcp_rx[i].xTail = 0;
	}

	g_console_tx.xHead	= 0;
	g_console_tx.xTail	= 0;
	g_console_tx.xPost	= 0;

	g_bReadIndex		= 0;
	g_bWriteIndex		= 0;
	g_bCommandConsole	= -1;
} // cc3000_sm_clear_buffers

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
void cc3000_sm_ping_gateway(void)
{
	#if (CC3000_SM_STATE & CC3000_SM_STATE_IPCONFIG)
	static unsigned long ulMillis = 0;

	if (cc3000_state.bWlanStatus && STATE_IDLE == g_State && ulMillis < millis())
	{
		ulMillis = millis() + PING_MIN_WAIT_TIME;
		
		cc3000_sm_set_state(STATE_NETAPP_PING_SEND);
	}
	#endif
} // cc3000_sm_ping_gateway

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
void cc3000_sm_get_ipconfig(void)
{
	#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
	static unsigned long ulMillis = 0;

	if (cc3000_state.bWlanStatus && STATE_IDLE == g_State && ulMillis < millis())
	{
		ulMillis = millis() + IPCONFIG_MIN_WAIT_TIME;
		
		cc3000_sm_set_state(STATE_NETAPP_IPCONFIG);
	}
	#endif
} // cc3000_sm_get_ipconfig

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
char cc3000_sm_check_state_change(uint16_t wCommand)
{
	if (wCommand == g_wCommandCompleted)
		return 1;

	if (g_ulWaitTime < g_ulMillis)
	{
		#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STATE | CC3000_TRACE_LEVEL_ERROR))
		debug_str(PSTR("!!!!!!!! State Machine Reply Timeout !!!!!!!! State: "));
		debug_int(g_State);
		debug_nl();
		#endif
	
		cc3000_stop();

		// Delay before trying to restart
		g_ulDelay	= millis() + INIT_CC3000_MIN_WAIT_TIME;

		g_State		= STATE_IDLE;
	}

	return 0;
} // cc3000_sm_check_state_change
