//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_accurategeom_c[] =
	"@(#) $Id$";

#include "FbbTable.h"
#include "Qscat.h"
#include "Sigma0.h"
#include "InstrumentGeom.h"
#include "CheckFrame.h"

FbbTable::FbbTable()
  : a(NULL), b(NULL), c(NULL), d(NULL)
{
  _azimuthStepSize=two_pi/BYU_AZIMUTH_BINS;
  return;
}

int
FbbTable::Allocate()
{

  a=(float***)make_array(sizeof(float),3,BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS,BYU_AZIMUTH_BINS);
  if(a==NULL)return(0);

  b=(float***)make_array(sizeof(float),3,BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS,BYU_AZIMUTH_BINS);
  if(b==NULL)return(0);


  c=(float***)make_array(sizeof(float),3,BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS,BYU_AZIMUTH_BINS);
  if(c==NULL)return(0);


  d=(float***)make_array(sizeof(float),3,BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS,BYU_AZIMUTH_BINS);
  if(d==NULL)return(0);

  return(1);
}

int
FbbTable::Deallocate()
{
  free_array((void*)a,3,BYU_NUM_BEAMS,
             BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
  free_array((void*)b,3,BYU_NUM_BEAMS, 
             BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
  free_array((void*)c,3,BYU_NUM_BEAMS,
             BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
  free_array((void*)d,3,BYU_NUM_BEAMS, 
             BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
  return(1);
}

FbbTable::~FbbTable()
{
  if(a!=NULL)
    Deallocate();
  a=NULL;
  return;
}


int 
FbbTable::Read( 
       const char*         ibeam_file, 
       const char*         obeam_file)
{
  FILE* ifp[2];
  ifp[0]=fopen(ibeam_file,"r");
  if(ifp[0]==NULL){
    fprintf(stderr,"Cannot open FbbTable file %s\n",ibeam_file);
    return(0);
  }
  ifp[1]=fopen(obeam_file,"r");
  if(ifp[1]==NULL){
    fprintf(stderr,"Cannot open FbbTable file %s\n",obeam_file);
    return(0);
  }
  //-------------------------------------------------//
  // Create  Arrays                                  //
  //-------------------------------------------------//
  if(!Allocate()) return(0);

  //-------------------------------------------------//
  //  Loop through beams                             //
  //-------------------------------------------------//
  for(int bm=0; bm<2;bm++){
    //--------------------------------------------------//
    //   Loop through Orbit Times                       //
    //--------------------------------------------------//
    for(int o=0;o<BYU_ORBIT_POSITION_BINS;o++){
      //----------------------------------------------------//
      // Loop through Azimuths                              //
      //----------------------------------------------------//
      for(int ah=0;ah<BYU_AZIMUTH_BINS;ah++){
        char string[20];

        // Sanity Check on Orbit and azimuth;
	float orbit,azimuth;
        fscanf(ifp[bm],"%s",string);
	orbit=atof(string);
        fscanf(ifp[bm],"%s",string);
        azimuth=atof(string);
	if(fabs(o*BYU_TIME_INTERVAL_BETWEEN_STEPS-orbit)>0.01 ||
	   fabs(ah*_azimuthStepSize -azimuth*dtr) > 0.0001){
	  fprintf(stderr,"FbbTable::Read Error Sanity Check failed.\n");
	  return(0);
	}

	// A
	fscanf(ifp[bm],"%s",string);
	a[bm][o][ah]=atof(string);

	// B
	fscanf(ifp[bm],"%s",string);
	b[bm][o][ah]=atof(string);

	// C
	fscanf(ifp[bm],"%s",string);
	c[bm][o][ah]=atof(string);

	// D
	fscanf(ifp[bm],"%s",string);
	d[bm][o][ah]=atof(string);

      }
    }    
  }
  return(1);
}



//------------------//
// FbbTable::GetFbb //
//------------------//

float
FbbTable::GetFbb(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    CheckFrame*  cf)
{
  BYUXTable hack;
  float delta_freq = hack.GetDeltaFreq(spacecraft, qscat, cf);
  float orbit_position = qscat->cds.OrbitFraction();
  int beam_number = qscat->cds.currentBeamIdx;
  float azim = qscat->sas.antenna.groundImpactAzimuthAngle;
  return(GetFbb(beam_number,azim,orbit_position,delta_freq));
}


float 
FbbTable::GetFbb(
		int           beam_number, 
		float         azimuth_angle, 
		float         orbit_position, 
		float         delta_freq)
{
  // Convert from orbit position to nominal orbit time
  float orbit_time=BYU_NOMINAL_ORBIT_PERIOD*orbit_position;

  // Interpolate tables
  float A=Interpolate(a[beam_number],orbit_time, azimuth_angle);
  float B=Interpolate(b[beam_number],orbit_time, azimuth_angle);
  float C=Interpolate(c[beam_number],orbit_time, azimuth_angle);
  float D=Interpolate(d[beam_number],orbit_time, azimuth_angle);


  // Frequency Compensate
  float delta_bin=delta_freq/FFT_BIN_SIZE;
  float fbb=A+B*delta_bin+C*delta_bin*delta_bin
    +D*delta_bin*delta_bin*delta_bin;
  return(fbb);
}

float 
FbbTable::Interpolate(
    float**         table, 
    float           orbit_time,
    float           azimuth_angle)
{	
        // calculate floating point index
	float fazi = azimuth_angle / _azimuthStepSize;
        float ftime = orbit_time   / BYU_TIME_INTERVAL_BETWEEN_STEPS;
        
	// calculate indices (don't worry about range)
	int a1 = (int)fazi;
	int t1= (int)ftime;
        int a2= a1 + 1;
        int t2= t1+ 1;

	// calculate coefficients
        float ca1,ca2,ct1,ct2;
	ca1 = (float)a2 - fazi;
	ca2 = fazi - (float)a1;
	if(t1<BYU_ORBIT_POSITION_BINS-1){
	  ct1 = (float)t2 - ftime;
	  ct2 = ftime - (float)t1;
	}
	else{
	  float end_t=BYU_NOMINAL_ORBIT_PERIOD/BYU_TIME_INTERVAL_BETWEEN_STEPS;
	  ct1= (end_t-ftime)/(end_t-(float)t1);
          ct2= (ftime-(float)t1)/(end_t-(float)t1);
	}

	// wrap indices into range
        t2%=BYU_ORBIT_POSITION_BINS;
        a2%=BYU_AZIMUTH_BINS;
        
	float retval=ct1*ca1*table[t1][a1]+ct1*ca2*table[t1][a2]
	             +ct2*ca1*table[t2][a1]+ct2*ca2*table[t2][a2];
	return(retval);
}

