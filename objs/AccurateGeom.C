//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_accurategeom_c[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "Antenna.h"
#include "Matrix3.h"
#include "GenericGeom.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "LonLat.h"
#include "InstrumentGeom.h"
#include "AccurateGeom.h"
#include "Interpolate.h"
#include "Array.h"
#include "Misc.h"

//--------------//
// IntegrateSlices //
//--------------//

int
IntegrateSlices(
	double			time,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	int				slices_per_spot,
	MeasSpot*		meas_spot)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//------------------------//
	// determine slicing info //
	//------------------------//

	float total_freq = slices_per_spot * instrument->sliceBandwidth;
	float min_freq = -total_freq / 2.0;

	//-------------------------------------------------//
	// calculate Doppler shift and receiver gate delay //
	//-------------------------------------------------//

	Vector3 vector;
	double look, azimuth;

	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);

	vector.SphericalSet(1.0, look, azimuth);
	TargetInfoPackage tip;
	RangeAndRoundTrip(&antenna_frame_to_gc, spacecraft, vector, &tip);

	if (! Get2WayElectricalBoresight(beam, tip.roundTripTime,
		instrument->antenna.spinRate,&look, &azimuth))
	{
		return(0);
	}

	vector.SphericalSet(1.0, look, azimuth);		// boresight
	DopplerAndDelay(&antenna_frame_to_gc, spacecraft, instrument, vector);

	//-------------------//
	// for each slice... //
	//-------------------//

	for (int slice_idx = 0; slice_idx < slices_per_spot; slice_idx++)
	{
		//-------------------------//
		// create a new measurment //
		//-------------------------//

		Meas* meas = new Meas();
		meas->pol = beam->polarization;
	
		//----------------------------------------//
		// determine the baseband frequency range //
		//----------------------------------------//

		float f1 = min_freq + slice_idx * instrument->sliceBandwidth;
		float f2 = f1 + instrument->sliceBandwidth;

		//--------------------------------------//
		// find the centroid of the slice       //
                // We'll simply use the peak gain point //
                // at the central frequency             //
		//--------------------------------------//

		EarthPosition centroid;
		Vector3 look_vector;

		float centroid_freq=(f1+f2)/2.0;


		// guess at a reasonable slice frequency tolerance of .1%
		float ftol = fabs(f1 - f2) / 1000.0;
                double centroid_look=look;
                double centroid_azimuth=azimuth;
                float dummy;

		if (! FindPeakGainAtFreq(&antenna_frame_to_gc, spacecraft,
					 instrument,centroid_freq,ftol,
					 &centroid_look, &centroid_azimuth,
					 &dummy))
		  return(0);

		
		look_vector.SphericalSet(1.0, centroid_look, centroid_azimuth);
                if(!TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
			       look_vector, &tip))
		  return(0);
		centroid=tip.rTarget;
		
                /**** for now use looktol and azitol of .01 degrees ***/
		float looktol=0.1*dtr;
                float azitol=0.1*dtr;

		/*** for now use azimuth range of 10 degrees ***/
                float azirange=10.0*dtr;
                float azimin=centroid_azimuth-azirange/2.0;
		int numazi=(int)(azirange/azitol);

		meas->value=0.0;

		//----------------------------------------//
                // Choose high gain side of slice         //
                // and direction of look angle increment  //
                //----------------------------------------//
		float high_gain_freq;
		int look_scan_dir;
                if(fabs(f2)>fabs(f1)){
		  high_gain_freq=f1;
		  if(f1<f2) look_scan_dir=-1;
		  else look_scan_dir=+1;
		}
                else{
		  high_gain_freq=f2;	
		  if(f1>f2) look_scan_dir=-1;
		  else look_scan_dir=+1;
		}

                //--------------------------------------//
                // loop through azimuths and integrate  //
                //----------------------------------------//
                for(int a=0; a<numazi;a++){
		  float azi=a*azitol+azimin;
		  float start_look=centroid_look;

		  /*******************************/
                  /** find starting look angle ***/
                  /*******************************/
		  if(! FindLookAtFreq(&antenna_frame_to_gc,spacecraft,instrument,
				 high_gain_freq,ftol,&start_look,azi))
		    return(0);
		  float lk=start_look;
		  while(1){
                   
                    // get integration box corners
		    Outline box;
                    float look1=lk;
		    float look2=lk+looktol*look_scan_dir;
		    float azi1=azi;
		    float azi2=azi+azitol;
		    if (! FindBoxCorners(&antenna_frame_to_gc,spacecraft,instrument,
				 look1,look2,azi1,azi2, &box))
		      return(0);

		    
		  
		    //**************************//
		    //**** Determine Center of box ****//
		    //**************************//
		    Vector3 box_center;
                    box_center.SphericalSet(1.0, (look1+look2)/2.0,
					    (azi1+azi2)/2.0);
		                   
		    /******************************/
                    /** Check to see if look angle */
                    /** scan is finished          */
                    /******************************/

		    if(!TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
				  box_center, &tip))
		      return(0);

		    if((tip.basebandFreq > f1 && tip.basebandFreq > f2) ||
		      (tip.basebandFreq < f1 && tip.basebandFreq < f2)) break;
		    

		    float gatgar, range, area;

		    /******************************/
                    /** Calculate Box Area        */
                    /******************************/
		    area=box.Area();
		    range=tip.slantRange;
		    if(! PowerGainProduct(&antenna_frame_to_gc, spacecraft,
					    instrument, (look1+look2)/2.0,
					    (azi1+azi2)/2.0, &gatgar))
			return(0);

		      
		    /*********************************/
		    /*** Add AG/R^4 to sum           */
                    /*********************************/

		    meas->value+=area*gatgar/(range*range*range*range);

                    /*********************************/
		    /* Goto next box                  */
		    /*********************************/

		    lk+=look_scan_dir*looktol;
		    
		  }
		}

		//---------------------------//
		// generate measurement data //
		//---------------------------//

		// get local measurement azimuth
		CoordinateSwitch gc_to_surface =
			centroid.SurfaceCoordinateSystem();
		Vector3 rlook_surface = gc_to_surface.Forward(look_vector);
		double r, theta, phi;
		rlook_surface.SphericalGet(&r, &theta, &phi);
		meas->eastAzimuth = phi;

		// get incidence angle
		meas->incidenceAngle = centroid.IncidenceAngle(look_vector);
		meas->centroid = centroid;

		//-----------------------------//
		// add measurement to meas spot //
		//-----------------------------//

		meas_spot->Append(meas);
	}
	return(1);
}

int            
FindBoxCorners(CoordinateSwitch* antenna_frame_to_gc, 
	       Spacecraft* spacecraft, Instrument* instrument,
	       float look1, float look2, float azi1, float azi2,
	       Outline* box)
{
  Vector3 vector;
  EarthPosition* corner;
  TargetInfoPackage tip;


  //----------------------------//
  // Deallocate old box         //
  //----------------------------//
  box->FreeContents();

  //---------------------------------------------//
  // Calculate first corner, add to Outline      //
  //---------------------------------------------//

  vector.SphericalSet(1.0,look1,azi1);
  if(! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector, &tip))
    return(0);
  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

  //---------------------------------------------//
  // Calculate second corner, add to Outline      //
  //---------------------------------------------//

  vector.SphericalSet(1.0,look1,azi2);
  if(! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector, &tip))
    return(0);
  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

  //---------------------------------------------//
  // Calculate third corner, add to Outline      //
  //---------------------------------------------//

  vector.SphericalSet(1.0,look2,azi2);
  if(! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector, &tip))
    return(0);
  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

  //---------------------------------------------//
  // Calculate fourth corner, add to Outline      //
  //---------------------------------------------//

  vector.SphericalSet(1.0,look2,azi1);
  if(! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector, &tip))
    return(0);
  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

  return(1);
}

int             
FindLookAtFreq(CoordinateSwitch* antenna_frame_to_gc,
	       Spacecraft* spacecraft, Instrument* instrument,
	       float target_freq, float freq_tol, float* look,
	       float azimuth){
  Vector3 vector;
  TargetInfoPackage tip;

  /**** Choose look angles on either side of the target frequency ***/
  float start_look=*look-2*dtr;
  float end_look=*look+2*dtr;
  float mid_look, actual_freq, dlookdfreq;
  float start_freq=target_freq-1;
  float end_freq=target_freq-1;
  while(1){

    vector.SphericalSet(1.0,start_look,azimuth);
    if( ! TargetInfo(antenna_frame_to_gc,spacecraft,instrument,vector,&tip))
        return(0);
    start_freq=tip.basebandFreq;

    vector.SphericalSet(1.0,end_look,azimuth);
    if( ! TargetInfo(antenna_frame_to_gc,spacecraft,instrument,vector,&tip))
      return(0);
    end_freq=tip.basebandFreq;

    if(target_freq < start_freq && target_freq > end_freq) break;

    start_look -= dtr;
    end_look += dtr;
  }

  do{
    dlookdfreq=(end_look-start_look)/(end_freq-start_freq);
    mid_look=start_look + dlookdfreq*(target_freq-start_freq);
    vector.SphericalSet(1.0,mid_look,azimuth);
    if( ! TargetInfo(antenna_frame_to_gc,spacecraft,instrument,vector,&tip))
      return(0);
    actual_freq=tip.basebandFreq;
    if(actual_freq < target_freq){
      end_look=mid_look;
      end_freq=actual_freq;
    }
    else{
      start_look=mid_look;
      start_freq= actual_freq;
    }
    }while(fabs(actual_freq-target_freq)>freq_tol);
  *look=mid_look;
  return(1);
}




