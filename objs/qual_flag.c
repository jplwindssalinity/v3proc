/*******************************************************************************
   Function:  driver (main)
   Purpose:  Driver to check sphere_normal_prob
   Author:  R. Glenister
*******************************************************************************/

#include <stdio.h>
#define BAD_VALUE -100
#define AT_WIDTH 1624
#define CT_WIDTH 76
#define NUM_MEAS 4

int sphere_normal_prob( int ndata, float * scaled_data, float * probability );

int main()

{

  float s0_meas[1624][76][4];
  float s0_calc[1624][76][4];
  float std[1624][76][4];
  float prob_array[1624][76];
  float array[4];
  int a,c,n, bad_cell;
  float prob;
 
  fread(s0_meas, sizeof(float), CT_WIDTH * AT_WIDTH * NUM_MEAS, stdin);
  fread(s0_calc, sizeof(float), CT_WIDTH * AT_WIDTH * NUM_MEAS, stdin);
  fread(std, sizeof(float), CT_WIDTH * AT_WIDTH * NUM_MEAS, stdin);

  for(a=0;a<AT_WIDTH;a++){
    for(c=0;c<CT_WIDTH;c++){
      bad_cell=0;
      for(n=0;n<NUM_MEAS;n++){
        if(s0_meas[a][c][n]==BAD_VALUE){
	  bad_cell=1;
          prob_array[a][c]=-3;
          break;
	}
	array[n]=s0_meas[a][c][n]-s0_calc[a][c][n];
        array[n]/=std[a][c][n];
	/*** printf("%d %d %d s0_meas %g s0_calc %g std %g val %g\n",a,c,n,s0_meas[a][c][n],s0_calc[a][c][n],std[a][c][n],array[n]); **/
      }
      if(!bad_cell){
	if( sphere_normal_prob(NUM_MEAS,array,&prob) ){
	  prob_array[a][c]=-3;
	}
	else prob_array[a][c]=prob;
	/*** printf("PROB %g\n",prob); ***/
      }
    }
  }
  
  fwrite(prob_array, sizeof(float), CT_WIDTH * AT_WIDTH, stdout); 
  return 0;
}
