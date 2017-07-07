/*
 * cc3000_event_handler.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifndef CC3000_EVENT_HANDLER_H_
#define CC3000_EVENT_HANDLER_H_

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_HCI_Defines
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Values that can be used as HCI Commands and HCI Packet header defines
//
//*****************************************************************************

#define HCI_TYPE_OFFSET									(0)

// Offsets for types 0x01 (command) and 0x04 (event)
#define HCI_OPCODE_LSB_OFFSET							(1)
#define HCI_OPCODE_MSB_OFFSET							(2)
#define HCI_ARGS_LENGTH_OFFSET							(3)
#define HCI_ARGS_OFFSET									(4)

// Offsets for type 0x02 (data)
#define HCI_DATA_OPERATION								(1)
#define HCI_DATA_ARGS_LENGTH_OFFSET						(2)
#define HCI_DATA_PAYLOAD_LENGTH_LSB_OFFSET				(3)
#define HCI_DATA_PAYLOAD_LENGTH_MSB_OFFSET				(4)
#define HCI_DATA_ARGS_OFFSET							(5)

//*****************************************************************************
//
// Values that can be used as HCI Commands and HCI Packet header defines
//
//*****************************************************************************

#define HCI_EVENT_PATCHES_DRV_REQ						(1)
#define HCI_EVENT_PATCHES_FW_REQ						(2)
#define HCI_EVENT_PATCHES_BOOTLOAD_REQ					(3)


#define HCI_CMND_WLAN_BASE  						   (0x0000)
#define HCI_CMND_WLAN_CONNECT  							0x0001
#define HCI_CMND_WLAN_DISCONNECT   						0x0002
#define HCI_CMND_WLAN_IOCTL_SET_SCANPARAM    			0x0003
#define HCI_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY  		0x0004
#define HCI_CMND_WLAN_IOCTL_ADD_PROFILE  				0x0005
#define HCI_CMND_WLAN_IOCTL_DEL_PROFILE  				0x0006
#define HCI_CMND_WLAN_IOCTL_GET_SCAN_RESULTS  			0x0007
#define HCI_CMND_EVENT_MASK    							0x0008
#define HCI_CMND_WLAN_IOCTL_STATUSGET 					0x0009
#define HCI_CMND_WLAN_IOCTL_SIMPLE_CONFIG_START     	0x000A
#define HCI_CMND_WLAN_IOCTL_SIMPLE_CONFIG_STOP      	0x000B
#define HCI_CMND_WLAN_IOCTL_SIMPLE_CONFIG_SET_PREFIX	0x000C
#define HCI_CMND_WLAN_CONFIGURE_PATCH					0x000D


#define HCI_CMND_SOCKET_BASE   							0x1000
#define HCI_CMND_SOCKET        							0x1001
#define HCI_CMND_BIND          							0x1002
#define HCI_CMND_RECV          							0x1004
#define HCI_CMND_ACCEPT        							0x1005
#define HCI_CMND_LISTEN        							0x1006
#define HCI_CMND_CONNECT       							0x1007
#define HCI_CMND_BSD_SELECT 							0x1008
#define HCI_CMND_SETSOCKOPT    							0x1009
#define HCI_CMND_GETSOCKOPT    							0x100A
#define HCI_CMND_CLOSE_SOCKET  							0x100B
#define HCI_CMND_RECVFROM      							0x100D
#define HCI_CMND_GETHOSTNAME   							0x1010
#define HCI_CMND_MDNS_ADVERTISE	   						0x1011


#define HCI_DATA_BASE									0x80

#define HCI_CMND_SEND           						(0x01 + HCI_DATA_BASE)	// Decrement free buffers
#define HCI_CMND_SENDTO        							(0x03 + HCI_DATA_BASE)	// Decrement free buffers
#define HCI_DATA_BSD_RECVFROM							(0x04 + HCI_DATA_BASE)
#define HCI_DATA_BSD_RECV								(0x05 + HCI_DATA_BASE)


#define HCI_CMND_NVMEM_CBASE							(0x0200)


#define HCI_CMND_NVMEM_CREATE_ENTRY 					(0x0203)
#define HCI_CMND_NVMEM_SWAP_ENTRY  						(0x0205)
#define HCI_CMND_NVMEM_READ    							(0x0201)
#define HCI_CMND_NVMEM_WRITE   							(0x0090)
#define HCI_CMND_NVMEM_WRITE_PATCH						(0x0204)
#define HCI_CMND_READ_SP_VERSION  						(0x0207)

#define HCI_CMND_READ_BUFFER_SIZE						0x400B
#define HCI_CMND_SIMPLE_LINK_START						0x4000

#define HCI_CMND_NETAPP_BASE							0x2000

#define HCI_NETAPP_DHCP				    				(0x0001 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_PING_SEND                        	(0x0002 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_PING_REPORT                      	(0x0003 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_PING_STOP                        	(0x0004 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_IPCONFIG                         	(0x0005 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_ARP_FLUSH			    			(0x0006 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_SET_DEBUG_LEVEL						(0x0008 + HCI_CMND_NETAPP_BASE)
#define HCI_NETAPP_SET_TIMERS							(0x0009 + HCI_CMND_NETAPP_BASE)


//*****************************************************************************
//
// Values that can be used as HCI Events defines
//
//*****************************************************************************
#define HCI_EVNT_WLAN_BASE								0x0000
#define HCI_EVNT_WLAN_CONNECT							0x0001
#define HCI_EVNT_WLAN_DISCONNECT						0x0002
#define HCI_EVNT_WLAN_IOCTL_ADD_PROFILE					0x0005


#define HCI_EVNT_SOCKET              					HCI_CMND_SOCKET
#define HCI_EVNT_BIND                					HCI_CMND_BIND
#define HCI_EVNT_RECV                					HCI_CMND_RECV
#define HCI_EVNT_ACCEPT              					HCI_CMND_ACCEPT
#define HCI_EVNT_LISTEN              					HCI_CMND_LISTEN
#define HCI_EVNT_CONNECT             					HCI_CMND_CONNECT
#define HCI_EVNT_SELECT              					HCI_CMND_BSD_SELECT
#define HCI_EVNT_CLOSE_SOCKET        					HCI_CMND_CLOSE_SOCKET
#define HCI_EVNT_RECVFROM            					HCI_CMND_RECVFROM
#define HCI_EVNT_SETSOCKOPT          					HCI_CMND_SETSOCKOPT
#define HCI_EVNT_GETSOCKOPT          					HCI_CMND_GETSOCKOPT
#define HCI_EVNT_BSD_GETHOSTBYNAME   					HCI_CMND_GETHOSTNAME
#define HCI_EVNT_MDNS_ADVERTISE   						HCI_CMND_MDNS_ADVERTISE

// In online documentation HCI_EVNT_SEND and HCI_EVNT_SENDTO are named
// HCI_EVNT_DATA_SEND and HCI_EVNT_DATA_SENDTO
#define HCI_EVNT_SEND									0x1003
#define HCI_EVNT_WRITE									0x100E
#define HCI_EVNT_SENDTO									0x100F

#define HCI_EVNT_PATCHES_REQ    						0x1000

// This should not be used. There are two commands that are 0x4XXX
//#define HCI_EVNT_UNSOL_BASE    							0x4000

#define HCI_EVNT_WLAN_UNSOL_BASE     					(0x8000)

#define HCI_EVNT_WLAN_UNSOL_CONNECT  	 				(0x0001 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_UNSOL_DISCONNECT   				(0x0002 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_UNSOL_INIT         				(0x0004 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_TX_COMPLETE         				(0x0008 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_UNSOL_DHCP         				(0x0010 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_ASYNC_PING_REPORT  				(0x0040 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE  		(0x0080 + HCI_EVNT_WLAN_UNSOL_BASE)
#define HCI_EVNT_WLAN_KEEPALIVE			 				(0x0200 + HCI_EVNT_WLAN_UNSOL_BASE)
#define	HCI_EVNT_BSD_TCP_CLOSE_WAIT      				(0x0800 + HCI_EVNT_WLAN_UNSOL_BASE)

#define HCI_EVNT_DATA_UNSOL_FREE_BUFF					0x4100

#define HCI_EVNT_NVMEM_CREATE_ENTRY 					HCI_CMND_NVMEM_CREATE_ENTRY
#define HCI_EVNT_NVMEM_SWAP_ENTRY 						HCI_CMND_NVMEM_SWAP_ENTRY

#define HCI_EVNT_NVMEM_READ     						HCI_CMND_NVMEM_READ
#define HCI_EVNT_NVMEM_WRITE    						(0x0202)

#define HCI_EVNT_READ_SP_VERSION						HCI_CMND_READ_SP_VERSION

#define HCI_EVNT_INPROGRESS								0xFFFF


#define HCI_DATA_RECVFROM       						0x84
#define HCI_DATA_RECV           						0x85
#define HCI_DATA_NVMEM_READ        						0x91

#define HCI_EVENT_CC3000_CAN_SHUT_DOWN 					0x99

//*****************************************************************************
//
// Offsets into various command and event buffers
//
//*****************************************************************************

// Number of buffers freed in HCI_EVNT_DATA_UNSOL_FREE_BUFF
#define HCI_FLOW_CONTROL_EVENT_OFFSET					5


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup CC3000_Event_Handler_API
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Asynchronous callback function
//!
//! Application supplied callback function that is called when an event
//! is recieved from the CC3000.
//!
//! @param	bHCI_Type
//! @param	wHCI_Opcode
//! @param	pbData
//! @param	wDataSize
//
//*****************************************************************************
typedef void (*pAsyncCallBackFunction_t) (uint8_t  bHCI_Type, 
										  uint16_t wHCI_Opcode,
										  uint8_t* pbData,
										  uint16_t wDataSize);

//! Global pointer to the asynchronous callback function
extern pAsyncCallBackFunction_t g_pAsyncCallBackFunction;

//*****************************************************************************
//
//! cc3000_event_handler
//!
//!  @return	none
//!
//!  @brief		Handles all incoming data from cc3000
//
//*****************************************************************************
extern void cc3000_event_handler(void);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_EVENT_HANDLER_H_ */
