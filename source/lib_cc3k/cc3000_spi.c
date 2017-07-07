/*
 * cc3000_spi.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_platform.h"
#include "cc3000_spi.h"
#include "cc3000_event_handler.h"
#include "cc3000_general.h"

typedef union
{
	uint8_t		uint8[2];
	uint16_t	uint16;
	int16_t		int16;
} word_t;

typedef union
{
	uint8_t		uint8[4];
	uint16_t	uint16[2];
	int16_t		int16[2];
	uint32_t	uint32;
	int32_t		int32;
} long_t;

#if (CC3000_TRACE_LEVEL > CC3000_TRACE_LEVEL_NONE)
void printHexChar(const uint8_t* data, const uint16_t numBytes);
#endif

uint8_t cc3000_spi_rx_buffer[CC3000_SPI_RX_BUFFER_SIZE];
rxint_t cc3000_spi_rx_buffer_size;

//*****************************************************************************
//
//! cc3000_spi_start_send
//!
//!  @param		hci_length	Length of data to send
//!
//!  @return	Returns if padding byte was required
//!
//!  @brief		Starts an SPI message to the CC3000. Asserts the chip select
//!				for the CC3000. 
//!
//!				The following five bytes are sent:
//!					01 MSB_HCI_LENGTH LSB_HCI_LENGTH 00 00
//
//*****************************************************************************
uint8_t cc3000_spi_start_send(uint16_t hci_length)
{
	uint16_t spi_length            = CC3000_SPI_HEADER_SIZE + hci_length;
	uint8_t  padding_byte_required = spi_length & 0x01;
	uint16_t wCount;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_spi_start_send\n\r"));
	debug_str(PSTR("hci_length: "));
	debug_int_hex_16bit(hci_length);
	debug_nl();
	#endif

	if (padding_byte_required)
	{
		hci_length++;
		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("(+1 padding byte)\n\r"));
		#endif
	}

	cc3000_assert_cs();

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("Waiting for IRQ line to go low\n\r"));
	#endif

	wCount = 0;
	while(cc3000_read_irq_pin())
		if (wCount++ >= MAX_WHILE_COUNT)
		{
			#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
			debug_str(PSTR("Exceeded timeout waiting for IRQ line to go low\n\r"));
			#endif
			
			return padding_byte_required;
		}

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("Got it\n\r"));
	#endif

	cc3000_spi_send(SPI_OPERATION_WRITE);	// 0x01
	cc3000_spi_send(hci_length >> 8);		// length msb
	cc3000_spi_send(hci_length &  0xFF);	// length lsb
	cc3000_spi_send(0);						// busy 0
	cc3000_spi_send(0);						// busy 1

	return (padding_byte_required);
} // cc3000_spi_start_send

//*****************************************************************************
//
//! cc3000_spi_finish_send
//!
//!  @param		padding_byte_required
//!
//!  @return	none
//!
//!  @brief		Finishes SPI message by sending any required padding byte and
//!				deasserts the chip select for the CC3000.
//
//*****************************************************************************
void cc3000_spi_finish_send(uint8_t padding_byte_required)
{

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_spi_finish_send\n\r"));
	#endif

	if (padding_byte_required)
	{
		cc3000_spi_send(0);
	}

	cc3000_deassert_cs();
} // cc3000_spi_finish_send

//*****************************************************************************
//
//! cc3000_spi_receive
//!
//!  @param		none
//!
//!  @return	On success returns 1, on error returns 0
//!
//!  @brief		Receives data from the CC3000. Any received data is stored in
//!				the cc3000_spi_rx_buffer buffer.
//!
//!				The following bytes are sent:
//!					03 00 00
//!
//!				The following bytes are then read:
//!					MSB_DATA_LENGTH LSB_DATA_LENGTH [DATA_LENGTH of bytes]
//!
//!				cc3000_spi_rx_buffer contains the data bytes
//!				(DATA_LENGTH is not included)
//!
//!				cc3000_event_handler is called with cc3000_spi_rx_buffer
//
//*****************************************************************************
uint16_t cc3000_spi_receive(void)
{
	uint16_t count;
//	uint16_t spi_length;	// Length of SPI payload

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_spi_receive\n\r"));
	#endif

	// Verify if there is anything to read from the CC3000
	// The CC3000 needs to be initialized and the IRQ line low
	if(!cc3000_state.bInitStatus || cc3000_read_irq_pin())
		return 1;

	cc3000_assert_cs();

	cc3000_spi_send(SPI_OPERATION_READ);	// 0x03
	cc3000_spi_send(0);						// busy 0
	cc3000_spi_send(0);						// busy 1

	cc3000_spi_rx_buffer_size  = cc3000_spi_recv();
	cc3000_spi_rx_buffer_size  = cc3000_spi_rx_buffer_size << 8;
	cc3000_spi_rx_buffer_size += cc3000_spi_recv();


	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("SPI_READ: "));
	debug_int(cc3000_spi_rx_buffer_size);
	debug_str(PSTR(" bytes\n\r"));
	#endif


	if (0 == cc3000_spi_rx_buffer_size)
	{
		cc3000_deassert_cs();

		return 0;
	}

	if ((cc3000_spi_rx_buffer_size > CC3000_SPI_RX_BUFFER_SIZE))
	{
		#if (CC3000_TRACE_LEVEL & (CC3000_TRACE_LEVEL_DEBUG | CC3000_TRACE_LEVEL_ERROR))
		debug_str(PSTR("ERR: SPI length: "));
		debug_int(cc3000_spi_rx_buffer_size);
		debug_nl();
		#endif

		// Copy first three bytes of header
		cc3000_spi_rx_buffer[0] = cc3000_spi_recv();
		cc3000_spi_rx_buffer[1] = cc3000_spi_recv();
		cc3000_spi_rx_buffer[2] = cc3000_spi_recv();

		// Throw away rest of SPI record
		for (count = 3; count < cc3000_spi_rx_buffer_size; count++)
			cc3000_spi_recv();
			
		cc3000_spi_rx_buffer_size = 3;
	}
	else
	{
		for (count = 0; count < cc3000_spi_rx_buffer_size; count++)
		{
			cc3000_spi_rx_buffer[count] = cc3000_spi_recv();
		}
	}

	cc3000_deassert_cs();

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DUMP_SPI_RX_BUFF)
	printHexChar(cc3000_spi_rx_buffer, cc3000_spi_rx_buffer_size);
	#endif

	cc3000_event_handler();
	
	return 1;
} // cc3000_spi_receive

//*****************************************************************************
//
//! cc3000_spi_get_rx_buffer_uint16
//!
//!  @param  	offset
//!
//!  @return  	Returns 2 bytes from the return buffer as uint16_t
//!
//!  @brief
//!
//
//*****************************************************************************
uint16_t cc3000_spi_get_rx_buffer_uint16(uint8_t offset)
{
	word_t ret_data;

	ret_data.uint8[1] = cc3000_spi_rx_buffer[1+offset];
	ret_data.uint8[0] = cc3000_spi_rx_buffer[0+offset];

	return ret_data.uint16;
} // cc3000_spi_get_rx_buffer_uint16

//*****************************************************************************
//
//! cc3000_spi_get_rx_buffer_uint32
//!
//!  @param  	offset
//!
//!  @return  	Returns 4 bytes from the return buffer as uint32_t
//!
//!  @brief
//!
//
//*****************************************************************************
uint32_t cc3000_spi_get_rx_buffer_uint32(uint8_t offset)
{
	long_t ret_data;

	ret_data.uint8[3] = cc3000_spi_rx_buffer[3+offset];
	ret_data.uint8[2] = cc3000_spi_rx_buffer[2+offset];
	ret_data.uint8[1] = cc3000_spi_rx_buffer[1+offset];
	ret_data.uint8[0] = cc3000_spi_rx_buffer[0+offset];

	return ret_data.uint32;
} // cc3000_spi_get_rx_buffer_uint32

//*****************************************************************************
//
//! cc3000_spi_get_rx_buffer_int32
//!
//!  @param  	offset
//!
//!  @return  	Returns 4 bytes from the return buffer as int32_t
//!
//!  @brief
//!
//
//*****************************************************************************
int32_t cc3000_spi_get_rx_buffer_int32(uint8_t offset)
{
	long_t ret_data;

	ret_data.uint8[3] = cc3000_spi_rx_buffer[3+offset];
	ret_data.uint8[2] = cc3000_spi_rx_buffer[2+offset];
	ret_data.uint8[1] = cc3000_spi_rx_buffer[1+offset];
	ret_data.uint8[0] = cc3000_spi_rx_buffer[0+offset];

	return ret_data.int32;
} // cc3000_spi_get_rx_buffer_int32

//*****************************************************************************
//
//! cc3000_spi_get_rx_buffer_stream
//!
//!  @param  	data	Pointer to buffer
//!
//!  @param  	offset	Offset
//!
//!  @param  	size	Bytes to receive
//!
//!  @return  	none
//!
//!  @brief  	Fills the given buffer with a data stream
//!				and stops if the size exceeds the args_length
//!
//
//*****************************************************************************
void cc3000_spi_get_rx_buffer_stream(uint8_t* data, uint8_t offset, uint8_t size)
{
	uint8_t count;

	for(count = 0; count < size; count++)
	{
		*data++ = cc3000_spi_rx_buffer[count+offset];
	}
} // cc3000_spi_get_rx_buffer_stream
