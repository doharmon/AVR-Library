/*
 * cc3000_nvmemh
 *
 *  Created on: 12.06.2014
 *      Author: David Harmon
 */
#ifndef __NVRAM_H__
#define __NVRAM_H__

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifdef  __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_Nvmem_API
//! @{
//
//*****************************************************************************

/****************************************************************************
**
**	Definitions for File IDs
**	
****************************************************************************/
/* NVMEM file ID - system files*/
#define NVMEM_NVS_FILEID 							(0)
#define NVMEM_NVS_SHADOW_FILEID 					(1)
#define NVMEM_WLAN_CONFIG_FILEID 					(2)
#define NVMEM_WLAN_CONFIG_SHADOW_FILEID 			(3)
#define NVMEM_WLAN_DRIVER_SP_FILEID					(4)
#define NVMEM_WLAN_FW_SP_FILEID						(5)
#define NVMEM_MAC_FILEID 							(6)
#define NVMEM_FRONTEND_VARS_FILEID 					(7)
#define NVMEM_IP_CONFIG_FILEID 						(8)
#define NVMEM_IP_CONFIG_SHADOW_FILEID 				(9)
#define NVMEM_BOOTLOADER_SP_FILEID 					(10)
#define NVMEM_RM_FILEID			 					(11)

/* NVMEM file ID - user files */
#define NVMEM_AES128_KEY_FILEID	 					(12)
#define NVMEM_SHARED_MEM_FILEID	 					(13)

/*!  Max entry in order to invalid nvmem */
#define NVMEM_MAX_ENTRY								(16)

/*! Length of buffer for a MAC address */
#define	MAC_ADDR_LEN								(6)


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
void cc3000_nvmem_read(uint32_t ulFileId, uint32_t ulLength, uint32_t ulOffset);

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
extern signed long cc3000_nvmem_write(unsigned long ulFileId, unsigned long ulLength, unsigned long ulEntryOffset, unsigned char *buff);


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
extern unsigned char cc3000_nvmem_set_mac_address(unsigned char *mac);


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
extern void cc3000_nvmem_get_mac_address(void);


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
extern unsigned char cc3000_nvmem_write_patch(unsigned long ulFileId, unsigned long spLength, const unsigned char *spData);


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
extern void cc3000_nvmem_read_sp_version(void);

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
extern int8_t cc3000_nvmem_create_entry(unsigned long file_id, unsigned long newlen);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // __NVRAM_H__
