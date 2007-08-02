/* ** CPLX.C ** A collection of complex number routines. 
** 
** complex C_add (complex a, complex b) 
** complex C_sub (complex a, complex b) 
** complex C_mul (complex a, complex b) 
** complex C_div (complex a, complex b) 
** complex C_para (complex a, complex b) 
** return sum, difference, product, quotient or parallel 
** of complex quantities a and b. 
** 
** complex C_conj (complex a) 
** complex C_inv (complex a) 
** return complex conj or invert of complex quantity a 
** 
** float C_mag (complex a) 
** float C_ang (complex a) 
** return mag or angle (in rads) of complex quantity a 
** 
*/ 

#include <stdio.h> 
#include <math.h> 

typedef struct
{
   float r;   /* real part */
   float i;   /* imag part */
} complex;

typedef struct
{
   double r;   /* real part */
   double i;   /* imag part */
} doublecomplex;

complex C_cplx (float r, float i)
{
	complex result;
	result.r= r;
	result.i= i;
	return result;
}

complex C_conj (complex a) 
{ 
	complex result; 
	result.r = a.r; 
	result.i = -a.i; 
	return result; 
} 

complex C_assign(complex a)
{
	complex result; 
	result.r = a.r; 
	result.i = a.i; 
	return result; 
}

complex C_add (complex a, complex b) 
{ 
	complex result; 
	result.r = a.r + b.r; 
	result.i = a.i + b.i; 
	return result; 
} 

complex C_sub (complex a, complex b) 
{
	complex result; 
	result.r = a.r - b.r; 
	result.i = a.i - b.i; 
	return result; 
} 

complex C_mul (complex a, complex b) 
{ 
	complex result; 
	result.r = a.r*b.r - a.i*b.i; 
	result.i = a.r*b.i + b.r*a.i; 
	return result; 
} 

complex C_div (complex a, complex b) 
{ 
	complex c, result, num; float denom; 

	c = C_conj(b); 
	num = C_mul (a, c); 
	denom = b.r*b.r + b.i*b.i + 1.2e-63; /*to prevent division by zero*/ 

	result.r = num.r / denom; 
	result.i = num.i / denom; 
	return result; 
} 

complex C_para (complex a, complex b) 
{ 
	complex result, num, denom; 

	num = C_mul (a, b); 
	denom = C_add(a, b); 

	result = C_div (num, denom); 
	return result; 
} 

complex C_inv (complex a) 
{ 
	complex result, num; 

	num.r = 1.0; 
	num.i = 0.0; 
	result = C_div (num, a); 
	return result; 
} 


float C_ang (complex a) 
{ 
	float result; 

	result = (float) atan2 ((double) a.i, (double) a.r + 1e-99); 
	/* Note that 1e-99 is added to avoid computing the atan of 
	** 90 degrees */ 
	return result; 
} 

float C_mag (complex a) 
{ 
	float result; 

	result = (float) sqrt ( (double) (a.r*a.r + a.i*a.i)); 
	return result; 
} 

complex C_exp(float f)
{
	complex result;

        result.r= cos(f);
        result.i= sin(f);
        return result;
}

        