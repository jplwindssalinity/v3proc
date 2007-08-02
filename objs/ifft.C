// Take the inverse FFT of complex sequence of length 2^m

#include <math.h>
#include "cplx.h"


void fft(complex *a, int m);

void ifft(complex *a, int m)
{

	int n, i;
	
	n= pow(2,m);
	for (i=1; i <= n; i++)
	{
		a[i]= C_conj(a[i]);
	}

	fft(a,m);

	for (i=1; i <= n; i++)
	{
		a[i]= C_div(C_conj(a[i]),C_cplx((float) n,0.0));
	}
}
	