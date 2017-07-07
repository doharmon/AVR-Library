/*---------------------------------------------------------------------------
 *
 * SeaTalk_config.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

#ifndef SEATALK_CONFIG_H_
#define SEATALK_CONFIG_H_

//////////////////////////////////////////////////////////////////////////
//! Minimum time in milliseconds to wait since last byte received before
//! attempting to transmit.
//! Bus is idle when +12V for at least 10/4800 seconds (2.083 ms)
#define stconfigMIN_WAIT_TIME		(10)
//#define stconfigMIN_WAIT_TIME		(254)

#define stconfigTIMER_MASK			0x0000003F
#define stconfigTIMER0				0x00000001
#define stconfigTIMER1				0x00000002
#define stconfigTIMER2				0x00000004
#define stconfigTIMER3				0x00000008
#define stconfigTIMER4				0x00000010
#define stconfigTIMER5				0x00000020

#define stconfigOCR_MASK			0x000001C0
#define stconfigOCR_A				0x00000040
#define stconfigOCR_B				0x00000080
#define stconfigOCR_C				0x00000100

#define stconfigICP_MASK			0x0000FC00
#define stconfigICP0				0x00000400
#define stconfigICP1				0x00000800
#define stconfigICP2				0x00001000
#define stconfigICP3				0x00002000
#define stconfigICP4				0x00004000
#define stconfigICP5				0x00008000

#define stconfigINT_MASK			0x00FF0000
#define stconfigINT0				0x00010000
#define stconfigINT1				0x00020000
#define stconfigINT2				0x00040000
#define stconfigINT3				0x00080000
#define stconfigINT4				0x00100000
#define stconfigINT5				0x00200000
#define stconfigINT6				0x00400000
#define stconfigINT7				0x00800000

#define stconfigPCPORT_MASK		0x0F000000
#define stconfigPCPORTA			0x01000000
#define stconfigPCPORTB			0x02000000
#define stconfigPCPORTC			0x04000000
#define stconfigPCPORTD			0x08000000

#define stconfigPCBIT0				0x01
#define stconfigPCBIT1				0x02
#define stconfigPCBIT2				0x04
#define stconfigPCBIT3				0x08
#define stconfigPCBIT4				0x10
#define stconfigPCBIT5				0x20
#define stconfigPCBIT6				0x40
#define stconfigPCBIT7				0x80

#endif /* SEATALK_CONFIG_H_ */
