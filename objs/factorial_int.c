/*******************************************************************************
   Function:  factorial_int
   Purpose:  Calculate n! as an int.
   Author:  R. Glenister
*******************************************************************************/

#include <stdio.h>
#define MAX_VALUE_FOR_INT_FACT 12

int factorial_int ( int n )

{

  if ( n > MAX_VALUE_FOR_INT_FACT ) {

    fprintf( stderr, "factorial_int:  input integer %d is greater than max.\n",
      MAX_VALUE_FOR_INT_FACT );
    return 1;

  }

  if ( n > 0 ) {

    return ( n * factorial_int ( n - 1 ) );

  } else if ( n == 0 ) {

    return 1;

  } else {

    fprintf( stderr, "factorial_int:  input integer %d is negative.\n",
      n );

    return 1;

  }

}
