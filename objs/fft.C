// Take the FFT of complex sequence of length 2^m //

#include <math.h>
#include "cplx.h"

void fft(complex *a, int m)
{
   complex u,w,t;
   int n,nm1,i,j;

   n= pow(2,m);
   nm1= n-1;

   j= 1;

   for (i=1; i <= nm1; i++)
   {
      if (i < j)
      {
         complex t;
         
         t= C_assign(a[j]);
         a[j]= C_assign(a[i]);
         a[i]= C_assign(t);
      }
	
      {
         int k;

         k= n/2;
	
         while (k <= j)
         {
            j -= k;
            k /= 2;
         }

         j += k;
      }
   }

   {
      float pi=3.141592653589793;
      int i,j,l,le,le1,ip;
      complex u,w,t;

      
      for (l=1; l <= m; l++)
      {

         le= pow(2,l);
         le1= le/2;
         u= C_cplx(1.0,0.0);
         w= C_cplx(cos(pi/le1),sin(pi/le1));
  
         for (j=1; j<=le1; j++)
         {
            for (i=j; i<=le; i+=n)
            {
               ip= i+le1;
               t= C_mul(a[ip],u);
               a[ip]= C_sub(a[i],t);
               a[i]= C_add(a[i],t);
            }
            u= C_mul(u,w);
         }
      }
   }
}         
      



	