/*
 * cc3000_nvmem.c
 *
 *  Created on: 12.06.2014
 *      Author: David Harmon
 */


#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "cc3000_nvmem.h"
#include "cc3000_hci.h"
#include "cc3000_socket.h"
#include "cc3000_event_handler.h"

//*****************************************************************************
//
// Prototypes for the structures for APIs.
//
//*****************************************************************************

#define NVMEM_CREATE_PARAMS_LEN		(8)
#define NVMEM_WRITE_PARAMS_LEN		(16)

//*****************************************************************************
//
//!  cc3000_nvmem_read
//!
//!  @param  ulFileId   nvmem file id:\n
//!                     NVMEM_NVS_FILEID, NVMEM_NVS_SHADOW_FILEID,
//!                     NVMEM_WLAN_CONFIG_FILEID, NVMEM_WLAN_CONFIG_SHADOW_FILEID,
//!                     NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!                     NVMEM_MAC_FILEID, NVMEM_FRONTEND_VARS_FILEID,
//!                     NVMEM_IP_CONFIG_FILEID, NVMEM_IP_CONFIG_SHADOW_FILEID,
//!                     NVMEM_BOOTLOADER_SP_FILEID, NVMEM_RM_FILEID,
//!                     and user files 12-15.
//!  @param  ulLength   Number of bytes to read 
//!  @param  ulOffset   Offset in file from where to read  
//!
//!  @return none
//!
//!  @brief  Reads data from the file referred by the ulFileId parameter. 
//!          Reads data from file ulOffset till length. Err if the file can't
//!          be used, is invalid, or if the read is out of bounds. 
//!	 
//*****************************************************************************
void cc3000_nvmem_read(uint32_t ulFileId, uint32_t ulLength, uint32_t ulOffset)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_read\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_NVMEM_READ, 3 * sizeof(uint32_t));
	cc3000_hci_send_uint32(ulFileId);
	cc3000_hci_send_uint32(ulLength);
	cc3000_hci_send_uint32(ulOffset);
	cc3000_hci_end_command();
} // cc3000_nvmem_read

//*****************************************************************************
//
//!  cc3000_nvmem_write
//!
//!  @param  ulFileId nvmem file id:\n
//!                   NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!                   NVMEM_MAC_FILEID, NVMEM_BOOTLOADER_SP_FILEID,
//!                   and user files 12-15.
//!  @param  ulLength       Number of bytes to write  
//!  @param  ulEntryOffset  Offset in file to start write operation from 
//!  @param  buff           Data to write
//!
//!  @return      On success 0, error otherwise.
//!
//!  @brief       Write data to nvmem.
//!               Writes data to file referred by the ulFileId parameter. 
//!               Writes data to file ulOffset till ulLength.The file id will be 
//!               marked invalid till the write is done. The file entry doesn't
//!               need to be valid - only allocated.
//!               NOTE: This routine is not currently implemented.
//!	 
//*****************************************************************************
signed long cc3000_nvmem_write(unsigned long ulFileId, 
							   unsigned long ulLength, 
							   unsigned long ulEntryOffset, 
							   unsigned char *buff)
{
	return -1;
/*
	long iRes;
	unsigned char *ptr;
	unsigned char *args;
	
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_write\n\r"));
	#endif

	iRes = EFAIL;
	
	ptr = tSLInformation.pucTxCommandBuffer;
	args = (ptr + SPI_HEADER_SIZE + HCI_DATA_CMD_HEADER_SIZE);
	
	// Fill in HCI packet structure
	args = UINT32_TO_STREAM(args, ulFileId);
	args = UINT32_TO_STREAM(args, 12);
	args = UINT32_TO_STREAM(args, ulLength);
	args = UINT32_TO_STREAM(args, ulEntryOffset);
	
	memcpy((ptr + SPI_HEADER_SIZE + HCI_DATA_CMD_HEADER_SIZE + 
					NVMEM_WRITE_PARAMS_LEN),buff,ulLength);
	#if (DEBUG_MODE == 1)
	PRINT_F("Writing:\t");
	for (uint8_t i=0; i<ulLength; i++)
	{
	    PRINT_F("0x");
	    printHex(buff[i]);
	    PRINT_F(", ");
	}
	PRINT_F("\n\r");
	#endif

	// Initiate a HCI command but it will come on data channel
	hci_data_command_send(HCI_CMND_NVMEM_WRITE, ptr, NVMEM_WRITE_PARAMS_LEN,
												ulLength);
	
	SimpleLinkWaitEvent(HCI_EVNT_NVMEM_WRITE, &iRes);
	
	return(iRes);
*/
} // cc3000_nvmem_write

//*****************************************************************************
//
//!  cc3000_nvmem_set_mac_address
//!
//!  @param  mac   MAC address to be set
//!
//!  @return      On success 0, error otherwise.
//!
//!  @brief       Write MAC address to EEPROM. 
//!               MAC address as appears over the air (OUI first)
//!               NOTE: This routine is not currently implemented.
//!
//	 
//*****************************************************************************
unsigned char cc3000_nvmem_set_mac_address(unsigned char *mac)
{
	return -1;
/*
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_set_mac_address\n\r"));
	#endif

	return  cc3000_nvmem_write(NVMEM_MAC_FILEID, MAC_ADDR_LEN, 0, mac);
*/
} // cc3000_nvmem_set_mac_address

//*****************************************************************************
//
//!  cc3000_nvmem_get_mac_address
//!
//!  @param		none
//!
//!  @return	On success 0, error otherwise.
//!
//!  @brief		Read MAC address from EEPROM. 
//!             MAC address as appears over the air (OUI first)
//!	 
//*****************************************************************************
void cc3000_nvmem_get_mac_address(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_get_mac_address\n\r"));
	#endif

	return cc3000_nvmem_read(NVMEM_MAC_FILEID, MAC_ADDR_LEN, 0);
} // cc3000_nvmem_get_mac_address

//*****************************************************************************
//
//!  cc3000_nvmem_write_patch
//!
//!  @param  ulFileId   nvmem file id:\n
//!                     NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!  @param  spLength   Number of bytes to write 
//!  @param  spData     SP data to write
//!
//!  @return     On success 0, error otherwise.
//!
//!  @brief      program a patch to a specific file ID. 
//!              The SP data is assumed to be organized in 2-dimensional.
//!              Each line is SP_PORTION_SIZE bytes long. Actual programming is 
//!              applied in SP_PORTION_SIZE bytes portions.
//!              NOTE: This routine is not currently implemented.
//!	 
//*****************************************************************************
unsigned char cc3000_nvmem_write_patch(unsigned long ulFileId, unsigned long spLength, const uint8_t *spData)
{
	return -1;
/*
	unsigned char 	status = 0;
	unsigned short	offset = 0;
	unsigned char*      spDataPtr = (unsigned char*)spData;
	uint8_t rambuffer[SP_PORTION_SIZE];

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_write_patch\n\r"));
	#endif

	while ((status == 0) && (spLength >= SP_PORTION_SIZE))
	{
		for (uint8_t i=0; i<SP_PORTION_SIZE; i++)
		{
			rambuffer[i] = pgm_read_byte(spData + i + offset);
		}

		#if (DEBUG_MODE == 1)
		PRINT_F("Writing: "); printDec16(offset); PRINT_F("\t");
		for (uint8_t i=0; i<SP_PORTION_SIZE; i++)
		{
			PRINT_F("0x");
			printHex(rambuffer[i]);
			PRINT_F(", ");
		}
		PRINT_F("\n\r");
		#endif
	
		status = nvmem_write(ulFileId, SP_PORTION_SIZE, offset, rambuffer);
		offset += SP_PORTION_SIZE;
		spLength -= SP_PORTION_SIZE;
		spDataPtr += SP_PORTION_SIZE;
	}
	
	if (status != 0)
	{
		// NVMEM error occurred
		return status;
	}
	
	if (spLength != 0)
	{
	  memcpy_P(rambuffer, spDataPtr, SP_PORTION_SIZE);
	  // if reached here, a reminder is left
	  status = nvmem_write(ulFileId, spLength, offset, rambuffer);
	}
	
	return status;
*/
} // cc3000_nvmem_write_patch

//*****************************************************************************
//
//!  cc3000_nvmem_read_sp_version
//!
//!  @return    On success 0, error otherwise.
//!
//!  @brief     Sends the command HCI_CMND_READ_SP_VERSION to read the package
//!             version. The state machine stores the return value in
//!				cc3000_state.abFirmware. First number indicates package ID and 
//!             the second number indicates package build number.
//
//*****************************************************************************
void cc3000_nvmem_read_sp_version(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_read_sp_version\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_READ_SP_VERSION, 0);
	cc3000_hci_end_command();
} // cc3000_nvmem_read_sp_version

//*****************************************************************************
//
//!  cc3000_nvmem_create_entry
//!
//!  @param      ulFileId    nvmem file Id:\n
//!                            * NVMEM_AES128_KEY_FILEID: 12
//!                            * NVMEM_SHARED_MEM_FILEID: 13
//!                            * and fileIDs 14 and 15
//!  @param      ulNewLen    Entry ulLength  
//!
//!  @return     On success 0, error otherwise.
//!
//!  @brief      Create new file entry and allocate space on the NVMEM. 
//!              Applies only to user files.
//!              Modify the size of file.
//!              If the entry is unallocated - allocate it to size 
//!              ulNewLen (marked invalid).
//!              If it is allocated then deallocate it first.
//!              To just mark the file as invalid without resizing - 
//!              set ulNewLen=0.
//!              NOTE: This routine is not currently implemented.
//	 
//*****************************************************************************
int8_t cc3000_nvmem_create_entry(unsigned long ulFileId, unsigned long ulNewLen)
{
	return -1;
/*
	unsigned char *ptr; 
	unsigned char *args;
	int8_t retval;
	
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_nvmem_create_entry\n\r"));
	#endif

	ptr = tSLInformation.pucTxCommandBuffer;
	args = (ptr + HEADERS_SIZE_CMD);
	
	// Fill in HCI packet structure
	args = UINT32_TO_STREAM(args, ulFileId);
	args = UINT32_TO_STREAM(args, ulNewLen);
	
	// Initiate a HCI command
	hci_command_send(HCI_CMND_NVMEM_CREATE_ENTRY,ptr, NVMEM_CREATE_PARAMS_LEN);
	
	SimpleLinkWaitEvent(HCI_CMND_NVMEM_CREATE_ENTRY, &retval);
	return(retval);
*/
} // cc3000_nvmem_create_entry
