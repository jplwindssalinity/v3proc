/*******************************************************************************
  Function:  erf_cheb
  Purpose:  Calculate the error function to good accuracy using
             an approximate formula.  The worst case fractional error is less
             than 1.2 x 10^-7.
  Author:  R. Glenister
*******************************************************************************/
#include "function.h"



float erf_cheb( float x )

{

  return ( 1.0 - erfcomp_cheb( x ) );

}
