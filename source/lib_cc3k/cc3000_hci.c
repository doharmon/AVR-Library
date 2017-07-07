/*
 * cc3000_hci.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_hci.h"
#include "cc3000_platform.h"
#include "cc3000_spi.h"

uint8_t  padding_byte_required;

//*****************************************************************************
//
//! cc3000_hci_start_command
//!
//!  @param		opcode
//!
//!  @param		args_length
//!
//!  @return	Returns if padding byte was required
//!
//!  @brief		Starts the HCI command to the CC3000. This routine is used
//!				when the arguments will be sent using the various HCI send
//!				routines. The routine cc3000_hci_end_command is used to end
//!				the command.
//
//*****************************************************************************
uint8_t cc3000_hci_start_command(uint16_t opcode, uint8_t args_length)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_start_command\n\r"));
	debug_str(PSTR("opcode: "));
	debug_int_hex_16bit(opcode);
	#endif

	padding_byte_required = cc3000_spi_start_send(HCI_COMMAND_HEADER_LENGTH + args_length);

	cc3000_spi_send(HCI_TYPE_COMMAND);
	cc3000_spi_send(opcode &  0xFF);
	cc3000_spi_send(opcode >> 8);
	cc3000_spi_send(args_length);

	return padding_byte_required;
} // cc3000_hci_start_command

//*****************************************************************************
//
//! cc3000_hci_send_command
//!
//!  @param		opcode
//!
//!  @param		args_length
//!
//!  @param		args
//!
//!  @return	Returns if padding byte was required
//!
//!  @brief		Sends an HCI command when the arguments are already arranged.
//!				This routine is used rather than using cc3000_hci_start_command
//!				and then sending each separate argument.
//
//*****************************************************************************
void cc3000_hci_send_command(uint16_t opcode, uint8_t args_length, uint8_t *args)
{

	uint8_t  padding_byte;
	uint16_t count;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_send_command\n\r"));
	debug_str(PSTR("opcode: "));
	debug_int_hex_16bit(opcode);
	#endif

	padding_byte = cc3000_spi_start_send(HCI_COMMAND_HEADER_LENGTH + args_length);

	cc3000_spi_send(HCI_TYPE_COMMAND);
	cc3000_spi_send(opcode & 0xFF);
	cc3000_spi_send(opcode >> 8);
	cc3000_spi_send(args_length);

	if (args_length > 0)
	{
		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("HCI args:\n\r"));
		#endif
		for (count = 0; count < args_length; count++)
		{
			cc3000_spi_send(args[count]);
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
			debug_int_hex(args[count]);
			#endif
		}
	}

	cc3000_spi_finish_send(padding_byte);
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_nl();
	debug_str(PSTR("Done.\n\r"));
	#endif
} // cc3000_hci_send_command

//*****************************************************************************
//
//! cc3000_hci_end_command
//!
//!  @param		none
//!
//!  @return	none
//!
//!  @brief		Ends a command that was started with cc3000_hci_start_command.
//!				It sends any required padding byte.
//
//*****************************************************************************
void cc3000_hci_end_command(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_end_command\n\r"));
	#endif
	cc3000_spi_finish_send(padding_byte_required);

} // cc3000_hci_end_command

//*****************************************************************************
//
//! cc3000_hci_start_data
//!
//!  @param		opcode
//!
//!  @param		args_length
//!
//!  @param		data_length
//!
//!  @return	Returns if padding byte was required
//!
//!  @brief		Starts a data send to the CC3000.
//
//*****************************************************************************
uint8_t cc3000_hci_start_data(uint8_t opcode, uint8_t args_length, uint16_t data_length)
{
	uint16_t payload_length;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_start_data\n\r"));
	debug_str(PSTR("opcode = "));
	debug_int_hex(opcode);
	debug_nl();
	#endif

	payload_length = args_length + data_length;

	padding_byte_required = cc3000_spi_start_send(HCI_DATA_HEADER_LENGTH + payload_length);

	cc3000_spi_send(HCI_TYPE_DATA);
	cc3000_spi_send(opcode);
	cc3000_spi_send(args_length);
	cc3000_spi_send(payload_length & 0xFF);
	cc3000_spi_send(payload_length >> 8);

	return padding_byte_required;
} // cc3000_hci_start_data

//*****************************************************************************
//
//! cc3000_hci_send_data
//!
//!  @param		opcode
//!
//!  @param		args_length
//!
//!  @param		args
//!
//!  @param		data_length
//!
//!  @param		data
//!
//!  @return	none
//!
//!  @brief		Sends a data command to the CC3000. Used when the arguments
//!				and data values are already arranged to be sent.
//
//*****************************************************************************
void cc3000_hci_send_data(uint8_t opcode, uint8_t args_length, uint8_t *args, uint16_t data_length, uint8_t *data)
{
	uint8_t  padding_byte;
	uint16_t payload_length;
	uint16_t count;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_send_data\n\r"));
	debug_str(PSTR("opcode: "));
	debug_int_hex(opcode);
	debug_nl();
	#endif

	payload_length = args_length + data_length;

	padding_byte = cc3000_spi_start_send(HCI_DATA_HEADER_LENGTH + payload_length);

	cc3000_spi_send(HCI_TYPE_DATA);
	cc3000_spi_send(opcode);
	cc3000_spi_send(args_length);
	cc3000_spi_send(payload_length &  0xFF);
	cc3000_spi_send(payload_length >> 8);

	if (args_length > 0)
	{
		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("HCI args:"));
		#endif
		for (count = 0; count < args_length; count++)
		{
			cc3000_spi_send(args[count]);
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
			debug_int_hex(args[count]);
			#endif
		}
	}

	if (data_length > 0)
	{
		#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
		debug_str(PSTR("HCI data:"));
		#endif
		for (count = 0; count < data_length; count++)
		{
			cc3000_spi_send(data[count]);
			#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
			debug_int_hex(data[count]);
			#endif
		}
	}

	cc3000_spi_finish_send(padding_byte);
} // cc3000_hci_send_data

//*****************************************************************************
//
//! cc3000_hci_end_data
//!
//!  @param		none
//!
//!  @return	none
//!
//!  @brief		Ends a data send that was started with cc3000_hci_start_data.
//!				It sends any required padding byte.
//
//*****************************************************************************
void cc3000_hci_end_data(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hci_end_data\n\r"));
	#endif
	cc3000_spi_finish_send(padding_byte_required);
} // cc3000_hci_end_data

void cc3000_hci_send_uint32(uint32_t u)
{
	cc3000_spi_send((uint8_t) (u &   0xFF));
	cc3000_spi_send((uint8_t) ((u >> 8  ) & 0xFF));
	cc3000_spi_send((uint8_t) ((u >> 16 ) & 0xFF));
	cc3000_spi_send((uint8_t) ((u >> 24 ) & 0xFF));
} // cc3000_hci_send_uint32

void cc3000_hci_send_uint16(uint16_t u)
{
	cc3000_spi_send((uint8_t) (u  &  0xFF));
	cc3000_spi_send((uint8_t) ((u >> 8 ) & 0xFF));
} // cc3000_hci_send_uint16

void cc3000_hci_send_uint8(uint8_t u)
{
	cc3000_spi_send(u);
} // cc3000_hci_send_uint8

void cc3000_hci_send_stream(uint8_t *u, uint8_t size)
{
	uint8_t count;

	for(count = 0; count < size; count++)
	{
		cc3000_spi_send(u[count]);
	}
} // cc3000_hci_send_stream
