#include <stdlib.h>
#include <math.h>

typedef struct stConvert
{
	float f;	f;
	uint32_t	dw;
	uint8_t		ab[4];
} convert_t;

int main(void)
{
	uint8_t		b;
	convert_t 	convert;

	printf("const __flash unsigned long arctan[25] =\r\n");
	printf("{\r\n");
	for (b = 0; b < 25; b++)
	{
		printf("\t0x%08X,         // arctan(2^-%02d) = %.10f\r\n",
				((uint32_t)(atan(pow(2,-b))*pow(2,24))+1)>>1,
				b,
				atan(pow(2,-b)));
	}
	printf("};\r\n");
	printf("\r\n");
/*
	printf("const __flash unsigned long arctan[24] =\r\n");
	printf("{\r\n");
	for (b = 1; b < 25; b++)
	{
		printf("\t0x%08X,         // arctanh(2^-%02d) = %.10f\r\n",
				((uint32_t)(atanh(pow(2,-b))*pow(2,24))+1)>>1,
				b,
				atanh(pow(2,-b)));
	}
	printf("};\r\n");
	printf("\r\n");
*/
	printf("const __flash _Accum log_two_power_n_reversed[] =\r\n");
	printf("{\r\n");
	for (b = 16; b > 0; b--)
	{
		printf("\t0x.%08Xfp17,        // ln(2^%02d) = %9.6f\r\n",
				((uint32_t)(log(pow(2,b))*pow(2,16))+1)>>1,
				b,
				log(pow(2,b)));
	}
	printf("};\r\n");
	printf("\r\n");

	printf("const __flash _Accum log_one_plus_two_power_minus_n[] =\r\n");
	printf("{\r\n");
	for (b = 1; b <= 16; b++)
	{
		printf("\t0x.%08Xfp17,        // ln(1+2^-%02d) = %9.6f\r\n",
				((uint32_t)(log(1.0+pow(2,-b))*pow(2,16))+1)>>1,
				b,
				log(1.0+pow(2,-b)));
	}
	printf("};\r\n");
	printf("\r\n");

	printf("const __flash _Accum log_one_over_one_minus_two_power_minus_n[] =\r\n");
	printf("{\r\n");
	for (b = 1; b <= 16; b++)
	{
		printf("\t0x.%08Xfp17,        // ln(1/(1-2^-%02d)) = %9.6f\r\n",
				((uint32_t)(log(1.0/(1.0-pow(2,-b)))*pow(2,16))+1)>>1,
				b,
				log(1.0/(1.0-pow(2,-b))));
	}
	printf("};\r\n");
	printf("\r\n");
}