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

#ifndef _GCCFIX_CONFIG_H
#define _GCCFIX_CONFIG_H

#define GCC_CONFIG		0

#define BIG_ENDIAN		0
#define LITTLE_ENDIAN	1

#ifndef GCCFIX_CONFIG
#define GCCFIX_CONFIG	GCC_CONFIG
#endif

#if GCCFIX_CONFIG == GCC_CONFIG
#define BYTE_ORDER BIG_ENDIAN
#define LSHIFT_static(x, b)		((b) == 1 ? (x) + (x) : ((b) < 8 ? ((x) << (b)) : (x) * (1UL << (b))))
#define RSHIFT_static(x, b)		((x) >> (b))
#define LSHIFT_dynamic(x, b)	((x) << (b))
#define RSHIFT_dynamic(x, b)	((x) >> (b))
#endif

/*
#define SMULSKD
#define	SMULSKS
*/
#define MULKD
/*
#define MULKS
*/
#define LMULLKD
/*
#define LMULLKS
#define SDIVSKD
#define SDIVSKS
*/
#define DIVKD
/*
#define DIVKS
*/
#define LDIVLKD
/*
#define LDIVLKS
*/
#define SINCOSK
#define LSINCOSLK
/*
#define LSINCOSK
*/
#define ROUNDSKD
#define ROUNDKD
#define ROUNDSKS
#define ROUNDKS
#define ROUNDLKD
#define ROUNDLKS
#define COUNTLSSK
#define COUNTLSK
#define TANKD
/*
#define TANKS
*/
#define LTANLKD
/*
#define LTANLKS
#define LTANKD
#define LTANKS
*/
#define ATAN2K
#define LATAN2LK
#define CORDICCK
#define CORDICHK
#define SQRT
#define LOGK
#define LLOGLK
#define EXPK
#define POWK
#endif /* _GCCFIX_CONFIG_H */

