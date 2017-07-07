/*
 * test_avrfix.c
 *
 * Created: 9/27/2016 7:40:41 PM
 *  Author: dharmon
 */ 


#define	 F_CPU	16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../source/lib_avrfix/avrfix.h"
#include "../source/lib_uart/uart.h"
#include "../source/lib_xio/xio.h"

typedef union
{
	_sAcc				sk;		// AVR-Fix
	short _Accum		hk;		// GCC fixed point
} hk_t;

typedef union
{
	_Acc				q;		// AVR-Fix
	_Accum				k;		// GCC fixed point
} k_t;

typedef union
{
	long long			llq;	// AVR-Fix
	long _Accum			lk;		// GCC fixed point
} lk_t;

typedef union
{
	long long			llq;	// AVR-Fix
	long long _Accum	llk;	// GCC fixed point
} llk_t;

#define tohk(f)		((hk_t)f).hk
#define tok(f)		((k_t)f).k
#define tolk(f)		((lk_t)f).lk
#define tollk(f)	((llk_t)f).llk

int main(void)
{
	volatile k_t		kInc;
//	volatile k_t		kMin;
	volatile k_t		kMax;
	volatile k_t		k;
	
	kInc.k	= .01;
//	kMin.q	= ACCUM_MIN;
//	kMax.q	= ACCUM_MAX;
	
	sei();
	
	uart0_init(UART_BAUD_SELECT(14400, F_CPU));

//	xfprintf_P(uart0_putc, PSTR("%.8k %.8k\r\n"), tok(-PIk), tok(kInc));

	xfprintf_P(uart0_putc, PSTR("rad, sink, cosk, tank, atank\r\n"));
	for (k.q = -PIk; k.q <= PIk; k.q += kInc.q)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k, % .8k, % .8k\r\n"), tok(k), tok(sink(k.q)), tok(cosk(k.q)), tok(tank(k.q)), tok(atank(k.q)));
	}
	xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k, % .8k, % .8k\r\n"), tok(PIk), tok(sink(PIk)), tok(cosk(PIk)), tok(tank(PIk)), tok(atank(PIk)));

	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx, sqrtk, logk\r\n"));
	for (k = kInc; k.q <= PIk; k.q += kInc.q)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k\r\n"), tok(k), tok(sqrtk(k.q)), tok(logk(k.q)));
	}

	kInc.k = 0.25;
	kMax.k = 100;
	xfprintf_P(uart0_putc, PSTR("\r\n\r\nx:0.25, sqrtk, logk\r\n"));
	for (k = kInc; k.q <= kMax.q; k.q += kInc.q)
	{
		xfprintf_P(uart0_putc, PSTR("% .8k, % .8k, % .8k\r\n"), tok(k), tok(sqrtk(k.q)), tok(logk(k.q)));
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
						_sAcc	sk	 = PIsk;			// s 8.7	  2
						_Acc	k	 = PIk;				// s16.15	  4
						_lAcc	lk	 = PIlk;			// s 8.23	  4
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
