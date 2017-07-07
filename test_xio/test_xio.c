/*
 * test_xio.c
 *
 * Created: 3/17/2016 1:29:52 AM
 *  Author: dharmon
 */ 

/* Example hardware timer code
void timeout_init(void)
{	
	// call at startup, let it run until needed
	TCCR1A = 0;		// "Normal" mode
	TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10); // prescale /1024
	return;
}
#define	TICKS_PER_SEC (F_CPU/1024)			// ticks/sec with prescale /1024
#define	TIMEOUT_TIME (1 * TICKS_PER_SEC)	// timeout: 1 second
#define	reset_timeout() do { TCNT1 = 0; } while (0)
#define	timeout_event() (TCNT1 >= TIMEOUT_TIME)
*/

#define	 F_CPU	16000000UL

#include <avr/interrupt.h>
#include <util/delay.h>

#include "../source/lib_uart/uart.h"
#include "../source/lib_xio/xio.h"


//unsigned long u32div10(unsigned long lq);

int main(void)
{
	char	sMessage[]	= "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
						  "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
	
			 short		int		hp	 = 85;
						int		p	 = 21845;
			 long		int		lp	 = 1431655765;
			 short		int		hd	 = -86;
						int		d	 = -21846;
			 long		int		ld	 = -1431655766;
			 long long  int		lld	 = 0xAAAAAAAAAAAAAAAAUL;
	unsigned short		int		uhd	 = 170;
	unsigned			int		ud	 = 43690;
	unsigned long		int		uld	 = 2863311530;
	unsigned long long	int		ulld = 0xAAAAAAAAAAAAAAAAAUL;
	
														//			Bytes
			 short		_Fract	hr	 = -.1;				// s  .7	  1
						_Fract	r	 = -.1;				// s  .15	  2
			 long		_Fract	lr	 = -.1;				// s  .31	  4
			 long long  _Fract	llr	 = -.1;				// s  .63	  8
	unsigned short		_Fract	uhr  = .1;				//    .8	  1
	unsigned 			_Fract	ur   = .1;				//    .16	  2
	unsigned long		_Fract	ulr  = .1;				//    .32	  4
	unsigned long long  _Fract	ullr = .1;				//    .64	  8
			 short		_Accum	hk	 = -3.14159265;		// s 8.7	  2
						_Accum	k	 = -3.14159265;		// s16.15	  4
			 long		_Accum	lk	 = -3.14159265;		// s32.31	  8
			 long long  _Accum	llk  = -3.14159265;		// s16.47	  8
	unsigned short		_Accum	uhk  = 3.14159265;		//   8.8	  2
	unsigned 			_Accum	uk   = 3.14159265;		//  16.16	  4
	unsigned long		_Accum	ulk  = 3.14159265;		//  32.32	  8
	unsigned long long  _Accum	ullk = 3.14159265;		//  16.48	  8

	sei();	
	uart0_init(UART_BAUD_SELECT(9600, F_CPU));

	char  sz[20] = "0";
	char* psz    = sz;
	
	xatok(&psz, &k, xioFRACBITS_K, 0);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Testing xatok [%s]: %k\r\n"), sz, k);
	xputs(uart0_putc, sMessage);
	
	psz = sz;
	strncpy_P(sz, PSTR("-3.14159265"), sizeof(sz));
	xatok(&psz, &k, xioFRACBITS_K, 0);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Testing xatok [%s]: %k\r\n"), sz, k);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Test message. Buffer length %d.\r\n"), sizeof(sMessage));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, 10, PSTR("Another test message but buffer is too small."));
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nInvalid conversion specification %%t [%t]\r\n"), sizeof(sMessage));
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\n%s\r\n"), "String from RAM");
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%S\r\n"), PSTR("String from ROM"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Output ASCII char: [%c]\r\n"), '*');
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hd %d %ld\r\n"), hp,  p,  lp);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%20hd %20d %20ld\r\n"), hp,  p,  lp);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hd %d %ld\r\n"), hd,  d,  ld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hu %u %lu\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hX %X %lX\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hb %b %lb\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%20hd %20d %20ld\r\n"), hd,  d,  ld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%20hu %20u %20lu\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%20hX %20X %20lX\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%16hb %32b %40lb\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%020hd %020d %020ld\r\n"), hd,  d,  ld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%020hu %020u %020lu\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%020hX %020X %020lX\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%016hb %032b %040lb\r\n"), uhd, ud, uld);
	xputs(uart0_putc, sMessage);

//	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.0lq %.0lQ\r\n"), 0, lld, 0, ulld);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5hr\r\n"),  hr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5r\r\n"),   r);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5lr\r\n"),  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5hk\r\n"),  hk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5k\r\n"),   k);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5lk\r\n"),  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5hR\r\n"), uhr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5R\r\n"),  ur);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5lR\r\n"), ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5hK\r\n"), uhk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5K\r\n"),  uk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5lK\r\n"), ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hr %r %lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hR %R %lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hk %k %lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hK %K %lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10hr %10r %10lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10hR %10R %10lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10hk %10k %10lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10hK %10K %10lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10hr %-10r %-10lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10hR %-10R %-10lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10hk %-10k %-10lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10hK %-10K %-10lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10hr %+10r %+10lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10hR %+10R %+10lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10hk %+10k %+10lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10hK %+10K %+10lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10hr %-+10r %-+10lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10hR %-+10R %-+10lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10hk %-+10k %-+10lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10hK %-+10K %-+10lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10hr %- 10r %- 10lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10hR %- 10R %- 10lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10hk %- 10k %- 10lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10hK %- 10K %- 10lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nLeading zeros\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010hr %010r %010lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010hR %010R %010lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010hk %010k %010lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010hK %010K %010lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010hr %+010r %+010lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010hR %+010R %+010lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010hk %+010k %+010lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010hK %+010K %+010lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010hr % 010r % 010lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010hR % 010R % 010lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010hk % 010k % 010lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010hK % 010K % 010lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting [%%qQ]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hq %q %lq %lq\r\n"), 7, hk,  15, k,  31, lk,  47, llk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%hQ %Q %lQ %lQ\r\n"), 8, uhk, 16, uk, 32, ulk, 48, ullk);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%lq %lQ %lq %lQ\r\n"), 63, llr, 64, ullr, 47, llk, 48, ullk);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hr %.5r %.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hR %.5R %.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hk %.5k %.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hK %.5K %.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%10.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10.5hr %10.5r %10.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10.5hR %10.5R %10.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10.5hk %10.5k %10.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%10.5hK %10.5K %10.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%-10.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10.5hr %-10.5r %-10.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10.5hR %-10.5R %-10.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10.5hk %-10.5k %-10.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-10.5hK %-10.5K %-10.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%+10.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10.5hr %+10.5r %+10.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10.5hR %+10.5R %+10.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10.5hk %+10.5k %+10.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+10.5hK %+10.5K %+10.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%-+10.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10.5hr %-+10.5r %-+10.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10.5hR %-+10.5R %-+10.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10.5hk %-+10.5k %-+10.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%-+10.5hK %-+10.5K %-+10.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%- 10.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10.5hr %- 10.5r %- 10.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10.5hR %- 10.5R %- 10.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10.5hk %- 10.5k %- 10.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%- 10.5hK %- 10.5K %- 10.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%010.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010.5hr %010.5r %010.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010.5hR %010.5R %010.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010.5hk %010.5k %010.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%010.5hK %010.5K %010.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%+010.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010.5hr %+010.5r %+010.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010.5hR %+010.5R %+010.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010.5hk %+010.5k %+010.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%+010.5hK %+010.5K %+010.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%% 010.5]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010.5hr % 010.5r % 010.5lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010.5hR % 010.5R % 010.5lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010.5hk % 010.5k % 010.5lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("% 010.5hK % 010.5K % 010.5lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting accums [%%.5qQ]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hq %.5q %.5lq %.5lq\r\n"), 7, hk,  15, k,  31, lk,  47, llk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5hQ %.5Q %.5lQ %.5lQ\r\n"), 8, uhk, 16, uk, 32, ulk, 48, ullk);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs [%%.5qQ]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.5lq %.5lQ %.5lq %.5lQ\r\n"), 63, llr, 64, ullr, 47, llk, 48, ullk);
	xputs(uart0_putc, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nTesting fracs/accums [%%.0]\r\n"));
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.0hr %.0r %.0lr\r\n"),  hr,  r,  lr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.0hR %.0R %.0lR\r\n"), uhr, ur, ulr);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.0hk %.0k %.0lk\r\n"),  hk,  k,  lk);
	xputs(uart0_putc, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("%.0hK %.0K %.0lK\r\n"), uhk, uk, ulk);
	xputs(uart0_putc, sMessage);
	
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("\r\nEnd of test\r\n"));
	xputs(uart0_putc, sMessage);
	
	while (1)
		;
}

#ifdef ORIG_MAIN
int main(void)
{
//	char	sMessage[]	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890`-=~!@#$%^&*()+\r\n";
	char	sMessage[]	= "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
	int		iSize		= sizeof(sMessage)-1;
	int		q;
	int		byte;
	int		peek;
//	long	fp;
	
//	char	sFPASCII1[]	= "4294967295";
//	char	sFPASCII2[]	= "qwerty.987654321";
//	char	sFPASCII3[]	= "1024.5678";
//	char	sBuffer[40];
//	char*	ps;
	
// Debug on pin 4 and 5
//PORTD	|= _BV(PIND4) | _BV(PIND5) | _BV(PIND6);	// Set pins high
//DDRD	|= _BV(PIND4) | _BV(PIND5) | _BV(PIND6);	// Configure as output
//PORTB	|= _BV(PINB0) | _BV(PINB1) | _BV(PINB2) | _BV(PINB3) | _BV(PINB4) | _BV(PINB5);
//DDRB	|= _BV(PINB0) | _BV(PINB1) | _BV(PINB2) | _BV(PINB3) | _BV(PINB4) | _BV(PINB5);
//PORTB	= 0;

	sei();
	
//	unsigned _Accum fq;
//	unsigned _Accum fp = 1024.3;
//	int	sz = sizeof(fp);
//	ps = sFPASCII3;
	
//	xatoq(&ps, &fq, 16, 1);
//	xsprintf_P(sBuffer, PSTR("%lr\r\n"), 16, fq);
//	xsprintf_P(sBuffer, PSTR("%lr\r\n"), 16, fp);
//	fq += fp;
//	xsprintf_P(sBuffer, PSTR("%lr\r\n"), 16, fq);
//	xsprintf_P(sBuffer, PSTR("%d\r\n"), sz);
/*
	unsigned long y;
	for (unsigned long x = 0xF0000000; x < 0xFFFFFFFF; x += 0x00010001)
	{
		ps = sBuffer;
		xsprintf_P(sBuffer, PSTR("%ld\r\n"), x);
		xatoi(&ps, &y);
		
		//if (abs(x - y) > 1)
		if (x != y)
		{
			xsprintf_P(sBuffer, PSTR("Error x = %08lX\r\n"), x);
		}
	}

	ps = sFPASCII1;
	xatoq(&ps, &fp, 0, 1);
//	ps = sFPASCII2;
//	xatoq(&ps, &fp, 16, 0);
//	ps = sFPASCII3;
//	xatoq(&ps, &fp, 16, 0);

//	xatoi(&ps, &fp);

	unsigned long v;
	for (unsigned long u = 0x10001000; u < 0xFFFFFFFF; u += 0x00010001)
	{
		ps = sBuffer;
		xsprintf_P(sBuffer, PSTR("%.9lr\r\n"), 32, u);
		xatoq(&ps, &v, 32, 1);
		
		if (abs(u - v) > 4)
//		if (u != v)
		{
			xsprintf_P(sBuffer, PSTR("Error u = %08lX\r\n"), u);
		}
	}
*/
//	T0SerialBegin(14400);
//	T1SerialBegin(14400);
	T2SerialBegin(38400);

#define SerialWriteByte T2BSerialWrite
#define tsconfigRX

#if defined(SerialWriteByte)

	xdev_out(SerialWriteByte);

	char c = 0xFF;

//	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %30.5lq\r\n\r\n"), 0x00118100Ul);
//	xputs(SerialWriteByte, sMessage);

	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Test message. Buffer length %d.\r\n\r\n"), sizeof(sMessage));
	xputs(SerialWriteByte, sMessage);
	xsnprintf_P(sMessage, 10, PSTR("Another test message but buffer is too small.\r\n"));
	xprintf_P(PSTR("Buffer too small: [%%s]\r\n\r\n"), sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5k\r\n\r\n"), 0x00118100Ul);
	xputs(SerialWriteByte, sMessage);
	xsnprintf_P(sMessage, sizeof(sMessage), PSTR("Printed to string buffer: %.5lk\r\n\r\n"), 0x0000011180000000UlL);
	xputs(SerialWriteByte, sMessage);
	xsnprintf_P(sMessage, 30, PSTR("Printed to string buffer: %.5k\r\n\r\n"), 0x00118100Ul);
	xprintf_P(PSTR("Buffer too small: [%%s]\r\n\r\n"), sMessage);
	xsnprintf_P(sMessage, 30, PSTR("Printed to string buffer: %.5lk\r\n\r\n"), 0x0000011180000000UlL);
	xprintf_P(PSTR("Buffer too small: [%%s]\r\n\r\n"), sMessage);
	xsnprintf_P(sMessage, 0, PSTR("Printed to string buffer: %.5lk\r\n\r\n"), 0x0000011180000000UlL);
	xprintf_P(PSTR("Buffer too small: [%%s]\r\n\r\n"), sMessage);

	xprintf_P(PSTR("lib_xio test.\r\nUsing xprintf_P to print these first two lines.\r\n"));
	
	xsprintf_P(sMessage, PSTR("Printed to string buffer: %.5lk\r\n\r\n"), 0x0000001180000000UlL);
	xputs(SerialWriteByte, sMessage);

	xprintf_P(PSTR("[%lk]\r\n\r\n"), 0x0000001180000000UlL);
	
	xlqtoa(SerialWriteByte, 4UlL, 64, 32, xioFLAG_UNSIGNED, 60);
	xprintf_P(PSTR("\r\n"));
	xlqtoa(SerialWriteByte, 0xFFFFFFFE80000000UlL, 32, 60, 0, 15);
	xprintf_P(PSTR("\r\n"));
	xlqtoa(SerialWriteByte, 0x0000001180000000UlL, 32, 32, xioFLAG_UNSIGNED, 9);
	xprintf_P(PSTR("\r\n"));
	xlqtoa(SerialWriteByte, 0x8000000000000000UlL, 0, 32, xioFLAG_UNSIGNED, 9);
	xprintf_P(PSTR("\r\n"));
	xqtoa(SerialWriteByte, 0xFFFFFFFF, 0, 32, xioFLAG_UNSIGNED, 9);
	xprintf_P(PSTR("\r\n"));
	xlqtoa(SerialWriteByte, 0xFFFFFFFFFFFFFFFFUlL, 0, 32, xioFLAG_UNSIGNED, 9);
	xprintf_P(PSTR("\r\n"));

	unsigned long ii = 0x00038100Ul;
	int jj;
	for (jj = 0; jj < 33; jj++ )
	{
		xprintf_P(PSTR("%2d:\r\n"), jj);
		xqtoa(SerialWriteByte, ii, jj, 30, 0, 20);
		xprintf_P(PSTR("\r\n"));
	}

	unsigned long long i;
	int j;

	xlqtoa(SerialWriteByte, 0x0000000000000000UlL, 0, 32, xioFLAG_UNSIGNED, 9);
	xprintf_P(PSTR("\r\n"));
	for (i = 1, j = 0; j < 64; i <<= 1, j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), j);
		xlqtoa(SerialWriteByte, i, 0, 32, xioFLAG_UNSIGNED, 9);
		xprintf_P(PSTR("\r\n"));
	}	

	xprintf_P(PSTR("\r\n"));
	for (i = 1, j = 0; j < 64; i = (i << 1) + 1, j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), j);
		xlqtoa(SerialWriteByte, i, 0, 32, xioFLAG_UNSIGNED, 9);
		xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\n    "));
	xlqtoa(SerialWriteByte, 0x0000000000000000UlL, 64, 32, xioFLAG_UNSIGNED, 60);
	xprintf_P(PSTR("\r\n"));
	for (i = 1, j = 0; j < 64; i <<= 1, j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), 64-j);
		xlqtoa(SerialWriteByte, i, 64, 32, xioFLAG_UNSIGNED, 60);
		xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\n    "));
	xlqtoa(SerialWriteByte, 0x0000000000000000UlL, 64, 32, xioFLAG_UNSIGNED, 18);
	xprintf_P(PSTR("\r\n"));
	for (i = 1, j = 0; j < 64; i = (i << 1) + 1, j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), 64-j);
		xlqtoa(SerialWriteByte, i, 64, 32, xioFLAG_UNSIGNED, 18);
		xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\nUsing string buffer for next output\r\n"));
	i = 0x7FFFFFFFFFFFFFFFUlL;
	for (j = 0; j < 65; j++ )
	{
		//xprintf_P(PSTR("%2d: \r\n"), j);
		//xlqtoa(SerialWriteByte, i, j, 60, xioFLAG_UNSIGNED, 30);
		xsprintf_P(sMessage, PSTR("%2d: %40.15lq\r\n\r\n"), j, j, i);
		xputs(SerialWriteByte, sMessage);
		//xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\n"));
	i = 0xFFFFFFFFFFFFFFFFUlL;
	for (j = 0; j < 65; j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), j);
		xlqtoa(SerialWriteByte, i, j, 60, xioFLAG_UNSIGNED, 15);
		xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\n"));
	i = 0x7FFFFFFFFFFFFFFFUlL;
	for (j = 0; j < 65; j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), j);
		xlqtoa(SerialWriteByte, i, j, 60, xioFLAG_UNSIGNED, 15);
		xprintf_P(PSTR("\r\n"));
	}

	xprintf_P(PSTR("\r\n"));
	i = 0xFFFFFFFFFFFFFFFFUlL;
	for (j = 0; j < 65; j++ )
	{
		xprintf_P(PSTR("%2d: \r\n"), j);
		xlqtoa(SerialWriteByte, i, j, 60, 0,15);
		xprintf_P(PSTR("\r\n"));
	}

/*
	for (int i = 0; i < iSize; i++)
	{
//		byte = sT2Serialread();
//		if (byte >= 0)
//			xputc(SerialWriteByte, byte);
		q = qdiv(int2q(100),int2q(i));
		xsprintf_P(sMessage, PSTR("[%02d] 100/i = % 8q; % 9.3q; % 10.4q; 0x%04X; %016b\r\n\r\n"), i, q, q, q, q, q);
//		xputs(T2ASerialWriteByte, sMessage);
		xputs(T2BSerialWrite, sMessage);
//		xputs(T1AAsciiSerialWrite, sMessage);
//		xputs(T1BSerialWriteByte, sMessage);
//		xputs(T0AAsciiSerialWrite, sMessage);
//		xputs(T0BSerialWriteByte, sMessage);
		//			xsprintf_P(sMessage, PSTR("[%02d] 100/i = % 8q\r\n\r\n"), i, q16div(int2q16(100),int2q16(i)));
		//			xfprintf_P(SerialWriteByte, PSTR("%s\r\n"), sMessage);
		//			SerialWriteByte(sMessage[i]);
//		_delay_ms(20);
	}
*/
#endif	// tsconfgTX

#if defined(tsconfigRX)

    while(1)
    {
/*
		byte = T0BAsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);
*/
/*
		byte = T0PD0AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD1AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD2AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD3AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD4AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD5AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);

		byte = T0PD7AsciiSerialread();
		if (byte >= 0)
			xputc(T0AAsciiSerialWrite, byte);
*/
		peek = T2ASerialAvailable();
//		xprintf_P(PSTR("Available: %X %d\r\n\r\n"), peek, peek);
		if (peek > 0)
		{
			peek = T2ASerialPeek();
			byte = T2ASerialread();
			if (peek != byte)
				xprintf_P(PSTR("Error: peek does not equal byte\r\n"));
			if (byte >= 0)
				xputc(T2BSerialWrite, byte);
		}
/*
		byte = T2PB2AsciiSerialread();
		if (byte >= 0)
			xputc(T2BAsciiSerialWrite, byte);

		byte = T2PB3AsciiSerialread();
		if (byte >= 0)
			xputc(T2BAsciiSerialWrite, byte);

		byte = T2PB4AsciiSerialread();
		if (byte >= 0)
			xputc(T2BAsciiSerialWrite, byte);

		byte = T2PB5AsciiSerialread();
		if (byte >= 0)
			xputc(T2BAsciiSerialWrite, byte);
*/
/*
		if (T0BSerialOverflow())
		{
			xprintf_P(PSTR("\r\nRX buffer overflow\r\n"));

			do 
			{
				byte = sT2Serialread();
				xprintf_P(PSTR("%02X: %04X\r\n"), (char)(byte & 0xFF), byte);
				if (byte >= 0)
				{
					xputs_P(SerialWriteByte, PSTR(": "));
					xputc(SerialWriteByte, byte);
					xputs_P(SerialWriteByte, PSTR("\r\n"));
				}
			} while (byte >= 0);
		}
*/
//		xprintf_P(PSTR("\r\nrx_byte = [%%02X]\r\n\r\n"), rx_byte);
//		_delay_ms(30);
    }

#endif	// tsconfigRX

	while (1)
	{
		
	}
}
#endif // ORIG_MAIN