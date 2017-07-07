/*
 * cc3000_general.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_config.h"
#include "cc3000_nvmem.h"		// Needed for MAC_ADDR_LEN
#include "cc3000_platform.h"

#ifndef CC3000_GENERAL_H_
#define CC3000_GENERAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_General_API
//! @{
//
//*****************************************************************************

#define CC3000_SW_VERSION			1.00	//!< CC3000 Driver Version

#define CC3000_IS_HW_INIT			0x01	//!< Set/Clear in init_status after start/stop
#define CC3000_IS_BUFFER_SIZE		0x02	//!< Set after CC3000 buffer size is read

#define CC3000_SEND_HEADER_SIZE		(100)	//!< Size of header (32) in send command (using larger value)

typedef struct _cc3000_state
{
	uint8_t		bInitStatus;				//!< Set to CC3000_IS_HW_INIT and CC3000_IS_BUFFER_SIZE
	uint8_t		bConnected;					//!< Set after connected to AP. Cleared on unsolicited disconnect
	uint8_t		bDhcpComplete;				//!< Set after DHCP IP received. Cleared on unsolicited disconnect or DHCP IP is zeroed
	uint8_t		bWlanStatus;				//!< Set after listen command reply. Cleared on unsolicited disconnect
	uint8_t		abIP[4];					//!< Set/Cleared after DHCP IP received
	#if (CC3000_SM_STATE & CC3000_SM_STATE_PING)
	uint8_t		abIPGateway[4];				//!< IP address of the DHCP gateway
	#endif
	char*		sSSID;						//!< Name of the Access Point
	char*		sPWD;						//!< Password for Access Point
	uint16_t	wPort;						//!< Port to bind to
	uint8_t		abFirmware[2];				//!< Firmware Version: Major.Minor
	uint8_t		abMAC[MAC_ADDR_LEN];		//!< MAC address
	uint8_t		bMaxFreeBuffers;			//!< Maximum free buffers in CC3000
	uint8_t		bFreeBuffers;				//!< Count of free buffers in CC3000
	uint16_t	uiBufferLength;				//!< Length of buffers in CC3000
	uint16_t	uiMaxSendBytes;				//!< Maximum data bytes that can be sent to the CC3000
	uint16_t	uiNoBuffers;				//!< No free buffers in CC3000
	uint16_t	uiExceedFreeBuffers;		//!< Bug. Exceeded maximum free buffers.
	uint16_t	uiTotalGood;				//!< Total good writes to clients
	uint8_t		bOverflow;					//!< Non zero if a TX or RX buffer overflows
	uint16_t	uiTxOverflow;				//!< Count of TX buffer overflows
	uint16_t	uiRxOverflow;				//!< Count of RX buffer overflows
	uint16_t	uiSelectWriteFail;			//!< Count of select write fails
	uint16_t	uiSelectExceedMaxWriteFail;	//!< Count of exceeding maximum select write fails
	uint16_t	uiSelectError;				//!< Count of select function returning error
	uint16_t	uiSocketInactive;			//!< Socket inactive on send or recv command
	uint16_t	uiKeepAlive;				//!< Count of keep alive events
	uint32_t	ulKeepAliveMillis;			//!< Milliseconds of last keep alive event
}__cc3000_state;

extern __cc3000_state cc3000_state;

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
extern void cc3000_start(uint8_t patches_request);

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
extern void cc3000_stop();

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
extern void cc3000_general_req_buffer_size(void);

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
extern uint8_t cc3000_general_read_buffer_size();

extern uint8_t* uns32_to_stream(uint8_t* i, uint32_t n);
extern uint8_t* uns16_to_stream(uint8_t* i, uint16_t n);
extern uint16_t stream_to_uns16(uint8_t* i);
extern uint32_t stream_to_uns32(uint8_t* i);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_GENERAL_H_ */
