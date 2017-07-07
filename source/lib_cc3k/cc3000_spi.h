/*
 * cc3000_spi.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#ifndef CC3000_SPI_H_
#define CC3000_SPI_H_

#include "cc3000_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_SPI_API
//! @{
//
//*****************************************************************************

#define SPI_OPERATION_WRITE 		0x01
#define SPI_OPERATION_READ  		0x03

#define CC3000_SPI_HEADER_SIZE		5
#define CC3000_SPI_RX_BUFFER_SIZE	CC3000_RECV_SIZE
//#define CC3000_SPI_RX_BUFFER_SIZE	250

extern uint8_t cc3000_spi_rx_buffer[CC3000_SPI_RX_BUFFER_SIZE];
extern rxint_t cc3000_spi_rx_buffer_size;

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
//!				The following five bytes are sent sent:
//!					01 MSB_HCI_LENGTH LSB_HCI_LENGTH 00 00
//
//*****************************************************************************
extern uint8_t cc3000_spi_start_send(uint16_t hci_length);

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
extern void cc3000_spi_finish_send(uint8_t padding_byte_required);

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
extern uint16_t cc3000_spi_receive(void);

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
extern uint16_t cc3000_spi_get_rx_buffer_uint16(uint8_t offset);

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
extern uint32_t cc3000_spi_get_rx_buffer_uint32(uint8_t offset);

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
extern int32_t cc3000_spi_get_rx_buffer_int32(uint8_t offset);

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
extern void cc3000_spi_get_rx_buffer_stream(uint8_t *data,uint8_t offset,uint8_t size);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_SPI_H_ */
