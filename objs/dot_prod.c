/*******************************************************************************
  Function:  dot_prod
  Purpose:  Calculate the dot product of two vectors.
  Author:  R. Glenister
*******************************************************************************/

float dot_prod( int ndata, float *data1, float *data2 )

{

  int idata;
  float sum = 0.;

  for( idata = 0; idata < ndata; idata ++ ) {

    sum += data1[ idata ] * data2[ idata ];

  }

  return sum;

}
