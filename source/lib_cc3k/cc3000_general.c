/*
 * cc3000_general.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_config.h"
#include "cc3000_general.h"
#include "cc3000_platform.h"
#include "cc3000_event_handler.h"
#include "cc3000_spi.h"
#include "cc3000_hci.h"
#include "cc3000_state_machine.h"

#include <util/delay.h>

//! The global state of the CC3000
__cc3000_state cc3000_state;

//*****************************************************************************
//
//! cc3000_start
//!
//!  @param  	patched_required
//!
//!  @return  	none
//!
//!  @brief  	Startup routine for the CC3000
//!
//
//*****************************************************************************
void cc3000_start(uint8_t patches_request)
{
	uint16_t	wCount;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STARTUP)
	debug_str(PSTR("cc3000_start\n\r"));
	#endif

	cc3000_state.bConnected            = 0;
	cc3000_state.bDhcpComplete         = 0;
	cc3000_state.bInitStatus           = 0;
	cc3000_state.bWlanStatus           = 0;

	for (int i = 0; i < MAX_WLAN_CLIENTS; i++)
		g_abClients[i] = -1;
	
	cc3000_sm_clear_buffers();

	cc3000_set_pin_PWR();
	
	// Need a delay here for power up?

	cc3000_clear_pin_WL_EN();

	// It takes ~0.4 ms for the IRQ line to go high
	_delay_us(400);

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STARTUP)
	debug_str(PSTR("Waiting for IRQ line to go high\n\r"));
	#endif
	wCount = 0;
	while (cc3000_read_irq_pin() != 1)
	{
		if (wCount++ >= MAX_WHILE_COUNT)
		{
			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STARTUP | CC3000_TRACE_LEVEL_ERROR))
			debug_str(PSTR("Exceeded timeout waiting for IRQ line to go high\n\r"));
			#endif
			
			return;
		}
	}
	
	cc3000_set_pin_WL_EN();

	// It takes ~63.5 ms for the IRQ line to go low
	_delay_us(63500);

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STARTUP)
	debug_str(PSTR("Waiting for IRQ line to go low\n\r"));
	#endif
	wCount = 0;
	while (cc3000_read_irq_pin() != 0)
		if (wCount++ >= MAX_WHILE_COUNT)
		{
			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STARTUP | CC3000_TRACE_LEVEL_ERROR))
			debug_str(PSTR("Exceeded timeout waiting for IRQ line to go low\n\r"));
			#endif
			
			return;
		}

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STARTUP)
	debug_str(PSTR("Sending start command\n\r"));
	#endif

	// Time for the magic start up sequence
	cc3000_assert_cs();

	_delay_us(50);

	cc3000_spi_send(SPI_OPERATION_WRITE);	// 0x01
	cc3000_spi_send(0);						// length msb
	cc3000_spi_send(5);						// length lsb
	cc3000_spi_send(0);						// busy 0

	_delay_us(100);

	cc3000_spi_send(0);						// busy 1
	cc3000_spi_send(HCI_TYPE_COMMAND);
	cc3000_spi_send(HCI_CMND_SIMPLE_LINK_START &  0xFF);
	cc3000_spi_send(HCI_CMND_SIMPLE_LINK_START >> 8);
	cc3000_spi_send(1);						// 1 byte payload
	cc3000_spi_send(patches_request);

	cc3000_deassert_cs();
	
	cc3000_state.bInitStatus |= CC3000_IS_HW_INIT;
} // cc3000_start

//*****************************************************************************
//
//! cc3000_stop
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Stop the CC3000
//!
//
//*****************************************************************************
void cc3000_stop()
{
	uint16_t	wCount;
	
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_STARTUP)
	debug_str(PSTR("cc3000_stop\n\r"));
	#endif

	cc3000_clear_pin_WL_EN();

	// Wait for IRQ line to go high
	wCount = 0;
	while (cc3000_read_irq_pin() != 1)
		if (wCount++ >= MAX_WHILE_COUNT)
		{
			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_STARTUP | CC3000_TRACE_LEVEL_ERROR))
			debug_str(PSTR("Exceeded timeout waiting for IRQ line to go high\n\r"));
			#endif
		}

	cc3000_state.bConnected            = 0;
	cc3000_state.bDhcpComplete         = 0;
	cc3000_state.bInitStatus           = 0;
	cc3000_state.bWlanStatus           = 0;

	for (int i = 0; i < MAX_WLAN_CLIENTS; i++)
		g_abClients[i] = -1;
	
	cc3000_clear_pin_PWR();
} // cc3000_stop

//*****************************************************************************
//
//! cc3000_general_req_buffer_size
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Requests the buffers of the CC3000
//!
//
//*****************************************************************************
void cc3000_general_req_buffer_size(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_general_req_buffer_size\n\r"));
	#endif

	cc3000_hci_send_command(HCI_CMND_READ_BUFFER_SIZE, 0, 0);
} // cc3000_general_req_buffer_size

//*****************************************************************************
//
//! cc3000_general_read_buffer_size
//!
//!  @param  	hci_free_buffers 	Pointer to global free buffers
//!
//!  @param  	hci_buffer_length	Pointer to global buffer length
//!
//!  @return  	Status of CC3000 buffers???
//!
//!  @brief  	Reads the buffers of the CC3000
//!
//
//*****************************************************************************
uint8_t cc3000_general_read_buffer_size()
{
	cc3000_state.bMaxFreeBuffers	= cc3000_spi_rx_buffer[5];
	cc3000_state.bFreeBuffers		= cc3000_state.bMaxFreeBuffers;
	cc3000_state.uiBufferLength		= cc3000_spi_get_rx_buffer_uint16(6);
	cc3000_state.uiMaxSendBytes		= cc3000_state.uiBufferLength - CC3000_SEND_HEADER_SIZE;

	cc3000_state.bInitStatus |= CC3000_IS_BUFFER_SIZE;

	return 0;
} // cc3000_general_read_buffer_size
