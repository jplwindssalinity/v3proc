//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_accurategeom_c[] =
	"@(#) $Id$";

#include "BYUXTable.h"
#include "Qscat.h"
#include "Sigma0.h"
#include "InstrumentGeom.h"

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

//----------------------//
// BYUXTable::GetXTotal //
//----------------------//

float
BYUXTable::GetXTotal(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas)
{
  // true Es_cal based on true PtGr
  float Es_cal = qscat->ses.transmitPower * qscat->ses.rxGainEcho /
                 qscat->ses.loopbackLoss / qscat->ses.loopbackLossRatio *
                 qscat->ses.txPulseWidth;
  float X = GetXTotal(spacecraft, qscat, meas,
        Es_cal);
  return(X);
}

//----------------------//
// BYUXTable::GetXTotal //
//----------------------//

float
BYUXTable::GetXTotal(
    Spacecraft*   spacecraft,
    Qscat*        qscat,
    Meas*         meas,
    float         Es_cal)
{
  float X=GetX(spacecraft, qscat, meas);

  //--------------------------------------------------//
  // Compute the Xcal portion of the overall X factor.
  // Reference: IOM-3347-98-019.
  //--------------------------------------------------//

  double Xcal;
  radar_Xcal(qscat,Es_cal,&Xcal);

  return(X*Xcal); // Total X
}

//-----------------//
// BYUXTable::GetX //
//-----------------//

float
BYUXTable::GetX(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas)
{
  float delta_freq = GetDeltaFreq(spacecraft, qscat);
  float orbit_position = qscat->cds.OrbitFraction();
  int beam_number = qscat->cds.currentBeamIdx;
  float azim = qscat->sas.antenna.groundImpactAzimuthAngle;
  int sliceno = meas->startSliceIdx;
  return(GetX(beam_number,azim,orbit_position,sliceno,delta_freq));
}

//-------------------------//
// BYUXTable::GetDeltaFreq //
//-------------------------//

float
BYUXTable::GetDeltaFreq(
    Spacecraft*  spacecraft,
    Qscat*       qscat)
{
        //-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(qscat->sas.antenna);
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);
	int beam_number = qscat->cds.currentBeamIdx;

	//-------------------------------------------//
        // Determine Nominal Look and Azimuth Angles //
	//-------------------------------------------//

	float look;
    float azim = 0.0;

	switch(beam_number){
	case 0:
	  look=BYU_INNER_BEAM_LOOK_ANGLE*dtr;
	  azim+=BYU_INNER_BEAM_AZIMUTH_ANGLE*dtr;
	  break;
	case 1:
	  look=BYU_OUTER_BEAM_LOOK_ANGLE*dtr;
	  azim+=BYU_INNER_BEAM_AZIMUTH_ANGLE*dtr;
	  break;
	default:
	  fprintf(stderr,"BYUXTable:GetDeltaFreq: Bad Beam Number\n");
	  exit(0);
	}

        
	Vector3 nominal_boresight;
    nominal_boresight.SphericalSet(1.0,look,azim);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna, antenna->groundImpactAzimuthAngle);

        //--------------------------------//
        // Determine Delta Frequency      //
        //--------------------------------//

	TargetInfoPackage tip;
	if (! TargetInfo(&antenna_frame_to_gc, spacecraft, qscat,
        nominal_boresight, &tip))
    {
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


