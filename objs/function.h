/*******************************************************************************
  File:  function.h
  Purpose:  header file for math functions.
*******************************************************************************/

int doub_factorial_int ( int n );

int factorial_int(int n );

float erf_cheb( float x );

float erfcomp_cheb( float x );

float linear_interp( int ndata, float *x_data, float *y_data, float x);

double doub_factorial_double( int n );

double factorial_double( int n );

double stirling4( int n );
