#ifndef _MATH_H
#define _MATH_H

uint32_t		u32div10(uint32_t);
inline uint32_t	u32div100(uint32_t ul)
				{
					return u32div10(u32div10(ul));
				}

uint32_t		q14u32div10(uint32_t);
_Accum			ku32div100(uint32_t);
#endif // _MATH_H

