/*
 * cc3000_netapp.h
 *
 *  Created on: 12.09.2013
 *      Author: Johannes
 */
#ifndef CC3000_NETAPP_H_
#define CC3000_NETAPP_H_

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_Netapp_API
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//!  cc3000_netapp_dhcp
//!
//!  @param  aucIP
//!  @param  aucSubnetMask
//!  @param  aucDefaultGateway
//!  @param  aucDNSServer
//!
//!  @return      None, but cc3000_spi_get_rx_buffer_uint32() should read 0 on success
//!
//!  @brief       netapp_dhcp is used to configure the network interface,
//!               static or dynamic (DHCP).\n In order to activate DHCP mode,
//!               aucIP, aucSubnetMask, aucDefaultGateway must be 0.
//!               The default mode of CC3000 is DHCP mode.
//!               Note that the configuration is saved in non volatile memory
//!               and thus preserved over resets.
//!
//! @note         If the mode is altered a reset of CC3000 device is required
//!               in order to apply changes. Also note that asynchronous event
//!               of DHCP_EVENT, which is generated when an IP address is
//!               allocated either by the DHCP server or due to static
//!               allocation is generated only upon a connection to the
//!               AP was established.
//!
//*****************************************************************************
extern void cc3000_netapp_dhcp(uint32_t aucIP, uint32_t aucSubnetMask, uint32_t aucDefaultGateway, uint32_t aucDNSServer);

//*****************************************************************************
//
//!  cc3000_netapp_timeout_values
//!
//!  @param  aucDHCP    DHCP lease time request, also impact
//!                     the DHCP renew timeout. Range: [0-0xffffffff] seconds,
//!                     0 or 0xffffffff == infinity lease timeout.
//!                     Resolution:10 seconds. Influence: only after
//!                     reconnecting to the AP.
//!                     Minimal bound value: MIN_TIMER_VAL_SECONDS - 20 seconds.
//!                     The parameter is saved into the CC3000 NVMEM.
//!                     The default value on CC3000 is 14400 seconds.
//!
//!  @param  aucARP     ARP refresh timeout, if ARP entry is not updated by
//!                     incoming packet, the ARP entry will be  deleted by
//!                     the end of the timeout.
//!                     Range: [0-0xffffffff] seconds, 0 == infinity ARP timeout
//!                     Resolution: 10 seconds. Influence: on runtime.
//!                     Minimal bound value: MIN_TIMER_VAL_SECONDS - 20 seconds
//!                     The parameter is saved into the CC3000 NVMEM.
//!	                    The default value on CC3000 is 3600 seconds.
//!
//!  @param  aucKeepalive   Keepalive event sent by the end of keepalive timeout
//!                         Range: [0-0xffffffff] seconds, 0 == infinity timeout
//!                         Resolution: 10 seconds.
//!                         Influence: on runtime.
//!                         Minimal bound value: MIN_TIMER_VAL_SECONDS - 20 sec
//!                         The parameter is saved into the CC3000 NVMEM.
//!                         The default value on CC3000 is 10 seconds.
//!
//!  @param  aucInactivity   Socket inactivity timeout, socket timeout is
//!                          refreshed by incoming or outgoing packet, by the
//!                          end of the socket timeout the socket will be closed
//!                          Range: [0-0xffffffff] sec, 0 == infinity timeout.
//!                          Resolution: 10 seconds. Influence: on runtime.
//!                          Minimal bound value: MIN_TIMER_VAL_SECONDS - 20 sec
//!                          The parameter is saved into the CC3000 NVMEM.
//!	                         The default value on CC3000 is 60 seconds.
//!
//!  @return       None, but cc3000_spi_get_rx_buffer_uint32() should read 0 on success
//!
//!  @brief       Set new timeout values. Function set new timeout values for:
//!               DHCP lease timeout, ARP  refresh timeout, keepalive event
//!               timeout and socket inactivity timeout
//!
//! @note         If a parameter set to non zero value which is less than 20s,
//!               it will be set automatically to 20s.
//!
//*****************************************************************************
extern void cc3000_netapp_timeout_values(uint32_t aucDHCP, uint32_t aucARP, uint32_t aucKeepalive, uint32_t aucInactivity);

//*****************************************************************************
//
//!  cc3000_netapp_set_debug_level
//!
//!  @param[in] level    Debug level. Bitwise [0-8], 0(disable) or 1(enable).
//!                      Bitwise map: 0 - Critical message,
//!                                   1 - Information message,
//!									  2 - Core messages,
//!									  3 - HCI messages,
//!									  4 - Network stack messages,
//!									  5 - WLAN messages,
//!									  6 - WLAN driver messages,
//!									  7 - EEPROM messages,
//!                         		  8 - General messages.
//!									  Default: 0x13f. Saved: no
//!
//!  @return  On success, zero is returned. On error, -1 is returned
//!
//!  @brief   Debug messages sent via the UART debug channel, this function
//!           enables/disables the debug level
//!
//*****************************************************************************
extern void cc3000_netapp_set_debug_level(uint32_t ulLevel);

//*****************************************************************************
//
//!  cc3000_netapp_ping_send
//!
//!  @param  ip				Destination IP address. LSB is high byte of IP.
//!  @param  pingAttempts	Number of echo requests to send
//!  @param  pingSize		Send buffer size which may be up to 1400 bytes
//!  @param  pingTimeout	Time to wait for a response, in milliseconds.
//!
//!  @brief   Send ICMP ECHO_REQUEST to network hosts
//!
//!  @note    If an operation finished successfully asynchronous ping report 
//!           event will be generated. The report structure is as defined by
//!           structure netapp_pingreport_args_t.
//!
//!  @warning Calling this function while a previous Ping Requests are in 
//!           progress will stop the previous ping request.
//!
//*****************************************************************************
extern void cc3000_netapp_ping_send(uint32_t ip, uint32_t ulPingAttempts, uint32_t ulPingSize, uint32_t ulPingTimeout);

//*****************************************************************************
//
//!  cc3000_netapp_ipconfig
//!
//!  @brief   Obtain the CC3000 Network interface information. Note that the
//!           information is available only after the WLAN connection was
//!           established. Calling this function before associated, will cause
//!           non-defined values to be returned.
//!
//!  @note    The function is useful for figuring out the IP Configuration of
//!           the device when DHCP is used and for figuring out the SSID of
//!           the Wireless network the device is associated with.
//!
//*****************************************************************************
extern void cc3000_netapp_ipconfig(void);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_NETAPP_H_ */
