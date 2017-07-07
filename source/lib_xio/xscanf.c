/*-------------------------------------------------------------------------------------------------
 *	xscanf.c
 *
 * 	(c) 2017 David O Harmon
 *
 *	Based on:
 *
 *	NMEA library
 *	URL: http://nmea.sourceforge.net
 *	Author: Tim (xtimor@gmail.com)
 *	Licence: http://www.gnu.org/licenses/lgpl.html
 *	$Id: tok.c 17 2008-03-11 11:56:11Z xtimor $
 *-------------------------------------------------------------------------------------------------
 */

#include "xio.h"

#define XSCANF_TOKS_COMPARE   (1)
#define XSCANF_TOKS_PERCENT   (2)
#define XSCANF_TOKS_STAR      (3)
#define XSCANF_TOKS_WIDTH     (4)
#define XSCANF_TOKS_TYPE      (5)

#define XSCANF_CONVSTR_BUF	11

/*-------------------------------------------------------------------------------------------------
 *
 * \brief Convert ROM string to number
 *-------------------------------------------------------------------------------------------------
 */
static int xscanf_atoi_P(const __flash char* str, uint8_t str_sz)
{
    char buff[XSCANF_CONVSTR_BUF];
	char* pbuff = buff;
    long res = 0;

    if(str_sz < XSCANF_CONVSTR_BUF)
    {
        memcpy_P(buff, str, str_sz);
        buff[str_sz] = '\0';
        xatoi(&pbuff, &res);
    }

    return (int)res;
}

/*-------------------------------------------------------------------------------------------------
 * \brief Convert string to number
 *-------------------------------------------------------------------------------------------------
 */
static int xscanf_atoi(const char* str, uint8_t str_sz)
{
    char buff[XSCANF_CONVSTR_BUF];
	char* pbuff = buff;
    long res = 0;

    if(str_sz < XSCANF_CONVSTR_BUF)
    {
        memcpy(buff, str, str_sz);
        buff[str_sz] = '\0';
        xatoi(&pbuff, &res);
    }

    return (int)res;
}

/*-------------------------------------------------------------------------------------------------
 * \brief Convert string to fraction number
 *-------------------------------------------------------------------------------------------------
 */
static _Accum xscanf_atok(const char* str, uint8_t str_sz)
{
    char buff[XSCANF_CONVSTR_BUF];
	char* pbuff = buff;
    _Accum res = 0;

    if(str_sz < XSCANF_CONVSTR_BUF)
    {
        memcpy(buff, str, str_sz);
        buff[str_sz] = '\0';
        xatok(&pbuff, &res, xioFRACBITS_K, 0);
    }

    return res;
}

/*-------------------------------------------------------------------------------------------------
 * \brief Read formatted input from a string
 *-------------------------------------------------------------------------------------------------
 */
uint8_t xsscanf_P(const char* buff, const __flash char* format, ...)
{
    const char*			beg_tok;
    va_list 			arg_ptr;
    void*				parg_target;
    uint8_t 			tok_type	= XSCANF_TOKS_COMPARE;
    uint8_t 			width 		= 0;
    uint8_t 			ignore 		= 0;
    const __flash char*	beg_fmt 	= 0;
    int 				snum		= 0;
	int					unum 		= 0;
    uint8_t 			tok_count 	= 0;

    va_start(arg_ptr, format);
    
    for(; *format; ++format)
    {
        switch(tok_type)
        {
        case XSCANF_TOKS_COMPARE:
            if('%' == *format)
                tok_type = XSCANF_TOKS_PERCENT;
            else if(*buff++ != *format)
                goto fail;
            break;

        case XSCANF_TOKS_PERCENT:
            width = 0;
			ignore = 0;
            beg_fmt = format;
            tok_type = XSCANF_TOKS_STAR;
			
        case XSCANF_TOKS_STAR:
			if ('*' == *format)
			{
	            tok_type = XSCANF_TOKS_WIDTH;
				beg_fmt++;
				ignore = 1;
				break;
			}

        case XSCANF_TOKS_WIDTH:
            if(isdigit(*format))
                break;
            {
                tok_type = XSCANF_TOKS_TYPE;
                if(format > beg_fmt)
                    width = xscanf_atoi_P(beg_fmt, (uint8_t)(format - beg_fmt));
            }

        case XSCANF_TOKS_TYPE:
            beg_tok = buff;

            if(!width && ('c' == *format) && *buff != format[1])
                width = 1;

            if(width)
            {
                if(buff + width <= end_buf)
                    buff += width;
                else
                    goto fail;
            }
            else
            {
                if(!format[1] || (0 == (buff = (char *)memchr(buff, format[1], end_buf - buff))))
                    buff = end_buf;
            }

            if(buff > end_buf)
                goto fail;

            tok_type = XSCANF_TOKS_COMPARE;

			if (ignore)
				break;
				
            tok_count++;

            parg_target = 0; width = (uint8_t)(buff - beg_tok);

            switch(*format)
            {
				case 'c':
					parg_target = (void *)va_arg(arg_ptr, char *);
					if(width && 0 != (parg_target))
						*((char *)parg_target) = *beg_tok;
					break;
				case 's':
					parg_target = (void *)va_arg(arg_ptr, char *);
					if(width && 0 != (parg_target))
					{
						memcpy(parg_target, beg_tok, width);
						((char *)parg_target)[width] = '\0';
					}
					break;
				case 'k':
					parg_target = (void *)va_arg(arg_ptr, _Accum *);
					if(width && 0 != (parg_target))
						*((_Accum *)parg_target) = xscanf_atok(beg_tok, width);
					break;
				case 'r':
					parg_target = (void *)va_arg(arg_ptr, _Fract *);
					if(width && 0 != (parg_target))
						*((_Fract *)parg_target) = xscanf_atok(beg_tok, width);
					break;
            };

            if(parg_target)
                break;
            if(0 == (parg_target = (void *)va_arg(arg_ptr, int *)))
                break;
            if(!width)
                break;

            switch(*format)
            {
				case 'd':
				case 'i':
					snum = xscanf_atoi(beg_tok, width);
					memcpy(parg_target, &snum, sizeof(int));
					break;
				case 'u':
					unum = xscanf_atoi(beg_tok, width);
					memcpy(parg_target, &unum, sizeof(unsigned int));
					break;
				default:
					goto fail;
            };

            break;
        };
    }

fail:

    va_end(arg_ptr);

    return tok_count;
}
