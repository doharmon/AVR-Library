/*
 * test_eqeval.c
 *
 * Created: 5/2/2017 5:42:50 PM
 * Author : David
 */ 

#ifdef __AVR_ATmega328P__
#define	 F_CPU	16000000UL
#endif

#ifdef __AVR_ATmega1284P__
#define	 F_CPU	22118400UL
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include "../source/lib_eqeval/eqeval.h"
#include "../source/lib_xio/xio.h"
#include "../source/lib_uart/uart.h"
#include "../source/lib_mcurses/mcurses.h"
#include "../source/lib_gccfix/gccfix.h"

extern uint8_t						g_atokCode[];
extern uint8_t						g_itokCode;
extern const __flash eqevSymbol_t	g_aSymbol[];

#define scr			(&screen)
#define win			(&window)

const __flash TERM g_aterm[] =
{
	{	mcurses_phyio_putc, mcurses_phyio_getbyte, mcurses_phyio_available, mcurses_phyio_flushoutput	},
};

void dumpCode(void)
{
	uint8_t 		iCode;
	eqevConvert_t	convert;

	xprintf_P(PSTR("\r\n"));
	for (iCode = 0; iCode < EQEV_SIZE_CODESTACK; iCode++)
	{
		xprintf_P(PSTR("%03d Token: %02X"), iCode, g_atokCode[iCode]);
		if (EQEV_TOK_FP == g_atokCode[iCode])
		{
			convert.b[0] = g_atokCode[++iCode];
			convert.b[1] = g_atokCode[++iCode];
			convert.b[2] = g_atokCode[++iCode];
			convert.b[3] = g_atokCode[++iCode];
			xprintf_P(PSTR(" %k"), convert.k);
		}
		else if (EQEV_TOK_NEG == g_atokCode[iCode])
		{
			xprintf_P(PSTR(" NEG"));
		}
		else if (EQEV_TOK_END == g_atokCode[iCode])
		{
			xprintf_P(PSTR(" END\r\n"));
			break;
		}
		else if (EQEV_T_INT == (EQEV_TOK_INT_MASK & g_atokCode[iCode]))
		{
			xprintf_P(PSTR(" %d"), (EQEV_T_INT-1) & g_atokCode[iCode]);
		}
		else
		{
//			for (uint8_t i = 0; i < EQEV_SYMTAB_SIZE; i++)
			for (uint8_t i = 0; i < 19; i++)
			if (g_atokCode[iCode] == g_aSymbol[i].bToken)
			{
				xprintf_P(PSTR(" %S"), g_aSymbol[i].psSymbol);
				break;
			}
		}
		xprintf_P(PSTR("\r\n"));
	}
}

__attribute__((OS_main)) int main(void)
{
	uint8_t			err;
	char			sz[EQEV_SIZE_EQ];
	char*			psz;
	_Accum			x;
	SCREEN			screen;
	WINDOW			window;

	sei();

	newterm(0, scr, win);
	mcurses_term_scrreg(win, TRUE);
	xdev_out(mcurses_phyio_putc);

	mcurses_term_reset(win);

	xprintf_P(PSTR("Waiting for response from terminal\r\nMake sure terminal TX is connected to chip"));
	while (scr->bFlags & SCR_FLAG_QUERY_SIZE)
		wgetch(win);

	wclear(win);

	g_atokCode[0] = EQEV_TOK_END;
	
	xprintf_P(PSTR("Test program for the equation evaluator library\r\n"));

	while (1)
	{
		xprintf_P(PSTR("\r\nEnter an equation: "));
		mcurses_get_cursor_position(win);
		wgetnstr (win, sz, sizeof(sz));
		
		xprintf_P(PSTR("\r\n\r\n"), sz);
		
		strupr(sz);
		if (strncmp_P(sz, PSTR("X="), 2) == 0)
		{
			psz = sz+2;
			xatok(&psz, &x, xioFRACBITS_K, 0);
			xprintf_P(PSTR("For x = %k equation = %k\r\n"), x, eqeval_eval(x));
			continue;
		}
		
		if (strncmp_P(sz, PSTR("TABLE"), 5) == 0)
		{
			xprintf_P(PSTR("\r\n\r\n"), sz);
			for (x = -PIk; x <= PIk; x += 0.125K)
				xprintf_P(PSTR("  %10k  %10k\r\n"), x, eqeval_eval(x));
			xprintf_P(PSTR("  %10k  %10k\r\n"), PIk, eqeval_eval(PIk));
			xprintf_P(PSTR("\r\n"), sz);
			continue;
		}
		
		if (strncmp_P(sz, PSTR("DUMP"), 4) == 0)
		{
			dumpCode();
			continue;
		}

		err = eqeval_parse(sz);
		if (err)
		{
			xprintf_P(PSTR("\r\nParser error: %S\r\n\r\n"), eqeval_get_err_msg(err));
		}

		if (g_itokCode)
		{
			xprintf_P(PSTR("\r\nBefore Optimization:\r\n"));
			dumpCode();
			xprintf_P(PSTR("\r\nAfter Optimization:\r\n"));
			eqeval_optimize();
			dumpCode();
		}
	}
}

