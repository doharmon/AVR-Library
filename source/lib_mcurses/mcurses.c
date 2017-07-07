/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.c - mcurses lib
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

#include "mcurses.h"

#if MCURSES_USE_INTERNAL_TERM == 0
#include "../lib_tserial/tserial.h"
#endif // MCURSES_USE_INTERNAL_TERM == 0

#if MCURSES_USE_XIO > 0
#include "../lib_xio/xio.h"
#endif // MCURSES_USE_XIO > 0

#include <string.h>
#include <ctype.h>

#include <avr/interrupt.h>
#include <util/delay.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: Optional stdscr and current terminal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_STDSCR > 0
static WINDOW* g_pwinStdScr;													// Updated by newterm and mcurses_set_stdscr
#define stdscr g_pwinStdScr
#endif // MCURSES_USE_STDSCR > 0
#if MCURSES_USE_GLOBALTERM > 0
static SCREEN* g_pscrCurrent;													// Updated by newterm and set_term
#endif // MCURSES_USE_GLOBALTERM > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * VT command sequences
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_ENABLE_TODO > 0
#warning "Test if converting SEQ_XXX to const __flash strings saves space"
#endif
#define SEQ_CSI                                 PSTR("\033[")                   // code introducer
#define SEQ_CLEAR                               PSTR("\033[2J")                 // clear screen
#define SEQ_CLRTOBOT                            PSTR("\033[J")                  // clear to bottom
#define SEQ_CLRTOEOL                            PSTR("\033[K")                  // clear to end of line
#define SEQ_DELCH                               PSTR("\033[P")                  // delete character
#define SEQ_NEXTLINE                            PSTR("\033E")                   // goto next line (scroll up at end of scrolling region)
#define SEQ_INSERTLINE                          PSTR("\033[L")                  // insert line
#define SEQ_DELETELINE                          PSTR("\033[M")                  // delete line
#define SEQ_ATTRSET                             PSTR("\033[0")                  // set attributes, e.g. "\033[0;7;1m"
#define SEQ_ATTRSET_REVERSE                     PSTR(";7")                      // reverse
#define SEQ_ATTRSET_UNDERLINE                   PSTR(";4")                      // underline
#define SEQ_ATTRSET_BLINK                       PSTR(";5")                      // blink
#define SEQ_ATTRSET_BOLD                        PSTR(";1")                      // bold
#define SEQ_ATTRSET_DIM                         PSTR(";2")                      // dim
#define SEQ_ATTRSET_FCOLOR                      PSTR(";3")                      // foreground color
#define SEQ_ATTRSET_BCOLOR                      PSTR(";4")                      // background color
#define SEQ_INSERT_MODE                         PSTR("\033[4h")                 // set insert mode
#define SEQ_REPLACE_MODE                        PSTR("\033[4l")                 // set replace mode
#define SEQ_RESET_SCRREG                        PSTR("\033[r")                  // reset scrolling region
#define SEQ_LOAD_G1                             PSTR("\033)0")                  // load G1 character set
#define SEQ_CURSOR_VIS                          PSTR("\033[?25")                // set cursor visible/not visible
#define SEQ_REQUEST_SIZE						PSTR("\033[6n")					// request display size
#define SEQ_FULL_RESET							PSTR("\033c")					// terminal full reset
#define SEQ_ERASE_REC_AREA						PSTR("\033[%d;%d;%d;%d$z")		// erase rectangular area
#define SEQ_SET_SCRREG							PSTR("\033[%hd;%hdr")			// set scrolling region

																				// VT300+ command sequences
#define SEQ_ATTRSET_REVERSE_OFF					PSTR(";27")						// reverse off
#define SEQ_ATTRSET_UNDERLINE_OFF				PSTR(";24")						// underline off
#define SEQ_ATTRSET_BLINK_OFF					PSTR(";25")						// blink off
#define SEQ_ATTRSET_BOLD_OFF					PSTR(";22")						// bold off


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init, putc, getc for AVR
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#if MCURSES_USE_INTERNAL_TERM > 0

#define BAUD                                    MCURSES_BAUD
#include <util/setbaud.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Newer ATmegas, e.g. ATmega88, ATmega168
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR0H

#define UART0_UBRRH                             UBRR0H
#define UART0_UBRRL                             UBRR0L
#define UART0_UCSRA                             UCSR0A
#define UART0_UCSRB                             UCSR0B
#define UART0_UCSRC                             UCSR0C
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE0)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ01)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ00)
#ifdef URSEL0
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL0)
#else // URSEL0
#define UART0_URSEL_BIT_VALUE                   (0)
#endif // URSEL0
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN0)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN0)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE0)
#define UART0_UDR                               UDR0
#define UART0_U2X                               U2X0
#define UART0_RXC                               RXC0

#ifdef USART0_TXC_vect                                                  		// e.g. ATmega162 with 2 UARTs
#define UART0_TXC_vect                          USART0_TXC_vect
#define UART0_RXC_vect                          USART0_RXC_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#elif defined(USART0_TX_vect)                                           		// e.g. ATmega644 with 2 UARTs
#define UART0_TXC_vect                          USART0_TX_vect
#define UART0_RXC_vect                          USART0_RX_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#else // USART0_TXC_vect                                                 		// e.g. ATmega168 with 1 UART
#define UART0_TXC_vect                          USART_TX_vect
#define UART0_RXC_vect                          USART_RX_vect
#define UART0_UDRE_vect                         USART_UDRE_vect
#endif // USART0_TXC_vect

#define UART0_UDRIE                             UDRIE0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * ATmegas with 2nd UART, e.g. ATmega162
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR1H
#define UART1_UBRRH                             UBRR1H
#define UART1_UBRRL                             UBRR1L
#define UART1_UCSRA                             UCSR1A
#define UART1_UCSRB                             UCSR1B
#define UART1_UCSRC                             UCSR1C
#define UART1_UDRE_BIT_VALUE                    (1<<UDRE1)
#define UART1_UCSZ1_BIT_VALUE                   (1<<UCSZ11)
#define UART1_UCSZ0_BIT_VALUE                   (1<<UCSZ10)
#ifdef URSEL1
#define UART1_URSEL_BIT_VALUE                   (1<<URSEL1)
#else // URSEL1
#define UART1_URSEL_BIT_VALUE                   (0)
#endif // URSEL1
#define UART1_TXEN_BIT_VALUE                    (1<<TXEN1)
#define UART1_RXEN_BIT_VALUE                    (1<<RXEN1)
#define UART1_RXCIE_BIT_VALUE                   (1<<RXCIE1)
#define UART1_UDR                               UDR1
#define UART1_U2X                               U2X1
#define UART1_RXC                               RXC1

#ifdef USART1_TXC_vect                                                  		// e.g. ATmega162 with 2 UARTs
#define UART1_TXC_vect                          USART1_TXC_vect
#define UART1_RXC_vect                          USART1_RXC_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#else //  USART1_TXC_vect                                                   	// e.g. ATmega644 with 2 UARTs
#define UART1_TXC_vect                          USART1_TX_vect
#define UART1_RXC_vect                          USART1_RX_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#endif // USART1_TXC_vect

#define UART1_UDRIE                             UDRIE1
#endif // UBRR1H

#else // UBRR0H

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * older ATmegas, e.g. ATmega8, ATmega16
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define UART0_UBRRH                             UBRRH
#define UART0_UBRRL                             UBRRL
#define UART0_UCSRA                             UCSRA
#define UART0_UCSRB                             UCSRB
#define UART0_UCSRC                             UCSRC
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ1)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ0)
#ifdef URSEL
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL)
#else // URSEL
#define UART0_URSEL_BIT_VALUE                   (0)
#endif // URSEL
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE)
#define UART0_UDR                               UDR
#define UART0_U2X                               U2X
#define UART0_RXC                               RXC
#define UART0_UDRE_vect                         USART_UDRE_vect
#define UART0_TXC_vect                          USART_TXC_vect
#define UART0_RXC_vect                          USART_RXC_vect
#define UART0_UDRIE                             UDRIE

#endif // UBRR0H

static volatile uint8_t uart_txbuf[UART_TXBUFLEN];                         		// tx ringbuffer
static volatile uint8_t uart_txsize = 0;                                   		// tx size
static volatile uint8_t uart_rxbuf[UART_RXBUFLEN];                         		// rx ringbuffer
static volatile uint8_t uart_rxsize = 0;                                   		// rx size

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_init (void)
{
    UART0_UBRRH = UBRRH_VALUE;                                                  // set baud rate
    UART0_UBRRL = UBRRL_VALUE;

#if USE_2X
    UART0_UCSRA |= (1<<UART0_U2X);
#else // USE_2X
    UART0_UCSRA &= ~(1<<UART0_U2X);
#endif // USE_2X

    UART0_UCSRC = UART0_UCSZ1_BIT_VALUE | UART0_UCSZ0_BIT_VALUE | UART0_URSEL_BIT_VALUE;    // 8 bit, no parity
    UART0_UCSRB |= UART0_TXEN_BIT_VALUE | UART0_RXEN_BIT_VALUE | UART0_RXCIE_BIT_VALUE;     // enable UART TX, RX, and RX interrupt

    return 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_phyio_putc (uint8_t ch)
{
    static uint8_t uart_txstop  = 0;                                       		// tail

    while (uart_txsize >= UART_TXBUFLEN)                                        // buffer full?
    {                                                                           // yes
        ;                                                                       // wait
    }

    uart_txbuf[uart_txstop++] = ch;                                             // store character

    if (uart_txstop >= UART_TXBUFLEN)                                           // at end of ringbuffer?
    {                                                                           // yes
        uart_txstop = 0;                                                        // reset to beginning
    }

    cli();
    uart_txsize++;                                                              // increment used size
    sei();

    UART0_UCSRB |= (1 << UART0_UDRIE);                                          // enable UDRE interrupt

}

static uint8_t  uart_rxstart = 0;												// head

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint16_t
mcurses_phyio_getbyte (void)
{
    uint8_t         ch;

	if (0 == uart_rxsize)
		return ERR;

    ch = uart_rxbuf[uart_rxstart++];                                            // get character from ringbuffer

    if (uart_rxstart == UART_RXBUFLEN)                                          // at end of rx buffer?
    {                                                                           // yes
	    uart_rxstart = 0;                                                       // reset to beginning
    }

    cli();
    uart_rxsize--;                                                              // decrement size
    sei();

    return (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: available (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint16_t
mcurses_phyio_available (void)
{
    return uart_rxsize;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_phyio_flushoutput (void)
{
    while (uart_txsize)
		;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: UART interrupt handler, called if UART has received a character (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
ISR(UART0_RXC_vect)
{
    static uint8_t  uart_rxstop  = 0;                                      		// tail
    uint8_t         ch;

    ch = UART0_UDR;

    if (uart_rxsize < UART_RXBUFLEN)                                            // buffer full?
    {                                                                           // no
        uart_rxbuf[uart_rxstop++] = ch;                                         // store character

        if (uart_rxstop >= UART_RXBUFLEN)                                       // at end of ringbuffer?
        {                                                                       // yes
            uart_rxstop = 0;                                                    // reset to beginning
        }

        uart_rxsize++;                                                          // increment used size
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: UART interrupt handler, called if UART is ready to send a character (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
ISR(UART0_UDRE_vect)
{
    static uint8_t  uart_txstart = 0;                                      		// head
    uint8_t         ch;

    if (uart_txsize > 0)                                                        // tx buffer empty?
    {                                                                           // no
        ch = uart_txbuf[uart_txstart++];                                        // get character to send, increment offset

        if (uart_txstart == UART_TXBUFLEN)                                      // at end of tx buffer?
        {                                                                       // yes
            uart_txstart = 0;                                                   // reset to beginning
        }

        uart_txsize--;                                                          // decrement size

        UART0_UDR = ch;                                                         // write character, don't wait
    }
    else
    {
        UART0_UCSRB &= ~(1 << UART0_UDRIE);                                     // tx buffer empty, disable UDRE interrupt
    }
}

#endif // MCURSES_USE_INTERNAL_TERM > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a 3/2/1 digit integer number (raw)
 *
 * Here we don't want to use sprintf (too big on AVR/Z80) or itoa (not available on Z80)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puti (WINDOW* win, uint8_t i)
{
    uint8_t ii;

    if (i >= 10)
    {
        if (i >= 100)
        {
            ii = i / 100;
            waddch_raw (win, ii + '0');
            i -= 100 * ii;
        }

        ii = i / 10;
        waddch_raw (win, ii + '0');
        i -= 10 * ii;
    }

    waddch_raw (win, i + '0');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: convert ASCII number to uint8_t
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_XIO == 0
static uint8_t
mcurses_atoi (char* pc, uint8_t* pb)
{
	uint8_t bchars = 0;
	
	*pb = 0;
	while (*pc >= '0' && *pc <= '9')
	{
		*pb *= 10;
		*pb += *pc - '0';
		pc++;
		bchars++;
	}
	
	return bchars;
}
#endif // MCURSES_USE_XIO == 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: addch or insch a character
 *
 * Not static because wprtinw_P calls it
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_addch_or_insch (WINDOW* win, uint8_t ch, uint8_t insert)
{
	SCREEN* scr = win->scr;
	
    if (ch >= 0x80 && ch <= 0x9F)
    {
        if (!(scr->bFlags & SCR_FLAG_CHARSET))
        {
            waddch_raw (win, '\016');											// switch to G1 set
            scr->bFlags |= CHARSET_G1;
        }
        ch -= 0x20;																// subtract offset to G1 characters
    }
    else
    {
        if (scr->bFlags & SCR_FLAG_CHARSET)
        {
            waddch_raw (win, '\017');											// switch to G0 set
            scr->bFlags &= CHARSET_G0;
        }
    }

    if (insert)
    {
        if (! (scr->bFlags & SCR_FLAG_INSERT_MODE))
        {
            waddstr_P_raw (win, SEQ_INSERT_MODE);
            scr->bFlags |= SCR_FLAG_INSERT_MODE;
        }
    }
    else
    {
        if (scr->bFlags & SCR_FLAG_INSERT_MODE)
        {
            waddstr_P_raw (win, SEQ_REPLACE_MODE);
            scr->bFlags &= ~SCR_FLAG_INSERT_MODE;
        }
    }

    waddch_raw (win, ch);
	if ('\r' == ch)
		win->bCurx= 0;
	else if ('\n' == ch)
	{
		if (++win->bCury > win->bMaxy)
			win->bCury = win->bMaxy;											// At bottom of window
	}
	else
	{
		if (++win->bCurx > win->bMaxx)
		{
			win->bCurx = 0;														// Wrapped around to next line
			if (++win->bCury > win->bMaxy)
				win->bCury = win->bMaxy;										// At bottom of window
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: set scrolling region (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mysetscrreg (WINDOW* win, uint8_t top, uint8_t bottom)
{
    if (top == bottom)
    {
		if (win->bFlags & WIN_FLAG_PHYSSCRLRGN)
			wsetscrreg_phys(win, win->bPhysStart, win->bPhysEnd);
		else if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
			mcurses_term_scrreg(win, TRUE);
		else	
			waddstr_P_raw (win, SEQ_RESET_SCRREG);								// reset scrolling region
    }
    else
    {
		wsetscrreg_raw(win, top, bottom);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: get char based on nodelay flag
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_NODELAY > 0
static uint8_t
mcurses_getc(SCREEN* scr)
{
	uint8_t 	ch;
	getByte_t 	getByte = scr->term->getByte;
	
	if (scr->bFlags & SCR_FLAG_NODELAY)
		return getByte();
	
	do
	{
		ch = getByte();
	} while (ERR == ch);
	
	return ch;
}
#endif // MCURSES_USE_NODELAY > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: query screen size
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_query_screen_size(WINDOW* win)
{
	win->scr->bFlags |= SCR_FLAG_QUERY_SIZE;
	waddch_raw(win, KEY_ESCAPE); waddch_raw(win, '7');							// Save cursor position
	wmove_raw(win, 250, 250);													// Move to furthest position
    waddstr_P_raw(win, SEQ_REQUEST_SIZE);										// Request cursor position: <ESC>[<row>;<col>R
	waddch_raw(win, KEY_ESCAPE); waddch_raw(win, '8');							// Restore saved cursor position
}

 /*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: set stdscr to a window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_STDSCR > 0
WINDOW*
mcurses_set_stdscr(WINDOW* win)
{
	WINDOW* pwinOld;
	
	pwinOld = stdscr;
	stdscr	= win;
	
	return pwinOld;
}
#endif // MCURSES_USE_STDSCR > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: query terminal size
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_get_term_size(WINDOW* win)
{
	mcurses_query_screen_size(win);												// Get cursor position. wgetch needs to be called to read reply.
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: reset terminal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_term_reset(WINDOW* win)
{
    waddstr_P_raw(win, SEQ_FULL_RESET);											// Reset terminal
	_delay_ms(250);
//	mcurses_get_cursor_position(win);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: get current cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_get_cursor_position(WINDOW* win)
{
#if MCURSES_ENABLE_TODO > 0
	#warning "mcurses_get_cursor_position can hang if terminal's TX is not connected to the chip's RX"
#endif
    waddstr_P_raw(win, SEQ_REQUEST_SIZE);										// Request cursor position: <ESC>[<row>;<col>R. wgetch needs to be called to read reply.
	win->bCury = 0xFF;
	while (0xFF == win->bCury)
		wgetch(win);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: use terminal physical scroll margins
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_term_scrreg(WINDOW* win, uint8_t bUsePhysScrReg)
{
    if (!bUsePhysScrReg)
    {
        waddstr_P_raw (win, SEQ_RESET_SCRREG);                              	// reset scrolling region
		win->bFlags &= ~WIN_FLAG_TERMSCRLRGN;
    }
    else
    {
        waddstr_P_raw (win, SEQ_CSI);
        mcurses_puti (win, win->bBegy + 1);
        waddch_raw (win, ';');
        mcurses_puti (win, win->bBegy + win->bMaxy + 1);
        waddch_raw (win, 'r');
		win->bFlags |= WIN_FLAG_TERMSCRLRGN;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES EXTENSION: initialize new window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
mcurses_newwin(SCREEN* scr, WINDOW* win, uint8_t bBegy, uint8_t bBegx, uint8_t bRows, uint8_t bCols)
{
	if (bBegy > scr->bMaxy)
		bBegy = scr->bMaxy;

	if (bBegx > scr->bMaxx)
		bBegx = scr->bMaxx;

	if (bRows > scr->bMaxy)
		bRows = scr->bMaxy;

	if (bCols > scr->bMaxx)
		bCols = scr->bMaxx;

	win->bMaxy		= bRows;
	win->bMaxx		= bCols;
	win->bBegy		= bBegy;
	win->bBegx		= bBegx;
	win->bCury		= 0xFF;
	win->bCurx		= 0xFF;
	win->bScrlStart	= 0;
	win->bScrlEnd	= 0;
	win->wAttrs		= 0xFFFF;
	win->scr		= scr;
	
	if (0 == bBegy && 0 == bBegx && scr->bMaxy == bRows && scr->bMaxx == bCols)
		win->bFlags = WIN_FLAG_IS_UP | WIN_FLAG_CURSSET | WIN_FLAG_FULLWIN;
	else
		win->bFlags = WIN_FLAG_IS_UP | WIN_FLAG_CURSSET;

	wattrset(win, A_NORMAL);
	wclear(win);
	wmove(win, 0, 0);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set cursor visibility
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
curs_set (WINDOW* win, uint8_t visibility)
{
    waddstr_P_raw (win, SEQ_CURSOR_VIS);

    if (visibility == 0)
    {
        waddch_raw (win, 'l');
		win->bFlags &= ~WIN_FLAG_CURSSET;
    }
    else
    {
        waddch_raw (win, 'h');
		win->bFlags |= WIN_FLAG_CURSSET;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: endwin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
endwin (WINDOW* win)
{
	wsetscrreg_raw(win, 0, 0);													// clear any margins
    wmove (win, win->bMaxy, 0);													// move cursor to last line
    wclrtoeol (win);															// clear this line
    waddch_raw (win, '\017');													// switch to G0 set
    curs_set (win, TRUE);														// show cursor
    waddstr_P_raw(win, SEQ_REPLACE_MODE);										// reset insert mode
    wrefresh (win);																// flush output
//	mcurses_phyio_done ();														// end of physical I/O
    win->bFlags		= 0;
	win->scr->bIdx	= 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset halfdelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_ENABLE_TODO > 0
#warning "Implement halfdelay"
#endif
void
halfdelay (WINDOW* win, uint8_t tenths)
{
//    win->scr->bHalfdelay = tenths;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize new terminal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
newterm (uint8_t bIdx, SCREEN* scr, WINDOW* win)
{
	#if MCURSES_USE_INTERNAL_TERM > 0
	mcurses_phyio_init();
	#endif // MCURSES_USE_INTERNAL_TERM > 0

	#if MCURSES_USE_STDSCR > 0
	g_pwinStdScr		= win;
	#endif // MCURSES_USE_STDSCR > 0

	#if MCURSES_USE_GLOBALTERM > 0
	g_pscrCurrent		= scr;
	#endif // MCURSES_USE_GLOBALTERM > 0

	scr->bFlags		= SCR_FLAG_IS_UP | SCR_FLAG_NODELAY;
	scr->bMaxy		= MCURSES_LINES-1;											// Default term size
	scr->bMaxx		= MCURSES_COLS-1;
//	scr->bHalfdelay	= 0;
	scr->acBuf[0]	= 0;
	scr->bIdx		= 0;
	scr->term		= &g_aterm[bIdx];
	
	win->bFlags		= WIN_FLAG_FULLWIN;
	win->bMaxy		= scr->bMaxy;
	win->bMaxx		= scr->bMaxx;
	win->bBegy		= 0;
	win->bBegx		= 0;
	win->bCury		= 0xFF;
	win->bCurx		= 0xFF;
	win->bScrlStart	= 0;
	win->bScrlEnd	= 0;
	win->wAttrs		= 0xFFFF;
	win->scr		= scr;

	mcurses_query_screen_size(win);												// Get size of terminal. wgetch needs to be called to read reply.
	waddstr_P_raw(win, SEQ_LOAD_G1);											// Load graphic charset into G1
	wattrset(win, A_NORMAL);
	wclear(win);
	wmove(win, 0, 0);
	wrefresh(win);																// Wait until TX buffer is empty
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_GLOBALTERM > 0
void
newwin (WINDOW* win, uint8_t bBegy, uint8_t bBegx, uint8_t bRows, uint8_t bCols)
{
	mcurses_newwin(g_pscrCurrent, win, bBegy, bBegx, bRows, bCols);
}
#endif // MCURSES_USE_GLOBALTERM > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
nodelay (WINDOW* win, uint8_t flag)
{
	if (flag)
		win->scr->bFlags |= SCR_FLAG_NODELAY;
	else
		win->scr->bFlags &= ~SCR_FLAG_NODELAY;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set current terminal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_GLOBALTERM > 0
SCREEN*
set_term(SCREEN* scr)
{
	SCREEN* pscrOld;
	
	 pscrOld		= g_pscrCurrent;
	 g_pscrCurrent	= scr;
	 
	 return pscrOld;
}
#endif // MCURSES_USE_GLOBALTERM > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddch (WINDOW* win, const uint8_t ch)
{
    mcurses_addch_or_insch (win, ch, FALSE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add graphic character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddch_graphic_raw (WINDOW* win, uint8_t ch)
{
	SCREEN* scr = win->scr;

    if (ch >= 0x80 && ch <= 0x9F)
    {
		ch -= 0x20;																// subtract offset to G1 characters

        if (!(scr->bFlags & SCR_FLAG_CHARSET))
        {
	        waddch_raw (win, '\016');											// switch to G1 set
	        waddch_raw (win, ch);
	        waddch_raw (win, '\017');											// switch to G0 set
        }
		else
		{
	        waddch_raw (win, ch);
		}
    }
    else
    {
	    waddch_raw (win, ch);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddch_raw (WINDOW* win, uint8_t ch)
{
	win->scr->term->putByte(ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string from SRAM
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddnstr (WINDOW* win, const char * str, uint8_t cb)
{
	uint8_t b = 0;
	
    while (*str && b++ < cb)
    {
        mcurses_addch_or_insch (win, *str++, FALSE);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string from SRAM (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddnstr_raw (WINDOW* win, const char* str, uint8_t cb)
{
	uint8_t b = 0;
	
    while (*str != '\0' && b++ < cb)
    {
        waddch_raw (win, *str);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string from flash
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddnstr_P (WINDOW* win, const __flash char* str, uint8_t cb)
{
    uint8_t ch;
	uint8_t b = 0;
	
    while ((ch = *str) != '\0' && b++ < cb)
    {
        mcurses_addch_or_insch (win, ch, FALSE);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add graphic string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddnstr_P_graphic_raw (WINDOW* win, const __flash char* str, uint8_t cb)
{
    uint8_t ch;
	uint8_t b = 0;
	
    while ((ch = *str) != '\0' && b++ < cb)
    {
        waddch_graphic_raw (win, ch);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
waddnstr_P_raw (WINDOW* win, const __flash char* str, uint8_t cb)
{
    uint8_t ch;
	uint8_t b = 0;
	
    while ((ch = *str) != '\0' && b++ < cb)
    {
        waddch_raw (win, ch);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set attribute(s)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wattrset (WINDOW* win, uint16_t attrs)
{
    uint8_t        idx;

    if (attrs != win->wAttrs)
    {
        waddstr_P_raw (win, SEQ_ATTRSET);

        idx = (attrs & F_COLOR) >> 8;

        if (idx >= 1 && idx <= 8)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_FCOLOR);
            waddch_raw (win, idx - 1 + '0');
        }

        idx = (attrs & B_COLOR) >> 12;

        if (idx >= 1 && idx <= 8)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_BCOLOR);
            waddch_raw (win, idx - 1 + '0');
        }

        if (attrs & A_REVERSE)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_REVERSE);
        }
        if (attrs & A_UNDERLINE)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_UNDERLINE);
        }
        if (attrs & A_BLINK)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_BLINK);
        }
        if (attrs & A_BOLD)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_BOLD);
        }
        if (attrs & A_DIM)
        {
            waddstr_P_raw (win, SEQ_ATTRSET_DIM);
        }
        waddch_raw (win, 'm');
        win->wAttrs = attrs;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: draw border around window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wborder (WINDOW* win, char ls, char rs, char ts, char bs, char tl, char tr, char bl, char br)
{
	uint8_t y, x;

	getyx(win, y, x);

	if (0 == ls)
		ls = ACS_VLINE;

	if (0 == rs)
		rs = ACS_VLINE;

	if (0 == ts)
		ts = ACS_HLINE;

	if (0 == bs)
		bs = ACS_HLINE;

	if (0 == tl)
		tl = ACS_ULCORNER;

	if (0 == tr)
		tr = ACS_URCORNER;

	if (0 == bl)
		bl = ACS_LLCORNER;

	if (0 == br)
		br = ACS_LRCORNER;

	wmove(win, win->bBegy+1, win->bBegx);
	wvline(win, ls, win->bMaxy - win->bBegy - 1);
	wmove(win, win->bBegy+1, win->bMaxx);
	wvline(win, rs, win->bMaxy - win->bBegy - 1);
	wmove(win, win->bBegy, win->bBegx+1);
	whline(win, ts, win->bMaxx - win->bBegx - 1);
	wmove(win, win->bMaxy, win->bBegx+1);
	whline(win, bs, win->bMaxx - win->bBegx - 1);
	wmove_raw(win, win->bBegy, win->bBegx);
	waddch_graphic_raw(win, tl);
	wmove_raw(win, win->bBegy, win->bMaxx);
	waddch_graphic_raw(win, tr);
	wmove_raw(win, win->bMaxy, win->bBegx);
	waddch_graphic_raw(win, bl);
	wmove_raw(win, win->bMaxy, win->bMaxx);
	waddch_graphic_raw(win, br);
	
	win->bCury = 0xFF;															// Force wmove to send escape sequence
	wmove(win, y, x);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear terminal screen
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wclear (WINDOW* win)
{
	wclrrec(win, 0, 0, win->bMaxy, win->bMaxx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: Clear part of screen within physical margins
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wclear_phys (WINDOW* win)
{
#if MCURSES_ENABLE_TODO > 0
#warning "Do the Phys lines need to be offset from begy?"
#endif
	if (win->bFlags & WIN_FLAG_PHYSSCRLRGN)
		wclrrec(win, win->bPhysStart, 0, win->bPhysEnd-win->bPhysStart, win->bMaxx);
	else
		wclrrec(win, 0, 0, win->bMaxy, win->bMaxx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to bottom of window
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wclrtobot (WINDOW* win)
{
    waddstr_P_raw (win, SEQ_CLRTOBOT);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to end of line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wclrtoeol (WINDOW* win)
{
    waddstr_P_raw (win, SEQ_CLRTOEOL);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete character at cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wdelch (WINDOW* win)
{
    waddstr_P_raw (win, SEQ_DELCH);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read key
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_KEYS                ((KEY_F1 + 12) - 0x80)

static const __flash char * const __flash function_keys[MAX_KEYS] =
{
    (const __flash char[]) { "B"  },      // KEY_DOWN                 0x80                // Down arrow key
    (const __flash char[]) { "A"  },      // KEY_UP                   0x81                // Up arrow key
    (const __flash char[]) { "D"  },      // KEY_LEFT                 0x82                // Left arrow key
    (const __flash char[]) { "C"  },      // KEY_RIGHT                0x83                // Right arrow key
    (const __flash char[]) { "1~" },      // KEY_HOME                 0x84                // Home key
    (const __flash char[]) { "3~" },      // KEY_DC                   0x85                // Delete character key
    (const __flash char[]) { "2~" },      // KEY_IC                   0x86                // Ins char/toggle ins mode key
    (const __flash char[]) { "6~" },      // KEY_NPAGE                0x87                // Next-page key
    (const __flash char[]) { "5~" },      // KEY_PPAGE                0x88                // Previous-page key
    (const __flash char[]) { "4~" },      // KEY_END                  0x89                // End key
    (const __flash char[]) { "Z"  },      // KEY_BTAB                 0x8A                // Back tab key
#if 1 // VT400:
    (const __flash char[]) { "11~" },     // KEY_F(1)                 0x8B                // Function key F1
    (const __flash char[]) { "12~" },     // KEY_F(2)                 0x8C                // Function key F2
    (const __flash char[]) { "13~" },     // KEY_F(3)                 0x8D                // Function key F3
    (const __flash char[]) { "14~" },     // KEY_F(4)                 0x8E                // Function key F4
    (const __flash char[]) { "15~" },     // KEY_F(5)                 0x8F                // Function key F5
#else // Linux console
    (const __flash char[]) { "[A"  },     // KEY_F(1)                 0x8B                // Function key F1
    (const __flash char[]) { "[B"  },     // KEY_F(2)                 0x8C                // Function key F2
    (const __flash char[]) { "[C"  },     // KEY_F(3)                 0x8D                // Function key F3
    (const __flash char[]) { "[D"  },     // KEY_F(4)                 0x8E                // Function key F4
    (const __flash char[]) { "[E"  },     // KEY_F(5)                 0x8F                // Function key F5
#endif // 1
    (const __flash char[]) { "17~" },     // KEY_F(6)                 0x90                // Function key F6
    (const __flash char[]) { "18~" },     // KEY_F(7)                 0x91                // Function key F7
    (const __flash char[]) { "19~" },     // KEY_F(8)                 0x92                // Function key F8
    (const __flash char[]) { "20~" },     // KEY_F(9)                 0x93                // Function key F9
    (const __flash char[]) { "21~" },     // KEY_F(10)                0x94                // Function key F10
    (const __flash char[]) { "23~" },     // KEY_F(11)                0x95                // Function key F11
    (const __flash char[]) { "24~" }      // KEY_F(12)                0x96                // Function key F12
};

uint8_t
wgetch (WINDOW* win)
{
    uint8_t			ch;
	uint8_t			rch;
	uint8_t			iKey;
	SCREEN*			scr	= win->scr;;
	char*			pBuf;
	#if MCURSES_USE_XIO > 0
	long			xRow;
	long			xCol;
	#else // MCURSES_USE_XIO > 0
	uint8_t			xRow;
	uint8_t			xCol;
	#endif // MCURSES_USE_XIO > 0
	#if MCURSES_USE_NODELAY > 0
	#define GETBYTE	mcurses_getc(scr)
	#else // MCURSES_USE_NODELAY > 0
	getByte_t		getByte	= scr->term->getByte;
	#define GETBYTE	getByte()
	#endif // MCURSES_USE_NODELAY > 0

	// Two types of escape sequences are parsed:
	//		^[[...{Alpha|~}		Majority of the escape sequences
	//		^[O{Alpha}			Keypad escape sequences

	ch = GETBYTE;

	if ('\033' == ch && !(scr->bFlags & SCR_FLAG_ESC))
	{
		scr->bFlags	|= SCR_FLAG_ESC;											// Start of an escape sequence
		scr->bIdx		 = 0;
		ch = GETBYTE;
	}

	if (ERR == ch)
		return ERR;

	rch = ERR;
	if (scr->bFlags & SCR_FLAG_ESC)
	{
		do
		{
			switch (scr->bFlags & SCR_FLAG_ESC_STATE)
			{
				case SCR_FLAG_ESC:
					if ('[' == ch)
					{
						scr->bFlags |= SCR_FLAG_ESC_ALPHA_TILDE;				// Escape sequence will be type ^[[...{Alpha|~}
					}
					else if ('O' == ch)
					{
						scr->bFlags |= SCR_FLAG_ESC_KEYPAD;						// Keypad escape sequence ^[O{Alpha}
					}
					else if ('\033' == ch)
					{
						scr->bFlags &= ~SCR_FLAG_ESC_STATE;						// ESCAPE ESCAPE
						rch			  = KEY_ESCAPE;
					}
					else
					{
						scr->bFlags &= ~SCR_FLAG_ESC_STATE;						// Unknown sequence
						rch			  = ch;
					}
					break;
			
				case SCR_FLAG_ESC | SCR_FLAG_ESC_ALPHA_TILDE:					// Parse ^[[...{Alpha|~}
					if (scr->bIdx >= SCR_BUFSIZE-1)
					{
						scr->bFlags &= ~SCR_FLAG_ESC_STATE;						// Exceeded buffer
						rch			  = ch;
						break;
					}

					scr->acBuf[scr->bIdx++] = ch;

					if (isalpha(ch) || '~' == ch)								// Escape sequence ends with A-Z or ~
					{
						scr->bFlags &= ~SCR_FLAG_ESC_STATE;
						scr->acBuf[scr->bIdx] = 0;
						if ('R' == ch)
						{
							pBuf = scr->acBuf;									// ^[[<rows>;<cols>R
							#if MCURSES_USE_XIO > 0
							xatoi(&pBuf, &xRow);								// Get rows
							pBuf++;
							xatoi(&pBuf, &xCol);								// Get cols
							#else // MCURSES_USE_XIO > 0
							pBuf += mcurses_atoi(pBuf, &xRow);					// Get rows
							pBuf++;
							mcurses_atoi(pBuf, &xCol);							// Get cols
							#endif // MCURSES_USE_XIO > 0
							if (scr->bFlags & SCR_FLAG_QUERY_SIZE)
							{
								scr->bMaxy   = xRow-1;
								scr->bMaxx   = xCol-1;
								scr->bFlags &= ~SCR_FLAG_QUERY_SIZE;

								if (win->bFlags & WIN_FLAG_FULLWIN)
								{
									win->bMaxy = scr->bMaxy;
									win->bMaxx = scr->bMaxx;

									if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
										mcurses_term_scrreg(win, TRUE);
								}
							}
							else
							{
#if MCURSES_ENABLE_TODO > 0
#warning "Offset from begx and begy?"
#endif								
								win->bCury = xRow-1 - win->bBegy;
								win->bCurx = xCol-1 - win->bBegx;
							}
						}
						else
							for (iKey = 0; iKey < MAX_KEYS; iKey++)
							{
								if (!strcmp_P(scr->acBuf, function_keys[iKey]))
								{
									rch = iKey + 0x80;
									break;
								}
							}
					}
					break;
			
				case SCR_FLAG_ESC | SCR_FLAG_ESC_KEYPAD:						// Parse ^[O{Alpha}
					scr->bFlags &= ~SCR_FLAG_ESC_STATE;
					switch (ch)
					{
						case 'M':
							rch = KEY_CR;
							break;
						case 'Q':
							rch = '/';
							break;
						case 'R':
							rch = '*';
							break;
						case 'S':
							rch = '-';
							break;
						case 'l':
							rch = '+';
							break;
						case 'n':
							rch = '.';
							break;
						default:
							if (ch >= 'p' && ch <= 'y')
								rch = ch - 'p' + '0';							// Keypad 0 - 9
							break;
					}
					break;
			}
			if (ERR == rch)
			{
				ch = GETBYTE;

				if (ERR == ch)
					return ERR;
			}
		}
		while (scr->bFlags & SCR_FLAG_ESC_STATE);
	}
	else
	{
		if (0x7F == ch)
			rch = KEY_DC;
		else
			rch = ch;
	}

    return rch;
}
#undef GETBYTE

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read raw character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
wgetch_raw(WINDOW* win)
{
	return win->scr->term->getByte();;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read string (with mini editor built-in)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wgetnstr (WINDOW* win, char * str, uint8_t maxlen)
{
    uint8_t ch;
    uint8_t curlen = 0;
    uint8_t curpos = 0;
    uint8_t starty;
    uint8_t startx;
    uint8_t i;

    maxlen--;                               									// reserve one byte in order to store '\0' in last position
    getyx (win, starty, startx);            									// get current cursor position

    while ((ch = wgetch (win)) != KEY_CR)
    {
        switch (ch)
        {
            case KEY_LEFT:
                if (curpos > 0)
                {
                    curpos--;
                }
                break;
            case KEY_RIGHT:
                if (curpos < curlen)
                {
                    curpos++;
                }
                break;
            case KEY_HOME:
                curpos = 0;
                break;
            case KEY_END:
                curpos = curlen;
                break;
            case KEY_BACKSPACE:
                if (curpos > 0)
                {
                    curpos--;
                    curlen--;
                    wmove (win, starty, startx + curpos);

                    for (i = curpos; i < curlen; i++)
                    {
                        str[i] = str[i + 1];
                    }
                    str[i] = '\0';
                    wdelch(win);
                }
                break;

            case KEY_DC:
                if (curlen > 0)
                {
                    curlen--;
                    for (i = curpos; i < curlen; i++)
                    {
                        str[i] = str[i + 1];
                    }
                    str[i] = '\0';
                    wdelch(win);
                }
                break;

            default:
                if (curlen < maxlen && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127)      // Printable ASCII 7bit or printable 8bit ISO8859
                {
                    for (i = curlen; i > curpos; i--)
                    {
                        str[i] = str[i - 1];
                    }
                    winsch (win, ch);
                    str[curpos] = ch;
                    curpos++;
                    curlen++;
                }
        }
        wmove (win, starty, startx + curpos);
    }
    str[curlen] = '\0';
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read a field string using state machine (with mini editor)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
wgetnstr_field_sm(WINDOW* win, GetnstrState_t* pSM, char * str, uint8_t maxlen, uint8_t type)
{
    uint8_t ch;
    uint8_t i;
	uint8_t update;

	if (pSM->state < MC_SM_GETCH)												// Initialize state
	{
		pSM->state  = MC_SM_SELECTED;											// Any initial text in the field starts off as selected
		pSM->curlen = strlen(str);
		pSM->curpos = pSM->curlen;
	    getyx(win, pSM->starty, pSM->startx);
	    wmove_raw(win, pSM->starty, pSM->startx + pSM->curpos);
		if (type & MC_TB_DEC)													// Parse decimal values for sign and decimal points
		{
			if ('-' == str[0])
				pSM->state |= MC_SM_HAS_SIGN;
				
			for (i = 0; i < maxlen; i++)
				if ('.' == str[i])
				{
					pSM->state |= MC_SM_HAS_DEC;
					break;
				}
		}
	}

    maxlen--;																	// Reserve one byte for terminating 0x00
	
	ch = wgetch(win);															// This waits for a key if nodelay cleared

	if (type & (MC_TB_DEC | MC_TB_HEX) && isprint(ch))							// Field is numeric. Depending on type, valid chars are: 0-9,A-F,'.','-'
	{
		if (type & MC_TB_HEX)
		{
			if (isxdigit(ch))
				ch = toupper(ch);
			else
				ch = ERR;
		}
		else if (pSM->state & MC_SM_SELECTED)									// The field is highlighted. Therefore, waiting for first valid char.
		{
			if (!isdigit(ch) && !(type & MC_TB_SIGNED && '-' == ch) && !(type & MC_TB_FRAC && '.' == ch))
				ch = ERR;														// The field can only start with a digit, '-', or '.'
		}
		else
		{
			if (type & MC_TB_SIGNED && '-' == ch)
			{
				if (pSM->curpos > 0 || pSM->state & MC_SM_HAS_SIGN)				// Verify only one sign char and it's the first char
					ch = ERR;
				else
					pSM->state |= MC_SM_HAS_SIGN;
			}
			else if (0 == pSM->curpos && pSM->state & MC_SM_HAS_SIGN)			// Do not allow any chars before a sign character
			{
				ch = ERR;
			}
			else if (type & MC_TB_FRAC && '.' == ch)
			{
				if (pSM->state & MC_SM_HAS_DEC)									// Verify only one decimal point character
					ch = ERR;
				else
					pSM->state |= MC_SM_HAS_DEC;
			}
			else if (!isdigit(ch))
			{
				ch = ERR;
			}
		}
	}

	if (MC_SM_SELECTED & pSM->state && ERR != ch)								// Check if field text is still selected
	{
		if (isprint(ch) || KEY_BACKSPACE == ch || KEY_DC == ch)
		{
			pSM->state = MC_SM_GETCH;											// Delete selected text
		    wmove_raw(win, pSM->starty, pSM->startx);
			for (i = 0; i < pSM->curlen; i++)
				waddch_raw(win, ' ');
			pSM->curlen = 0;
			pSM->curpos = 0;
			*str		= 0;
		}
		else if (ch >= KEY_LEFT && ch <= KEY_END)								// Unselect text
		{
			pSM->state = MC_SM_GETCH;
			wmove_raw(win, pSM->starty, pSM->startx);
			waddstr_raw(win, str);
			if (KEY_HOME == ch)													// Force wmove to update on KEY_HOME
				win->bCurx=0xFF;												// Needed if lines towards bottom of routine are commented out
		}
	}
	
	update = 0;
    switch (ch)
    {
	    case KEY_LEFT:
		    if (pSM->curpos > 0)
			    pSM->curpos--;
		    break;

	    case KEY_RIGHT:
		    if (pSM->curpos < pSM->curlen)
			    pSM->curpos++;
		    break;

	    case KEY_HOME:
		    pSM->curpos = 0;
		    break;

	    case KEY_END:
		    pSM->curpos = pSM->curlen;
		    break;

	    case KEY_BACKSPACE:
	    case KEY_DC:
			if (KEY_BACKSPACE == ch)
			{
				if (pSM->curpos > 0)
				    pSM->curpos--;												// Turn backspace into a delete
				else
					break;														// Cannot backspace past start of field
			}
			else if (pSM->curpos == pSM->curlen)
				break;															// Cannot delete at end of text

		    pSM->curlen--;

		    if ('.' == str[pSM->curpos])
			    pSM->state &= ~MC_SM_HAS_DEC;
		    else if ('-' == str[pSM->curpos])
			    pSM->state &= ~MC_SM_HAS_SIGN;

		    for (i = pSM->curpos; i < pSM->curlen; i++)							// Shift chars right of cursor position to the left
			    str[i] = str[i + 1];

		    str[i] = '\0';
		    update = 1;
			break;

		case KEY_CR:
			if (type & MC_TB_CL)
				pSM->state = MC_SM_FINISHED;
			break;

	    default:
		    if (pSM->curlen < maxlen && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127)	// Printable ASCII 7bit or printable 8bit ISO8859
		    {
			    for (i = pSM->curlen; i > pSM->curpos; i--)						// Shift chars right of cursor position to the right
				    str[i] = str[i - 1];

			    str[pSM->curpos++] = ch;
				str[++pSM->curlen] = 0;
				update = 1;
			}
			break;
    }

	if (update)
	{
		curs_set(win, FALSE);
		wmove_raw(win, pSM->starty, pSM->startx);
		waddstr_raw(win, str);
		if (pSM->curlen < maxlen)
			waddch_raw(win, ' ');
	}
	if (ch != ERR)
	{
// The commented out lines below cause a problem if home is pressed when text is selected
//		if (type & MC_TB_CL)
			wmove(win, pSM->starty, pSM->startx + pSM->curpos);
//		else
//			wmove_raw(win, pSM->starty, pSM->startx + pSM->curpos);
	}
	if (update)
		curs_set(win, TRUE);

	pSM->ch = ch;

	return pSM->state;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read string using state machine (with mini editor built-in)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
wgetnstr_sm (WINDOW* win, GetnstrState_t* pSM, char * str, uint8_t maxlen)
{
    uint8_t ch;
    uint8_t i;
	uint8_t update;

	if (pSM->state < MC_SM_GETCH)												// Initialize state
	{
		pSM->state  = MC_SM_GETCH;
		pSM->curlen = 0;
		pSM->curpos = 0;
	    getyx(win, pSM->starty, pSM->startx);
	}

    maxlen--;																	// Reserve one byte for terminating 0x00
	
	ch = wgetch(win);															// This waits for a key if nodelay cleared
	
	update = 0;
    switch (ch)
    {
	    case KEY_LEFT:
		    if (pSM->curpos > 0)
			    pSM->curpos--;
		    break;

	    case KEY_RIGHT:
		    if (pSM->curpos < pSM->curlen)
			    pSM->curpos++;
		    break;

	    case KEY_HOME:
		    pSM->curpos = 0;
		    break;

	    case KEY_END:
		    pSM->curpos = pSM->curlen;
		    break;

	    case KEY_BACKSPACE:
	    case KEY_DC:
			if (KEY_BACKSPACE == ch)
			{
				if (pSM->curpos > 0)
				    pSM->curpos--;												// Turn backspace into a delete
				else
					break;														// Cannot backspace past start of field
			}
			else
			{
				if (pSM->curpos == pSM->curlen)
					break;														// Cannot delete at end of text
				else
					win->bCurx = 0;												// Need to force wmove at bottom of routine to update cursor
			}

		    pSM->curlen--;

		    for (i = pSM->curpos; i < pSM->curlen; i++)
			    str[i] = str[i + 1];

		    str[i] = '\0';
		    update = 1;
			break;

		case KEY_CR:
			str[pSM->curlen]	= 0;
			pSM->state			= MC_SM_FINISHED;
			break;

	    default:
		    if (pSM->curlen < maxlen && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127)	// printable ASCII 7bit or printable 8bit ISO8859
		    {
			    for (i = pSM->curlen; i > pSM->curpos; i--)
				    str[i] = str[i - 1];

			    str[pSM->curpos++] = ch;
				str[++pSM->curlen] = 0;
				update = 1;
			}
			break;
    }

	if (update)
	{
		curs_set(win, 0);
		wmove_raw(win, pSM->starty, pSM->startx);
		waddstr_raw(win, str);
		if (pSM->curlen < maxlen)
			waddch_raw(win, ' ');
	}
	if (ch != ERR)
		wmove(win, pSM->starty, pSM->startx + pSM->curpos);
	if (update)
		curs_set(win, 1);
	
	pSM->ch = ch;

	return pSM->state;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: horizontal line from cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
whline (WINDOW* win, char ch, uint8_t cb)
{
	uint8_t bCurx;
	uint8_t bGraphic = 0;
	
	getx(win, bCurx);
	if (cb > win->bMaxx - bCurx)
		cb = win->bMaxx - bCurx;
		
    if (ch >= 0x80 && ch <= 0x9F)
    {
		ch -= 0x20;																// Subtract offset to G1 characters
        if (!(win->scr->bFlags & SCR_FLAG_CHARSET))								// Check if already in G1 set
		{
			bGraphic++;
	        waddch_raw (win, '\016');											// Switch to G1 set
		}
	}

	for (uint8_t i = 0; i < cb; i++)
	{
		waddch_raw(win, ch);
	}

	if (bGraphic)
	    waddch_raw (win, '\017');												// Switch to G0 set

	wmove_raw(win, win->bCury, bCurx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
winsch (WINDOW* win, uint8_t ch)
{
    mcurses_addch_or_insch (win, ch, TRUE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: winsdelln line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
winsdelln (WINDOW* win, int8_t bLines)
{
    mysetscrreg (win, win->bScrlStart, win->bScrlEnd);							// set scrolling region
    wmove_raw (win, win->bCury, 0);												// go to current line
	if (bLines < 0)
	{
		while (bLines++)
			waddstr_P_raw (win, SEQ_DELETELINE);								// delete line
	}
	else
	{
		while (bLines--)
			waddstr_P_raw (win, SEQ_INSERTLINE);								// insert line
	}
    mysetscrreg (win, 0, 0);													// reset scrolling region
    wmove_raw (win, win->bCury, win->bCurx);									// restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
winsnstr (WINDOW* win, const char* sz, uint8_t cb)
{
	if (0xFF == cb)
	{
		while (*sz)
		{
			mcurses_addch_or_insch (win, *sz, TRUE);
			sz++;
		}
	}
	else
	{
		while (cb-- && *sz)
		{
			mcurses_addch_or_insch (win, *sz, TRUE);
			sz++;
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor
 *
 * Upper left corner is 0,0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
wmove (WINDOW* win, uint8_t y, uint8_t x)
{
	if (y > win->bMaxy || x > win->bMaxx)
		return ERR;
	
    if (win->bCury != y || win->bCurx != x)
    {
        win->bCury = y;
        win->bCurx = x;
        wmove_raw (win, y, x);
    }
	
	return OK;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor within physical margin else within term screen
 *
 * Upper left corner is 0,0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
wmove_phys (WINDOW* win, uint8_t y, uint8_t x)
{
	uint8_t b;

	if (win->bFlags & WIN_FLAG_PHYSSCRLRGN)
	{
		y += win->bPhysStart;
		b  = win->bPhysEnd;
	}
	else
	{
		b = win->bMaxy;
	}
	
	if (y > b || x > win->bMaxx)
		return ERR;

    if (win->bCury != y || win->bCurx != x)
    {
        win->bCury = y;
        win->bCurx = x;
        wmove_raw (win, y, x);
    }
	
	return OK;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor (raw)
 *
 * Upper left corner is 0,0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wmove_raw (WINDOW* win, uint8_t y, uint8_t x)
{
    waddstr_P_raw (win, SEQ_CSI);
    mcurses_puti (win, y + 1 + win->bBegy);
    waddch_raw (win, ';');
    mcurses_puti (win, x + 1 + win->bBegx);
    waddch_raw (win, 'H');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: refresh: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wrefresh (WINDOW* win)
{
	win->scr->term->flushOutput();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: wscrl
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wscrl (WINDOW* win, uint8_t bLines)
{
    mysetscrreg (win, win->bScrlStart, win->bScrlEnd);							// set scrolling region
    wmove_raw (win, win->bScrlEnd, 0);											// goto to last line of scrolling region
	while (bLines--)
		waddstr_P_raw (win, SEQ_NEXTLINE);										// next line
    mysetscrreg (win, 0, 0);													// reset scrolling region
    wmove_raw (win, win->bCury, win->bCurx);									// restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set scrolling region
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wsetscrreg (WINDOW* win, uint8_t t, uint8_t b)
{
    win->bScrlStart	= t;
    win->bScrlEnd		= b;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set physical scrolling region on terminal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wsetscrreg_phys (WINDOW* win, uint8_t t, uint8_t b)
{
    if (t == b)
    {
		if (win->bFlags & WIN_FLAG_TERMSCRLRGN)
			mcurses_term_scrreg(win, TRUE);
		else	
			waddstr_P_raw (win, SEQ_RESET_SCRREG);								// reset scrolling region
			
		win->bFlags	&= ~WIN_FLAG_PHYSSCRLRGN;
    }
    else
    {
		wsetscrreg_raw(win, t, b);

		win->bFlags	|= WIN_FLAG_PHYSSCRLRGN;
		win->bPhysStart = t;
		win->bPhysEnd   = b;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set margins without keeping track of top/bottom
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wsetscrreg_raw (WINDOW* win, uint8_t t, uint8_t b)
{
	waddstr_P_raw (win, SEQ_CSI);
	mcurses_puti (win, t + 1 + win->bBegy);
	waddch_raw (win, ';');
	mcurses_puti (win, b + 1 + win->bBegy);
	waddch_raw (win, 'r');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: vertical line from cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wvline (WINDOW* win, char ch, uint8_t cb)
{
	uint8_t bCury, bCurx;
	uint8_t bGraphic = 0;
	
	getyx(win, bCury, bCurx);
	if (cb > win->bMaxy - bCury)
		cb = win->bMaxy - bCury;
		
    if (ch >= 0x80 && ch <= 0x9F)
    {
		ch -= 0x20;																// Subtract offset to G1 characters
        if (!(win->scr->bFlags & SCR_FLAG_CHARSET))								// Check if already in G1 set
		{
			bGraphic++;
	        waddch_raw (win, '\016');											// Switch to G1 set
		}
	}

	for (uint8_t i = 1; i < cb; i++)
	{
		waddch_raw(win, ch);
		wmove_raw(win, bCury+i, bCurx);
	}
	waddch_raw(win, ch);

	if (bGraphic)
	    waddch_raw (win, '\017');												// Switch to G0 set

	wmove_raw(win, bCury, bCurx);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES CURSES EXTENSION: wclrrec
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
wclrrec(WINDOW* win, uint8_t y, uint8_t x, uint8_t height, uint8_t width)
{
	char	ach[20];

	y+= win->bBegy+1;
	x+= win->bBegx+1;

	xsnprintf_P(ach, sizeof(ach), SEQ_ERASE_REC_AREA, y, x, y+height, x+width);
	waddstr_raw(win, ach);
}
