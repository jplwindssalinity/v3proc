/*******************************************************************************
  Function:  stirling4
  Purpose:  Calculate stirlings series approximation to N! to fourth order
            in 1/N.
  Author:  R. Glenister
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "function.h"

#define MAX_VALUE_FOR_DOUBLE_REP 170

#define COEFZ_0 1.
#define COEFZ_1_NUM 1.
#define COEFZ_1_DEN 12.
#define COEFZ_2_NUM 1.
#define COEFZ_2_DEN 288.
#define COEFZ_3_NUM -139.
#define COEFZ_3_DEN 51840.
#define COEFZ_4_NUM -571.
#define COEFZ_4_DEN 2488320.

double stirling4( int n ) 

{

  double factor, n_doub, one_over_n1, one_over_n2, one_over_n3, one_over_n4,
    series;

  if( n > 0 ) {

    if( n > MAX_VALUE_FOR_DOUBLE_REP ) {

      fprintf( stderr, "stirling4:  Value %d exceeds max representable.\n",
        n);
      return 1;

    }

    n_doub = ( double ) n;

    factor = pow( n_doub / M_E, n_doub ) * sqrt( 2 * M_PI * n_doub );

    one_over_n1 = 1. / n_doub;
    one_over_n2 = one_over_n1 * one_over_n1;
    one_over_n3 = one_over_n2 * one_over_n1;
    one_over_n4 = one_over_n3 * one_over_n1;

    series = COEFZ_0 + ( COEFZ_1_NUM / COEFZ_1_DEN ) * one_over_n1 +
      ( COEFZ_2_NUM / COEFZ_2_DEN ) * one_over_n2 +
      ( COEFZ_3_NUM / COEFZ_3_DEN ) * one_over_n3 +
      ( COEFZ_4_NUM / COEFZ_4_DEN ) * one_over_n4;
   
    return ( factor * series );

  } else if( n == 0 ) {
 
    return 1.;

  } else {

    fprintf( stderr, "stirling4:  Invalid input %d\n", n );
    return 1.;

  }
}
