/*
 * cc3000_wlan.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */

#include "cc3000_wlan.h"
#include "cc3000_platform.h"
#include "cc3000_event_handler.h"
#include "cc3000_hci.h"
#include "cc3000_spi.h"

#include <string.h>

//*****************************************************************************
//
//! cc3000_wlan_disconnect
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Send command to disconnect from the WLAN
//!
//
//*****************************************************************************
void cc3000_wlan_disconnect(void) 
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_disconnect\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_DISCONNECT, 0);
	cc3000_hci_end_command();
} // cc3000_wlan_disconnect

//*****************************************************************************
//
//! cc3000_wlan_get_status
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Send command to get current WLAN status
//!
//
//*****************************************************************************
void cc3000_wlan_get_status(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_get_status\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_STATUSGET, 0);
	cc3000_hci_end_command();
} // cc3000_wlan_get_status

//*****************************************************************************
//
//! cc3000_wlan_set_scan_params
//!
//!  @param  	scan_frequency
//!
//!  @param  	channel_mask
//!
//!  @return  	none
//!
//!  @brief  	Send command to set the scan parameter
//!
//
//*****************************************************************************
void cc3000_wlan_set_scan_params(uint32_t scan_frequency, uint16_t channel_mask )
{
	uint8_t count;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_set_scan_params\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_SET_SCANPARAM, 25*4);
	cc3000_hci_send_uint32(36);				// This 36 appears to be undocumented and magic
	cc3000_hci_send_uint32(scan_frequency);
	cc3000_hci_send_uint32(20);				// min dwell time
	cc3000_hci_send_uint32(100);			// max dwell time
	cc3000_hci_send_uint32(5);				// max probe request between dwell time
	cc3000_hci_send_uint32(channel_mask);
	cc3000_hci_send_uint32((uint32_t)-120);	// rssi threshold
	cc3000_hci_send_uint32(0);				// SNR threshold
	cc3000_hci_send_uint32(205);			// probe tx power

	for (count = 0; count < 16; count++)
	{
		cc3000_hci_send_uint32(2000);		// each channel entry periodic scan
	}

	cc3000_hci_end_command();
} // cc3000_wlan_set_scan_params

//*****************************************************************************
//
//! cc3000_wlan_connect
//!
//!  @param  	ssid		SSID of the AP to connect to
//!
//!  @param  	sec_type	security type of the AP (can be WLAN_SEC_UNSEC,
//!							WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2)
//!
//!  @param  	key			AP key
//!
//!  @return  	none
//!
//!  @brief  	Send command to connect to a given AP
//!
//
//*****************************************************************************
void cc3000_wlan_connect(const char* ssid, uint8_t sec_type, const char* key)
{
	uint8_t key_len;
	uint8_t ssid_len;
	uint8_t count;

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_connect\n\r"));
	#endif

	ssid_len = strlen(ssid);
	key_len  = strlen(key);

	cc3000_hci_start_command(HCI_CMND_WLAN_CONNECT, 28 + ssid_len + key_len);
	cc3000_hci_send_uint32(0x0000001c);	// magic
	cc3000_hci_send_uint32(ssid_len);
	cc3000_hci_send_uint32(sec_type);
	cc3000_hci_send_uint32(0x10 + ssid_len);
	cc3000_hci_send_uint32(key_len);
	cc3000_hci_send_uint16(0);			// magic 0x0000

	// bssid
	for (count = 0; count < 6; count++)
	{
		cc3000_hci_send_uint8(0);
	}

	// ssid
	for (count = 0; count < ssid_len; count++)
	{
		cc3000_hci_send_uint8(ssid[count]);
	}

	// key
	if ((key) && (key_len))
	{
		for (count = 0; count < key_len; count++)
		{
			cc3000_hci_send_uint8(key[count]);
		}
	}

	cc3000_hci_end_command();
} // cc3000_wlan_connect

//*****************************************************************************
//
//! cc3000_wlan_set_event_mask
//!
//!  @param  	mask	Events to mask
//!
//!  @return  	none
//!
//!  @brief  	Send command to set the event mask for unsolicited events
//!
//
//*****************************************************************************
void cc3000_wlan_set_event_mask(uint16_t mask)
{

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_set_event_mask\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_EVENT_MASK, 4);
	cc3000_hci_send_uint32(mask);
	cc3000_hci_end_command();
} // cc3000_wlan_set_event_mask

//*****************************************************************************
//
//! cc3000_wlan_set_connection_policy
//!
//!  @param  	fast_connect	Use fast_connect
//!
//!  @param  	open_ap_connect	Connect to any open AP
//!
//!  @param  	use profiles	Use saved profiles to connect
//!
//!  @return  	none
//!
//!  @brief  	Send command to set the connection policy
//!
//
//*****************************************************************************
void cc3000_wlan_set_connection_policy(enum fast_connect_options_enum 	 fast_connect,
									   enum open_ap_connect_options_enum open_ap_connect,
									   enum use_profiles_options_enum 	 use_profiles)
{

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_set_event_mask\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_SET_CONNECTION_POLICY, 3*4);
	cc3000_hci_send_uint32(open_ap_connect);
	cc3000_hci_send_uint32(fast_connect);
	cc3000_hci_send_uint32(use_profiles);
	cc3000_hci_end_command();
} // cc3000_wlan_set_connection_policy

//*****************************************************************************
//
//! cc3000_wlan_req_scan_result
//!
//!  @param  	none
//!
//!  @return  	none
//!
//!  @brief  	Send command to request the WLAN scan results
//!
//
//*****************************************************************************
void cc3000_wlan_req_scan_result(void)
{

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_req_scan_result\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_GET_SCAN_RESULTS, 4);
	cc3000_hci_send_uint32(0x0000);	// magic
	cc3000_hci_end_command();
} // cc3000_wlan_req_scan_result

//*****************************************************************************
//
//! cc3000_wlan_get_scan_result
//!
//!  @param  	ap_entry	Pointer to AP entry list
//!
//!  @return  	none
//!
//!  @brief  	Saves and converts the received data stream to an AP entry
//!
//
//*****************************************************************************
void cc3000_wlan_get_scan_result(ap_entry_t *ap_entry)
{
	uint8_t ssid_length;
	uint8_t count;

	// ToDO: determine sane buffer size
	uint8_t wlan_data[57];

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_req_scan_result\n\r"));
	#endif

	count = cc3000_spi_rx_buffer_size;
	cc3000_spi_get_rx_buffer_stream(wlan_data,5,count);
	
	ap_entry->entry_count = wlan_data[0];
	ap_entry->scan_status = wlan_data[4];

	ap_entry->entry_valid = wlan_data[8] &  0x01;
	ap_entry->rssi        = wlan_data[8] >> 1;
	ap_entry->security    = wlan_data[9] &  0b00000011;
	ssid_length           = wlan_data[9] >> 2;

	for (count = 0; count < ssid_length; count++)
	{
		ap_entry->ssid[count] = wlan_data[12 + count];
	}
	ap_entry->ssid[count] = 0;
} // cc3000_wlan_get_scan_result

//*****************************************************************************
//
//!  wlan_add_profile
//!
//!  @param    ulSecType					WLAN_SEC_UNSEC,WLAN_SEC_WEP,WLAN_SEC_WPA,WLAN_SEC_WPA2
//!  @param    ucSsid						SSID up to 32 bytes
//!  @param    ulSsidLen					SSID length
//!  @param    ucBssid						BSSID 6 bytes
//!  @param    ulPriority					Profile priority. Lowest priority:0.
//!  @param    ulPairwiseCipher_Or_TxKeyLen	Key length for WEP security
//!  @param    ulGroupCipher_TxKeyIndex		Key index
//!  @param    ulKeyMgmt					KEY management
//!  @param    ucPf_OrKey					Security key
//!  @param    ulPassPhraseLen				Security key length for WPA\WPA2
//!
//!  @return    On success, zero is returned. On error, -1 is returned
//!
//!  @brief     When auto start is enabled, the device connects to
//!             station from the profiles table. Up to 7 profiles are supported.
//!             If several profiles configured the device choose the highest
//!             priority profile, within each priority group, device will choose
//!             profile based on security policy, signal strength, etc
//!             parameters. All the profiles are stored in CC3000 NVMEM.
//!
//!  @sa        wlan_ioctl_del_profile
//
//*****************************************************************************
void cc3000_wlan_add_profile(uint32_t ulSecType,
							 uint8_t *ucSsid,
							 uint32_t ulSsidLen,
							 uint8_t *ucBssid,
							 uint32_t ulPriority,
							 uint32_t ulPairwiseCipher_Or_TxKeyLen,
							 uint32_t ulGroupCipher_TxKeyIndex,
							 uint32_t ulKeyMgmt,
							 uint8_t *ucPf_OrKey,
							 uint32_t ulPassPhraseLen)
{
	uint8_t arg_len;
	uint8_t *p;
	int32_t i = 0;
	uint8_t bssid_zero[] = {0, 0, 0, 0, 0, 0};

	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_add_profile\n\r"));
	#endif

	// Setup arguments in accordance with the security type
	switch (ulSecType)
	{
		//OPEN
		case WLAN_SEC_UNSEC:
			arg_len = 14 + ETH_ALEN + ulSsidLen;
			cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_ADD_PROFILE, arg_len);
			cc3000_hci_send_uint32(0x00000014);
			cc3000_hci_send_uint32(ulSsidLen);
			cc3000_hci_send_uint16(0);

			if(ucBssid)
			{
				cc3000_hci_send_stream(ucBssid, ETH_ALEN);
			}
			else
			{
				cc3000_hci_send_stream(bssid_zero, ETH_ALEN);
			}

			cc3000_hci_send_uint32(ulPriority);
			cc3000_hci_send_stream(ucSsid, ulSsidLen);
			break;

		//WEP
		case WLAN_SEC_WEP:
			arg_len = 28 + ETH_ALEN + ulSsidLen + (4 * ulPairwiseCipher_Or_TxKeyLen);
			cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_ADD_PROFILE, arg_len);
			cc3000_hci_send_uint32(0x00000020);
			cc3000_hci_send_uint32(ulSsidLen);
			cc3000_hci_send_uint32(0);

			if(ucBssid)
			{
				cc3000_hci_send_stream(ucBssid, ETH_ALEN);
			}
			else
			{
				cc3000_hci_send_stream(bssid_zero, ETH_ALEN);
			}
			cc3000_hci_send_uint32(ulPriority);
			cc3000_hci_send_uint32(0x0000000C + ulSsidLen);
			cc3000_hci_send_uint32(ulPairwiseCipher_Or_TxKeyLen);
			cc3000_hci_send_uint32(ulGroupCipher_TxKeyIndex);
			cc3000_hci_send_stream(ucSsid, ulSsidLen);

			for(i = 0; i < 4; i++)
			{
				p = &ucPf_OrKey[i * ulPairwiseCipher_Or_TxKeyLen];
				cc3000_hci_send_stream(p, ulPairwiseCipher_Or_TxKeyLen);
			}
			break;

		//WPA
		//WPA2
		case WLAN_SEC_WPA:
		case WLAN_SEC_WPA2:
			arg_len = 34 + ETH_ALEN + ulSsidLen + ulPassPhraseLen;
			cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_ADD_PROFILE, arg_len);
			cc3000_hci_send_uint32(0x00000028);
			cc3000_hci_send_uint32(ulSsidLen);
			cc3000_hci_send_uint16(0);

			if(ucBssid)
			{
				cc3000_hci_send_stream(ucBssid, ETH_ALEN);
			}
			else
			{
				cc3000_hci_send_stream(bssid_zero, ETH_ALEN);
			}

			cc3000_hci_send_uint32(ulPriority);
			cc3000_hci_send_uint32(ulPairwiseCipher_Or_TxKeyLen);
			cc3000_hci_send_uint32(ulGroupCipher_TxKeyIndex);
			cc3000_hci_send_uint32(ulKeyMgmt);
			cc3000_hci_send_uint32(0x00000008 + ulSsidLen);
			cc3000_hci_send_uint32(ulPassPhraseLen);
			cc3000_hci_send_stream(ucSsid, ulSsidLen);
			cc3000_hci_send_stream(ucPf_OrKey, ulPassPhraseLen);
			break;
	}

	cc3000_hci_end_command();
} // cc3000_wlan_add_profile

//*****************************************************************************
//
//!  wlan_ioctl_del_profile
//!
//!  @param    index   Number of profile to delete
//!
//!  @return    On success, zero is returned. On error, -1 is returned
//!
//!  @brief     Delete WLAN profile
//!
//!  @Note      In order to delete all stored profile, set index to 255.
//!
//!  @sa        wlan_add_profile
//
//*****************************************************************************
void cc3000_wlan_del_profile(uint32_t ulIndex)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_wlan_del_profile\n\r"));
	#endif

	cc3000_hci_start_command(HCI_CMND_WLAN_IOCTL_DEL_PROFILE, 4);
	cc3000_hci_send_uint32(ulIndex);
	cc3000_hci_end_command();
} // cc3000_wlan_del_profile
