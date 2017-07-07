/*
 * cc3000_event_handler.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

//#include "lib_timer/Timer.h"
unsigned long millis(void);		// defined in lib_timer/Timer.c

#include "cc3000_event_handler.h"
#include "cc3000_platform.h"
#include "cc3000_general.h"
#include "cc3000_hci.h"
#include "cc3000_socket.h"
#include "cc3000_spi.h"
#include "cc3000_state_machine.h"

// Asynchronous callback function
pAsyncCallBackFunction_t g_pAsyncCallBackFunction = 0;

//*****************************************************************************
//
//! cc3000_event_handler
//!
//!  @return	none
//!
//!  @brief		Handles all incoming data from cc3000
//
//*****************************************************************************
void cc3000_event_handler(void)
{

	uint8_t  count;
	uint8_t  hci_type;
	uint8_t  buffer_count;
	uint16_t hci_opcode			= 0;
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	uint16_t hci_args_length	= 0;
	#endif

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_event_handler\n\r"));
	#endif

	// determine the message type
	hci_type = cc3000_spi_rx_buffer[HCI_TYPE_OFFSET];

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("type: "));
	debug_int_hex(hci_type);
	#endif

	switch (hci_type)
	{

		case HCI_TYPE_EVENT:

			hci_opcode = cc3000_spi_get_rx_buffer_uint16(HCI_OPCODE_LSB_OFFSET);

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
			hci_args_length   = cc3000_spi_rx_buffer[HCI_ARGS_LENGTH_OFFSET];
			debug_str(PSTR("opcode: "));
			debug_int_hex_16bit(hci_opcode);
			debug_str(PSTR("args_length: "));
			debug_int_hex_16bit(hci_args_length);
			#endif

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_ERROR)
			if (0 != cc3000_spi_rx_buffer[4])
			{
				debug_str(PSTR("Event message returned non-zero status: "));
				debug_int(cc3000_spi_rx_buffer[4]);
				debug_nl();
			}
			#endif

			g_wCommandCompleted = hci_opcode;

			// Is this an unsolicited event?
			if(HCI_EVNT_DATA_UNSOL_FREE_BUFF == hci_opcode)
			{
				// ======HCI Header====== =============HCI Payload=============
				//                                         =Flow Control Event=      =Flow Control Event=
				// Type EvntOpCode ArgLen Status CountFCEs Sckt ???? BuffsFreed ...  Sckt ???? BuffsFreed
				// 0    1     2     3      4     5    6    7    8    9     10
				// 0x04 0x00  0x41  Byte   Byte  LoBy HiBy 0x0X 0x01 LoBy  HiBy ...  0x0X 0x01 LoBy  HiBy 
				buffer_count = cc3000_spi_rx_buffer[HCI_FLOW_CONTROL_EVENT_OFFSET]; // CountFCEs: 16 bits but who cares
				for (count = 0; count < buffer_count; count++) 
				{
					cc3000_state.bFreeBuffers += cc3000_spi_rx_buffer[5+2+2+(count*4)];
				}
				if (cc3000_state.bFreeBuffers > cc3000_state.bMaxFreeBuffers)
				{
					#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_ERROR))
					debug_str(PSTR("Exceeded max free buffers!\n\r"));
					#endif
					cc3000_state.bFreeBuffers = cc3000_state.bMaxFreeBuffers;
					cc3000_state.uiExceedFreeBuffers++;
				}
				#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
				debug_str(PSTR("Free buffer. "));
				debug_int(cc3000_state.bFreeBuffers);
				debug_str(PSTR(" of "));
				debug_int(cc3000_state.bMaxFreeBuffers);
				debug_str(PSTR(" buffers free.\n\r"));
				#endif
			}
			else if(hci_opcode & HCI_EVNT_WLAN_UNSOL_BASE)
			{
				switch (hci_opcode)
				{
					case HCI_EVNT_WLAN_KEEPALIVE:
						// ======HCI Header====== =HCI Payload=
						// Type EvntOpCode ArgLen Status
						// 0    1     2     3      4
						// 0x04 0x00  0x82  0x01   Byte
						#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
						debug_str(PSTR("***We are still alive***\n\r"));
						#endif
						cc3000_state.uiKeepAlive++;
						cc3000_state.ulKeepAliveMillis = millis();
						break;

					case HCI_EVNT_WLAN_UNSOL_CONNECT:
						// ======HCI Header====== =HCI Payload=
						// Type EvntOpCode ArgLen Status
						// 0    1     2     3      4
						// 0x04 0x01  0x80  0x01   Byte
						#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_INFO))
						debug_str(PSTR("***We are connected***\n\r"));
						#endif
						cc3000_state.bConnected = 1;
						break;

					case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
						// ======HCI Header====== =HCI Payload=
						// Type EvntOpCode ArgLen Status
						// 0    1     2     3      4
						// 0x04 0x02  0x80  0x01   Byte
						#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_INFO))
						debug_str(PSTR("***We are disconnected***\n\r"));
						#endif

						cc3000_state.bConnected    = 0;
						cc3000_state.bDhcpComplete = 0;
						cc3000_state.bWlanStatus   = 0;
						
						#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_INFO))
						debug_str(PSTR("***Attempt to reconnect***\n\r"));
						#endif
						cc3000_sm_set_state(STATE_RESET_CONNECTION);
						break;

					case HCI_EVNT_WLAN_UNSOL_DHCP:
						// ======HCI Header====== ===============HCI Payload========================================================================
						// Type EvntOpCode ArgLen Status AssignedIPv4Address  AssignedIPSubnet  DefaultIPGateway DHCPServerAddress DNSServerAddress
						// 0    1     2     3      4     5    6    7    8    9    10   11  12  13   14   15  16  17   18   19  20  21   22   23  24
						// 0x04 0x10  0x80  0x15   Byte  LSB  Byte Byte MSB  LSB Byte Byte MSB LSB Byte Byte MSB LSB Byte Byte MSB LSB Byte Byte MSB
						// Notes:
						// 1) IP config parameters are received swapped
						// 2) IP config parameters are valid only if status is OK
						cc3000_state.abIP[3] = cc3000_spi_rx_buffer[5];
						cc3000_state.abIP[2] = cc3000_spi_rx_buffer[6];
						cc3000_state.abIP[1] = cc3000_spi_rx_buffer[7];
						cc3000_state.abIP[0] = cc3000_spi_rx_buffer[8];
						#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
						cc3000_state.abIPGateway[3] = cc3000_spi_rx_buffer[13];
						cc3000_state.abIPGateway[2] = cc3000_spi_rx_buffer[14];
						cc3000_state.abIPGateway[1] = cc3000_spi_rx_buffer[15];
						cc3000_state.abIPGateway[0] = cc3000_spi_rx_buffer[16];
						#endif
						if ( 0 == cc3000_spi_rx_buffer[4] )
						{
//							#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_INFO))
							debug_str(PSTR("***We got an IP***: "));
							debug_int(cc3000_state.abIP[0]);
							debug_putc('.');
							debug_int(cc3000_state.abIP[1]);
							debug_putc('.');
							debug_int(cc3000_state.abIP[2]);
							debug_putc('.');
							debug_int(cc3000_state.abIP[3]);
							debug_str(PSTR(" from: "));
							debug_int(cc3000_spi_rx_buffer[16]);
							debug_putc('.');
							debug_int(cc3000_spi_rx_buffer[15]);
							debug_putc('.');
							debug_int(cc3000_spi_rx_buffer[14]);
							debug_putc('.');
							debug_int(cc3000_spi_rx_buffer[13]);
							debug_nl();
//							#endif
							cc3000_state.bDhcpComplete = 1;
							
							// Create a server socket to listen to
							cc3000_sm_set_state(STATE_CREATE_SERVER_SOCKET);
						}
						else
						{
							// When the AP turns off, the CC3000 loses DHCP address and hangs
							#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_UNSOL | CC3000_TRACE_LEVEL_INFO))
							debug_str(PSTR("***DHCP address is zero\n\r***"));
							#endif
							cc3000_state.abIP[3] = 0;
							cc3000_state.abIP[2] = 0;
							cc3000_state.abIP[1] = 0;
							cc3000_state.abIP[0] = 0;
							cc3000_state.bDhcpComplete = 0;
						}

						break;

					case HCI_EVNT_BSD_TCP_CLOSE_WAIT:
						// ======HCI Header====== ========HCI Payload=======
						// Type EvntOpCode ArgLen Status ===SocketHandle===
						// 0    1     2     3      4     5    6    7    8
						// 0x04 0x10  0x80  0x15   Byte  LSB  Byte Byte MSB
						#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
						debug_str(PSTR("Close event for client: "));
						debug_int(cc3000_spi_rx_buffer[5]);
						debug_str(PSTR(", closing socket\n\r"));
						#endif
						cc3000_sm_socket_close(cc3000_spi_rx_buffer[5]);
						break;

					#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
					case HCI_EVNT_WLAN_ASYNC_PING_REPORT:
						#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
						debug_str(PSTR("***Ping report***\n\r"));
						debug_str(PSTR("Echo requests: "));
						debug_int(cc3000_spi_rx_buffer[5]);
						debug_str(PSTR("\n\rEcho replies:  "));
						debug_int(cc3000_spi_rx_buffer[9]);
						debug_str(PSTR("\n\rMin RTT (ms):  "));
						debug_int(cc3000_spi_rx_buffer[13]);
						debug_str(PSTR("\n\rMax RTT (ms):  "));
						debug_int(cc3000_spi_rx_buffer[17]);
						debug_str(PSTR("\n\rAvg RTT (ms):  "));
						debug_int(cc3000_spi_rx_buffer[21]);
						debug_nl();
						#endif
						break;
					#endif

					#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
					case HCI_EVNT_WLAN_UNSOL_INIT:
						debug_str(PSTR("***We are initialized***\n\r"));
						break;
						
					case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
						debug_str(PSTR("***Smart Configuration completed***\n\r"));
						break;

					default:
						debug_str(PSTR("***Default unsolicited WLAN event***\n\r"));
						break;
					#endif // (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_UNSOL)
				}
			}
			else
			{
				// This is a "normal event"
				switch (hci_opcode)
				{
					case HCI_EVNT_SEND: // HCI_EVNT_DATA_SEND in online documentation
						// ======HCI Header====== =============HCI Payload=============
						// Type EvntOpCode ArgLen Status ===SocketHandle===  NumberBytesErrorCode
						// 0    1     2     3      4     5    6    7    8    9    10   11   12
						// 0x04 0x03  0x10  0x09   Byte  LSB Byte Byte  MSB  LSB Byte Byte  MSB

						// Note: If the socket is inactive, a second HCI_EVNT_SEND event will be sent that
						//		 has ERROR_SOCKET_INACTIVE (-57) in the number of bytes field. This second
						//		 event is handled here by closing the socket.

						// Note: It appears that a second HCI_EVNT_SEND event will be sent that has a -1
						//       in the number of bytes field when there is an error writing to the socket.
						//       The socket never recovers. Close the socket.

						// Error Code is negative if there is an error writing to the socket
						if (ERROR_SOCKET_INACTIVE == *((int16_t*)(cc3000_spi_rx_buffer+9)))
						{
							#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_ERROR)
							debug_str(PSTR("ERROR_SOCKET_INACTIVE Closing socket: "));
							debug_int(cc3000_spi_rx_buffer[5]);
							debug_nl();
							#endif
							cc3000_state.uiSocketInactive++;
							cc3000_sm_socket_close( cc3000_spi_rx_buffer[5] );	// Socket descriptor
						}
						else if (-1 == *((int16_t*)(cc3000_spi_rx_buffer+9)))
						{
							// NOTE: For some reason when the send command returns -1 no clients can be
							//       sent data again. Even if the client's socket is closed another client
							//       cannot connect. Therefore the CC3000 must be restarted.
							#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_ERROR)
							debug_str(PSTR("Error sending to client. Restarting the CC3000 because of socket: "));
							debug_int(cc3000_spi_rx_buffer[5]);
							debug_nl();
							#endif
							cc3000_state.uiSocketInactive++;
							cc3000_stop();
						}
						else
						{
							if (cc3000_state.bFreeBuffers)
							{
								cc3000_state.uiTotalGood++;
								cc3000_state.bFreeBuffers--;	// Strangely, the host must keep track of free buffers
								#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_FREE_BUFFERS)
								debug_str(PSTR("CC3000 free buffers: ")); debug_int(cc3000_state.bFreeBuffers);
								debug_nl();
								#endif
							}
							else
							{
								#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_FREE_BUFFERS | CC3000_TRACE_LEVEL_ERROR))
								debug_str(PSTR("Miscount on free buffers. Value would be less than zero.\n\r"));
								#endif
							}
						}
						break;

/*
					case HCI_EVNT_RECV:
						// Note: This event may behave like HCI_EVNT_SEND above with regards to returning
						//		 ERROR_SOCKET_INACTIVE or -1 when the client is inactive or there is an
						//		 error. May need to close socket.
*/

						// Note: we do not have a break here. We want the buffer copied.

					#if (CC3000_SM_STATE & CC3000_SM_STATE_IPCONFIG)
					case HCI_NETAPP_IPCONFIG:
						#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_INFO)
						debug_str(PSTR("IP Config Request\n\r"));
						#endif
						break;
					#endif
				}
			}
			break;

		case HCI_TYPE_DATA:

			hci_opcode = cc3000_spi_rx_buffer[HCI_DATA_OPERATION];

			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
			// Payload length
			// Payload length includes arguments and data. Length of data
			// is payload length - argument length.
			hci_args_length = cc3000_spi_get_rx_buffer_uint16(HCI_DATA_PAYLOAD_LENGTH_LSB_OFFSET);
			debug_str(PSTR("opcode: "));
			debug_int_hex_16bit(hci_opcode);
			debug_str(PSTR("args_length (payload length): "));
			debug_int_hex_16bit(hci_args_length);
			#endif

			g_wCommandCompleted = hci_opcode;

			break;
	}
	
	if (g_pAsyncCallBackFunction)
		g_pAsyncCallBackFunction(hci_type, hci_opcode, cc3000_spi_rx_buffer, cc3000_spi_rx_buffer_size);
} // cc3000_event_handler
