/*******************************************************************************
   Function:  factorial_double
   Purpose:  Calculate n! to double precision.
   Author:  R. Glenister
*******************************************************************************/

#include <stdio.h>
#include "function.h"

#define MAX_VALUE_FOR_INT_FACT 12
#define MAX_VALUE_FOR_MULTIPLY 20
/* MAX_VALUE_FOR_MULTIPLY = greatest n such that n! fits in a double. */
#define MAX_VALUE_FOR_DOUBLE_REP 170

double factorial_double ( int n )

{

  if ( n > 0 ) {

    if ( n <= MAX_VALUE_FOR_INT_FACT ) {

      /* This is a faster multiply than float. */
      return ( ( double ) factorial_int( n ) );

    } else if ( n <= MAX_VALUE_FOR_MULTIPLY ) {

      return ( (double ) n * factorial_double ( n - 1 ) );

    } else if ( n <= MAX_VALUE_FOR_DOUBLE_REP ) {

      return stirling4( n );

    } else {

      fprintf( stderr, 
        "factorial_double:  input integer %d gives unrepresentable result.\n",
        n );

      return 1;

    }

  } else if ( n == 0 ) {

   return 1.;

  } else {

    /* n is negative. */

      fprintf( stderr,
    "factorial_double:  input integer %d gives unrepresentable result.\n",
    n );

    return 1;

  }
}
