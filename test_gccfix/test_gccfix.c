/*
 * test_gccfix.c
 *
 * Created: 9/27/2016 7:40:41 PM
 *  Author: dharmon
 */ 


#define	 F_CPU	16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../source/lib_gccfix/gccfix.h"
#include "../source/lib_uart/uart.h"
#include "../source/lib_xio/xio.h"

int main(void)
{
//	volatile _sAccum	hk;
	volatile _Accum		k;
	volatile _lAccum	lk;
	
	sei();
	uart0_init(UART_BAUD_SELECT(14400, F_CPU));

	/////////////////////////////////////////////////////////////////////////
	// Test _sAccum functions
/*
	// sin, cos from -pi to pi
	xfprintf_P(uart0_putc, PSTR("rad, ssinsk, scossk\r\n"));
	for (hk = -PIsk; hk <= PIsk; hk += 0.01HK)
	{
		xfprintf_P(uart0_putc, PSTR("% .8hk, % .8hk, % .8hk\r\n"), hk, ssinsk(hk), scossk(hk));
	}
	xfprintf_P(uart0_putc, PSTR("% .8hk, % .8hk, % .8hk\r\n"), PIsk, ssinsk(PIsk), scossk(PIsk));
*/
	/////////////////////////////////////////////////////////////////////////
	// Test _lAccum functions

	xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, PIlk/5, LACCUM_FBIT, lsinlk(PIlk/5), LACCUM_FBIT, lcoslk(PIlk/5), LACCUM_FBIT, ltanlk(PIlk/5), LACCUM_FBIT, latanlk(PIlk/5));
	// sin, cos, tan, atan from -pi to pi
	xfprintf_P(uart0_putc, PSTR("rad, lsinlk, lcoslk, ltanlk, latanlk\r\n"));
	for (lk = -PIlk; lk <= PIlk; lk += ftolk(0.01))
	{
		xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, lk, LACCUM_FBIT, lsinlk(lk), LACCUM_FBIT, lcoslk(lk), LACCUM_FBIT, ltanlk(lk), LACCUM_FBIT, latanlk(lk));
	}
	xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, PIlk, LACCUM_FBIT, lsinlk(PIlk), LACCUM_FBIT, lcoslk(PIlk), LACCUM_FBIT, ltanlk(PIlk), LACCUM_FBIT, latanlk(PIlk));

	// sqrt, log from 0.01 to PI
	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx, lsqrtlk, lloglk\r\n"));
	for (lk = ftolk(0.01); lk <= PIlk; lk += ftolk(0.01))
	{
		xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, lk, LACCUM_FBIT, lsqrtlk(lk), LACCUM_FBIT, lloglk(lk));
	}
	xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, PIlk, LACCUM_FBIT, lsqrtlk(PIlk), LACCUM_FBIT, lloglk(PIlk));

	// sqrt, log from 0.25 to 100
	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx:0.25, lsqrtlk, lloglk\r\n"));
	for (lk = ftolk(0.25); lk <= ftolk(100.0); lk += ftolk(0.25))
	{
		xfprintf_P(uart0_putc, PSTR("% .8q, % .8q, % .8q\r\n"), LACCUM_FBIT, lk, LACCUM_FBIT, lsqrtlk(lk), LACCUM_FBIT, lloglk(lk));
	}

	/////////////////////////////////////////////////////////////////////////
	// Test _Accum functions	

	// sin, cos, tan, atan from -pi to pi	
	xfprintf_P(uart0_putc, PSTR("rad, sink, cosk, tank, atank\r\n"));
	for (k = -PIk; k <= PIk; k += 0.01K)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k, % .8k, % .8k\r\n"), k, sink(k), cosk(k), tank(k), atank(k));
	}
	xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k, % .8k, % .8k\r\n"), PIk, sink(PIk), cosk(PIk), tank(PIk), atank(PIk));

	// sqrt, log from 0.01 to PI
	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx, sqrtk, logk\r\n"));
	for (k = 0.01K; k <= PIk; k += 0.01K)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k\r\n"), k, sqrtk(k), logk(k));
	}
	xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k\r\n"), PIk, sqrtk(PIk), logk(PIk));

	// sqrt, log from 0.25 to 100
	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx:0.25, sqrtk, logk\r\n"));
	for (k = 0.25K; k <= 100K; k += 0.25K)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k\r\n"), k, sqrtk(k), logk(k));
	}

	while (1)
		;

/*
	char	sMessage[]	= "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
	"|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
*/

/*
	short		int				hp	 = 85;
	int							p	 = 21845;
	long		int				lp	 = 1431655765;
	short		int				hd	 = -86;
	int							d	 = -21846;
	long		int				ld	 = -1431655766;
	long long  int				lld	 = 0xAAAAAAAAAAAAAAAAUL;
	unsigned short		int		uhd	 = 170;
	unsigned			int		ud	 = 43690;
	unsigned long		int		uld	 = 2863311530;
	unsigned long long	int		ulld = 0xAAAAAAAAAAAAAAAAAUL;
*/
	
														//			Bytes
/*	
	short				_Fract	hr	 = .1;				// s  .7	  1
						_Fract	r	 = .1;				// s  .15	  2
	long				_Fract	lr	 = .1;				// s  .31	  4
	long long			_Fract	llr	 = .1;				// s  .63	  8
	unsigned short		_Fract	uhr  = .1;				//    .8	  1
	unsigned 			_Fract	ur   = .1;				//    .16	  2
	unsigned long		_Fract	ulr  = .1;				//    .32	  4
	unsigned long long  _Fract	ullr = .1;				//    .64	  8
*/
/*
						_sAccum	sk	 = PIsk;			// s 8.7	  2
						_Accum	k	 = PIk;				// s16.15	  4
						_lAccum	lk	 = PIlk;			// s 8.23	  4
*/
//	long				_Accum	lk	 = PIlk;			// s32.31	  8
/*
	long long			int		lli	 = 0x1921FB54442D1LL;
	long long			_Accum	llk  = *((long long _Accum*)&lli);		// s16.47	  8
*/
/*
	long long			_Accum	llk  = 3.14159265;		// s16.47	  8
	unsigned short		_Accum	uhk  = 3.14159265;		//   8.8	  2
	unsigned 			_Accum	uk   = 3.14159265;		//  16.16	  4
	unsigned long		_Accum	ulk  = 3.14159265;		//  32.32	  8
	unsigned long long  _Accum	ullk = 3.14159265;		//  16.48	  8
*/
/*
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%04X %08lX %08lX"),  PIsk,  PIk,  PIlk);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%04X %08lX %08lX %.0lq"),  sk,  k,  lk, 0, llk);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.6q"),  PIsk,  PIk,  23, PIlk);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.6q %.14lq"),  tohk(sk),  tok(k),  23, lk, 47, llk);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(sqrtk(k)),  23, lsqrtlk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(cosk(k)),  23, lcoslk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(sink(k)),  23, lsinlk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(tank(k)),  23, ltanlk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(atank(k)),  23, latanlk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(cosk(sqrtk(k))),  23, lcoslk(lsqrtlk(lk)));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(sink(sqrtk(k))),  23, lsinlk(lsqrtlk(lk)));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(tank(sqrtk(k))),  23, ltanlk(lsqrtlk(lk)));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(atank(sqrtk(k))),  23, latanlk(lsqrtlk(lk)));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(logk(k)),  23, lloglk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5k %.7q"),  tok(log10k(k)),  23, llog10lk(lk));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hd %hd %hd %hd"), countlssk(sk), countlsk(k), countlslk(lk), countlsk(0));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.7q"), roundsk(sk, 0), roundk(k, 0), 23, roundlk(lk, 0));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.7q"), roundsk(sk, 1), roundk(k, 1), 23, roundlk(lk, 1));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.7q"), roundsk(sk, 2), roundk(k, 2), 23, roundlk(lk, 2));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.7q"), roundsk(sk, 3), roundk(k, 3), 23, roundlk(lk, 3));
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.7q"), roundsk(sk, 4), roundk(k, 4), 23, roundlk(lk, 4));
*/
}
