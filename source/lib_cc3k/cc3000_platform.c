/*
 * cc3000_platform.c
 *
 *  Created on: 05.09.2013
 *      Author: Johannes
 *
 *  Platform dependent functions
 *
 */

#include "../lib_xio/xio.h"
#include "cc3000_platform.h"

#define SPI_CLOCK_DIV	SPI_CLOCK_DIV2

// If this is defined, then we add a little delay
// before CS_ASSERT and after CS_DEASSERT and before cc3000_read_irq_pin
//#define CS_IS_TOO_FAST


#if (CC3000_TRACE_LEVEL > CC3000_TRACE_LEVEL_NONE)
// handle debug output
void debug_str(const char *string)
{
	xprintf_P(PSTR("%S"),string);
}

void debug_sram_str(const char *string)
{
	xprintf_P(PSTR("%s"),string);
}

void debug_int(unsigned int wert)
{
	xprintf_P(PSTR("%u"),wert);
}

void debug_int_hex(unsigned char wert)
{
	xprintf_P(PSTR("0x%02X "),wert);
}

void debug_putc(unsigned char wert)
{
	xprintf_P(PSTR("%c"),wert);
}

void debug_nl(void)
{
	xprintf_P(PSTR("\n\r"));
}

void debug_int_hex_16bit(uint16_t wert)
{
	xprintf_P(PSTR("0x%04X\n\r"),wert);
}

void printHexChar(const uint8_t* data, const uint16_t numBytes)
{
	uint16_t szPos      = 0;
	uint16_t GroupsOf16 = numBytes/16;
	uint16_t Remainder  = numBytes%16;

	for (uint8_t i = 0; i < 28; i++)
		xprintf_P(PSTR("-"));
	
	xprintf_P(PSTR("Bytes: %d"), numBytes);

	for (uint8_t i = 0; i < 28; i++)
	xprintf_P(PSTR("-"));

	debug_nl();
	
	// Print complete groups of 16 bytes
	for (uint16_t i = 0; i < GroupsOf16; i++)
	{
		for (uint8_t j = 0; j < 16; j++, szPos++)
		{
			xprintf_P(PSTR("%02X"), data[szPos]);

			if (j != 16 - 1)
				xprintf_P(PSTR(" "));
		}

		xprintf_P(PSTR("  "));
		szPos -= 16;

		for (uint8_t j=0; j < 16; j++, szPos++)
		{
			if (data[szPos] < 0x20 || data[szPos] > 0x7E)
				xprintf_P(PSTR("."));
			else
				xprintf_P(PSTR("%c"), data[szPos]);
		}

		debug_nl();
	}

	// Print remaining bytes
	for (uint8_t j = 0; j < 16; j++, szPos++)
	{
		if (j < Remainder)
		{
			xprintf_P(PSTR("%02X"), data[szPos]);

			if (j != Remainder - 1)
				xprintf_P(PSTR(" "));
		}
		else
		{
			xprintf_P(PSTR("   "));
		}
	}

	xprintf_P(PSTR("  "));
	szPos -= 16;

	for (uint8_t j=0; j < Remainder; j++, szPos++)
	{
		if (data[szPos] <= 0x20 || data[szPos] >= 0x7E)
			xprintf_P(PSTR("."));
		else
			xprintf_P(PSTR("%c"), data[szPos]);
	}

	debug_nl();
}
#endif // (CC3000_TRACE_LEVEL > CC3000_TRACE_LEVEL_NONE)


//*****************************************************************************
//
//! cc3000_hw_setup
//!
//!  @brief	Hardware setup. This routine should be called before any other
//!			CC3000 APIs.
//
//*****************************************************************************
void cc3000_hw_setup(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_hw_setup\n\r"));
	#endif

	#if defined(__AVR_ATmega1284P__)
	// Set WLAN_PWR pin to output and turn power off to CC3000
	CC3000_PWR_DIRECTION |=  _BV(CC3000_PWR_PIN);  // Set as output
	CC3000_PWR_PORT      &= ~_BV(CC3000_PWR_PIN);  // Clear pin
	#endif

	// Set POWER_EN pin to output and disable the CC3000 by default
	CC3000_ENABLE_DIRECTION |=  _BV(CC3000_ENABLE_PIN);  // Set as output
	CC3000_ENABLE_PORT      &= ~_BV(CC3000_ENABLE_PIN);  // Clear pin
  
	// Set CS pin to output (don't de-assert yet)
	CC3000_SLAVE_SELECT_DIRECTION |= _BV(CC3000_SLAVE_SELECT_PIN); 
	CC3000_SLAVE_SELECT_PORT      |= _BV(CC3000_SLAVE_SELECT_PIN);

	// Set interrupt/gpio pin to input
	CC3000_IRQ_DIRECTION &= ~_BV(CC3000_IRQ_PIN); // Set as input
	CC3000_IRQ_PORT      |=  _BV(CC3000_IRQ_PIN); // Set pull up

	// Initialize SPI (Mode 1)
	// Enable SPI, set as master, set SPI mode and clock speed, MSB first
	// Note: There is no SPI Serial Transfer Complete Interrupt routine, 
	//       therefore, SPIE is not set.
	SPCR = _BV(SPE) | _BV(MSTR) | SPI_MODE1 | (SPI_CLOCK_DIV & SPI_CLOCK_MASK);
	SPSR = (SPI_CLOCK_DIV >> 2);
  
	// Configure MISO as input with pull up
	CC3000_SPI_DIRECTION &= ~_BV(CC3000_SPI_MISO);
	CC3000_SPI_PORT      |=  _BV(CC3000_SPI_MISO);
  
	// Configure MOSI and SCK as outputs
	CC3000_SPI_DIRECTION |= _BV(CC3000_SPI_MOSI) | _BV(CC3000_SPI_SCK);
} // cc3000_hw_setup

//*****************************************************************************
//
//! cc3000_set_pin_PWR
//!
//!  @brief  Sets the power pin. Powers on the CC3000.
//
//*****************************************************************************
void cc3000_set_pin_PWR(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_set_pin_PWR\n\r"));
	#endif

	#if defined(__AVR_ATmega1284P__)
	CC3000_PWR_PORT |= _BV(CC3000_PWR_PIN);
	#endif
} // cc3000_set_pin_PWR

//*****************************************************************************
//
//! cc3000_clear_pin_PWR
//!
//!  @brief  Clears the power pin. Powers off the CC3000.
//
//*****************************************************************************
void cc3000_clear_pin_PWR(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_clear_pin_PWR\n\r"));
	#endif

	#if defined(__AVR_ATmega1284P__)
	CC3000_PWR_PORT &= ~_BV(CC3000_PWR_PIN);
	#endif
} // cc3000_clear_pin_PWR

//*****************************************************************************
//
//! cc3000_set_pin_WL_EN
//!
//!  @brief  Sets the enable pin
//
//*****************************************************************************
void cc3000_set_pin_WL_EN(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_set_pin_WL_EN\n\r"));
	#endif

	CC3000_ENABLE_PORT |= _BV(CC3000_ENABLE_PIN);
} // cc3000_set_pin_WL_EN

//*****************************************************************************
//
//! cc3000_clear_pin_WL_EN
//!
//!  @brief  Clears the enable pin
//
//*****************************************************************************
void cc3000_clear_pin_WL_EN(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_clear_pin_WL_EN\n\r"));
	#endif

	CC3000_ENABLE_PORT &= ~_BV(CC3000_ENABLE_PIN);
} // cc3000_clear_pin_WL_EN

//*****************************************************************************
//
//! cc3000_read_irq_pin
//!
//!  @brief    Reads the state of the IRQ pin
//!
//!  @return   If pin is high 1 is returned else 0 is returned
//
//*****************************************************************************
uint8_t cc3000_read_irq_pin(void)
{
	#ifdef CS_IS_TOO_FAST
	_delay_us(100);
	#endif
	return (CC3000_IRQ_PORT_READ & _BV(CC3000_IRQ_PIN)) ? 1 : 0;
} // cc3000_read_irq_pin

//*****************************************************************************
//
//! cc3000_assert_cs
//!
//!  @brief  Asserts SPI Chip Select for the CC3000
//
//*****************************************************************************
void cc3000_assert_cs(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_Assert_cs\n\r"));
	#endif

	// Set pin low
	CC3000_SLAVE_SELECT_PORT &= ~_BV(CC3000_SLAVE_SELECT_PIN);

	#ifdef CS_IS_TOO_FAST
	_delay_us(50);
	#endif
} // cc3000_assert_cs

//*****************************************************************************
//
//! cc3000_deassert_cs
//!
//!  @brief  Deasserts SPI Chip Select for the CC3000
//
//*****************************************************************************
void cc3000_deassert_cs(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
	debug_str(PSTR("cc3000_deassert_cs\n\r"));
	#endif

	// Set pin high
	CC3000_SLAVE_SELECT_PORT |= _BV(CC3000_SLAVE_SELECT_PIN);

	#ifdef CS_IS_TOO_FAST
	_delay_us(50);
	#endif
} // cc3000_deassert_cs

//*****************************************************************************
//
//! cc3000_spi_send
//!
//!  @param  data    Data to send
//!
//!  @brief  Sends 1 byte to the CC3000
//
//*****************************************************************************
void cc3000_spi_send(uint8_t data)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
//	debug_str(PSTR("cc3000_spi_send\n\r"));
	#endif

	SPDR = data;

    while (!(SPSR & _BV(SPIF)))
      ;
} // cc3000_spi_send

//*****************************************************************************
//
//! cc3000_spi_recv
//!
//!  @return	Returns 1 byte
//!
//!  @brief		Receives 1 byte from CC3000
//
//*****************************************************************************
uint8_t cc3000_spi_recv(void)
{
	#if (CC3000_TRACE_LEVEL & CC3000_TRACE_LEVEL_DEBUG)
//	debug_str(PSTR("cc3000_spi_recv\n\r"));
	#endif

    SPDR = 0x03;

    while (!(SPSR & _BV(SPIF)))
      ;

	return SPDR;
} // cc3000_spi_recv
