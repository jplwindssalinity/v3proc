/******************************************************************************
  Function:  erfcomp_cheb
  Purpose:  Calculate the complimentary error function to good accuracy using 
            an approximate formula.  The worst case fractional error is less 
            than 1.2 x 10^-7.
  Author:  R. Glenister


******************************************************************************/
#include <math.h>
#include "function.h"

#define ERF_C1 -1.26551223
#define ERF_C2  1.00002368
#define ERF_C3  0.37409196
#define ERF_C4  0.09678418
#define ERF_C5 -0.18628806
#define ERF_C6  0.27886807
#define ERF_C7 -1.13520398
#define ERF_C8  1.48851587
#define ERF_C9 -0.82215223
#define ERF_C10 0.17087277

float erfcomp_cheb( float x )

{

  float t, z, ans;

  z = fabs( x );
  t = 1.0 / ( 1.0 + 0.5 * z );

  ans = t * exp( -z * z + 
        ERF_C1 + t * ( ERF_C2 + t * ( ERF_C3 + t * ( ERF_C4 + t * ( 
        ERF_C5 + t * ( ERF_C6 + t * ( ERF_C7 + t * ( ERF_C8 + t * ( 
        ERF_C9 + t * ERF_C10 ) ) ) ) ) ) ) ) );

  return ans;

}
