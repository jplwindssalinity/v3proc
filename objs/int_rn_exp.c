/*******************************************************************************
   Function:  int_rn_exp
   Purpose:  Compute the integral from 0 to R of r^n exp(-r^2/2).
   Author:  R. Glenister
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include "function.h"


int int_rn_exp( int n, float radius, float * result )

{

  int iter, n_current, num_its;

  float double_factorial, last_term, one_over_radius_sq, radius_sq, rpow, sum;

  /* Calculate 1 / r^2 for use later. */
  if( radius != 0. ) {

    radius_sq = radius * radius;

    one_over_radius_sq  = 1. / radius_sq;

  } else {

    *result = 0.;
    return 0;

  }



  /* Calculate the number of iterations (terms in the series). */
  /* An ndata-dimensional data vector requires the (ndata - 1) integral. */
  if( ( n % 2 ) == 0 ) {

    num_its = n / 2;
      /* This is sqrt( pi/2 ) erf( R/sqrt(2) ). */
    last_term = M_SQRT2 / M_2_SQRTPI * erf_cheb( radius * M_SQRT1_2 );

  } else {

    num_its = ( n + 1 ) / 2;
    last_term = 1.;

  }

  double_factorial = 1;
  rpow = pow( radius, n - 1 );
  n_current = n - 1;
  sum = 0.;

  /* Loop over terms in the series.  "num_its -1":  do the last term separately
     so that the last value of double_factorial is kept.  */
  for ( iter = 0; iter < num_its - 1; iter ++ ) {

    sum +=   rpow * double_factorial;
    rpow *= one_over_radius_sq; /* Reduce rpow by a factor of the radius sq. */

    double_factorial *= n_current;
    n_current -= 2;

  }

  sum += rpow * double_factorial;

  sum *= - exp( - radius_sq / 2 );
  sum += double_factorial * last_term;

  *result = sum;

  return 0;

}
