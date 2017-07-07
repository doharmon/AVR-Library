/*
 * test_math.c
 *
 * Created: 10/13/2016 8:52:52 PM
 *  Author: dharmon
 */ 


#include <avr/io.h>
#include "../source/lib_math/math.h"

int main(void)
{
	uint32_t	ul;
	_Accum		k;
	_Accum		q;
	
	k = ku32div100(0x000FFFFF);
	q = q14u32div10(0x000068F5);
	q = q14u32div10(0x000FFFFF);
	
	ul = 0x000FFFFF;
	ul = u32div10(ul);
	
	ul = 0x000FFFFF;
	ul = u32div100(ul);
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}