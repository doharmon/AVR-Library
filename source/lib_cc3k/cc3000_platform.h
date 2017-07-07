/*
 * cc3000_platform.h
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 */
#ifndef CC3000_PLATFORM_H_
#define CC3000_PLATFORM_H_

#include <avr/io.h>

#include "cc3000_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup CC3000_Platform_API
//! @{
//
//*****************************************************************************

// ATMEL ATMEGA168 & 328 / ARDUINO
//
//                                    +-\/-+
//              (PCINT14/RESET) PC6  1|    |28 PC5 (ADC5/SCL/PCINT13) (AI5)
//     (D0)       (PCINT16/RXD) PD0  2|    |27 PC4 (ADC4/SDA/PCINT12) (AI4)
//     (D1)       (PCINT17/TXD) PD1  3|    |26 PC3 (ADC3/PCINT11)     (AI3)
//     (D2)      (PCINT18/INT0) PD2  4|    |25 PC2 (ADC2/PCINT10)     (AI2)
// PWM (D3) (PCINT19/OC2B/INT1) PD3  5|    |24 PC1 (ADC1/PCINT9)      (AI1)
//     (D4)    (PCINT20/XCK/T0) PD4  6|    |23 PC0 (ADC0/PCINT8)      (AI0)
//                              VCC  7|    |22 GND
//                              GND  8|    |21 AREF
//         (PCINT6/XTAL1/TOSC1) PB6  9|    |20 AVCC
//         (PCINT7/XTAL2/TOSC2) PB7 10|    |19 PB5 (SCK/PCINT5)       (D13)
// PWM (D5)   (PCINT21/OC0B/T1) PD5 11|    |18 PB4 (MISO/PCINT4)      (D12)
// PWM (D6) (PCINT22/OC0A/AIN0) PD6 12|    |17 PB3 (MOSI/OC2A/PCINT3) (D11) PWM
//     (D7)      (PCINT23/AIN1) PD7 13|    |16 PB2 (SS/OC1B/PCINT2)   (D10) PWM
//     (D8)  (PCINT0/CLKO/ICP1) PB0 14|    |15 PB1 (OC1A/PCINT1)      (D9)  PWM
//                                    +----+
//
// ATMEL ATMEGA1284
//
//                                    +-\/-+
//             (PCINT8/XCK0/T0) PB0  1|    |40 PA0 (ADC0/PCINT0)
//             (PCINT9/CLKO/T1) PB1  2|    |39 PA1 (ADC1/PCINT1)
//          (PCINT10/INT2/AIN0) PB2  3|    |38 PA2 (ADC2/PCINT2)
//          (PCINT11/OC0A/AIN1) PB3  4|    |37 PA3 (ADC3/PCINT3)
//            (PCINT12/OC0B/SS) PB4  5|    |36 PA4 (ADC4/PCINT4)
//          (PCINT13/ICP3/MOSI) PB5  6|    |35 PA5 (ADC5/PCINT5)
//          (PCINT14/OC3A/MISO) PB6  7|    |34 PA6 (ADC6/PCINT6)
//           (PCINT15/OC3B/SCK) PB7  8|    |33 PA7 (ADC7/PCINT7)
//                            RESET  9|    |32 AREF
//                              VCC 10|    |31 GND
//                              GND 11|    |30 AVCC
//                            XTAL1 12|    |29 PC7 (TOSC2/PCINT23)
//                            XTAL2 13|    |28 PC6 (TOSC1/PCINT22)
//            (PCINT24/RXD0/T3) PD0 14|    |27 PC5 (TDI/PCINT21)
//               (PCINT25/TXD0) PD1 15|    |26 PC4 (TDO/PCINT20)
//          (PCINT26/RXD1/INT0) PD2 16|    |25 PC3 (TMS/PCINT19)
//          (PCINT27/TXD1/INT1) PD3 17|    |24 PC2 (TCK/PCINT18)
//          (PCINT28/XCK1/OC1B) PD4 18|    |23 PC1 (SDA/PCINT17)
//               (PCINT29/OC1A) PD5 19|    |22 PC0 (SCL/PCINT16)
//           (PCINT30/OC2B/ICP) PD6 20|    |21 PD7 (OC2A/PCINT31)
//                                    +----+
//

//*****************************************************************************
//
// SPI Registers
//
//   SPCR
//     7    6    5    4    3    2    1    0
//   SPIE SPE  DORD MSTR CPOL CPHA SPR1 SPR0
//     |    |    |    |    |    |    |    |
//     |    |    |    |    |    |    +----+-- SPI_CLOCK_DIV#
//     |    |    |    |    +----+------------ SPI_MODE#
//     |    |    |    +---------------------- Master(1) / Slave(0)
//     |    |    +--------------------------- MSBFIRST(0), LSBFIRST(1)
//     |    +-------------------------------- SPI Enable (1)
//     +------------------------------------- SPI Interrupt Enable (1)
//
//   SPSR
//     7    6    5    4    3    2    1    0
//   SPIF WCOL   -    -    -    -    -  SPI2X
//     |    |                             |
//     |    |                             +-- SPI_CLOCK_DIV# >> 2
//     |    +-- Read only. Set when SPDR written to during transfer. 
//     |         Cleared by reading SPSR then accessing SPDR.
//     +------- Read only. Set when transfer is complete. Cleared when SPI
//              interrupt handler called or when SPIF bit and SPDR are read.
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for hardware SPI configuration
//
//*****************************************************************************
#define SPI_CLOCK_DIV2    0x04
#define SPI_CLOCK_DIV4    0x00
#define SPI_CLOCK_DIV8    0x05
#define SPI_CLOCK_DIV16   0x01
#define SPI_CLOCK_DIV32   0x06
#define SPI_CLOCK_DIV64   0x02
#define SPI_CLOCK_DIV128  0x03

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK    0x0C  //!< CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK   0x03  //!< SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  //!< SPI2X = bit 0 on SPSR

#define SPI_MSB_FIRST  0x00
#define SPI_LSB_FIRST  0x01

//*****************************************************************************
//
// Pins for SPI interface to the CC3000 board
//
//*****************************************************************************
#if defined(__AVR_ATmega328P__)

#define CC3000_SPI_MOSI					PINB3
#define CC3000_SPI_MISO					PINB4
#define CC3000_SPI_SCK					PINB5
#define CC3000_SPI_DIRECTION			DDRB
#define CC3000_SPI_PORT					PORTB
#define CC3000_ENABLE_DIRECTION			DDRD
#define CC3000_ENABLE_PORT				PORTD
#define CC3000_ENABLE_PIN				PIND5
#define CC3000_SLAVE_SELECT_DIRECTION	DDRB
#define CC3000_SLAVE_SELECT_PORT		PORTB
#define CC3000_SLAVE_SELECT_PIN			PINB2
#define CC3000_IRQ_DIRECTION			DDRD
#define CC3000_IRQ_PORT					PORTD
#define CC3000_IRQ_PORT_READ			PIND
#define CC3000_IRQ_PIN					PIND3

#elif defined(__AVR_ATmega1284P__)
#define CC3000_SPI_MOSI					PINB5
#define CC3000_SPI_MISO					PINB6
#define CC3000_SPI_SCK					PINB7
#define CC3000_SPI_DIRECTION			DDRB
#define CC3000_SPI_PORT					PORTB
#define CC3000_ENABLE_DIRECTION			DDRC
#define CC3000_ENABLE_PORT				PORTC
#define CC3000_ENABLE_PIN				PINC3
#define CC3000_SLAVE_SELECT_DIRECTION	DDRC
#define CC3000_SLAVE_SELECT_PORT		PORTC
#define CC3000_SLAVE_SELECT_PIN			PINC4
#define CC3000_IRQ_DIRECTION			DDRC
#define CC3000_IRQ_PORT					PORTC
#define CC3000_IRQ_PORT_READ			PINC
#define CC3000_IRQ_PIN					PINC2
#define CC3000_PWR_DIRECTION			DDRC
#define CC3000_PWR_PORT					PORTC
#define CC3000_PWR_PIN					PINC5

#else

#error "Unknown board"

#endif

#if (CC3000_TRACE_LEVEL > CC3000_TRACE_LEVEL_NONE)
#include <avr/pgmspace.h>
extern void debug_int(unsigned int wert);
extern void debug_int_hex(unsigned char wert);
extern void debug_putc(unsigned char wert);
extern void debug_nl(void);
extern void debug_int_hex_16bit(uint16_t wert);
extern void debug_str(const char *string);
extern void debug_sram_str(const char *string);
void printHexChar(const uint8_t* data, const uint16_t numBytes);
#else
#define debug_int()
#define debug_int_hex()
#define debug_putc()
#define debug_nl()
#define debug_int_hex_16bit()
#define debug_str()
#define debug_sram_str()
#define printHexChar()
#endif // (CC3000_TRACE_LEVEL > CC3000_TRACE_LEVEL_NONE)

//*****************************************************************************
//
//! Define for time out for while loops
//
//*****************************************************************************
#define MAX_WHILE_COUNT		0x0100

//*****************************************************************************
//
//! cc3000_hw_setup
//!
//!  @brief	Hardware setup. This routine should be called before any other
//!			CC3000 APIs.
//
//*****************************************************************************
extern void cc3000_hw_setup(void);

//*****************************************************************************
//
//! cc3000_set_pin_PWR
//!
//!  @brief  Sets the power pin. Powers on the CC3000.
//
//*****************************************************************************
extern void cc3000_set_pin_PWR(void);

//*****************************************************************************
//
//! cc3000_clear_pin_PWR
//!
//!  @brief  Clears the power pin. Powers off the CC3000.
//
//*****************************************************************************
extern void cc3000_clear_pin_PWR(void);

//*****************************************************************************
//
//! cc3000_set_pin_WL_EN
//!
//!  @brief  Sets the enable pin
//
//*****************************************************************************
extern void cc3000_set_pin_WL_EN(void);

//*****************************************************************************
//
//! cc3000_clear_pin_WL_EN
//!
//!  @brief  Clears the enable pin
//
//*****************************************************************************
extern void cc3000_clear_pin_WL_EN(void);

//*****************************************************************************
//
//! cc3000_read_irq_pin
//!
//!  @brief    Reads the state of the IRQ pin
//!
//!  @return   If pin is high 1 is returned else 0 is returned
//
//*****************************************************************************
extern uint8_t cc3000_read_irq_pin(void);

//*****************************************************************************
//
//! cc3000_assert_cs
//!
//!  @brief  Asserts SPI Chip Select for the CC3000
//
//*****************************************************************************
extern void cc3000_assert_cs(void);

//*****************************************************************************
//
//! cc3000_deassert_cs
//!
//!  @brief  Deasserts SPI Chip Select for the CC3000
//
//*****************************************************************************
extern void cc3000_deassert_cs(void);

//*****************************************************************************
//
//! cc3000_spi_send
//!
//!  @param  data    Data to send
//!
//!  @brief  Sends 1 byte to the CC3000
//
//*****************************************************************************
extern void cc3000_spi_send(uint8_t data);

//*****************************************************************************
//
//! cc3000_spi_recv
//!
//!  @return	Returns 1 byte
//!
//!  @brief		Receives 1 byte from CC3000
//
//*****************************************************************************
extern uint8_t cc3000_spi_recv(void);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CC3000_PLATFORM_H_ */
