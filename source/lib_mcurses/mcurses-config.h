/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses-config.h - configuration file for mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
 * Modified by David Harmon 2016
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

 
#ifndef MCURSES_CONFIG_H_
#define MCURSES_CONFIG_H_

#define MCURSES_BAUD                9600L			// UART baudrate, use 115200 on STM32Fxx
#define MCURSES_UART_NUMBER         0               // UART number on STM32Fxxx (1-6), else ignored

#define UART_TXBUFLEN				128				// TX buffer size
#define UART_RXBUFLEN				16				// RX buffer size

#define MCURSES_LINES               24              // 24 lines
#define MCURSES_COLS                80              // 80 columns

#define MCURSES_USE_INTERNAL_TERM	1				// Use the mcurses' USART routines
#define MCURSES_USE_XIO				1				// Use the xio library
#define MCURSES_USE_GLOBALTERM		0				// Use set_term and newwin functions
#define MCURSES_USE_STDSCR			0				// Use stdscr and related functions
#define MCURSES_USE_NODELAY			0				// Enable nodelay functionality
#define MCURSES_ENABLE_TODO			0				// Enable warnings describing TODO items

#endif /* MCURSES_CONFIG_H_ */
