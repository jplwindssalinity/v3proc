/******************************************************************************
   Function:  doub_factorial_int
   Purpose:  Calculate n!! as an int.
   Author:  R. Glenister
******************************************************************************/

#include <stdio.h>
#define MAX_VALUE_FOR_INT_DOUB_FACT 19

int doub_factorial_int ( int n )

{

  if ( n > MAX_VALUE_FOR_INT_DOUB_FACT ) {

    fprintf( stderr, "doub_factorial_int:  input integer %d is greater than max.\n",
      MAX_VALUE_FOR_INT_DOUB_FACT );
    return 1;

  }

  if ( n > 1 ) {

    return ( n * doub_factorial_int ( n - 2 ) );

  } else if ( n == 0 || n == 1 ) {

    return 1;

  } else {

    fprintf( stderr, "doub_factorial_int:  input integer %d is negative.\n",
      n );

    return 1;

  }

}
