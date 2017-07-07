/*
 * cc3000_hci.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifndef CC3000_HCI_H_
#define CC3000_HCI_H_

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_HCI_API
//! @{
//
//*****************************************************************************

#define HCI_COMMAND_HEADER_LENGTH 	4
#define HCI_DATA_HEADER_LENGTH 		5

#define  HCI_TYPE_COMMAND   		0x01
#define  HCI_TYPE_DATA				0x02
#define  HCI_TYPE_PATCH         	0x03
#define  HCI_TYPE_EVENT          	0x04

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
extern uint8_t cc3000_hci_start_command(uint16_t opcode, uint8_t args_length);

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
extern void cc3000_hci_send_command(uint16_t opcode, uint8_t args_length, uint8_t *args);

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
extern void cc3000_hci_end_command(void);

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
extern uint8_t cc3000_hci_start_data(uint8_t opcode, uint8_t args_length, uint16_t data_length);

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
extern void cc3000_hci_send_data(uint8_t opcode, uint8_t args_length, uint8_t *args, uint16_t data_length, uint8_t *data);

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
extern void cc3000_hci_end_data(void);

extern void cc3000_hci_send_uint32(uint32_t u);
extern void cc3000_hci_send_uint16(uint16_t u);
extern void cc3000_hci_send_uint8(uint8_t u);
extern void cc3000_hci_send_stream(uint8_t *u, uint8_t size);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_HCI_H_ */
