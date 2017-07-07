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
#ifndef TEST_ON_PC
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "avrfix.h"
#include "avrfix_config.h"

#endif
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

extern void cordicck(_Acc* x, _Acc* y, _Acc* z, uint8_t iterations, uint8_t mode);
extern void cordichk(_Acc* x, _Acc* y, _Acc* z, uint8_t iterations, uint8_t mode);
extern void cordiccsk(_sAcc* x, _sAcc* y, _sAcc* z, uint8_t mode);
extern void cordichsk(_sAcc* x, _sAcc* y, _sAcc* z, uint8_t mode);

#ifdef SMULSKD
_sAcc smulskD(_sAcc x, _sAcc y)
{
  return ss(RSHIFT_static(sl(x)*sl(y), SACCUM_FBIT));
}
#endif

#ifdef SMULSKS
_sAcc smulskS(_sAcc x, _sAcc y)
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
_Acc mulkD(_Acc x, _Acc y)
{
#ifdef FX_DEFAULT_FRACBITS
  #if BYTE_ORDER == BIG_ENDIAN
    #  define LO 0
    #  define HI 1
  #else
    #  define LO 1
    #  define HI 0
  #endif
  unsigned short xs[2];
  unsigned short ys[2];
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  y = absk(y);
  *((_Acc*)xs) = absk(x);
  *((_Acc*)ys) = y;
  x = sl(xs[HI])*y + sl(xs[LO])*ys[HI];
  *((_Acc*)xs) = ul(xs[LO])*ul(ys[LO]);
  if(positive)
     return x + us(xs[HI]);
  else
     return -(x + us(xs[HI]));
  #undef HI
  #undef LO
#endif

#ifdef FX_GCC_FRACBITS
  union
  {
	_Acc	q;
	_Accum	k;  
  } xg, yg;

  xg.q = x;
  yg.q = y;
//xprintf_P(PSTR("xg: %8.6lq yg: %8.6lq\r\n"), xg.f, yg.f);
  xg.k *= yg.k;
  return xg.q;
#endif
}
#endif

#ifdef MULKS
_Acc mulkS(_Acc x, _Acc y)
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
  *((_Acc*)xs) = absk(x);
  *((_Acc*)ys) = absk(y);
  mul = ul(xs[HI]) * ul(ys[HI]);
  if(mul > 32767)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  mul =   LSHIFT_static(mul, ACCUM_FBIT)
        + ul(xs[HI])*ul(ys[LO])
        + ul(xs[LO])*ul(ys[HI])
        + RSHIFT_static(ul(xs[LO]*ys[LO]), ACCUM_FBIT);
  if(mul & 0x80000000)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  return (positive ? (long)mul : -(long)mul);
  #undef HI
  #undef LO
}
#endif

#ifdef LMULLKD
_lAcc lmullkD(_lAcc x, _lAcc y)
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
_lAcc lmullkS(_lAcc x, _lAcc y)
{
  lAccum_container xc, yc;
  unsigned long mul;
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  x = labslk(x);
  y = labslk(y);
  *((_lAcc*)&xc) = x;
  *((_lAcc*)&yc) = y;
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
  return (positive ? (long)mul : -(long)mul);
}
#endif

#ifdef SDIVSKD
_sAcc sdivskD(_sAcc x, _sAcc y)
{
  return ss((sl(x) << SACCUM_FBIT) / y);
}
#endif

#ifdef SDIVSKS
_sAcc sdivskS(_sAcc x, _sAcc y)
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
_Acc divkD(_Acc x, _Acc y) 
{
  _Acc result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (x > 0 && y > 0)) ? 1 : 0;
  x = absk(x);
  y = absk(y);
  /* Align x leftmost to get maximum precision */

  for (i=0 ; i<ACCUM_FBIT ; i++)
  {
    if (x >= ACCUM_MAX / 2) break;
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
  i = (ACCUM_FBIT - i) - j;
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

#ifdef DIVKS
_Acc divkS(_Acc x, _Acc y) 
{
  _Acc result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  if(y == 0)
     return (x < 0 ? ACCUM_MIN : ACCUM_MAX);
  x = absk(x);
  y = absk(y);

  for (i=0 ; i<ACCUM_FBIT ; i++)
  {
    if (x >= ACCUM_MAX / 2) break;
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
_lAcc ldivlkD(_lAcc x, _lAcc y) 
{
  _lAcc result;
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
_lAcc ldivlkS(_lAcc x, _lAcc y) 
{
  _lAcc result;
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
_Acc sincosk(_Acc angle, _Acc* cosp)
{
  _Acc 		x;
  _Acc 		y 				= 0;
  uint8_t 	correctionCount = 0;
  uint8_t 	quadrant 		= 1;

  if(cosp == NULL)
     cosp = &x;

  /* move large values into [0,2 PI] */
  #define MAX_CORRECTION_COUNT 1
  while(angle >= 2*PIk)
  {
    angle -= 2*PIk;
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
    angle += 2*PIk;
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

  if(angle > PIk) 
  {
    angle = angle - PIk;
    quadrant += 2;
  }
  if(angle > (PIk/2 + 1)) 
  {
    angle = PIk - angle + 1;
    quadrant += 1;
  }
  if(angle == 0) 
  {
    *cosp = (quadrant == 2 || quadrant == 3 ? -itok(1) : itok(1));
    return 0;
  }
  *cosp = CORDICC_GAIN;
  angle = LSHIFT_static(angle, 8);
  cordicck(cosp, &y, &angle, ACCUM_FBIT, 0);
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
  return y;
}
#endif

#ifdef LSINCOSLK
_lAcc lsincoslk(_lAcc angle, _lAcc* cosp)
{
  _lAcc x;
  _lAcc y 					= 0;
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
  cordicck(cosp, &y, &angle, LACCUM_FBIT, 0);
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
_lAcc lsincosk(_Acc angle, _lAcc* cosp)
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
_sAcc roundskD(_sAcc f, uint8_t n)
{
   n = SACCUM_FBIT - n;
   if(f >= 0) 
   {
      return (f & (0xFFFF << n)) + ((f & (1 << (n-1))) << 1);
   } 
   else 
   {
      return (f & (0xFFFF << n)) - ((f & (1 << (n-1))) << 1);
   }
}
#endif

#ifdef ROUNDKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Acc roundkD(_Acc f, uint8_t n)
{
   n = ACCUM_FBIT - n;
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

#ifdef ROUNDSKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_sAcc roundskS(_sAcc f, uint8_t n)
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
_Acc roundkS(_Acc f, uint8_t n)
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
_lAcc roundlkD(_lAcc f, uint8_t n)
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
_Acc roundlkS(_lAcc f, uint8_t n)
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
uint8_t countlssk(_sAcc f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 2;
   for(i = 0; i < 15; i++) 
   {
      if((*pf & 0x40) != 0)
         break;
      f = LSHIFT_static(f, 1);
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
uint8_t countlsk(_Acc f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 3;
   for(i = 0; i < 31; i++) 
   {
      if((*pf & 0x40) != 0)
         break;
      f = LSHIFT_static(f, 1);
   }
   return i;
}
#endif

#ifdef TANKD
_Acc tankD(_Acc angle)
{
  _Acc sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkD(sin, cos);
}
#endif

#ifdef TANKS
_Acc tankS(_Acc angle)
{
  _Acc sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkS(sin, cos);
}
#endif

#ifdef LTANLKD
_lAcc ltanlkD(_lAcc angle)
{
  _lAcc sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkD(sin, cos);
}
#endif

#ifdef LTANLKS
_lAcc ltanlkS(_lAcc angle)
{
  _lAcc sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif

#ifdef LTANKD
_lAcc ltankD(_Acc angle)
{
  _lAcc sin, cos;
  sin = lsincosk(angle, &cos);
  return ldivlkD(sin, cos);
}
#endif

#ifdef LTANKS
_lAcc ltankS(_Acc angle)
{
  _lAcc sin, cos;
  sin = lsincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif

#ifdef ATAN2K
_Acc atan2kInternal(_Acc x, _Acc y)
{
  _Acc 		z 	= 0;
  uint8_t 	i 	= 0;
  uint8_t* 	px	= ((uint8_t*)&x) + 3, *py = ((uint8_t*)&y) + 3;

  for(;!(*px & 0x60) && !(*py & 0x60) && i < 8;i++) 
  {
    x = LSHIFT_static(x, 1);
    y = LSHIFT_static(y, 1);
  }
  if(i > 0) 
  {
    cordicck(&x, &y, &z, ACCUM_FBIT, 1);
    return RSHIFT_static(z, 8);
  } 
  else 
  {
    return PIk/2 - divkD(x, y) - 1;
  }
}

_Acc atan2k(_Acc x, _Acc y)
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
      return x + PIk/2 + 1;
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
_lAcc latan2lk(_lAcc x, _lAcc y)
{
  uint8_t 	signX, signY;
  _Acc 		z	= 0;
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
    x = RSHIFT_static(x, 1);
    y = RSHIFT_static(y, 1);
  }
  cordicck(&x, &y, &z, LACCUM_FBIT, 1);
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
 * iterations is the fractal bit count (16 for _Acc, 24 for _lAcc)
 * and now the only variable, the execution time depends on.
 */
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

void cordicck(_Acc* px, _Acc* py, _Acc* pz, uint8_t iterations, uint8_t mode)
{
  register uint8_t i;
  _Acc x, y, z, xH;
  x = *px;
  y = *py;
  z = *pz;
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
  *px = x;
  *py = y;
  *pz = z;
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
 * iterations is the fractal bit count (16 for _Acc, 24 for _lAcc)
 */
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

void cordichk(_Acc* px, _Acc* py, _Acc* pz, uint8_t iterations, uint8_t mode)
{
  register uint8_t i, j;
  _Acc x, y, z, xH;
  x = *px;
  y = *py;
  z = *pz;
  for (i = 1; i <= iterations; i++) 
  {
    for(j = 0; j < 2; j++) {/*repeat iterations 4, 13, 40, ... 3k+1*/
      xH = x;
      if((mode && y <= 0) || (!mode && z >= 0)) 
	  {
        x += RSHIFT_dynamic(y, i);
        y += RSHIFT_dynamic(xH, i);
        z -= arctanh[i-1];
      }
      else 
	  {
        x -= RSHIFT_dynamic(y, i);
        y -= RSHIFT_dynamic(xH, i);
        z += arctanh[i-1];
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
_Acc sqrtk_uncorrected(_Acc a, int8_t pow2, uint8_t cordic_steps)
{
  _Acc x, y, z;
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
  cordichk(&x, &y, &z, cordic_steps, 1);
//xprintf_P(PSTR("sqrtk_uncorrected x: %8k y: %8k z: %8k\r\n"), x, y, z);
  return (pow2 < 0 ? RSHIFT_dynamic(x, -pow2) : LSHIFT_dynamic(x, pow2));
}
#endif

#ifdef LOGK
_Acc logk(_Acc a)
{
  register int8_t pow2 = 8;
  _Acc x, y, z;
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
  cordichk(&x, &y, &z, ACCUM_FBIT, 1);
//xprintf_P(PSTR("logk x: %8k y: %8k z: %8k\r\n"), x, y, z);
//xprintf_P(PSTR("logk >>: %8k ln2*pow: %8k ln: %8k\r\n"), RSHIFT_static(z, 7), LOG2k*pow2, RSHIFT_static(z, 7) + LOG2k*pow2);
  return RSHIFT_static(z, 7) + LOG2k*pow2;
}
#endif

#ifdef LLOGLK
_lAcc lloglk(_lAcc a)
{
  register int8_t pow2 = 0;
  _Acc x, y, z;
  if(a <= 0)
    return LACCUM_MIN;
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
  cordichk(&x, &y, &z, LACCUM_FBIT, 1);
  return LSHIFT_static(z, 1) + LOG2lk*pow2;
}
#endif
