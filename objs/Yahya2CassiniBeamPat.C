// Routine to convert Yahya to Cassini (AmbigSim) style beam pattern

#include "Yahya2CassiniBeamPat.h"

void Yahya2CassiniBeamPat(float gain[], float gain_in[], YahyaAntenna Ant)
{
   float pi=3.141592653589793;

   float YahyaPat1[MaxAngIncSize2][3],YahyaPat2[MaxAngIncSize2][3];
   int NPatSize;

   float spacing_in_rad,max_angle,min_angle,angle_step,angle;  
   float beam_elev[MaxAngIncSize],beam_azi[MaxAngIncSize];

   float theta[MaxAngIncSize2],phi[MaxAngIncSize2],azi[MaxAngIncSize2];
   float elev[MaxAngIncSize2],elev_in[MaxAngIncSize2];
   float x[MaxAngIncSize2],y[MaxAngIncSize2],z[MaxAngIncSize2];

   int i,j,k,M; 
   int index_i,index_i_in,index_j;

   float offset;


/* Get Yahya antenna beam pattern */

   YahyaBeamPattern(YahyaPat1,YahyaPat2,NPatSize,Ant);


   spacing_in_rad= 0.05*pi/180.0;

   max_angle= 5.0;  
   min_angle= -5.0;
   angle_step= 0.05;

   M= (int) ((max_angle - min_angle)/angle_step + 1);

   for (i=1; i <= M; i++)
   {
      angle= min_angle + (i-1)*angle_step;
      beam_elev[i]= angle;
      beam_azi[i]= angle;
   }

   for (i=1; i <= NPatSize; i++)
   {
      theta[i]= YahyaPat1[i][1]*pi/180.0;
      phi[i]= YahyaPat1[i][2]*pi/180.0;

      x[i]= sin(theta[i])*cos(phi[i]);
      y[i]= sin(theta[i])*sin(phi[i]);
      z[i]= cos(theta[i]);

      elev[i]= asin(y[i]);
      elev_in[i]= -elev[i];

      azi[i]= asin(x[i]/cos(elev[i]));

      elev[i]= elev[i]*180.0/pi; /* -5 to +5 */
      elev_in[i]= elev_in[i]*180.0/pi; /* -5 to +5 */   
      azi[i]= azi[i]*180.0/pi; /* -5 to +5 */

      index_i= (int) ( (elev[i] - min_angle) / angle_step ) + 1;
      index_i_in= (int) ( (elev_in[i] - min_angle) / angle_step ) + 1;
      index_j=  (int) ( (azi[i] - min_angle) / angle_step ) + 1;

      if (index_i <= 0) index_i= 1;
      if (index_i_in <= 0) index_i_in=1;
      if (index_j <= 0) index_j=1;
      if (index_i > M) index_i=M;
      if (index_i_in>M) index_i_in=M;
      if (index_j > M) index_j=M;
   
      /* (azi-ele) */
      gain[index_j,index_i]= YahyaPat1[i][3];
      gain_in[index_j,index_i_in]= YahyaPat1[i][3];
   }

   /* fill up zero values in gain */
   
   offset= 5;

   for (i=1; i <= M -offset; i++)
   {
      for (j=1; j <= M; j++) 
      {  
         if (gain[i,j] == 0.0)
         {
            for (k=1; k <= offset; k++)
            {
               if (gain[i+k,j] != 0.0)
               {
                  gain[i,j] = gain[i+k,j];
                  break;
               }
             }
          }
       }
    }
      
   for (i= M-offset; i <= M; i++)
   {
      for (j=1; j <= M; j++) 
      {  
         if (gain[i,j] == 0.0)
         {
            for (k=1; k <= 2*offset; k++)
            {
               if (gain[i-k,j] != 0.0)
               {
                  gain[i,j] = gain[i-k,j];
                  break;
               }
             }
          }
       }
    }

   for (i=1; i <= M; i++)
   {
      for (j=1; j <= M-offset; j++) 
      {  
         if (gain[i,j] == 0.0)
         {
            for (k=1; k <= 2*offset; k++)
            {
               if (gain[i,j+k] != 0.0)
               {
                  gain[i,j] = gain[i,j+k];
                  break;
               }
             }
          }
       }
    }

      
   /* fill up zero values in gain_in */
   
   offset= 5;

   for (i=1; i <= M -offset; i++)
   {
      for (j=1; j <= M; j++) 
      {  
         if (gain_in[i,j] == 0.0)
         {
            for (k=1; k <= offset; k++)
            {
               if (gain_in[i+k,j] != 0.0)
               {
                  gain_in[i,j] = gain_in[i+k,j];
                  break;
               }
             }
          }
       }
    }
      
   for (i= M-offset; i <= M; i++)
   {
      for (j=1; j <= M; j++) 
      {  
         if (gain_in[i,j] == 0.0)
         {
            for (k=1; k <= 2*offset; k++)
            {
               if (gain_in[i-k,j] != 0.0)
               {
                  gain_in[i,j] = gain_in[i-k,j];
                  break;
               }
             }
          }
       }
    }

   for (i=1; i <= M; i++)
   {
      for (j=1; j <= M-offset; j++) 
      {  
         if (gain_in[i,j] == 0.0)
         {
            for (k=1; k <= 2*offset; k++)
            {
               if (gain_in[i,j+k] != 0.0)
               {
                  gain_in[i,j] = gain_in[i,j+k];
                  break;
               }
            }
         }
      }
   }
}   

         
      












