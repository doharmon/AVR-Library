/*
 * cc3000_wlan.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */
#ifndef CC3000_WLAN_H_
#define CC3000_WLAN_H_

#include "cc3000_config.h"
#include "cc3000_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_WLAN_API
//! @{
//
//*****************************************************************************

#define ETH_ALEN			6
#define MAXIMAL_SSID_LENGTH	32

#define      WLAN_SEC_UNSEC 0
#define      WLAN_SEC_WEP	1
#define      WLAN_SEC_WPA	2
#define      WLAN_SEC_WPA2	3

enum open_ap_connect_options_enum 
{
	open_ap_auto_connect_disabled = 0,
	open_ap_auto_connect_enabled  = 1
};

enum fast_connect_options_enum
{
	fast_connect_disabled = 0,
	fast_connect_enabled  = 1
};

enum use_profiles_options_enum
{
	use_profiles_disabled = 0,
	use_profiles_enabled  = 1
};

enum con_status_enum
{
	con_disconnected = 0,
	con_scanning     = 1,
	con_connecting   = 2,
	con_connected    = 3
};

/**
	\struct ap_entry_t AP access point
*/

typedef struct _ap_entry_t
{
	uint8_t entry_count;	//!< Number of this entry point (zero for last)
	uint8_t scan_status;	//!< Status of the scan 0 = aged results, 1 = results valid, 2 = no result
	uint8_t entry_valid;	//!< Validity of the entry, 0 = invalid, 1 = valid
	uint8_t rssi;			//!< RSSI entry
	uint8_t security;		//!< Secutity type, 0 = Open, 1 = WEP, 2 = WPA, 3 = WPA2
	uint8_t ssid[32 + 1];	//!< SSID name
} ap_entry_t;

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
extern void cc3000_wlan_disconnect(void);

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
extern void cc3000_wlan_get_status(void);

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
extern void cc3000_wlan_set_scan_params(uint32_t scan_frequency, uint16_t channel_mask );

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
extern void cc3000_wlan_connect(const char* ssid, uint8_t sec_type, const char* key);

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
extern void cc3000_wlan_set_event_mask(uint16_t mask);

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
extern void cc3000_wlan_set_connection_policy(enum fast_connect_options_enum fast_connect,
											  enum open_ap_connect_options_enum open_ap_connect,
 											  enum use_profiles_options_enum use_profiles);

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
extern void cc3000_wlan_req_scan_result(void);

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
extern void cc3000_wlan_get_scan_result(ap_entry_t *ap_entry);

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
extern void cc3000_wlan_add_profile(uint32_t ulSecType,
									uint8_t *ucSsid,
									uint32_t ulSsidLen,
									uint8_t *ucBssid,
									uint32_t ulPriority,
									uint32_t ulPairwiseCipher_Or_TxKeyLen,
									uint32_t ulGroupCipher_TxKeyIndex,
									uint32_t ulKeyMgmt,
									uint8_t *ucPf_OrKey,
									uint32_t ulPassPhraseLen);

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
extern void cc3000_wlan_del_profile(uint32_t ulIndex);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_WLAN_H_ */
