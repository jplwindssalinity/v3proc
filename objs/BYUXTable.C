//==========================================================//
// Copyright (C) 1997, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.		    //
//==========================================================//

static const char rcs_id_accurategeom_c[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "Antenna.h"
#include "InstrumentGeom.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "LonLat.h"
#include "Meas.h"
#include "XTable.h"
#include "BYUXTable.h"

BYUXTable::BYUXTable()
{
  return;
}

BYUXTable::~BYUXTable()
{
  return;
}

int 
BYUXTable::Read( 
       const char*         ibeam_file, 
       const char*         obeam_file)
{
  FILE * ibeam_fp=fopen(ibeam_file,"r");
  if(ibeam_fp==NULL){
    fprintf(stderr,"Cannot open BYU X Factor file %s\n",ibeam_file);
    return(0);
  }
  FILE * obeam_fp=fopen(obeam_file,"r");
  if(obeam_fp==NULL){
    fprintf(stderr,"Cannot open BYU X Factor file %s\n",obeam_file);
    return(0);
  }
  if(!(xnom.ReadBYU(ibeam_fp,obeam_fp)))  return(0);
  if(!(a.ReadBYU(ibeam_fp,obeam_fp))) return(0);
  if(!(b.ReadBYU(ibeam_fp,obeam_fp))) return(0);
  if(!(c.ReadBYU(ibeam_fp,obeam_fp))) return(0);
  if(!(d.ReadBYU(ibeam_fp,obeam_fp))) return(0);
  return(1);
}

float
BYUXTable::GetXTotal(
     Spacecraft*         spacecraft,
     Instrument*         instrument,
     Meas*               meas)
{
  float X=GetXTotal(spacecraft,instrument,meas,instrument->transmitPower*
		     instrument->echo_receiverGain);
  return(X);
}

float
BYUXTable::GetXTotal(
     Spacecraft*         spacecraft,
     Instrument*         instrument,
     Meas*               meas,
     float               PtGr)
{
  float X=GetX(spacecraft,instrument,meas);
  int beam_number = instrument->antenna.currentBeamIdx;  
  float Gp2;
  if(beam_number==0) Gp2=pow(10.0,2*0.1*G0H);
  else Gp2=pow(10.0,2*0.1*G0V);
  float lambda = speed_light_kps / instrument->transmitFreq;
  X*=PtGr*lambda*lambda*Gp2;
  X/=64*pi*pi*pi*instrument->systemLoss;
  return(X);
}

float
BYUXTable::GetX(
     Spacecraft*         spacecraft,
     Instrument*         instrument,
     Meas*               meas)
{
  float delta_freq = GetDeltaFreq(spacecraft,instrument);
  float orbit_position = instrument->OrbitFraction();
  int beam_number = instrument->antenna.currentBeamIdx;
  float azim = instrument->antenna.azimuthAngle;
  int sliceno = meas->startSliceIdx;
  return(GetX(beam_number,azim,orbit_position,sliceno,delta_freq));
}


float
BYUXTable::GetDeltaFreq(
     Spacecraft*         spacecraft,
     Instrument*         instrument)
{

        //-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);
	int beam_number = antenna->currentBeamIdx;
  

	//-------------------------------------------//
        // Determine Nominal Look and Azimuth Angles //
	//-------------------------------------------//

	float look, azim;
	switch(beam_number){
	case 0:
	  look=BYU_INNER_BEAM_LOOK_ANGLE*dtr;
	  break;
	case 1:
	  look=BYU_OUTER_BEAM_LOOK_ANGLE*dtr;
	  break;
	default:
	  fprintf(stderr,"BYUXTable:GetDeltaFreq: Bad Beam Number\n");
	  exit(0);
	}

	azim=0.5*IdealRtt(spacecraft,instrument)*instrument->antenna.actualSpinRate+ instrument->antenna.azimuthAngle;
        
	Vector3 nominal_boresight;
        nominal_boresight.SphericalSet(1.0,look,azim);
        Attitude att;
	att.Set(0.0, 0.0, instrument->antenna.azimuthAngle, 1, 2, 3);
        CoordinateSwitch antennaPedestalToAntennaFrame(att);
        nominal_boresight=antennaPedestalToAntennaFrame.Forward(nominal_boresight);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);


        //--------------------------------//
        // Determine Delta Frequency      //
        //--------------------------------//

	TargetInfoPackage tip;
	if(!TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
			     nominal_boresight, &tip)){
		fprintf(stderr,"BYUXTable::GetDeltaFreq failed\n");
                fprintf(stderr,"Probably means earth_intercept not found\n");
		exit(1);		
	}
	return(tip.basebandFreq);
}


float 
BYUXTable::GetX(
		int           beam_number, 
		float         azimuth_angle, 
		float         orbit_position, 
		int           slice_number, 
		float         delta_freq)
{
  float X=xnom.RetrieveByRelativeSliceNumber(beam_number,azimuth_angle,orbit_position,slice_number);
  float A=a.RetrieveByRelativeSliceNumber(beam_number,azimuth_angle,orbit_position,slice_number);
  float B=b.RetrieveByRelativeSliceNumber(beam_number,azimuth_angle,orbit_position,slice_number);
  float C=c.RetrieveByRelativeSliceNumber(beam_number,azimuth_angle,orbit_position,slice_number);
  float D=d.RetrieveByRelativeSliceNumber(beam_number,azimuth_angle,orbit_position,slice_number);

  float delta_bin=delta_freq/FFT_BIN_SIZE;
  X+=A+B*delta_bin+C*delta_bin*delta_bin+D*delta_bin*delta_bin*delta_bin;
  X=pow(10.0,0.1*X);
  return(X);
}


