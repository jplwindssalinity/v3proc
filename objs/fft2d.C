// Take the fft of two-dimentional complex sequence of length n (power of 2)
// forward if inv = 0, inverse if inv = 1

#include <stdio.h>
#include <math.h>
#include "cplx.h"
#include "sizes.h"

void fft(complex *a, int m);
void ifft(complex *a, int m);

void fft2d(complex b[MaxArraySize][MaxArraySize], int n, int inv)
{
   int m, m2;
   int i,j;
   complex a[MaxArraySize];

/* find nextpow2 */

   m= 1;
   m2= pow(2,m);

   while (m2 < n)
   {
      m += 1;
      m2= pow(2,m);
   } 

/* 1d FFT across columns */

   for (i=1; i <= n; i++)
   {
      for (j=1; j <= n; j++)
      {
         a[j]= C_assign(b[i][j]);
      }

      if (inv == 0)
         fft(a,m);
      else
         ifft(a,m);

      for (j=1; j <= n; j++)
      {
         b[i][j]= C_assign(a[j]);
      }
   }

/* 1d FFT across rows */

   for (i=1; i <= n; i++)
   {
      for (j=1; j <= n; j++)
      {
         a[j]= C_assign(b[j][i]);
      }

      if (inv == 0)
         fft(a,m);
      else
         ifft(a,m);

      for (j=1; j <= n; j++)
      {
         b[j][i]= C_assign(a[j]);
      }
   }
}

