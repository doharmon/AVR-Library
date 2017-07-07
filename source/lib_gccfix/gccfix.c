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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "gccfix.h"
#include "gccfix_config.h"

#if BYTE_ORDER == BIG_ENDIAN
typedef struct 
{
   unsigned short ll;
   uint8_t lh;
   int8_t h;
} lAccum_container;
#else
typedef struct 
{
   int8_t h;
   uint8_t lh;
   unsigned short ll;
} lAccum_container;
#endif

#define us(x) ((unsigned short)(x))
#define ss(x) ((signed short)(x))
#define ul(x) ((unsigned long)(x))
#define sl(x) ((signed long)(x))

extern void cordicck(_Accum* x, _Accum* y, _Accum* z, uint8_t iterations, uint8_t mode);
extern void cordichk(_Accum* x, _Accum* y, _Accum* z, uint8_t iterations, uint8_t mode);
extern void cordiccsk(_sAccum* x, _sAccum* y, _sAccum* z, uint8_t mode);
extern void cordichsk(_sAccum* x, _sAccum* y, _sAccum* z, uint8_t mode);

#ifdef SMULSKD
_sAccum smulskD(_sAccum x, _sAccum y)
{
  return x*y;
}
#endif

#ifdef SMULSKS
_sAccum smulskS(_sAccum x, _sAccum y)
{
  long mul = RSHIFT_static(sl(x)*sl(y), SACCUM_FBIT);
  if(mul >= 0) 
  {
    if((mul & 0xFFFF8000) != 0)
      return SACCUM_MAX;
  } 
  else 
  {
    if((mul & 0xFFFF8000) != 0xFFFF8000)
      return SACCUM_MIN;
  }
  return sl(mul);
}
#endif

#ifdef MULKD
_Accum mulkD(_Accum x, _Accum y)
{
  return x*y;
}
#endif

#ifdef MULKS
_Accum mulkS(_Accum x, _Accum y)
{
  #if BYTE_ORDER == BIG_ENDIAN
    #  define LO 0
    #  define HI 1
  #else
    #  define LO 1
    #  define HI 0
  #endif
  unsigned short xs[2];
  unsigned short ys[2];
  unsigned long mul;
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  *((_Accum*)xs) = absk(x);
  *((_Accum*)ys) = absk(y);
  mul = ul(xs[HI]) * ul(ys[HI]);
  if(mul > 32767)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  mul =   LSHIFT_static(mul, ACCUM_FBIT)
        + ul(xs[HI])*ul(ys[LO])
        + ul(xs[LO])*ul(ys[HI])
        + RSHIFT_static(ul(xs[LO]*ys[LO]), ACCUM_FBIT);
  if(mul & 0x80000000)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  return (positive ? ltoq((long)mul) : ltoq(-(long)mul));
  #undef HI
  #undef LO
}
#endif

#ifdef LMULLKD
_lAccum lmullkD(_lAccum x, _lAccum y)
{
  lAccum_container *xc, *yc;
  xc = (lAccum_container*)&x;
  yc = (lAccum_container*)&y;
  return   sl(xc->h)*y + sl(yc->h)*(x&0x00FFFFFF)
         + ((ul(xc->lh)*ul(yc->lh))*256)
         + RSHIFT_static((ul(xc->lh)*ul(yc->ll) + ul(xc->ll)*ul(yc->lh)), 8)
         + (RSHIFT_static((ul(xc->lh)*ul(yc->ll) + ul(xc->ll)*ul(yc->lh)), 7)&1)
         + RSHIFT_static((ul(xc->ll)*ul(yc->ll)), LACCUM_FBIT);
}
#endif

#ifdef LMULLKS
_lAccum lmullkS(_lAccum x, _lAccum y)
{
  lAccum_container xc, yc;
  unsigned long mul;
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  x = labslk(x);
  y = labslk(y);
  *((_lAccum*)&xc) = x;
  *((_lAccum*)&yc) = y;
  mul = xc.h * yc.h;
  x &= 0x00FFFFFF;
  y &= 0x00FFFFFF;
  if(mul > 127)
     return (positive ? LACCUM_MAX : LACCUM_MIN);
  mul =   LSHIFT_static(mul, LACCUM_FBIT) + ul(xc.h)*y + ul(yc.h)*x +
        + (ul(xc.lh)*ul(yc.lh)*256)
        + RSHIFT_static((ul(xc.lh)*ul(yc.ll) + ul(xc.ll)*ul(yc.lh)), 8)
        + (RSHIFT_static((ul(xc.lh)*ul(yc.ll) + ul(xc.ll)*ul(yc.lh)), 7)&1)
        + RSHIFT_static((ul(xc.ll)*ul(yc.ll)), LACCUM_FBIT);
  if(mul & 0x80000000)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  return (positive ? ltolq((long)mul) : ltolq(-(long)mul));
}
#endif

#ifdef SDIVSKD
_sAccum sdivskD(_sAccum x, _sAccum y)
{
  return x/y;
}
#endif

#ifdef SDIVSKS
_sAccum sdivskS(_sAccum x, _sAccum y)
{
  long div;
  if(y == 0)
     return (x < 0 ? SACCUM_MIN : SACCUM_MAX);
  div = (sl(x) << SACCUM_FBIT) / y;
  if(div >= 0) 
  {
    if((div & 0xFFFF8000) != 0)
      return SACCUM_MAX;
  } 
  else 
  {
    if((div & 0xFFFF8000) != 0xFFFF8000)
      return SACCUM_MIN;
  }
  return ss(div);
}
#endif

#ifdef DIVKD
/* if y = 0, divkD will enter an endless loop */
_Accum divkD(_Accum x, _Accum y) 
{
	return x/y;
}
#endif

#ifdef DIVKS
_Accum divkS(_Accum x, _Accum y) 
{
  _Accum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  if(y == 0)
     return (x < 0 ? ACCUM_MIN : ACCUM_MAX);
  x = absk(x);
  y = absk(y);

  for (i=0 ; i<ACCUM_FBIT ; i++)
  {
    if (x >= ACCUM_MAX / 2) break;
    x = ltoq(LSHIFT_static(qtol(x), 1));
  }

  while((y & 1) == 0) 
  {
    y = RSHIFT_static(y, 1);
    j++;
  }

  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (ACCUM_FBIT - i) - j;
  if(i > 0)
     for(;i>0;i--) 
	 {
       if((result & 0x40000000) != 0) 
	   {
         return sign ? ACCUM_MAX : ACCUM_MIN;
       }
       result = LSHIFT_static(result, 1);
     }
  else if(i < 0) 
  {
     /* shift right except for 1 bit, which will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* round */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif

#ifdef LDIVLKD
/* if y = 0, ldivlkD will enter an endless loop */
_lAccum ldivlkD(_lAccum x, _lAccum y) 
{
  _lAccum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (x > 0 && y > 0)) ? 1 : 0;
  x = labslk(x);
  y = labslk(y);
  /* Align x leftmost to get maximum precision */

  for (i=0 ; i<LACCUM_FBIT ; i++)
  {
    if (x >= LACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }
  while((y & 1) == 0) 
  {
    y = RSHIFT_static(y, 1);
    j++;
  }
  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (LACCUM_FBIT - i) - j;
  if(i > 0)
     result = LSHIFT_dynamic(result, i);
  else if(i < 0) 
  {
     /* shift right except for 1 bit, which will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* determine if round is necessary */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif

#ifdef LDIVLKS
_lAccum ldivlkS(_lAccum x, _lAccum y) 
{
  _lAccum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  if(y == 0)
     return (x < 0 ? LACCUM_MIN : LACCUM_MAX);
  x = labslk(x);
  y = labslk(y);

  for (i=0 ; i<LACCUM_FBIT ; i++)
  {
    if (x >= LACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }

  while((y & 1) == 0) 
  {
    y = RSHIFT_static(y, 1);
    j++;
  }

  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (LACCUM_FBIT - i) - j;
  if(i > 0)
     for(;i>0;i--) 
	 {
       if((result & 0x40000000) != 0) 
	   {
         return sign ? LACCUM_MAX : LACCUM_MIN;
       }
       result = LSHIFT_static(result, 1);
     }
  else if(i < 0) 
  {
     /* shift right except for 1 bit, which will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* round */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif

#ifdef SINCOSK
_Accum sincosk(_Accum kAngle, _Accum* pkCosp)
{
  signed long	angle		= qtol(kAngle);
  signed long*	cosp		= pqtopl(pkCosp);
  signed long	x;
  signed long 	y 			= 0;
  uint8_t 	correctionCount = 0;
  uint8_t 	quadrant 		= 1;

  if(cosp == NULL)
     cosp = &x;

  /* move large values into [0,2 PI] */
  #define MAX_CORRECTION_COUNT 1
  while(angle >= 2*qtol(PIk))
  {
    angle -= 2*qtol(PIk);
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle++;
    } 
	else 
	{
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) 
  {
    angle += 2*qtol(PIk);
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle--;
    } 
	else 
	{
      correctionCount++;
    }
  }
  #undef MAX_CORRECTION_COUNT

  /* move small values into [0,2 PI] */
  #define MAX_CORRECTION_COUNT 5
  while(angle >= 2*qtol(PIk) + 1) 
  {
    angle -= 2*qtol(PIk) + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle++;
    } 
	else 
	{
      correctionCount++;
    }
  }
  if(correctionCount > 0) 
  {
    angle++;
  }
  correctionCount = 0;
  while(angle < 0) 
  {
    angle += 2*qtol(PIk) + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle--;
    } 
	else 
	{
      correctionCount++;
    }
  }
  if(correctionCount > 0) 
  {
    angle--;
  }
  #undef MAX_CORRECTION_COUNT

  if(angle > qtol(PIk)) 
  {
    angle = angle - qtol(PIk);
    quadrant += 2;
  }
  if(angle > (qtol(PIk)/2 + 1)) 
  {
    angle = qtol(PIk) - angle + 1;
    quadrant += 1;
  }
  if(angle == 0) 
  {
    *cosp = (quadrant == 2 || quadrant == 3 ? -1K : 1K);
    return 0;
  }
  *cosp = CORDICC_GAIN;
  angle = LSHIFT_static(angle, 8);
  cordicck(pltopq(cosp), pltopq(&y), pltopq(&angle), ACCUM_FBIT, 0);
  (*cosp) = RSHIFT_static(*cosp, 8);
  y       = RSHIFT_static(y, 8);
  switch(quadrant) 
  {
    case 2: 
      (*cosp) = -(*cosp);
      break;
    case 3:
      y 	  = -y;
      (*cosp) = -(*cosp);
      break;
    case 4:
      y 	  = -y;
      break;
    default:
	  break;
  }
  return ltoq(y);
}
#endif

#ifdef LSINCOSLK
_lAccum lsincoslk(_lAccum angle, _lAccum* cosp)
{
  _lAccum x;
  _lAccum y 				= 0;
  uint8_t correctionCount;
  uint8_t quadrant 			= 1;

  if(cosp == NULL)
     cosp = &x;

  /* move values into [0, 2 PI] */
  #define MAX_CORRECTION_COUNT 1
  correctionCount = 0;
  while(angle >= 2*PIlk) 
  {
    angle -= 2*PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle++;
    } 
	else 
	{
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) 
  {
    angle += 2*PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle--;
    } 
	else 
	{
      correctionCount++;
    }
  }
  #undef MAX_CORRECTION_COUNT

  if(angle > PIlk) 
  {
    angle = angle - PIlk;
    quadrant += 2;
  }
  if(angle > (PIlk/2)) 
  {
    angle = PIlk - angle;
    quadrant += 1;
  }
  if(angle == 0) 
  {
    *cosp = (quadrant == 2 || quadrant == 3 ? -itolk(1) : itolk(1));
    return 0;
  }
  *cosp = CORDICC_GAIN;
  cordicck(plqtopq(cosp), plqtopq(&y), plqtopq(&angle), LACCUM_FBIT, 0);
  switch(quadrant) 
  {
    case 2: 
      (*cosp) = -(*cosp);
      break;
    case 3: 
      y 	  = -y;
      (*cosp) = -(*cosp);
      break;
    case 4: 
      y 	  = -y;
      break;
    default:
	  break;
  }
  return y;
}
#endif

#ifdef LSINCOSK
_lAccum lsincosk(_Accum angle, _lAccum* cosp)
{
  uint8_t correctionCount = 0;
  /* move large values into [0,2 PI] */
  #warning "May need to change to use 2*PIk instead of PIlk as in sincosk"
  #define MAX_CORRECTION_COUNT 1
  while(angle >= PIlk) 
  { /* PIlk = PIk * 2^8 */
    angle -= PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle++;
    } 
	else 
	{
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) 
  {
    angle += PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle--;
    } 
	else 
	{
      correctionCount++;
    }
  }
  #undef MAX_CORRECTION_COUNT

  /* move small values into [0,2 PI] */
  #define MAX_CORRECTION_COUNT 5
  while(angle >= 2*PIk + 1) 
  {
    angle -= 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle++;
    } 
	else 
	{
      correctionCount++;
    }
  }
  if(correctionCount > 0) 
  {
    angle++;
  }
  correctionCount = 0;
  while(angle < 0) 
  {
    angle += 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) 
	{
      correctionCount = 0;
      angle--;
    } 
	else 
	{
      correctionCount++;
    }
  }
  if(correctionCount > 0) 
  {
    angle--;
  }
  #undef MAX_CORRECTION_COUNT
  return lsincoslk(LSHIFT_static(angle, (LACCUM_FBIT - ACCUM_FBIT)), cosp);
}
#endif

#ifdef ROUNDSKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_sAccum roundskD(_sAccum hk, uint8_t n)
{
  signed short h = hqtoh(hk);

   n = SACCUM_FBIT - n;
   if(h >= 0) 
   {
      return (h & (0xFFFF << n)) + ((h & (1 << (n-1))) << 1);
   } 
   else 
   {
      return (h & (0xFFFF << n)) - ((h & (1 << (n-1))) << 1);
   }
}
#endif

#ifdef ROUNDKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Accum roundkD(_Accum k, uint8_t n)
{
  signed long l = qtol(k);

   n = ACCUM_FBIT - n;
   if(l >= 0) 
   {
      return (l & (0xFFFFFFFF << n)) + ((l & (1 << (n-1))) << 1);
   } 
   else 
   {
      return (l & (0xFFFFFFFF << n)) - ((l & (1 << (n-1))) << 1);
   }
}
#endif

#ifdef ROUNDSKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_sAccum roundskS(_sAccum f, uint8_t n)
{
   if(n > SACCUM_FBIT) 
   {
      return 0;
   }
   return roundskD(f, n);
}
#endif

#ifdef ROUNDKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Accum roundkS(_Accum f, uint8_t n)
{
   if(n > ACCUM_FBIT) 
   {
      return 0;
   }
   return roundkD(f, n);
}
#endif

#ifdef ROUNDLKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_lAccum roundlkD(_lAccum f, uint8_t n)
{
   n = LACCUM_FBIT - n;
   if(f >= 0) 
   {
      return (f & (0xFFFFFFFF << n)) + ((f & (1 << (n-1))) << 1);
   } 
   else 
   {
      return (f & (0xFFFFFFFF << n)) - ((f & (1 << (n-1))) << 1);
   }
}
#endif

#ifdef ROUNDLKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_lAccum roundlkS(_lAccum f, uint8_t n)
{
   if(n > LACCUM_FBIT) 
   {
      return 0;
   }
   return roundlkD(f, n);
}
#endif

#ifdef COUNTLSSK
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
uint8_t countlssk(_sAccum f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 2;
   for(i = 0; i < 15; i++) 
   {
      if((*pf & 0x40) != 0)
         break;
      f = LSHIFT_static(hqtoh(f), 1);
   }
   return i;
}
#endif

#ifdef COUNTLSK
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
uint8_t countlsk(_Accum f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 3;
   for(i = 0; i < 31; i++) 
   {
      if((*pf & 0x40) != 0)
         break;
      f = ltoq(LSHIFT_static(qtol(f), 1));
   }
   return i;
}
#endif

#ifdef TANKD
_Accum tankD(_Accum angle)
{
  _Accum sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= ltoq(2L))
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkD(sin, cos);
}
#endif

#ifdef TANKS
_Accum tankS(_Accum angle)
{
  _Accum sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkS(sin, cos);
}
#endif

#ifdef LTANLKD
_lAccum ltanlkD(_lAccum angle)
{
  _lAccum sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= ltolq(2L))
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkD(sin, cos);
}
#endif

#ifdef LTANLKS
_lAccum ltanlkS(_lAccum angle)
{
  _lAccum sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif

#ifdef LTANKD
_lAccum ltankD(_Accum angle)
{
  _lAccum sin, cos;
  sin = lsincosk(angle, &cos);
  return ldivlkD(sin, cos);
}
#endif

#ifdef LTANKS
_lAccum ltankS(_Accum angle)
{
  _lAccum sin, cos;
  sin = lsincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif

#ifdef ATAN2K
_Accum atan2kInternal(_Accum x, _Accum y)
{
  _Accum 	z 	= 0;
  uint8_t 	i 	= 0;
  uint8_t* 	px	= ((uint8_t*)&x) + 3, *py = ((uint8_t*)&y) + 3;

  for(;!(*px & 0x60) && !(*py & 0x60) && i < 8;i++) 
  {
    x = ltoq(LSHIFT_static(qtol(x), 1));
    y = ltoq(LSHIFT_static(qtol(y), 1));
  }
  if(i > 0) 
  {
    cordicck(&x, &y, &z, ACCUM_FBIT, 1);
    return ltoq(RSHIFT_static(qtol(z), 8));
  } 
  else 
  {
    return PIk/2 - divkD(x, y) - ltoq(1L);
  }
}

_Accum atan2k(_Accum x, _Accum y)
{
  uint8_t signX, signY;
  if(y == 0)
     return 0;
  signY = (y < 0 ? 0 : 1);
  if(x == 0)
     return (signY ? ACCUM_MAX : ACCUM_MIN);
  signX = (x < 0 ? 0 : 1);
  x = atan2kInternal(absk(x), absk(y));
  if(signY) 
  {
    if(signX) 
	{
      return x;
    } 
	else 
	{
      return x + PIk/2 + ltoq(1L);
    }
  } 
  else 
  {
    if(signX) 
	{
      return -x;
    } 
	else 
	{
      return -x - PIk/2 - 1;
    }
  }
}
#endif

#ifdef LATAN2LK
_lAccum latan2lk(_lAccum x, _lAccum y)
{
  uint8_t 	signX, signY;
  _Accum 	z	= 0;
  uint8_t*	px	= ((uint8_t*)&x) + 3, *py = ((uint8_t*)&y) + 3;

  if(y == 0)
     return 0;
  signY = (y < 0 ? 0 : 1);
  if(x == 0)
     return (signY ? ACCUM_MAX : ACCUM_MIN);
  signX = (x < 0 ? 0 : 1);
  if(!signX)
     x = -x;
  if(!signY)
     y = -y;
  if((*px & 0x40) || (*py & 0x40)) 
  {
    x = ltolq(RSHIFT_static(lqtol(x), 1));
    y = ltolq(RSHIFT_static(lqtol(y), 1));
  }
  cordicck(plqtopq(&x), plqtopq(&y), &z, LACCUM_FBIT, 1);
  if(signY) 
  {
    if(signX) 
	{
      return z;
    } 
	else 
	{
      return z+PIlk/2;
    }
  } 
  else 
  {
    if(signX) 
	{
      return -z;
    } 
	else 
	{
      return -z-PIlk/2;
    }
  }
}
#endif

#ifdef CORDICCK
/*
 * calculates the circular CORDIC method in both modes
 * mode = 0:
 * Calculates sine and cosine with input z and output x and y. To be exact
 * x has to be CORDIC_GAIN instead of itok(1) and y has to be 0.
 *
 * mode = 1:
 * Calculates the arctangent of y/x with output z. No correction has to be
 * done here.
 *
 * iterations is the fractal bit count (16 for _Accum, 24 for _lAccum)
 * and now the only variable, the execution time depends on.
 */
/*
const __flash unsigned long arctan[25] = 
{	
	ftolk(0.78539816339744830961566084581988),		// arctan(2^0)
	ftolk(0.46364760900080611621425623146121),		// arctan(2^-1)
	ftolk(0.24497866312686415417208248121128),		// arctan(2^-2)
	ftolk(0.12435499454676143503135484916387), 		// arctan(2^-3)
	ftolk(0.06241880999595734847397911298551), 		// arctan(2^-4)
	ftolk(0.03123983343026827625371174489249), 		// arctan(2^-5)
	ftolk(0.01562372862047683080280152125657), 		// arctan(2^-6)
	ftolk(0.0078123410601011112964633918422), 		// arctan(2^-7)
	ftolk(0.00390623013196697182762866531142), 		// arctan(2^-8)
	ftolk(0.00195312251647881868512148262508), 		// arctan(2^-9)
	ftolk(9.7656218955931943040343019971729e-4),	// arctan(2^-10)
	ftolk(4.8828121119489827546923962564485e-4),	// arctan(2^-11)
	ftolk(2.4414062014936176401672294325966e-4),	// arctan(2^-12)
	ftolk(1.2207031189367020423905864611796e-4),	// arctan(2^-13)
	ftolk(6.1035156174208775021662569173829e-5),	// arctan(2^-14)
	ftolk(3.0517578115526096861825953438536e-5),	// arctan(2^-15)
	ftolk(1.5258789061315762107231935812698e-5),	// arctan(2^-16)
	ftolk(7.6293945311019702633884823401051e-6),	// arctan(2^-17)
	ftolk(3.814697265606496282923075616373e-6),		// arctan(2^-18)
	ftolk(1.9073486328101870353653693059172e-6),	// arctan(2^-19)
	ftolk(9.5367431640596087942067068992311e-7),	// arctan(2^-20)
	ftolk(4.7683715820308885992758382144925e-7),	// arctan(2^-21)
	ftolk(2.3841857910155798249094797721893e-7),	// arctan(2^-22)
	ftolk(1.1920928955078068531136849713792e-7),	// arctan(2^-23)
  	ftolk(5.9604644775390554413921062141789e-8)		// arctan(2^-24)
};
*/
const __flash unsigned long arctan[25] =
{
	0x006487ED,         // arctan(2^-00) = 0.7853981634
	0x003B58CE,         // arctan(2^-01) = 0.4636476090
	0x001F5B76,         // arctan(2^-02) = 0.2449786631
	0x000FEADD,         // arctan(2^-03) = 0.1243549945
	0x0007FD57,         // arctan(2^-04) = 0.0624188100
	0x0003FFAB,         // arctan(2^-05) = 0.0312398334
	0x0001FFF5,         // arctan(2^-06) = 0.0156237286
	0x0000FFFF,         // arctan(2^-07) = 0.0078123411
	0x00008000,         // arctan(2^-08) = 0.0039062301
	0x00004000,         // arctan(2^-09) = 0.0019531225
	0x00002000,         // arctan(2^-10) = 0.0009765622
	0x00001000,         // arctan(2^-11) = 0.0004882812
	0x00000800,         // arctan(2^-12) = 0.0002441406
	0x00000400,         // arctan(2^-13) = 0.0001220703
	0x00000200,         // arctan(2^-14) = 0.0000610352
	0x00000100,         // arctan(2^-15) = 0.0000305176
	0x00000080,         // arctan(2^-16) = 0.0000152588
	0x00000040,         // arctan(2^-17) = 0.0000076294
	0x00000020,         // arctan(2^-18) = 0.0000038147
	0x00000010,         // arctan(2^-19) = 0.0000019073
	0x00000008,         // arctan(2^-20) = 0.0000009537
	0x00000004,         // arctan(2^-21) = 0.0000004768
	0x00000002,         // arctan(2^-22) = 0.0000002384
	0x00000001,         // arctan(2^-23) = 0.0000001192
	0x00000000          // arctan(2^-24) = 0.0000000596
};

void cordicck(_Accum* px, _Accum* py, _Accum* pz, uint8_t iterations, uint8_t mode)
{
  register uint8_t i;
  signed long x, y, z, xH;
  x = qtol(*px);
  y = qtol(*py);
  z = qtol(*pz);
  for (i = 0; i <= iterations; i++) 
  {
    xH = x;
    if((mode && y <= 0) || (!mode && z >= 0)) 
	{
      x -= RSHIFT_dynamic(y, i);
      y += RSHIFT_dynamic(xH, i);
      z -= arctan[i];
    }
    else 
	{
      x += RSHIFT_dynamic(y, i);
      y -= RSHIFT_dynamic(xH, i);
      z += arctan[i];
    }
  }
  *px = ltoq(x);
  *py = ltoq(y);
  *pz = ltoq(z);
}
#endif

#ifdef CORDICHK
/*
 * calculates the hyperbolic CORDIC method in both modes
 * mode = 0:
 * Calculates hyperbolic sine and cosine with input z and output x and y.
 * To be exact x has to be CORDICH_GAIN instead of itok(1) and y has to be 0.
 * This mode is never used in this library because of target limitations.
 *
 * mode = 1:
 * Calculates the hyperbolic arctangent of y/x with output z. No correction
 * has to be done here.
 *
 * iterations is the fractal bit count (16 for _Accum, 24 for _lAccum)
 */
/*
const __flash unsigned long arctanh[24] = 
{
	ftolk(0.54930614433405484569762261846126),		// arctanh(2^-1)
	ftolk(0.25541281188299534160275704815183),		// arctanh(2^-2)
	ftolk(0.12565721414045303884256886520094), 		// arctanh(2^-3)
	ftolk(0.06258157147700300712676502386221), 		// arctanh(2^-4)
	ftolk(0.03126017849066699476401224517265), 		// arctanh(2^-5)
	ftolk(0.01562627175205221137920177875164), 		// arctanh(2^-6)
	ftolk(0.00781265895154042091032347127604), 		// arctanh(2^-7)
	ftolk(0.00390626986839682605312756336971), 		// arctanh(2^-8)
	ftolk(0.00195312748353254999865077088685),		// arctanh(2^-9)
	ftolk(9.7656281044103584096445002988533e-4),	// arctanh(2^-10)
	ftolk(4.8828128880511282676100662627116e-4),	// arctanh(2^-11)
	ftolk(2.4414062985063858292797225210244e-4),	// arctanh(2^-12)
	ftolk(1.2207031310632980660296307873709e-4),	// arctanh(2^-13)
	ftolk(6.1035156325791225317150609727891e-5),	// arctanh(2^-14)
	ftolk(3.0517578134473903148761958402143e-5),	// arctanh(2^-15)
	ftolk(1.5258789063684237893098936432323e-5),	// arctanh(2^-16)
	ftolk(7.6293945313980297366218574175518e-6),	// arctanh(2^-17)
	ftolk(3.8146972656435037170772475010538e-6),	// arctanh(2^-18)
	ftolk(1.9073486328148129646346407915023e-6),	// arctanh(2^-19)
	ftolk(9.5367431640653912057932962562125e-7),	// arctanh(2^-20)
	ftolk(4.7683715820316114007241618841151e-7),	// arctanh(2^-21)
	ftolk(2.3841857910156701750905202308922e-7),	// arctanh(2^-22)
	ftolk(1.1920928955078181468863150287171e-7),	// arctanh(2^-23)
	ftolk(5.9604644775390695586078937858512e-8)		// arctanh(2^-24)
};
*/
const __flash unsigned long arctanh[24] =
{
	0x00464FAA,         // arctanh(2^-01) = 0.54930614433405484569762261846126
	0x0020B15E,         // arctanh(2^-02) = 0.25541281188299534160275704815183
	0x00101589,         // arctanh(2^-03) = 0.12565721414045303884256886520094
	0x000802AC,         // arctanh(2^-04) = 0.06258157147700300712676502386221
	0x00040055,         // arctanh(2^-05) = 0.03126017849066699476401224517265
	0x0002000B,         // arctanh(2^-06) = 0.01562627175205221137920177875164
	0x00010001,         // arctanh(2^-07) = 0.00781265895154042091032347127604
	0x00008000,         // arctanh(2^-08) = 0.00390626986839682605312756336971
	0x00004000,         // arctanh(2^-09) = 0.00195312748353254999865077088685
	0x00002000,         // arctanh(2^-10) = 9.7656281044103584096445002988533e-4
	0x00001000,         // arctanh(2^-11) = 4.8828128880511282676100662627116e-4
	0x00000800,         // arctanh(2^-12) = 2.4414062985063858292797225210244e-4
	0x00000400,         // arctanh(2^-13) = 1.2207031310632980660296307873709e-4
	0x00000200,         // arctanh(2^-14) = 6.1035156325791225317150609727891e-5
	0x00000100,         // arctanh(2^-15) = 3.0517578134473903148761958402143e-5
	0x00000080,         // arctanh(2^-16) = 1.5258789063684237893098936432323e-5
	0x00000040,         // arctanh(2^-17) = 7.6293945313980297366218574175518e-6
	0x00000020,         // arctanh(2^-18) = 3.8146972656435037170772475010538e-6
	0x00000010,         // arctanh(2^-19) = 1.9073486328148129646346407915023e-6
	0x00000008,         // arctanh(2^-20) = 9.5367431640653912057932962562125e-7
	0x00000004,         // arctanh(2^-21) = 4.7683715820316114007241618841151e-7
	0x00000002,         // arctanh(2^-22) = 2.3841857910156701750905202308922e-7
	0x00000001,         // arctanh(2^-23) = 1.1920928955078181468863150287171e-7
	0x00000000          // arctanh(2^-24) = 5.9604644775390695586078937858512e-8
};

void cordichk(_Accum* px, _Accum* py, _Accum* pz, uint8_t iterations, uint8_t mode)
{
  register uint8_t i, j;
  _Accum x, y, z, xH;
  x = *px;
  y = *py;
  z = *pz;
  for (i = 1; i <= iterations; i++) 
  {
    for(j = 0; j < 2; j++) {/*repeat iterations 4, 13, 40, ... 3k+1*/
      xH = x;
      if((mode && y <= 0) || (!mode && z >= 0)) 
	  {
        x += ltoq(RSHIFT_dynamic(qtol(y), i));
        y += ltoq(RSHIFT_dynamic(qtol(xH), i));
        z -= ltoq(arctanh[i-1]);
      }
      else 
	  {
        x -= ltoq(RSHIFT_dynamic(qtol(y), i));
        y -= ltoq(RSHIFT_dynamic(qtol(xH), i));
        z += ltoq(arctanh[i-1]);
      }
      if(i != 4 && i != 13)
        break;
    }
//xprintf_P(PSTR("cordichk i:%2d x: %8k y: %8k z: %8k\r\n"), (uint16_t)i, x, y, z);
  }
  *px = x;
  *py = y;
  *pz = z;
}
#endif

#ifdef SQRT
_Accum sqrtk_uncorrected(_Accum k, int8_t pow2, uint8_t cordic_steps)
{
  signed long a = qtol(k);
  signed long x, y;
  _Accum z;
  if(a <= 0)
    return 0;
  /* The cordich method works only within [0.03, 2]
   * for other values the following identity is used:
   *
   * sqrt(2^n * a) = sqrt(a) * sqrt(2^n) = sqrt(a) * 2^(n/2)
   *
   * Here, the interval [0.06, 1] is taken, because the
   * number of shifts may be odd and the correction shift
   * may be outside the original interval in that case.
   */
  for(; a > itolk(1); pow2++)
    a = RSHIFT_static(a, 1);
  for(; a < ftolk(0.06); pow2--)
    a = LSHIFT_static(a, 1);
  /* pow2 has to be even */
  if(pow2 > 0 && pow2 & 1) 
  {
    pow2--;
    a = LSHIFT_static(a, 1);
  } 
  else if(pow2 < 0 && pow2 & 1) 
  {
    pow2++;
    a = RSHIFT_static(a, 1);
  }
  pow2 = RSHIFT_static(pow2, 1);
  x = a + ftolk(0.25);
  y = a - ftolk(0.25);
  z = 0;
//xprintf_P(PSTR("sqrtk_uncorrected x: %8k y: %8k z: %8k\r\n"), x, y, z);
  cordichk(pltopq(&x), pltopq(&y), &z, cordic_steps, 1);
//xprintf_P(PSTR("sqrtk_uncorrected x: %8k y: %8k z: %8k\r\n"), x, y, z);
  return (pow2 < 0 ? ltoq(RSHIFT_dynamic(x, -pow2)) : ltoq(LSHIFT_dynamic(x, pow2)));
}
#endif

#ifdef LOGK
_Accum logk(_Accum k)
{
  signed long a = qtol(k);
  register int8_t pow2 = 8;
  signed long x, y, z;
  if(a <= 0)
    return ACCUM_MIN;
  /* The cordic method works only within [1, 9]
   * for other values the following identity is used:
   *
   * log(2^n * a) = log(a) + log(2^n) = log(a) + n log(2)
   */
  for(; a > itolk(9); pow2++)
    a = RSHIFT_static(a, 1);
  for(; a < itolk(1); pow2--)
    a = LSHIFT_static(a, 1);
  x = a + itolk(1);
  y = a - itolk(1);
  z = 0;
  cordichk(pltopq(&x), pltopq(&y), pltopq(&z), ACCUM_FBIT, 1);
//xprintf_P(PSTR("logk x: %8k y: %8k z: %8k\r\n"), x, y, z);
//xprintf_P(PSTR("logk >>: %8k ln2*pow: %8k ln: %8k\r\n"), RSHIFT_static(z, 7), LOG2k*pow2, RSHIFT_static(z, 7) + LOG2k*pow2);
  return ltoq(RSHIFT_static(z, 7) + qtol(LOG2k)*pow2);
}
#endif

#ifdef LLOGLK
_lAccum lloglk(_lAccum a)
{
  register int8_t pow2 = 0;
  _Accum x, y, z;
  if(a <= 0)
    return LACCUM_MIN;
  /* The cordic method works only within [1, 9]
   * for other values the following identity is used:
   *
   * log(2^n * a) = log(a) + log(2^n) = log(a) + n log(2)
   */
  for(; a > itolk(9); pow2++)
    a = ltolq(RSHIFT_static(lqtol(a), 1));
  for(; a < itolk(1); pow2--)
    a = ltolq(LSHIFT_static(lqtol(a), 1));
  x = a + itolk(1);
  y = a - itolk(1);
  z = 0;
  cordichk(&x, &y, &z, LACCUM_FBIT, 1);
  return ltolq(LSHIFT_static(qtol(z), 1)) + LOG2lk*pow2;
}
#endif

#ifdef EXPK
/* Based on code from the file fixed.cpp from:
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 * (C) Copyright 2007 Anthony Williams
 */

const int8_t max_power = ACCUM_IBIT;
/*
const __flash _Accum log_two_power_n_reversed[] =
{
	ftok(11.090354888959124950675713943331),		// ln(2^16)
	ftok(10.397207708399179641258481821873),		// ln(2^15)
	ftok(9.7040605278392343318412497004145),		// ln(2^14)
	ftok(9.0109133472792890224240175789563),		// ln(2^13)
	ftok(8.3177661667193437130067854574981),		// ln(2^12)
	ftok(7.6246189861593984035895533360399),		// ln(2^11)
	ftok(6.9314718055994530941723212145818),		// ln(2^10)
	ftok(6.2383246250395077847550890931236),		// ln(2^9)
	ftok(5.5451774444795624753378569716654),		// ln(2^8)
	ftok(4.8520302639196171659206248502072),		// ln(2^7)
	ftok(4.1588830833596718565033927287491),		// ln(2^6)
	ftok(3.4657359027997265470861606072909),		// ln(2^5)
	ftok(2.7725887222397812376689284858327),		// ln(2^4)
	ftok(2.0794415416798359282516963643745),		// ln(2^3)
	ftok(1.3862943611198906188344642429164),		// ln(2^2)
	ftok(0.6931471805599453094172321214582)			// ln(2^1)
};

const __flash _Accum log_one_plus_two_power_minus_n[] =
{
	ftok(0.40546510810816438197801311546435),		// ln(1+2^-1)
	ftok(0.22314355131420975576629509030983),		// ln(1+2^-2)
	ftok(0.11778303565638345453879410947052),		// ln(1+2^-3)
	ftok(0.06062462181643484258060613204042),		// ln(1+2^-4)
	ftok(0.03077165866675368837102820759677),		// ln(1+2^-5)
	ftok(0.01550418653596525415085404604245),		// ln(1+2^-6)
	ftok(0.00778214044205494894746290006114),		// ln(1+2^-7)
	ftok(0.00389864041565732301393734309584),		// ln(1+2^-8)
	ftok(0.00195122013126174943967404953184),		// ln(1+2^-9)
	ftok(9.7608597305545889596082490801719e-4),		// ln(1+2^-10)
	ftok(4.8816207950135118853704969264541e-4),		// ln(1+2^-11)
	ftok(2.4411082752736270916047908582345e-4),		// ln(1+2^-12)
	ftok(1.220628625256773716230553671622e-4),		// ln(1+2^-13)
	ftok(6.1033293680638524913158789648964e-5),		// ln(1+2^-14)
	ftok(3.0517112473186378569069514168995e-5),		// ln(1+2^-15)
	ftok(1.5258672648362397405757325134889e-5)		// ln(1+2^-16)
};

const __flash _Accum log_one_over_one_minus_two_power_minus_n[] =
{
	ftok(0.69314718055994530941723212145818),		// ln(1/(1-2^-1))
	ftok(0.28768207245178092743921900599383),		// ln(1/(1-2^-2))
	ftok(0.13353139262452262314634362093135),		// ln(1/(1-2^-3))
	ftok(0.06453852113757117167292391568399),		// ln(1/(1-2^-4))
	ftok(0.03174869831458030115699628274853),		// ln(1/(1-2^-5))
	ftok(0.01574835696813916860754951146083),		// ln(1/(1-2^-6))
	ftok(0.00784317746102589287318404249094),		// ln(1/(1-2^-7))
	ftok(0.00391389932113632909231778364357),		// ln(1/(1-2^-8))
	ftok(0.00195503483580335055762749224187),		// ln(1/(1-2^-9))
	ftok(9.7703964782661278596807515175347e-4),		// ln(1/(1-2^-10))
	ftok(4.8840049810887446498496355989691e-4),		// ln(1/(1-2^-11))
	ftok(2.4417043217391445669546541838143e-4),		// ln(1/(1-2^-12))
	ftok(1.2207776368698224158287079031198e-4),		// ln(1/(1-2^-13))
	ftok(6.1037018970943925721142429806818e-5),		// ln(1/(1-2^-14))
	ftok(3.0518043795761427728454402635291e-5),		// ln(1/(1-2^-15))
	ftok(1.5258905479006078380440547729758e-5),		// ln(1/(1-2^-16))
};
*/
const __flash _Accum log_two_power_n_reversed[] =
{
	0x.00058B91fp17,        // ln(2^16) = 11.090355
	0x.000532D8fp17,        // ln(2^15) = 10.397208
	0x.0004DA1Ffp17,        // ln(2^14) =  9.704061
	0x.00048166fp17,        // ln(2^13) =  9.010913
	0x.000428ADfp17,        // ln(2^12) =  8.317766
	0x.0003CFF4fp17,        // ln(2^11) =  7.624619
	0x.0003773Afp17,        // ln(2^10) =  6.931472
	0x.00031E81fp17,        // ln(2^09) =  6.238325
	0x.0002C5C8fp17,        // ln(2^08) =  5.545177
	0x.00026D0Ffp17,        // ln(2^07) =  4.852030
	0x.00021456fp17,        // ln(2^06) =  4.158883
	0x.0001BB9Dfp17,        // ln(2^05) =  3.465736
	0x.000162E4fp17,        // ln(2^04) =  2.772589
	0x.00010A2Bfp17,        // ln(2^03) =  2.079442
	0x.0000B172fp17,        // ln(2^02) =  1.386294
	0x.000058B9fp17         // ln(2^01) =  0.693147
};

const __flash _Accum log_one_plus_two_power_minus_n[] =
{
	0x.000033E6fp17,        // ln(1+2^-01) =  0.405465
	0x.00001C90fp17,        // ln(1+2^-02) =  0.223144
	0x.00000F14fp17,        // ln(1+2^-03) =  0.117783
	0x.000007C3fp17,        // ln(1+2^-04) =  0.060625
	0x.000003F0fp17,        // ln(1+2^-05) =  0.030772
	0x.000001FCfp17,        // ln(1+2^-06) =  0.015504
	0x.000000FFfp17,        // ln(1+2^-07) =  0.007782
	0x.00000080fp17,        // ln(1+2^-08) =  0.003899
	0x.00000040fp17,        // ln(1+2^-09) =  0.001951
	0x.00000020fp17,        // ln(1+2^-10) =  0.000976
	0x.00000010fp17,        // ln(1+2^-11) =  0.000488
	0x.00000008fp17,        // ln(1+2^-12) =  0.000244
	0x.00000004fp17,        // ln(1+2^-13) =  0.000122
	0x.00000002fp17,        // ln(1+2^-14) =  0.000061
	0x.00000001fp17,        // ln(1+2^-15) =  0.000031
	0x.00000000fp17         // ln(1+2^-16) =  0.000015
};

const __flash _Accum log_one_over_one_minus_two_power_minus_n[] =
{
	0x.000058B9fp17,        // ln(1/(1-2^-01)) =  0.693147
	0x.000024D3fp17,        // ln(1/(1-2^-02)) =  0.287682
	0x.00001118fp17,        // ln(1/(1-2^-03)) =  0.133531
	0x.00000843fp17,        // ln(1/(1-2^-04)) =  0.064539
	0x.00000410fp17,        // ln(1/(1-2^-05)) =  0.031749
	0x.00000204fp17,        // ln(1/(1-2^-06)) =  0.015748
	0x.00000101fp17,        // ln(1/(1-2^-07)) =  0.007843
	0x.00000080fp17,        // ln(1/(1-2^-08)) =  0.003914
	0x.00000040fp17,        // ln(1/(1-2^-09)) =  0.001955
	0x.00000020fp17,        // ln(1/(1-2^-10)) =  0.000977
	0x.00000010fp17,        // ln(1/(1-2^-11)) =  0.000488
	0x.00000008fp17,        // ln(1/(1-2^-12)) =  0.000244
	0x.00000004fp17,        // ln(1/(1-2^-13)) =  0.000122
	0x.00000002fp17,        // ln(1/(1-2^-14)) =  0.000061
	0x.00000001fp17,        // ln(1/(1-2^-15)) =  0.000031
	0x.00000001fp17         // ln(1/(1-2^-16)) =  0.000015
};

_Accum expk(_Accum k)
{
	if(k >= log_two_power_n_reversed[0])
	{
		return ACCUM_MAX;
	}
	if(k < -log_two_power_n_reversed[(ACCUM_IBIT+ACCUM_FBIT)-2*ACCUM_FBIT])
	{
		return 0K;
	}
	if(!k)
	{
		return 1K;
	}

	_Accum res = 1K;

	if(k > 0)
	{
		int8_t 				  power 	= max_power;
		const __flash _Accum* log_entry = (const __flash _Accum*)log_two_power_n_reversed;
		_Accum 				  temp		= k;
		while(temp && power > (-(int8_t)ACCUM_FBIT))
		{
			while(!power || (temp < *log_entry))
			{
				if(!power)
				{
					log_entry = (const __flash _Accum*)log_one_plus_two_power_minus_n;
				}
				else
				{
					++log_entry;
				}
				--power;
			}
			temp -= *log_entry;
			if(power < 0)
			{
				res += (res >> (-power));
			}
			else
			{
				res <<= power;
			}
		}
	}
	else
	{
		int8_t 				  power 	= ACCUM_FBIT;
		const __flash _Accum* log_entry = (const __flash _Accum*)log_two_power_n_reversed+(max_power-power);
		_Accum 				  temp 		= k;

		while(temp && power > (-(int8_t)ACCUM_FBIT))
		{
			while(!power || (temp > (-*log_entry)))
			{
				if(!power)
				{
					log_entry = (const __flash _Accum*)log_one_over_one_minus_two_power_minus_n;
				}
				else
				{
					++log_entry;
				}
				--power;
			}
			temp += *log_entry;
			if(power < 0)
			{
				res -= (res >> (-power));
			}
			else
			{
				res >>= power;
			}
		}
	}
	
	return res;
}
#endif

#ifdef POWK
_Accum powk(_Accum b, _Accum e)
{
#warning "Need to finish powk function"
	q_t qt;
	
	if (0 == e)
		return 1;

	if (b > 0)
		return expk((e)*logk(b));
		
	if (0 == b)
		return 0;

	qt.q = e;
	
	if (qt.ul == ((~(ACCUM_FACTOR-1)) & qt.ul))
	{
		return ((ACCUM_FACTOR)&qt.ul)?-1K*powk(-b,e):powk(-b,e);
	}

	return 0;		
}
#endif
