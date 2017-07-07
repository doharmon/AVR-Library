/*
 * test_cc3k.c
 *
 * Created: 3/5/2016 7:48:55 AM
 *  Author: dharmon
 */ 

#define	 F_CPU	16000000UL

#include <avr/interrupt.h>
#include <string.h>

#include "../source/lib_timer/Timer.h"

//#include "../source/lib_tserial/tserial.h"
#include "../source/lib_uart/uart.h"
#include "../source/lib_xio/xio.h"

// ----------------------------------------------------------------------------------
// Includes for the CC3000 library
// ----------------------------------------------------------------------------------
// CC3000_TRACE_LEVEL defined in cc3000_config.h
#include "../source/lib_cc3k/cc3000_config.h"
#include "../source/lib_cc3k/cc3000_event_handler.h"
#include "../source/lib_cc3k/cc3000_general.h"
#include "../source/lib_cc3k/cc3000_hci.h"
#include "../source/lib_cc3k/cc3000_platform.h"
#include "../source/lib_cc3k/cc3000_socket.h"
#include "../source/lib_cc3k/cc3000_spi.h"
#include "../source/lib_cc3k/cc3000_state_machine.h"

//#define SerialWriteByte T0ASerialWrite
#define SerialWriteByte uart0_putc

//////////////////////////////////////////////////////////////////////////////
//! CC3000 Asynchronous Callback Function
//!
//! This routine is called by the CC3000 event handler to notify when
//! unsolicited events have been received.
//////////////////////////////////////////////////////////////////////////////
void AsyncCallBackFunction(uint8_t bHCI_Type, uint16_t wHCI_Opcode, uint8_t* pbData, uint16_t wDataSize)
{
//	xprintf_P(PSTR("AsyncCallBackFunction\n\r"));
//	if (HCI_CMND_ACCEPT != wHCI_Opcode && HCI_CMND_BSD_SELECT != wHCI_Opcode)
//		printHexChar(pbData, wDataSize);

	int8_t	sbSocket;
	int8_t	bWIFI = 4;
	
	// Only process if WiFi monitoring is turned on
	if (!bWIFI)
		return;
	
	if (bWIFI > 2)
	{
		xprintf_P(PSTR("CC3000 HCI Type: %02X HCI Opcode: %04X\n\r"), bHCI_Type, wHCI_Opcode);
	}
	
	if (bWIFI > 3)
	{
		// Do not dump accept replies or keep alive events
		if (!(0x04 == bHCI_Type && (HCI_CMND_ACCEPT == wHCI_Opcode || HCI_EVNT_WLAN_KEEPALIVE == wHCI_Opcode)))
			printHexChar(pbData, wDataSize);
	}

	if (HCI_TYPE_EVENT == bHCI_Type)
	{
		switch (wHCI_Opcode)
		{

			case HCI_EVNT_SEND:
			if (bWIFI > 1)		// Only send if WiFi verbose is enabled
			{
				xprintf_P(PSTR("CC3000 bytes sent to WiFi client: %lu\n\r"), *((int32_t*)&pbData[9]));
			}
			break;

			case HCI_EVNT_DATA_UNSOL_FREE_BUFF:
			if (bWIFI > 1)		// Only send if WiFi verbose is enabled
			{
				uint8_t bBufferCount;
				uint8_t bFreedBuffers = 0;
				
				bBufferCount = pbData[HCI_FLOW_CONTROL_EVENT_OFFSET];
				for (uint8_t count = 0; count < bBufferCount; count++)
				{
					bFreedBuffers += pbData[5+2+2+(count*4)];
				}
				
				xprintf_P(PSTR("CC3000 freed buffers: %d available buffers: %d\n\r"),
							bFreedBuffers, cc3000_state.bFreeBuffers);
			}
			break;

			case HCI_EVNT_WLAN_KEEPALIVE:
			xprintf_P(PSTR("CC3000 keep alive received\n\r"));
			break;
			
			case HCI_EVNT_WLAN_UNSOL_CONNECT:
			xprintf_P(PSTR("CC3000 connected to AP\n\r"));
			break;
			
			case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
			xprintf_P(PSTR("CC3000 no longer connected to AP\n\r"));
			break;
			
			case HCI_EVNT_WLAN_UNSOL_DHCP:
			xprintf_P(PSTR("CC3000 received DHCP IP address: %d.%d.%d.%d from: %d.%d.%d.%d\n\r"),
						pbData[8],  pbData[7],  pbData[6],  pbData[5],
						pbData[16], pbData[15], pbData[14], pbData[13]);
			break;
			
			case HCI_EVNT_BSD_TCP_CLOSE_WAIT:
			xprintf_P(PSTR("CC3000 client socket closed: %d\n\r"), pbData[5]);
			break;

			case HCI_CMND_ACCEPT:
			sbSocket = (pbData[9]);
			if (sbSocket >= 0)
			{
				xprintf_P(PSTR("CC3000 accept client socket: %d with IP address: %d.%d.%d.%d\n\r"),
							sbSocket,
							pbData[20], pbData[19], pbData[18], pbData[17]);
			}
			break;
		}
	}
} // AsyncCallBackFunction

int main(void)
{
	char		sSSID[33]	= "sSSID";
	char		sPWD[27]	= "sPWD";
	uint16_t	wPORT		= CC3000_DEFAULT_PORT;
	uint32_t	ulMillis	= millis();
	char		sMessage[40];

	sei();
	
	initTimer();

//	T0SerialBegin(14400);
	uart0_init(UART_BAUD_SELECT(14400,16000000L));

//	xdev_out(SerialWriteByte);
	xdev_out(uart0_putc);

	xprintf_P(PSTR("lib_cc3k test\n\r"));

	///////////////////////////////////////////////////////////////////////////////////
	// Set CC3000 variables
	///////////////////////////////////////////////////////////////////////////////////
	g_pAsyncCallBackFunction	= AsyncCallBackFunction;
	cc3000_state.sSSID			= sSSID;
	cc3000_state.sPWD			= sPWD;
	cc3000_state.wPort			= wPORT;
	
	///////////////////////////////////////////////////////////////////////////////////
	// Initialize the CC3000
	///////////////////////////////////////////////////////////////////////////////////
	debug_str(PSTR("Initialize the CC3000\n\r"));
	cc3000_hw_setup();
	cc3000_start(0);
	cc3000_sm_set_state(STATE_RESET_CONNECTION);

    while(1)
    {
		if (cc3000_sm_client_connected())
		{
			if (millis() - ulMillis > 1000)
			{
				ulMillis = millis();
				xsprintf_P(sMessage, PSTR("%lu\n\r"), ulMillis);
			}
			else
				sMessage[0] = 0;
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Poll the CC3000 for any events and call the state machine
		///////////////////////////////////////////////////////////////////////////////////
		cc3000_spi_receive();
		cc3000_sm_accept_client();					// Accept any pending connecting clients
//		cc3000_sm_ping_gateway();					// Ping DHCP gateway
//		cc3000_sm_get_ipconfig();					// Get current IP configuration
		cc3000_sm_get_client_message();				// Read any client messages
//		cc3000_sm_write_client(g_bCommandConsole);	// Write any pending buffer to WiFi console
		cc3000_sm_write_all_clients((uint8_t*)sMessage, strlen(sMessage));		// Write any pending messages
		cc3000_state_machine();
		
		///////////////////////////////////////////////////////////////////////////////////
		// Check if it has been too long since the last CC3000 Keep Alive event
		///////////////////////////////////////////////////////////////////////////////////
//		if (cc3000_state.bInitStatus)
//			if (cc3000_state.ulKeepAliveMillis + (((uint32_t)CC3000_KEEP_ALIVE_DELAY * 1000) * 3) < ulMillis)
//				restartCC3000();

		///////////////////////////////////////////////////////////////////////////////////
		// Check for any received client messages
		///////////////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < MAX_WLAN_CLIENTS; i++)
			if (g_aTcp_rx[i].xHead != g_aTcp_rx[i].xTail)
			{
//				xprintf_P(PSTR("Head: %d; Tail: %d\n\r"), g_aTcp_rx[i].xHead, g_aTcp_rx[i].xTail);
//				printHexChar(g_aTcp_rx[i].abBuffer, CC3000_SPI_RX_BUFFER_SIZE);
				for (int j = g_aTcp_rx[i].xHead; g_aTcp_rx[i].xTail != j; j++, j &= CLIENT_RECV_MASK)
					if (g_aTcp_rx[i].abBuffer[j])
						xputc(SerialWriteByte, g_aTcp_rx[i].abBuffer[j]);
					
				xprintf_P(PSTR("\n\r"));
				g_aTcp_rx[i].xHead = g_aTcp_rx[i].xTail;
			}
    }
}