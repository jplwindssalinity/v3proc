/******************************************************************************

  Function:  sphere_normal_prob
  Purpose:  Calculate spherical normal probability, given an ndata-dimensioned
    vector of values.
  Author:  R. Glenister

******************************************************************************/

#include <stdio.h>
#include <math.h>
#include "function.h"

float dot_prod( int ndata, float *data1, float *data2);

int int_rn_exp( int n, float radius, float * result );

int sphere_normal_prob( int ndata, float * scaled_data, float * probability )

{

  int ndata_o_2, retint;
  float factor, integral, sum;
  double radius, radius_sq;


  /* Calculate the square of the radius of the vector. */
  radius_sq = (double) dot_prod( ndata, scaled_data, scaled_data );

  if( radius_sq >= 0. ) {

    radius = sqrt( radius_sq );

  } else {

    *probability = 0.;
    fprintf( stderr, "sphere_normal_prob:  Error.  Radius^2 = %f < 0.\n",
      radius_sq );
    return 1;

  }

  /* Calculate I_{n - 1} . */
  retint = int_rn_exp( ndata - 1, radius, &integral);

  sum = integral;

  /* Now multiply by the normalization:  n/( 2^(n/2) n/2 ! ) for n even
                                         sqrt( 2 / PI ) / (n-2)!! for n odd */

  if( ( ndata % 2 ) == 0 ) {

    ndata_o_2 = ndata / 2;
    factor = ndata / ( pow( 2., (double) ndata_o_2 ) * 
      factorial_double( ndata_o_2 ) );

  } else {

    factor = (float) ( sqrt( 2. / M_PI ) / doub_factorial_double( ndata - 2 ) );

  }

  sum *= factor;

  *probability = sum;
  return 0;

}
