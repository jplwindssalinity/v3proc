/*******************************************************************************
   Function:  doub_factorial_double
   Purpose:  Calculate n!! to double precision.
   Author:  R. Glenister
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include "function.h"

#define MAX_VALUE_FOR_INT_DOUB_FACT 19
#define MAX_VALUE_FOR_MULTIPLY 20
#define MAX_VALUE_FOR_DOUBLE_REP 170

double doub_factorial_double ( int n )

{

  int n_m_1_over_2, n_over_2;

  if ( n > 1 ) {

    if( n <= MAX_VALUE_FOR_INT_DOUB_FACT ) {

      return ( ( double ) doub_factorial_int ( n ) );

    } else if ( n <= MAX_VALUE_FOR_MULTIPLY ) {

      return ( n * doub_factorial_double( n - 2 ) );

    } else if ( n <= MAX_VALUE_FOR_DOUBLE_REP ) {

      if( ( n % 2 ) == 0 ) {

        n_over_2 = n / 2;

        return ( pow( 2., (double ) n_over_2 ) * stirling4( n_over_2 ) );

      } else {

        n_m_1_over_2 = ( n - 1 ) / 2;
 
        return ( stirling4( n ) /
          ( pow( 2., (double) n_m_1_over_2 ) * stirling4( n_m_1_over_2 ) ) );
      }

    } else {

      fprintf( stderr, 
        "doub_factorial_double:  Result from %d!! is not representable.\n", n );
      return -1.;

    }

  } else if ( n == 0 || n == 1 ) {

    return 1.;

  } else {

    fprintf( stderr, "doub_factorial_double:  input integer %d is negative.\n",
      n );

    return -1.;

  }

}
