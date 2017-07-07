/****************************************************************
 *                                                              *
 *         ____    ____    ____                                 *
 *       // ___| // __ \ // __ \                                *
 *      // / ___// /  \_// /  \_|_____                          *
 *      | | |__ | |    _| |  //_ _   /.                         *
 *      \\ \__| |\ \__/ \\ \_|/ |___/.                          *
 *       \______/\_____/.\_____/|__.___  ___                    *
 *        ......  ......  ...||  _/_\  \////.                   *
 *                           || |.| |\  ///.                    *
 *      					 |__|.|_|///  \                     *
 *                            .... ./__/\__\                    *
 *                                   ........                   *
 * Fixed Point Library                                          *
 *                                                              *
 * Version 1.0                                                  *
 * David Harmon 2017-03-09                                      *
 *                                                              *
 * Based on:                                                    *
 *                                                              *
 *          __ ____  _________                                  *
 *         /_ \\\  \/   /|    \______                           *
 *        //   \\\     /|| D  /_    /.                          *
 *       //     \\\_  /.||    \ ___/.                           *
 *      /___/\___\\__/. |__|\__\__.___  ___                     *
 *       ....  .......   ...||  _/_\  \////.                    *
 *                          || |.| |\  ///.                     *
 *                          |__|.|_|///  \                      *
 *                           .... ./__/\__\                     *
 *                                  ........                    *
 * Fixed Point Library                                          *
 * according to                                                 *
 * ISO/IEC DTR 18037                                            *
 *                                                              *
 * Version 1.0.1                                                *
 * Maximilan Rosenblattl, Andreas Wolf 2007-02-07               *
 ****************************************************************/

#ifndef _GCCFIX_H
#define _GCCFIX_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* The following datatypes are supported:
 *       _sAccum with  s8.7  bit (2 bytes) format ~1.5 decimal points precision	short _Accum
 *       _Accum  with s16.15 bit (4 bytes) format ~4 decimal points precision	Builtin GCC datatype
 *       _lAccum with  s8.23 bit (4 bytes) format ~6 decimal points precision	signed long
 */

typedef short _Accum _sAccum;
typedef signed long  _lAccum;

/* Pragmas for defining overflow behavior */

#define DEFAULT    0
#define SAT        1

#ifndef FX_ACCUM_OVERFLOW
#define FX_ACCUM_OVERFLOW DEFAULT
#endif

/* Pragmas for internal use */

#define SACCUM_IBIT		8
#define SACCUM_FBIT		7
#define ACCUM_IBIT		16
#define ACCUM_FBIT		15
#define LACCUM_IBIT		8
#define LACCUM_FBIT		23

#define SACCUM_MIN		htohq(-32767)
#define SACCUM_MAX		htohq(32767)
#define ACCUM_MIN		ltoq(-2147483647L)
#define ACCUM_MAX		ltoq(2147483647L)
#define LACCUM_MIN		ltolq(-2147483647L)
#define LACCUM_MAX		ltolq(2147483647L)

#define SACCUM_FACTOR	((short)1 << SACCUM_FBIT)
#define ACCUM_FACTOR	((long)1 << ACCUM_FBIT)
#define LACCUM_FACTOR	((long)1 << LACCUM_FBIT)

/* Mathematical constants */

#define PIsk			ftosk(3.1415926535897932384626433832795)
#define PIk				ftok(3.1415926535897932384626433832795)
#define PIlk			ftolk(3.1415926535897932384626433832795)

#define LOG2k			ftok(0.69314718055994530941723212145818)
#define LOG2lk			ftolk(0.69314718055994530941723212145818)

#define LOG10k			ftok(2.3025850929940456840179914546844)
#define LOG10lk			ftolk(2.3025850929940456840179914546844)

#ifndef NULL
#define NULL ((void*)0)
#endif

/* conversion Functions */

#define itosk(i)		((_sAccum)(i))
#define itok(i)			((_Accum)(i))
#define itolk(i)		((_lAccum)(i) << LACCUM_FBIT)

#define sktoi(k)		((int8_t)(k))
#define ktoi(k)			((signed short)(k))
#define lktoi(k)		((int8_t)((k) >> LACCUM_FBIT))

#define sktok(sk)		((_Accum)(sk))
#define ktosk(k)		((_sAccum)(k))

#define sktolk(sk)		((_lAccum)(sk) << (LACCUM_FBIT-SACCUM_FBIT))
#define lktosk(lk)		((_sAccum)((lk) >> (LACCUM_FBIT-SACCUM_FBIT)))

#define ktolk(k)		((_Accum)(k) << (LACCUM_FBIT-ACCUM_FBIT))
#define lktok(lk)		((_lAccum)(lk) >> (LACCUM_FBIT-ACCUM_FBIT))

#define ftosk(f)		((_sAccum)(f))
#define ftok(f)			((_Accum)(f))
#define ftolk(f)		((_lAccum)((f) * (1L << LACCUM_FBIT)))

#define sktof(sk)		((float)((_sAccum)(sk) / (1L << SACCUM_FBIT)))
#define ktod(k)			((double)((_sAccum)(k) / (1L << SACCUM_FBIT)))
#define lktod(lk)		((double)((_sAccum)(lk) / (1L << SACCUM_FBIT)))

typedef union uhq_t
{
	_sAccum			hq;
	signed short	h;
//	unsigned long	ul;
} hq_t;

typedef union uq_t
{
	_Accum			q;
	_lAccum			lq;
	signed long		l;
	unsigned long	ul;
} q_t;

#define htohq(f)		(((hq_t)(f)).hq)
#define hqtoh(f)		(((hq_t)(f)).h)
#define qtolq(f)		(((q_t)(f)).lq)
#define qtol(f)			(((q_t)(f)).l)
#define lqtoq(f)		(((q_t)(f)).q)
#define lqtol(f)		(((q_t)(f)).l)
#define ltoq(f)			(((q_t)(f)).q)
#define ltolq(f)		(((q_t)(f)).lq)
#define pqtoplq(f)		((_lAccum*)(f))
#define pqtopl(f)		((signed long*)(f))
#define plqtopq(f)		((_Accum*)(f))
#define pltopq(f)		((_Accum*)(f))

/* Main Functions */

extern _sAccum	smulskD(_sAccum, _sAccum);
extern _Accum	mulkD(_Accum, _Accum);
extern _lAccum	lmullkD(_lAccum, _lAccum);

extern _sAccum	sdivskD(_sAccum, _sAccum);
extern _Accum	divkD(_Accum, _Accum);
extern _lAccum	ldivlkD(_lAccum, _lAccum);

extern _sAccum	smulskS(_sAccum, _sAccum);
extern _Accum	mulkS(_Accum, _Accum);
extern _lAccum	lmullkS(_lAccum, _lAccum);

extern _sAccum	sdivskS(_sAccum, _sAccum);
extern _Accum	divkS(_Accum, _Accum);
extern _lAccum	ldivlkS(_lAccum, _lAccum);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define smulsk(a,b)	smulskD((a),(b))
  #define mulk(a,b)		mulkD((a),(b))
  #define lmullk(a,b)	lmullkD((a),(b))
  #define sdivsk(a,b)	sdivskD((a),(b))
  #define divk(a,b)		divkD((a),(b))
  #define ldivlk(a,b)	ldivlkD((a),(b))
#elif FX_ACCUM_OVERFLOW == SAT
  #define smulsk(a,b)	smulskS((a),(b))
  #define mulk(a,b)		mulkS((a),(b))
  #define lmullk(a,b)	lmullkS((a),(b))
  #define sdivsk(a,b)	sdivskS((a),(b))
  #define divk(a,b)		divkS((a),(b))
  #define ldivlk(a,b)	ldivlkS((a),(b))
#endif

/* Support Functions */

#define mulikD(i,k)		ktoi((i) * (k))
#define mulilkD(i,lk)	lktoi((i) * (lk))

#define divikD(i,k)		ktoi(divkD(itok(i),(k)))
#define divilkD(i,lk)	lktoi(ldivlkD(itolk(i),(lk)))

#define kdiviD(a,b)		divkD(itok(a),itok(b))
#define lkdiviD(a,b)	ldivlkD(itolk(a),itolk(b))

#define idivkD(a,b)		ktoi(divkD((a),(b)))
#define idivlkD(a,b)	lktoi(ldivlkD((a),(b)))

#define mulikS(i,k)		ktoi(mulkS(itok(i),(k)))
#define mulilkS(i,lk)	lktoi(lmullkS(itolk(i),(lk)))

#define divikS(i,k)		ktoi(divkS(itok(i),(k)))
#define divilkS(i,lk)	lktoi(ldivlkS(itolk(i),(lk)))

#define kdiviS(a,b)		divkS(itok(a),itok(b))
#define lkdiviS(a,b)	ldivlkS(itolk(a),itolk(b))

#define idivkS(a,b)		ktoi(divkS((a),(b)))
#define idivlkS(a,b)	lktoi(ldivlkS((a),(b)))

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define mulik(a,b)	mulikD((a),(b))
  #define mulilk(a,b)	mulilkD((a),(b))
  #define divik(a,b)	divikD((a),(b))
  #define divilk(a,b)	divilkD((a),(b))
  #define kdivi(a,b)	kdiviD((a),(b))
  #define lkdivi(a,b)	lkdiviD((a),(b))
  #define idivk(a,b)	idivkD((a),(b))
  #define idivlk(a,b)	idivlkD((a),(b))
#elif FX_ACCUM_OVERFLOW == SAT
  #define mulik(a,b)	mulikS((a),(b))
  #define mulilk(a,b)	mulilkS((a),(b))
  #define divik(a,b)	divikS((a),(b))
  #define divilk(a,b)	divilkS((a),(b))
  #define kdivi(a,b)	kdiviS((a),(b))
  #define lkdivi(a,b)	lkdiviS((a),(b))
  #define idivk(a,b)	idivkS((a),(b))
  #define idivlk(a,b)	idivlkS((a),(b))
#endif

/* Abs Functions */

#define sabssk(f)	((f) < 0 ? (-(f)) : (f))
#define absk(f)		((f) < 0 ? (-(f)) : (f))
#define labslk(f)	((f) < 0 ? (-(f)) : (f))

/* Rounding Functions */

extern _sAccum	roundskD(_sAccum f, uint8_t n);
extern _Accum	roundkD(_Accum f, uint8_t n);
extern _lAccum	roundlkD(_lAccum f, uint8_t n);

extern _sAccum	roundskS(_sAccum f, uint8_t n);
extern _Accum	roundkS(_Accum f, uint8_t n);
extern _lAccum	roundlkS(_lAccum f, uint8_t n);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define roundsk(f, n)		roundskD((f), (n))
  #define roundk(f, n)		roundkD((f), (n))
  #define roundlk(f, n)		roundlkD((f), (n))
#elif FX_ACCUM_OVERFLOW == SAT
  #define roundsk(f, n)		roundskS((f), (n))
  #define roundk(f, n)		roundkS((f), (n))
  #define roundlk(f, n)		roundlkS((f), (n))
#endif

/* countls Functions */

/*
The integer return value of the above functions is defined as follows:
	-	if the value of the fixed-point argument is non-zero, the return value
		is the largest integer k for which  the expression a<<k does not overflow
	-	if the value of the fixed-point argument is zero, an integer value is
		returned that is at least as large as N-1, where N is the total number
		of (nonpadding) bits of the fixed-point type of the argument

Source: http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1005.pdf Page 16
*/

extern uint8_t			countlssk(_sAccum f);
extern uint8_t			countlsk(_Accum f);
#define countlslk(f)	countlsk((f))

/* Special Functions */

#define CORDICC_GAIN	ftolk(0.6072529350088812561694)		// 1.64676025812107^-1	Circular
#define CORDICH_GAIN	ftolk(1.207497067763)				// 0.8281593609602^-1	Hyperbolic
// Hyperbolic constant from slide 43 
// from: http://studylib.net/doc/14981388/part-vi-function-evaluation-parts-chapters
//#define CORDICH_GAIN	ftolk(1.20749742)					// Original approximation

//#define CORDICC_GAIN 10188012
//#define CORDICH_GAIN 20258445

extern _Accum sqrtk_uncorrected(_Accum,int8_t,uint8_t);

#define sqrtkD(a)   (sqrtk_uncorrected(a, -8, ACCUM_FBIT) * lqtoq(CORDICH_GAIN/256))
#define lsqrtlkD(a) lmullkD(sqrtk_uncorrected(a, 0, LACCUM_FBIT), CORDICH_GAIN*2)
//#define lsqrtlkD(a) lmullkD(sqrtk_uncorrected(a, 2, LACCUM_FBIT), CORDICH_GAIN)	// Not as accurate as above
//#define lsqrtlkD(a) lmullkD(sqrtk_uncorrected(a, 0, LACCUM_FBIT), CORDICH_GAIN)	// Original: Wrong result

#define sqrtkS(a)   mulkS(sqrtk_uncorrected(a, -8, ACCUM_FBIT), CORDICH_GAIN/256)
#define lsqrtlkS(a) lmullkS(sqrtk_uncorrected(a, 0, LACCUM_FBIT), CORDICH_GAIN*2)	// Need to test
//#define lsqrtlkS(a) lmullkS(sqrtk_uncorrected(a, 2, LACCUM_FBIT), CORDICH_GAIN)	// Need to test
//#define lsqrtlkS(a) lmullkS(sqrtk_uncorrected(a, 0, LACCUM_FBIT), CORDICH_GAIN)	// Original: Need to test

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define sqrtk(a)		sqrtkD(a)
  #define lsqrtlk(a)	lsqrtlkD(a)
#else
  #define sqrtk(a)		sqrtkS(a)
  #define lsqrtlk(a)	lsqrtlkS(a)
#endif

extern _Accum	sincosk(_Accum, _Accum*);
extern _lAccum	lsincoslk(_lAccum, _lAccum*);
extern _lAccum	lsincosk(_Accum, _lAccum*);
extern _sAccum	ssincossk(_sAccum, _sAccum*);

#define sink(a)   sincosk((a), NULL)
#define lsinlk(a) lsincoslk((a), NULL)
#define lsink(a)  lsincosk((a), NULL)
#define ssinsk(a) ssincossk((a), NULL)

#define cosk(a)   sink((a) + PIk/2 + ltoq(1L))
#define lcoslk(a) lsinlk((a) + PIlk/2)
#define lcosk(a)  lsink((a) + PIk/2 + ltoq(1L))
#define scossk(a) ssinsk((a) + PIsk/2)

extern _Accum	tankD(_Accum);
extern _lAccum	ltanlkD(_lAccum);
extern _lAccum	ltankD(_Accum);

extern _Accum	tankS(_Accum);
extern _lAccum	ltanlkS(_lAccum);
extern _lAccum	ltankS(_Accum);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define tank(a)	tankD((a))
  #define ltanlk(a) ltanlkD((a))
  #define ltank(a)	ltankD((a))
#elif FX_ACCUM_OVERFLOW == SAT
  #define tank(a)	tankS((a))
  #define ltanlk(a) ltanlkS((a))
  #define ltank(a)	ltankS((a))
#endif

extern _Accum		atan2k(_Accum, _Accum);
extern _lAccum		latan2lk(_lAccum, _lAccum);

#define atank(a)	atan2k(1K, (a))
#define latanlk(a)	latan2lk(qtolq(1K), (a))

extern _Accum			logk(_Accum);
extern _lAccum			lloglk(_lAccum);

#define log2k(x)		(divk(logk((x)), LOG2k))
#define log10k(x)		(divk(logk((x)), LOG10k))
#define logak(a, x)		(divk(logk((x)), logk((a))))

#define llog2lk(x)		(ldivlk(lloglk((x)), LOG2lk))
#define llog10lk(x)		(ldivlk(lloglk((x)), LOG10lk))
#define llogalk(a, x)	(ldivlk(lloglk((x)), lloglk((a))))

extern _Accum			expk(_Accum);
extern _Accum			powk(_Accum b, _Accum e);

#endif /* _GCCFIX_H */
