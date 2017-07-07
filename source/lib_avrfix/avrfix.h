/****************************************************************
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

#ifndef _AVRFIX_H
#define _AVRFIX_H

#ifndef TEST_ON_PC
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif

/* Only two datatypes are used from the ISO/IEC standard:
 * short _Acc with s7.8 bit format
 *       _Acc with s15.16 bit format
 * long  _Acc with  s7.24 bit format
 */

typedef signed short _sAcc;
typedef signed long  _Acc;
typedef signed long  _lAcc;

/* Pragmas for defining overflow behavior */

#define DEFAULT    0
#define SAT        1

#ifndef FX_ACCUM_OVERFLOW
#define FX_ACCUM_OVERFLOW DEFAULT
#endif

/* Pragmas for internal use */

//#define FX_DEFAULT_FRACBITS		// Use default fractional bits
#define FX_GCC_FRACBITS				// Use GCC fractional bits

#ifdef FX_DEFAULT_FRACBITS
#define SACCUM_IBIT		7
#define SACCUM_FBIT		8
#define ACCUM_IBIT		15
#define ACCUM_FBIT		16
#define LACCUM_IBIT		7
#define LACCUM_FBIT		24
#endif

#ifdef FX_GCC_FRACBITS
#define SACCUM_IBIT		8
#define SACCUM_FBIT		7
#define ACCUM_IBIT		16
#define ACCUM_FBIT		15
#define LACCUM_IBIT		8
#define LACCUM_FBIT		23
#endif

#define SACCUM_MIN		-32767
#define SACCUM_MAX		32767
#define ACCUM_MIN		-2147483647L
#define ACCUM_MAX		2147483647L
#define LACCUM_MIN		-2147483647L
#define LACCUM_MAX		2147483647L

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

/*
#define PIsk			804
#define PIk				205887
#define PIlk			52707179

#define LOG2k			45426
#define LOG2lk			11629080

#define LOG10k			150902
#define LOG10lk			38630967
*/

#ifndef NULL
#define NULL ((void*)0)
#endif

/* conversion Functions */

#define itosk(i)		((_sAcc)(i) << SACCUM_FBIT)
#define itok(i)			((_Acc)(i)  << ACCUM_FBIT)
#define itolk(i)		((_lAcc)(i) << LACCUM_FBIT)

#define sktoi(k)		((int8_t)((k) >> SACCUM_FBIT))
#define ktoi(k)			((signed short)((k) >> ACCUM_FBIT))
#define lktoi(k)		((int8_t)((k) >> LACCUM_FBIT))

#define sktok(sk)		( (_Acc)(sk) << (ACCUM_FBIT-SACCUM_FBIT))
#define ktosk(k)		((_sAcc)((k) >> (ACCUM_FBIT-SACCUM_FBIT)))

#define sktolk(sk)		((_lAcc)(sk) << (LACCUM_FBIT-SACCUM_FBIT))
#define lktosk(lk)		((_sAcc)((lk) >> (LACCUM_FBIT-SACCUM_FBIT)))

#define ktolk(k)		((_Acc)(k) << (LACCUM_FBIT-ACCUM_FBIT))
#define lktok(lk)		((_lAcc)(lk) >> (LACCUM_FBIT-ACCUM_FBIT))

#define ftosk(f)		((_sAcc)((f)  * (1L << SACCUM_FBIT)))
#define ftok(f)			((_Acc)((f)  * (1L << ACCUM_FBIT)))
#define ftolk(f)		((_lAcc)((f) * (1L << LACCUM_FBIT)))

#define sktof(sk)		((float)((_sAcc)(sk) / (1L << SACCUM_FBIT)))
#define ktod(k)			((double)((_sAcc)(k) / (1L << SACCUM_FBIT)))
#define lktod(lk)		((double)((_sAcc)(lk) / (1L << SACCUM_FBIT)))

/* Main Functions */

extern _sAcc	smulskD(_sAcc, _sAcc);
extern _Acc		mulkD(_Acc, _Acc);
extern _lAcc	lmullkD(_lAcc, _lAcc);

extern _sAcc	sdivskD(_sAcc, _sAcc);
extern _Acc		divkD(_Acc, _Acc);
extern _lAcc	ldivlkD(_lAcc, _lAcc);

extern _sAcc	smulskS(_sAcc, _sAcc);
extern _Acc		mulkS(_Acc, _Acc);
extern _lAcc	lmullkS(_lAcc, _lAcc);

extern _sAcc	sdivskS(_sAcc, _sAcc);
extern _Acc		divkS(_Acc, _Acc);
extern _lAcc	ldivlkS(_lAcc, _lAcc);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define smulsk(a,b)	smulskD((a),(b))
  #define mulk(a,b)		mulkD((a),(b))
  #define lmullk(a,b)	lmullkD((a), (b))
  #define sdivsk(a,b)	sdivskD((a), (b))
  #define divk(a,b)		divkD((a), (b))
  #define ldivlk(a,b)	ldivlkD((a), (b))
#elif FX_ACCUM_OVERFLOW == SAT
  #define smulsk(a,b)	smulskS((a),(b))
  #define mulk(a,b)		mulkS((a),(b))
  #define lmullk(a,b)	lmullkS((a), (b))
  #define sdivsk(a,b)	sdivskS((a), (b))
  #define divk(a,b)		divkS((a), (b))
  #define ldivlk(a,b)	ldivlkS((a), (b))
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

extern _sAcc	roundskD(_sAcc f, uint8_t n);
extern _Acc		roundkD(_Acc f, uint8_t n);
extern _lAcc	roundlkD(_lAcc f, uint8_t n);

extern _sAcc	roundskS(_sAcc f, uint8_t n);
extern _Acc		roundkS(_Acc f, uint8_t n);
extern _lAcc	roundlkS(_lAcc f, uint8_t n);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define roundsk(f, n)		roundskD((f), (n))
  #define roundk(f, n)		roundkD((f), (n))
  #define roundlk(f, n)		roundlkD((f), (n))
#elif FX_ACCUM_OVERFLOW == SAT
  #define roundsk(f, n)		roundskS((f), (n))
  #define roundk(f, n)	roundkS((f), (n))
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

extern uint8_t			countlssk(_sAcc f);
extern uint8_t			countlsk(_Acc f);
#define countlslk(f)	countlsk((f))

/* Special Functions */

#define CORDICC_GAIN	ftolk(0.6072529350088812561694)		// 1.64676025812107^-1	Circular
#define CORDICH_GAIN	ftolk(1.207497067763)				// 0.8281593609602^-1	Hyperbolic
// Hyperbolic constant from slide 43 
// from: http://studylib.net/doc/14981388/part-vi-function-evaluation-parts-chapters
//#define CORDICH_GAIN	ftolk(1.20749742)					// Original approximation

//#define CORDICC_GAIN 10188012
//#define CORDICH_GAIN 20258445

extern _Acc sqrtk_uncorrected(_Acc,int8_t,uint8_t);

#define sqrtkD(a)   mulkD(sqrtk_uncorrected(a, -8, ACCUM_FBIT), CORDICH_GAIN/256)
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
  #define lsqrtlk(a)	 lsqrtlkS(a)
#endif

extern _Acc		sincosk(_Acc, _Acc*);
extern _lAcc	lsincoslk(_lAcc, _lAcc*);
extern _lAcc	lsincosk(_Acc, _lAcc*);
extern _sAcc	ssincossk(_sAcc, _sAcc*);

#define sink(a)   sincosk((a), NULL)
#define lsinlk(a) lsincoslk((a), NULL)
#define lsink(a)  lsincosk((a), NULL)
#define ssinsk(a) ssincossk((a), NULL)

#define cosk(a)   sink((a) + PIk/2 + 1)
#define lcoslk(a) lsinlk((a) + PIlk/2)
#define lcosk(a)  lsink((a) + PIk/2 + 1)
#define scossk(a) ssinsk((a) + PIsk/2)

extern _Acc		tankD(_Acc);
extern _lAcc	ltanlkD(_lAcc);
extern _lAcc	ltankD(_Acc);

extern _Acc		tankS(_Acc);
extern _lAcc	ltanlkS(_lAcc);
extern _lAcc	ltankS(_Acc);

#if FX_ACCUM_OVERFLOW == DEFAULT
  #define tank(a)	tankD((a))
  #define ltanlk(a) ltanlkD((a))
  #define ltank(a)	ltankD((a))
#elif FX_ACCUM_OVERFLOW == SAT
  #define tank(a)	tankS((a))
  #define ltanlk(a) ltanlkS((a))
  #define ltank(a)	ltankS((a))
#endif

extern _Acc			atan2k(_Acc, _Acc);
extern _lAcc		latan2lk(_lAcc, _lAcc);

#define atank(a)	atan2k(itok(1), (a))
#define latanlk(a)	latan2lk(itolk(1), (a))

extern _Acc				logk(_Acc);
extern _lAcc			lloglk(_lAcc);

#define log2k(x)		(divk(logk((x)), LOG2k))
#define log10k(x)		(divk(logk((x)), LOG10k))
#define logak(a, x)		(divk(logk((x)), logk((a))))

#define llog2lk(x)		(ldivlk(lloglk((x)), LOG2lk))
#define llog10lk(x)		(ldivlk(lloglk((x)), LOG10lk))
#define llogalk(a, x)	(ldivlk(lloglk((x)), lloglk((a))))

#endif /* _AVRFIX_H */
