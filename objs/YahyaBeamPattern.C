// Find the antenna pattern for an offset parabolic reflector antenna

#include <stdio.h>
#include <math.h>
#include "cplx.h"
#include "YahyaBeamPattern.h"

void YahyaBeamPattern(float pattern1[][3], float pattern2[][3], int NPatSize, YahyaAntenna Ant)
{
   int NFFT= MaxArraySize;
//   complex ex[NFFT][NFFT], ey[NFFT][NFFT]
   complex ex[MaxArraySize][MaxArraySize], ey[MaxArraySize][MaxArraySize];
   complex ephi, etheta, ecross;
   complex jay;
   int i,j,ii,jj;

   float alam,ak,diam,flen,bw1,bw2,theta0,disp,blockage;

   float thstar,x0,delta;
   int ndim;

   float x,y,r,apx,apy,thetap,phip,theta,phi,rho,field;
   float power_max,power,eymax,alpha,co,cross,vtheta,vphi,sx,sy;
   int imax,jmax,i0,j0;
   complex ctmp,ctmp1,ctmp2;
   float theta1,phi1;

   float vcross;

   alam= Ant.GetAlam();
   ak=2*3.14159/alam;
   diam= Ant.GetDiam();
   flen= Ant.GetFlen();
   bw1= 0.0175*Ant.GetBw1();
   bw2= 0.0175*Ant.GetBw2();
   theta0= 0.0175*Ant.GetTheta0();
   disp= Ant.GetDisp();
   blockage= Ant.GetBlockage();

   jay= C_cplx(0.0,1.0);

   for (i=1; i <= NFFT; i++)
   {
      for (j=1; j <= NFFT; j++)
      {
         ex[i][j]= C_cplx(0.0,0.0);
         ey[i][j]= C_cplx(0.0,0.0);
      }
   }

/* angle to edge */

   thstar= atan(diam/(2.0*flen - pow(diam,2)/(8.0*flen)));

/* Newton's method to get thstar for offset */

   for (i=1; i <= 5; i++)
   {
      thstar=thstar+(diam*cos(thstar)-4.*flen*sin(thstar)+diam*cos(theta0))/(diam*sin(thstar)+4.*flen*cos(thstar));
   }

   x0=2.*flen*sin(theta0)/(cos(theta0)+cos(thstar));

   delta= alam/2.0;
   ndim= (int) diam/delta;

/* Find field over aperture, note that i increases along
   x dir and j along y; assume y-polarized feed */

   for (i=(NFFT-ndim)/2; i <= (NFFT+ndim)/2; i++)
   {
      for (j=(NFFT-ndim)/2; j <= (NFFT+ndim)/2; j++)
      {
         x=(i-NFFT/2+1)*delta+x0;
	 y=(j-NFFT/2+1)*delta;
	 r = sqrtf(pow(x,2)+pow(y,2));

         if (sqrtf(pow((x-x0),2)+pow(y,2)) > diam/2.0)
         {
            apx= 0.0;
            apy= 0.0;
         }
         else
         {
            thetap = atan(r/(flen-pow(r,2)/(4.*flen)));

	    if ((y ==0.0) && (x == 0.0)) 
     	       phip=0.;
	    else
	       phip = atan2(y,x);

            theta=acos(sin(theta0)*sin(thetap)*cos(phip)+cos(theta0)*cos(thetap));
            phi=atan2(sin(phip)*sin(thetap),sin(thetap)*cos(phip)*cos(theta0)-cos(thetap)*sin(theta0));
            rho=2.*flen/(1.+cos(thetap)); 

            field= sqrtf(antnna(theta,phi,bw1,bw2))/rho; /* scalar field */

            /* now assume y-polarized feed */

            apx=field*sin(phip)*cos(phip)*(1.-cos(thetap))/sqrtf(1.-pow(sin(phip),2)*pow(sin(thetap),2));
            apy=-field*(pow(sin(phip),2)*cos(thetap)+pow(cos(phip),2))/sqrtf(1.-pow(sin(phip),2)*pow(sin(thetap),2));
         }

         if (r < blockage/2.0) /* zero field over feed */
         {
            apx= 0.0;
            apy= 0.0;
         }

         /* field modified for off-axis feed on Petzval surface
            from Ruze 1965; amplitude unchanged by disp */
         
         ctmp1= C_cplx(apx,0.0); 
         ctmp2= C_exp(-ak*disp*x/flen/(1.+pow((r/(2.*flen)),2)));
         ex[i][j]= C_mul(ctmp1,ctmp2);      
    
         ctmp1= C_cplx(apy,0.0);
         ctmp2= C_exp(-ak*disp*x/flen/(1.+pow((r/(2.*flen)),2)));
         ey[i][j]= C_mul(ctmp1,ctmp2);
      }
   }

/* Calculate the P-vector (see Stuzman and Thiele, p. 382-383) */

   fft2d(ex,NFFT,0);
   fft2d(ey,NFFT,0);
  
/* Calculate antenna patterns, phi = 0 is x direction: phi = 90 plane
   we expect xpol mainly in phi=90 plane (iff offset) and only plot that */

   power_max= 10.0;
   for (i=1; i <= NAngSizeP; i++)
   {
      for (j=1; j <= NAngSizeP; j++)
      {
         power= pow(C_mag(ey[i][j]),2);
         if (power > power_max)
         {
            power_max= power;
            imax= i;
            jmax= j;
         }
      }
  } 

   eymax= sqrtf(power_max);
   alpha= asin((imax-1)*alam/(NFFT*delta));

   co= 0.0;
   cross= 0.0;

   for (i=-200; i <= 200; i++)
   {
      theta=asin((i-1)*alam/(NFFT*delta));

      if (i >= 1) j=i;
      if (i < 1) j=1;

      ctmp.r= eymax;
      ctmp.i= 0.0;
      etheta= C_div(C_mul(jay,ey[imax][j]),ctmp); /* co-pol, phi=90 */
      ecross= C_div(C_mul(jay,ex[imax][j]),ctmp); /* x-pol, phi=90 */
      ephi= C_div(C_mul(jay,ey[j][1]),ctmp); /* co-pol, phi=0 */

      vtheta= C_mag(etheta);
      if (vtheta <= 1.0e-5) vtheta= 1.0e-5;

      vphi= C_mag(ephi);
      if (vphi <= 1.0e-5) vphi= 1.0e-5;

      vcross= C_mag(ecross);
      if (vcross <= 1.0e-5) vcross= 1.0e-5;

      if (abs(i) <  45) 
      {
         co= co + pow(vtheta,4);
         cross= cross + pow(vcross,2)*pow(vtheta,2);
      }
   }

/* Store 2-D pattern

   ii= 0;

   for (i = NAngSizeN; i <= NAngSizeP; i++)
   {
      sx=((i-1)*alam/(float(NFFT)*delta));

      if (i >= 1) i0= i;
      if (i < 1) i0= i+NFFT;

      for (j = NAngSizeN; j <= NAngSizeP, j++)
      {
         sy=((j-1)*alam/(float(NFFT)*delta));

         if (j >= 1) j0= j;
         if (j < 1) j0= j+NFFT;

         theta= asin(sqrtf(pow(sx,2)+pow(sy,2)));
         phi= atan2(sy,sx);

         theta1= acos(sin(theta)*cos(phi)*sin(alpha) + cos(theta)*cos(alpha));
         phi1= atan2(sin(theta)*cos(phi)*cos(alpha)  - cos(theta)*sin(alpha),-sin(theta)*sin(phi));

         vy=C_mag(C_div(ey[i0,j0],ctmp));

         if (vy <= 1.0e-5) vy= 1.0e-5;

         pattern1[ii][0]= 57.3*theta1;
         pattern1[ii][1]= 57.3*phi1;
         pattern1[ii][2]= 20.0*alog10(vy);

         pattern2[ii][0]= 57.3*theta;
         pattern2[ii][1]= 57.3*phi;
         pattern2[ii][2]= 20.0*alog10(vy);

         ii += 1;
      }
   }

   NPatSize= NAngSizeP - NAngSizeN + 1; /* number of rows in the antenna pattern */
}









      







      
      














            

