//==========================================================//
// Copyright (C) 2000, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.		    //
//==========================================================//

static const char rcs_id_enof_table_c[] =
	"@(#) $Id$";

#include"Array.h"
#include"ENOFTable.h"
#include"math.h"

ENOF_Table::ENOF_Table()
  : table(NULL), numCrossTrackBins(76), numSpeedBins(25)
{
  return;
}

ENOF_Table::~ENOF_Table(){
  if(table!=NULL){
    free_array((void*)table,3,2,numCrossTrackBins,numSpeedBins);
    table=NULL;
  }
  return;
}

int ENOF_Table::Compute(
  float s0_sum[2], 
  int count[2], 
  int      cti, 
  float enof[2]){
  float repspeed=GetRepSpeed(s0_sum,count);
  enof[0]=Interpolate(repspeed,cti,0);
  enof[1]=Interpolate(repspeed,cti,1);
  //  printf("rpsd %g nof0 %g nof1 %g\n",repspeed,enof[0],enof[1]);
  return(1);
}

int ENOF_Table::Read(char * filename){
  if(table==NULL){
    if(!Allocate()) return(0);
  }
  FILE* ifp=fopen(filename,"r");
  if(ifp==NULL) return(0);
  for(int beam=1;beam>=0;beam--){
    for(int cti=0;cti<numCrossTrackBins;cti++){
      if(fread((void*)&(table[beam][cti][0]),sizeof(float),numSpeedBins,ifp)
	 !=(unsigned int)numSpeedBins){
	fclose(ifp);
	return(0);
      }
    }
  }
  fclose(ifp);
  return(1);
}

float ENOF_Table::Interpolate(
  float rep_speed,
  int   cti,
  int beam){
  float coef[2];
  int idx[2];
  speedIndex.GetLinearCoefsClipped(rep_speed, idx, coef); 
  float retval=table[beam][cti][idx[0]]*coef[0]+
    table[beam][cti][idx[1]]*coef[1];
  //printf("cti %d beam %d idx0 %d idx1 %d coef0 %g coef1 %g\n",
  // cti,beam,idx[0],idx[1],coef[0],coef[1]);
// printf("tab0 %g tab1 %g\n",table[beam][cti][idx[0]], table[beam][cti][idx[1]] ); 
  return(retval);
}

float ENOF_Table::GetRepSpeed(
  float s0_sum[2], 
  int count[2]){

  //     Initialization

  float rep_speed =  0.0;
  float v_pol_sigma0 = s0_sum[1]/count[1];

  //     Code branches based on whether a valid h_pol measurement is available.

  if (count[0]!=0){

    //     Calculates desensitized sigma0 value.  
    //     This can be done only when both
    //     h_pol and v_pol measurements are available.
      
         float h_pol_sigma0=s0_sum[0]/count[0];
         float sigma0_wo_rain =  v_pol_sigma0 - s0_coef[0] *
	   h_pol_sigma0 * ( 1.0 - h_pol_sigma0 / s0_coef[1]);

	 //    Tests rain desensitized sigma0 value against
	 //    maximum that can be applied to
	 //    wind speed model.

         if ( sigma0_wo_rain > s0_v_h_max ){

	   rep_speed = REP_SPEED_MAX;     

	   //     Calculates a representative wind speed based on 
	   //     rain desensitized sigma0.

	 }
         else {
          
            rep_speed = 
                wind_v_h_coef[0] * sqrt(sigma0_wo_rain) + 
                wind_v_h_coef[1] * sigma0_wo_rain       +
                wind_v_h_coef[2] * pow(sigma0_wo_rain,2.0)  +
                wind_v_h_coef[3] * pow(sigma0_wo_rain,3.0)  +
                wind_v_h_coef[4] * pow(sigma0_wo_rain,4.0);  
      
         }
  }
  else {

    //     Only v_pol sigma0s are available.  
    //     Sets the rain desensitized sigma0 to the
    //     value of the v_pol mean, and uses an alternative 
    //     model to determine a representative wind speed.

         float sigma0_wo_rain = v_pol_sigma0;

	 //  Calculates a representative wind speed based on rain 
	 // desensitized sigma0.

         if ( sigma0_wo_rain > s0_v_max ){

            rep_speed = REP_SPEED_MAX;     
         
	 }
         
	 else {

            rep_speed = 
                wind_v_coef[0] * sqrt(sigma0_wo_rain) + 
                wind_v_coef[1] * sigma0_wo_rain       +
                wind_v_coef[2] * pow(sigma0_wo_rain,2.0)  +
                wind_v_coef[3] * pow(sigma0_wo_rain,3.0)  +
                wind_v_coef[4] * pow(sigma0_wo_rain,4.0);  

         }       
      
  }
      
  return(rep_speed);
 
}

int ENOF_Table::Allocate(){
  table=(float***)make_array(sizeof(float),3,2,numCrossTrackBins,
			     numSpeedBins);
  speedIndex.SpecifyEdges(REP_SPEED_MIN,REP_SPEED_MAX,numSpeedBins);
  if(table==NULL) return(0);
  return(1);
}
