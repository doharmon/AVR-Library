/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.h - include file for mcurses lib
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

#ifndef __MCURSES_H
#define __MCURSES_H

#include <stdint.h>
#include <stdarg.h>

#include <avr/pgmspace.h>

#include "mcurses-config.h"

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * some constants
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE                    (1)																// true
#define FALSE                   (0)																// false
#endif // TRUE

#define OK                      (0)
#define ERR                     (255)

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * attributes, may be ORed
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define A_NORMAL                0x0000															// normal
#define A_UNDERLINE             0x0001															// underline
#define A_REVERSE               0x0002															// reverse
#define A_BLINK                 0x0004															// blink
#define A_BOLD                  0x0008															// bold
#define A_DIM                   0x0010															// dim
#define A_STANDOUT              A_BOLD															// standout (same as bold)

#define F_BLACK                 0x0100															// foreground black
#define F_RED                   0x0200															// foreground red
#define F_GREEN                 0x0300															// foreground green
#define F_BROWN                 0x0400															// foreground brown
#define F_BLUE                  0x0500															// foreground blue
#define F_MAGENTA               0x0600															// foreground magenta
#define F_CYAN                  0x0700															// foreground cyan
#define F_WHITE                 0x0800															// foreground white
#define F_YELLOW                F_BROWN															// some terminals show brown as yellow (with A_BOLD)
#define F_COLOR                 0x0F00															// foreground mask

#define B_BLACK                 0x1000															// background black
#define B_RED                   0x2000															// background red
#define B_GREEN                 0x3000															// background green
#define B_BROWN                 0x4000															// background brown
#define B_BLUE                  0x5000															// background blue
#define B_MAGENTA               0x6000															// background magenta
#define B_CYAN                  0x7000															// background cyan
#define B_WHITE                 0x8000															// background white
#define B_YELLOW                B_BROWN															// some terminals show brown as yellow (with A_BOLD)
#define B_COLOR                 0xF000															// background mask

 /*---------------------------------------------------------------------------------------------------------------------------------------------------
 * State machine for wgetnstr_sm and wgetnstr_field_sm
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct getnstrstate 
{
	uint8_t state;																				// Set to MC_SM_START before calling wgetnstr_sm
	uint8_t curlen;																				// Current length of text in input field
	uint8_t curpos;																				// Postion of cursor within input field
	uint8_t starty;																				// Start row for input field`
	uint8_t startx;																				// Start column for input field
	uint8_t ch;																					// Last keystroke
} GetnstrState_t;

#define MC_SM_START				0x00															// Start state
#define MC_SM_FINISHED			0x01															// Enter/Return pressed
#define MC_SM_GETCH				0x02															// Parse the next input character
#define MC_SM_SELECTED			0x04															// Text field is selected
#define MC_SM_RESERVED1			0x08															// Reserved for use by other libraries
#define MC_SM_RESERVED2			0x10															// Reserved for use by other libraries
#define MC_SM_RESERVED3			0x20															// Reserved for use by other libraries
#define MC_SM_HAS_SIGN			0x40															// Field has a sign character
#define MC_SM_HAS_DEC			0x80															// Field has a decimal point character
#define MC_SM_HAS_SIGN_DEC		0xC0															// Field has both sign and decimal point characters

#define MC_TB_TEXT				0x01															// Type bit for text fields
#define MC_TB_BIN				0x02															// Type bit for binary fields
#define MC_TB_OCT				0x04															// Type bit for octal fields
#define MC_TB_DEC				0x08															// Type bit for decimal fields
#define MC_TB_HEX				0x10															// Type bit for hexadecimal fields
#define MC_TB_CL				0x20															// Type bit for command line
#define MC_TB_FRAC				0x40															// Type bit if decimal has fractional part
#define MC_TB_SIGNED			0x80															// Type bit if decimal is signed

																								// Field types for wgetnstr_field_sm
#define MC_T_TEXT				MC_TB_TEXT														// Text field
#define MC_T_HEX				MC_TB_HEX														// Hexadecimal field
#define MC_T_INT				(MC_TB_DEC | MC_TB_SIGNED)										// Signed integer field
#define MC_T_POS_INT			MC_TB_DEC														// Unsigned integer field
#define MC_T_FP					(MC_TB_DEC | MC_TB_SIGNED | MC_TB_FRAC)							// Signed fixed point field
#define MC_T_POS_FP				(MC_TB_DEC | MC_TB_FRAC)										// Unsigned fixed point field
#define MC_T_CL					(MC_TB_TEXT | MC_TB_CL)											// Command line

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Terminal structure
 *
 * Note: putByte must be first element. wprintw assembly code must know offset for putByte.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef void		(*putByte_t)(uint8_t b);
typedef uint16_t	(*getByte_t)(void);
typedef uint16_t	(*available_t)(void);
typedef void		(*flushOutput_t)(void);

typedef struct term
{
	putByte_t		putByte;																	// Put byte in transmit buffer
	getByte_t		getByte;																	// Read byte from receive buffer (-1 if empty)
	available_t		available;																	// Return number of bytes in transmit buffer
	flushOutput_t	flushOutput;																// Pause until all bytes in transmit buffer have been sent
} TERM;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * The global array g_aterm needs to be defined and configured with all the terminals that well be used by the application
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern const __flash TERM g_aterm[];

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Screen structure
 *
 * Note: term must be first element. wprintw assembly code must know offset for term.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define SCR_BUFSIZE			9

typedef struct screen
{
	const __flash TERM*	term;																	// Pointer to terminal I/O functions
	uint8_t				bFlags;																	// Terminal screen flags and wgetch state
	uint8_t				bMaxy;																	// Maximum row, not height
	uint8_t				bMaxx;																	// Maximum column, not width
//	uint8_t				bHalfdelay;																// Delay wgetch waits in tenths of a second, maximum: 255
	char				acBuf[SCR_BUFSIZE];														// Escape sequence buffer
	uint8_t				bIdx;																	// Index to parse escape sequence
} SCREEN;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Values for screen bFlags member
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define SCR_FLAG_IS_UP				0x01														// Screen is initialized
#define SCR_FLAG_QUERY_SIZE			0x02														// A request for display size is pending
#define SCR_FLAG_INSERT_MODE		0x04														// Set when insert mode enabled
#define SCR_FLAG_CHARSET			0x08														// Set when charset is G1
#define SCR_FLAG_NODELAY			0x10														// Set when wgetch should not wait for a key
#define SCR_FLAG_ESC				0x20														// Parsing an escape sequence
#define SCR_FLAG_ESC_ALPHA_TILDE	0x40														// Parsing a ^[[...Alpha|~ sequence
#define SCR_FLAG_ESC_KEYPAD			0x80														// Parsing a ^[OAlpha sequence
#define SCR_FLAG_ESC_STATE		(SCR_FLAG_ESC | SCR_FLAG_ESC_ALPHA_TILDE | SCR_FLAG_ESC_KEYPAD)	// Escape parser state mask

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Window structure
 *
 * Note: scr must be first element. wprintw assembly code must know offset for scr.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct window WINDOW;

struct window
{
	SCREEN*		scr;																			// Pointer to terminal screen
	uint8_t		bFlags;																			// Window flags
	uint8_t 	bBegy;																			// Top left row (0 based)
	uint8_t		bBegx;																			// Top left column (0 based)
	uint8_t		bMaxy;																			// Maximum row, not height
	uint8_t		bMaxx;																			// Maximum column, not width
	uint8_t		bCury;																			// Current cursor row position (0 based, relative to bBegy)
	uint8_t 	bCurx;																			// Current cursor column position (0 based, relative to bBegx)
	uint8_t		bPhysStart;																		// Top line of physical scroll region (0 based, relative to bBegy)
	uint8_t		bPhysEnd;																		// Bottom line of physical scroll region (0 based, relative to bBegy)
	uint8_t		bScrlStart;																		// Top line of scroll region (0 based, relative to bBegy)
	uint8_t 	bScrlEnd;																		// Bottom line of scroll region (0 based, relative to bBegy)
	uint16_t	wAttrs;																			// Current attributes for non-space chars
};

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Values for window bFlags member
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define WIN_FLAG_IS_UP        	0x01															// Window is initialized
#define WIN_FLAG_CURSSET       	0x02															// Visibility of cursor. Set: visible
#define WIN_FLAG_FULLWIN        0x04															// Window is full-screen
#define WIN_FLAG_STATUSLINE     0x08															// Window has status line at bottom
#define WIN_FLAG_MENUBAR     	0x10															// Window has menu bar at top
#define WIN_FLAG_TERMSCRLRGN  	0x20															// Scrolling region is entire window minus menu or status line
#define WIN_FLAG_PHYSSCRLRGN  	0x40															// Scrolling region is a physical top/bottom margin
#define WIN_FLAG_FRAME			0x80															// Window has a frame/border. Not implemented.

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Functions from curses.h 6.0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void 		beep  (void);														// Implement?
uint8_t 	cbreak (void);														// Implement? Disable buffering. nocbreak to enable buffering
uint8_t 	copywin (const WINDOW*,WINDOW*,int,int,int,int,int,int,int);		// DEC copy rectangle?
void	 	curs_set (WINDOW*, uint8_t);										// 
void 		endwin (WINDOW*);													// 
uint8_t 	flushinp (void);													// Macro for flush input
void	 	halfdelay (WINDOW*, uint8_t);										// Implement?: getch pauses 1-255 tenths of a second before ERR. nocbreak to exit mode
WINDOW* 	initscr (void);														// Not needed?
void	 	mvprintw_P (uint8_t,uint8_t,const char *,...);						// Macro
uint8_t 	mvwin (WINDOW*,int,int);											// Implement?
void 		mvwprintw_P (WINDOW*,uint8_t,uint8_t,const char *,...);				// Test
void 		mvwprintw_P_raw (WINDOW*,uint8_t,uint8_t,const char *,...);			// Test
uint8_t 	napms (int);														// Macro. Millisecond delay
void		newterm (uint8_t bIdx, SCREEN* pscr, WINDOW* win);					// Make new term current term
void	 	newwin (WINDOW*,uint8_t,uint8_t,uint8_t,uint8_t);					// Implement. Call set_term first?
uint8_t 	nl (void);															// Implement? xio handles
uint8_t 	nocbreak (void);													// Implement? Buffer input until newline or carriage return
void 		nodelay (WINDOW*,uint8_t);											// Implement?
uint8_t 	nonl (void);														// Implement? xio handles.
uint8_t 	notimeout (WINDOW*,uint8_t);										// Implement? Maybe as a configurable option
uint8_t 	overlay (const WINDOW*,WINDOW*);									// DEC copy rectangle?
uint8_t 	overwrite (const WINDOW*,WINDOW*);									// DEC copy rectangle?
// 			printw_P (const char *,...);										// Macro Test
SCREEN*		set_term(SCREEN*);													// Test
uint8_t 	vwprintw (WINDOW*,const char*,va_list);								// Implement
void	 	waddch (WINDOW*, const uint8_t);									// 
void		waddch_graphic_raw (WINDOW*, uint8_t);								// Bypasses updating cursor
void		waddch_raw (WINDOW*, uint8_t);										// Bypasses updating cursor and graphic chars
void	 	waddnstr (WINDOW*,const char*,uint8_t);								// Implement cb field
void		waddnstr_raw (WINDOW*, const char*,uint8_t);						// Implement cb field. Bypasses updating cursor and graphic chars
void	 	waddnstr_P (WINDOW*,const __flash char*,uint8_t);					// Implement cb field
void		waddnstr_P_graphic_raw (WINDOW*, const __flash char*,uint8_t);		// Implement cb field. Bypasses updating cursor
void		waddnstr_P_raw (WINDOW*, const __flash char*,uint8_t);				// Implement cb field. Bypasses updating cursor and graphic chars
//			wattroff (WINDOW* win, uint16_t attrs);								// Test Macro of wattrset
//			wattron (WINDOW* win, uint16_t attrs);								// Test Macro of wattrset
void		wattrset (WINDOW* win, uint16_t attrs);								// 
void 		wborder (WINDOW*,char,char,char,char,char,char,char,char);			// Test
void 		wclear (WINDOW*);													// 
void 		wclear_phys (WINDOW*);												// Clear part of screen within physical margins
void 		wclrtobot (WINDOW*);												// 
void 		wclrtoeol (WINDOW*);												// 
void 		wdelch (WINDOW*);													// 
//	 		werase (WINDOW*);													// Macro of wclear
uint8_t 	wgetch (WINDOW*);													// 
uint8_t 	wgetch_raw (WINDOW*);												// No escape sequence parsing
void	 	wgetnstr (WINDOW*,char*,uint8_t);									// Test
uint8_t 	wgetnstr_field_sm (WINDOW*,GetnstrState_t*,char*,uint8_t,uint8_t);	// 
uint8_t 	wgetnstr_sm (WINDOW*,GetnstrState_t*,char*,uint8_t);				// 
void	 	whline (WINDOW*, char, uint8_t);									// 
void		winsch (WINDOW*, uint8_t);											// 
void	 	winsdelln (WINDOW*,int8_t);											// 
void	 	winsnstr (WINDOW*, const char*,uint8_t);							// Test
uint8_t	 	wmove (WINDOW*,uint8_t,uint8_t);									// 
uint8_t	 	wmove_phys (WINDOW*,uint8_t,uint8_t);								// Move cursor within physical margin else within term screen
void		wmove_raw (WINDOW*, uint8_t, uint8_t);								// Bypasses updating cursor and graphic chars
void	 	wprintw_P (WINDOW*, const __flash char*,...);						//
void	 	wprintw_P_raw (WINDOW*, const __flash char*,...);					// Bypasses updating cursor and graphic chars
void	 	wrefresh (WINDOW*);													// 
void	 	wscrl (WINDOW*,uint8_t);											// 
void 		wsetscrreg (WINDOW*,uint8_t,uint8_t);								// Software margins
void 		wsetscrreg_phys (WINDOW*,uint8_t,uint8_t);							// Use terminal physical margins
void 		wsetscrreg_raw (WINDOW*,uint8_t,uint8_t);							// Set margins without keeping track of top/bottom
void 		wtimeout (WINDOW*,int);												// Implement? Maybe as a configurable option
void 		wvline (WINDOW*,char,uint8_t);										// Test

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Most of the pseudo functions are macros that either provide compatibility
 * with older versions of curses, or provide inline functionality to improve
 * performance.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * These pseudo functions are always implemented as macros:
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define getyx(win,y,x)   						(y = getcury(win), x = getcurx(win))
#define gety(win,y)   							(y = getcury(win))
#define getx(win,x)   							(x = getcurx(win))
#define getbegyx(win,y,x)						(y = getbegy(win), x = getbegx(win))
#define getmaxyx(win,y,x)						(y = getmaxy(win), x = getmaxx(win))

/* It seems older SYSV curses versions define these */
#define getattrs(win)							(win)->wAttr
#define getcurx(win)							(win)->bCurx
#define getcury(win)							(win)->bCury
#define getbegx(win)							(win)->bBegx
#define getbegy(win)							(win)->bBegy
#define getmaxx(win)							(win)->bMaxx
#define getmaxy(win)							(win)->bMaxy

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * pseudo functions for standard screen from ncurses 6.0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if MCURSES_USE_STDSCR > 0
#define addch(ch)								waddch(stdscr,(ch))
#define addnstr(str,n)							waddnstr(stdscr,(str),(n))
#define addstr(str)								waddnstr(stdscr,(str),-1)
#define attroff(at)								wattroff(stdscr,(at))
#define attron(at)								wattron(stdscr,(at))
#define attrset(at)								wattrset(stdscr,(at))
#define attroff(at)								wattroff(stdscr,(at))
#define attron(at)								wattron(stdscr,(at))
#define clear()									wclear(stdscr)
#define clrtobot()								wclrtobot(stdscr)
#define clrtoeol()								wclrtoeol(stdscr)
#define delch()									wdelch(stdscr)
#define deleteln()								winsdelln(stdscr,-1)
#define erase()									werase(stdscr)
#define getch()									wgetch(stdscr)
#define getstr(str)								wgetstr(stdscr,(str))
#define insch(c)								winsch(stdscr,(c))
#define insdelln(n)								winsdelln(stdscr,(n))
#define insertln()								winsdelln(stdscr,1)
#define insnstr(s,n)							winsnstr(stdscr,(s),(n))
#define insstr(s)								winsstr(stdscr,(s))
#define move(y,x)								wmove(stdscr,(y),(x))
#define printw_P(str,__VA_ARGS__)				wprintw_P(stdscr,(str),__VA_ARGS__)
#define refresh()								wrefresh(stdscr)
#define scrl(n)									wscrl(stdscr,(n))
#define setscrreg(t,b)							wsetscrreg(stdscr,(t),(b))
#define timeout(delay)							wtimeout(stdscr,(delay))
#endif // MCURSES_USE_STDSCR > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mv functions from ncurses 6.0
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define mvwaddch(win,y,x,ch)					wmove((win),(y),(x)),	  waddch((win),(ch))
#define mvwaddch_graphic_raw(win,y,x,ch)		wmove((win),(y),(x)),	  waddch_graphic_raw((win),(ch))
#define mvwaddch_raw(win,y,x,ch)				wmove_raw((win),(y),(x)), waddch_raw((win),(ch))
#define mvwaddnstr(win,y,x,str,n)				wmove((win),(y),(x)),	  waddnstr((win),(str),(n))
#define mvwaddnstr_raw(win,y,x,str,n)			wmove_raw((win),(y),(x)), waddnstr_raw((win),(str),(n))
#define mvwaddnstr_P(win,y,x,str,n)				wmove((win),(y),(x)),	  waddnstr_P((win),(str),(n))
#define mvwaddnstr_P_graphic_raw(win,y,x,str,n)	wmove_raw((win),(y),(x)), waddnstr_P_graphic_raw((win),(str),(n))
#define mvwaddnstr_P_raw(win,y,x,str,n)			wmove_raw((win),(y),(x)), waddnstr_P_raw((win),(str),(n))
#define mvwaddstr(win,y,x,str)					wmove((win),(y),(x)),	  waddnstr((win),(str),-1)
#define mvwaddstr_raw(win,y,x,str)				wmove_raw((win),(y),(x)), waddnstr_raw((win),(str),-1)
#define mvwaddstr_P(win,y,x,str)				wmove((win),(y),(x)),	  waddnstr_P((win),(str),-1)
#define mvwaddstr_P_graphic_raw(win,y,x,str)	wmove_raw((win),(y),(x)), waddnstr_P_graphic_raw((win),(str),-1)
#define mvwaddstr_P_raw(win,y,x,str)			wmove_raw((win),(y),(x)), waddnstr_P_raw((win),(str),-1)
#define mvwdelch(win,y,x)						wmove((win),(y),(x)), 	  wdelch(win)
#define mvwgetch(win,y,x)						wmove((win),(y),(x)), 	  wgetch(win)
#define mvwgetnstr(win,y,x,str,n)				wmove((win),(y),(x)), 	  wgetnstr((win),(str),(n))
#define mvwgetstr(win,y,x,str)					wmove((win),(y),(x)), 	  wgetstr((win),(str))
#define mvwhline(win,y,x,c,n)					wmove((win),(y),(x)), 	  whline((win),(c),(n))
#define mvwinsch(win,y,x,c)						wmove((win),(y),(x)), 	  winsch((win),(c))
#define mvwinsnstr(win,y,x,s,n)					wmove((win),(y),(x)), 	  winsnstr((win),(s),(n))
#define mvwinsstr(win,y,x,s)					wmove((win),(y),(x)), 	  winsstr((win),(s))
#define mvwvline(win,y,x,c,n)					wmove((win),(y),(x)), 	  wvline((win),(c),(n))


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses macros
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define waddstr(win,str)						waddnstr(win,str,-1)
#define waddstr_raw(win,str)					waddnstr_raw(win,str,-1)
#define waddstr_P(win,str)						waddnstr_P(win,str,-1)
#define waddstr_P_raw(win,str)					waddnstr_P_raw(win,str,-1)
#define waddstr_P_graphic_raw(win,str)			waddnstr_P_graphic_raw(win,str,-1)
#define wattroff(win,at)						wattrset(win,win->wAttrs & ~(at))
#define wattron(win,at)							wattrset(win,win->wAttrs |  (at))
#define wdeleteln(win)							winsdelln(win,-1)
#define werase(win)								wclear(win)
#define winsertln(win)							winsdelln(win,1)
#define winsstr(win,s)							winsnstr(win,(s),-1)

#define scroll(win)								wscrl(win,1)
 
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses functions
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void		mcurses_phyio_putc (uint8_t ch);									// Internal UART routine
uint16_t	mcurses_phyio_getbyte (void);										// Internal UART routine
uint16_t	mcurses_phyio_available (void);										// Internal UART routine
void		mcurses_phyio_flushoutput (void);									// Internal UART routine
void		mcurses_get_term_size(WINDOW* pwin);								// Get size of terminal screen
void		mcurses_term_reset(WINDOW* pwin);									// Reset physical terminal screen
void		mcurses_get_cursor_position(WINDOW* pwin);							// Get current cursor position
void		mcurses_term_scrreg(WINDOW* pwin, uint8_t bUsePhysScrReg);			// Enable/disable physical scrolling region size of terminal minus menu and status line
void		mcurses_newwin(SCREEN* pscr, WINDOW* pwin, uint8_t bBegy, uint8_t bBegx, uint8_t bRows, uint8_t bCols);

void		wclrrec(WINDOW* pwin, uint8_t y, uint8_t x, uint8_t height, uint8_t width);

#if MCURSES_USE_XIO > 0
//extern void                     vprintw (WINDOW* pMCS, const __flash char *, va_list);	// add formatted string (va_list)
//extern void                     printw (WINDOW* pMCS, const __flash char *, ...);		// add formatted string (variable number of arguments)
#endif // MCURSES_USE_XIO > 0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses keys
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define KEY_TAB                 '\t'															// TAB key
#define KEY_CR                  '\r'															// RETURN key
#define KEY_BACKSPACE           '\b'															// Backspace key
#define KEY_ESCAPE              0x1B															// ESCAPE (pressed twice)

#define KEY_DOWN                0x80															// Down arrow key
#define KEY_UP                  0x81															// Up arrow key
#define KEY_LEFT                0x82															// Left arrow key
#define KEY_RIGHT               0x83															// Right arrow key
#define KEY_HOME                0x84															// Home key
#define KEY_DC                  0x85															// Delete character key
#define KEY_IC                  0x86															// Ins char/toggle ins mode key
#define KEY_NPAGE               0x87															// Next-page key
#define KEY_PPAGE               0x88															// Previous-page key
#define KEY_END                 0x89															// End key
#define KEY_BTAB                0x8A															// Back tab key
#define KEY_F1                  0x8B															// Function key F1
#define KEY_F(n)                (KEY_F1+(n)-1)													// Space for additional 12 function keys

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: Character sets
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CHARSET_G0     ~SCR_FLAG_CHARSET
#define CHARSET_G1      SCR_FLAG_CHARSET

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: draw boxes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_LRCORNER            0x8A															// DEC graphic 0x6A: lower right corner
#define ACS_URCORNER            0x8B															// DEC graphic 0x6B: upper right corner
#define ACS_ULCORNER            0x8C															// DEC graphic 0x6C: upper left corner
#define ACS_LLCORNER            0x8D															// DEC graphic 0x6D: lower left corner
#define ACS_PLUS                0x8E															// DEC graphic 0x6E: crossing lines
#define ACS_HLINE               0x91															// DEC graphic 0x71: horizontal line
#define ACS_LTEE                0x94															// DEC graphic 0x74: left tee
#define ACS_RTEE                0x95															// DEC graphic 0x75: right tee
#define ACS_BTEE                0x96															// DEC graphic 0x76: bottom tee
#define ACS_TTEE                0x97															// DEC graphic 0x77: top tee
#define ACS_VLINE               0x98															// DEC graphic 0x78: vertical line

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: other symbols
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_DIAMOND             0x80															// DEC graphic 0x60: diamond
#define ACS_CKBOARD             0x81															// DEC graphic 0x61: checker board
#define ACS_DEGREE              0x86															// DEC graphic 0x66: degree symbol
#define ACS_PLMINUS             0x87															// DEC graphic 0x66: plus/minus

#define ACS_S1                  0x8F															// DEC graphic 0x6F: scan line 1
#define ACS_S3                  0x90															// DEC graphic 0x70: scan line 3
#define ACS_S5                  0x91															// DEC graphic 0x71: scan line 5
#define ACS_S7                  0x92															// DEC graphic 0x72: scan line 7
#define ACS_S9                  0x93															// DEC graphic 0x73: scan line 9
#define ACS_LEQUAL              0x99															// DEC graphic 0x79: less/equal
#define ACS_GEQUAL              0x9A															// DEC graphic 0x7A: greater/equal
#define ACS_PI                  0x9B															// DEC graphic 0x7B: Pi
#define ACS_NEQUAL              0x9C															// DEC graphic 0x7C: not equal
#define ACS_STERLING            0x9D															// DEC graphic 0x7D: uk pound sign
#define ACS_BULLET              0x9E															// DEC graphic 0x7E: bullet

#endif // __MCURSES_H
