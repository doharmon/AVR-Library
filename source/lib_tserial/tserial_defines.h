/*---------------------------------------------------------------------------
 *
 * tserial_config.h
 *
 * Created: 12/25/2015 5:39:34 PM
 *  Author: dharmon
 *
 *-------------------------------------------------------------------------*/

#ifndef TSERIAL_CONFIG_H_
#define TSERIAL_CONFIG_H_

#define tsconfigTIMER_MASK			0x0000003F
#define tsconfigTIMER0				0x00000001
#define tsconfigTIMER1				0x00000002
#define tsconfigTIMER2				0x00000004
#define tsconfigTIMER3				0x00000008
#define tsconfigTIMER4				0x00000010
#define tsconfigTIMER5				0x00000020

#define tsconfigOCR_MASK			0x000001C0
#define tsconfigOCR_A				0x00000040
#define tsconfigOCR_B				0x00000080
#define tsconfigOCR_C				0x00000100

#define tsconfigASCII				0x00000200

#define tsconfigICP_MASK			0x0000FC00
#define tsconfigICP0				0x00000400
#define tsconfigICP1				0x00000800
#define tsconfigICP2				0x00001000
#define tsconfigICP3				0x00002000
#define tsconfigICP4				0x00004000
#define tsconfigICP5				0x00008000

#define tsconfigINT_MASK			0x00FF0000
#define tsconfigINT0				0x00010000
#define tsconfigINT1				0x00020000
#define tsconfigINT2				0x00040000
#define tsconfigINT3				0x00080000
#define tsconfigINT4				0x00100000
#define tsconfigINT5				0x00200000
#define tsconfigINT6				0x00400000
#define tsconfigINT7				0x00800000

#define tsconfigPCPORT_MASK			0x0F000000
#define tsconfigPCPORTA				0x01000000
#define tsconfigPCPORTB				0x02000000
#define tsconfigPCPORTC				0x04000000
#define tsconfigPCPORTD				0x08000000

#define tsconfigPCBIT0				0x01
#define tsconfigPCBIT1				0x02
#define tsconfigPCBIT2				0x04
#define tsconfigPCBIT3				0x08
#define tsconfigPCBIT4				0x10
#define tsconfigPCBIT5				0x20
#define tsconfigPCBIT6				0x40
#define tsconfigPCBIT7				0x80

#endif /* TSERIAL_CONFIG_H_ */
