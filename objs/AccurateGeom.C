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

//-----------------//
// IntegrateSlices //
//-----------------//

int
IntegrateSlices(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	MeasSpot*		meas_spot,
	int				num_look_steps_per_slice,
	float			azimuth_integration_range,
	float			azimuth_step_size)
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
	meas_spot->time = instrument->time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);




	//------------------------------------------------------//
	// calculate commanded receiver gate delay and duration //
	//------------------------------------------------------//

	if (instrument->useRgc)
	{
		if (! instrument->rangeTracker.SetInstrument(instrument))
		{
			fprintf(stderr,
			"IntegrateSlices: error setting instrument using range tracker\n");
			return(0);
		}
	}
	else
	{
		instrument->commandedRxGateWidth = beam->receiverGateWidth;
		double rtt = IdealRtt(spacecraft, instrument);
		if (! RttToCommandedReceiverDelay(instrument, rtt))
			return(0);
	}

	//---------------------------------------//
	// calculate commanded Doppler frequency //
	//---------------------------------------//
	if (instrument->useDtc)
	{
		if (! instrument->dopplerTracker.SetInstrument(instrument))
		{
			fprintf(stderr,
			"IntegrateSlices: error setting instrument using Doppler tracker\n");
			return(0);
		}
	}
	else
	{
		if (! IdealCommandedDoppler(spacecraft, instrument))
			return(0);
	}

	//-------------------------------------------------//
	// find beam center                                //
	//-------------------------------------------------//

	double look, azimuth;

	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);


	//-------------------//
	// for each slice... //
	//-------------------//

	int total_slices = instrument->GetTotalSliceCount();
	for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
	{
		//-------------------------//
		// create a new measurment //
		//-------------------------//

		Meas* meas = new Meas();
		meas->pol = beam->polarization;

//		int debug=0, debug2=0;
//		if(slice_idx==0 && beam->polarization==H_POL) debug2=0;
//		if(debug) printf("\n\nAntennaAzimuth %g\n",
//		 antenna->azimuthAngle/dtr);

		//----------------------------------------//
		// determine the baseband frequency range //
		//----------------------------------------//

		float f1, bw, f2;
		instrument->GetSliceFreqBw(slice_idx, &f1, &bw);
		f2 = f1 + bw;

		//----------------------------------//
		// find the centroid of the slice	//
		// We'll simply use the peak gain	//
		// point at the central frequency	//
		//----------------------------------//

		EarthPosition centroid;
		Vector3 look_vector;

		float centroid_freq=(f1+f2)/2.0;

		// guess at a reasonable slice frequency tolerance of .1%
		float ftol = bw / 1000.0;
		double centroid_look=look;
		double centroid_azimuth=azimuth;
		float dummy;

		if (! FindPeakGainAtFreq(&antenna_frame_to_gc, spacecraft,
					 instrument,centroid_freq,ftol,
					 &centroid_look, &centroid_azimuth,
					 &dummy))
		return(0);


		look_vector.SphericalSet(1.0, centroid_look, centroid_azimuth);

                TargetInfoPackage tip;
		if(!TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
			look_vector, &tip))
		{
			return(0);
		}
		centroid=tip.rTarget;


		float looktol;
//		float xarray[40*80];
//		for(int c=0;c<40*80;c++)xarray[c]=0.0;

		/*** for now use azimuth range of 2 degrees ***/
		float azimin=centroid_azimuth-azimuth_integration_range/2.0;
		int numazi=(int)(azimuth_integration_range/azimuth_step_size);

		meas->value=0.0;

		//------------------------------------------//
		// Choose high gain side of slice			//
		// and direction of look angle increment	//
		//------------------------------------------//

		float high_gain_freq, low_gain_freq;
		int look_scan_dir;
		if(fabs(f2)>fabs(f1))
		{
			high_gain_freq=f1;
			low_gain_freq=f2;
			if(f1<f2) look_scan_dir=-1;
			else look_scan_dir=+1;
		}
		else
		{
			high_gain_freq=f2;
			low_gain_freq=f1;
			if(f1>f2) look_scan_dir=-1;
			else look_scan_dir=+1;
		}

		//-------------------------------------//
		// loop through azimuths and integrate //
		//-------------------------------------//


			for(int a=0; a<numazi;a++){
		  float azi=a*azimuth_step_size+azimin;

		  float start_look=centroid_look;
		  float end_look=centroid_look;

//		  if(debug) printf("For Azimuth %g ....\n",azi);

		  /*******************************/
                  /** find starting look angle ***/
                  /*******************************/
		  if(! FindLookAtFreq(&antenna_frame_to_gc,spacecraft,instrument,
				 high_gain_freq,ftol,&start_look,azi))
		    return(0);

		  /*******************************/
                  /** find ending look angle ***/
                  /*******************************/
		  if(! FindLookAtFreq(&antenna_frame_to_gc,spacecraft,instrument,
				 low_gain_freq,ftol,&end_look,azi))
		    return(0);

		  float lk=start_look;
                  looktol=fabs(end_look-start_look)/(float)num_look_steps_per_slice;
                  int look_num=0;
		  while(1){

                    // get integration box corners
		    Outline box;
                    float look1=lk;
		    float look2=lk+looktol*look_scan_dir;
		    float azi1=azi;
		    float azi2=azi+azimuth_step_size;
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

//		    if((look_scan_dir == 1 && tip.basebandFreq < low_gain_freq)
//		                           ||
//	      (look_scan_dir==-1 && tip.basebandFreq > low_gain_freq))

		    if(look_num>=num_look_steps_per_slice)
		      break;

//		    if(debug) printf("Look=%g Freq=%g \n",(look1+look2)/2.0,
//				     tip.basebandFreq);

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

//		    if(debug) printf("     Area %g Range %g GatGar %g \n",
//				     area, range, gatgar);
//		    if(debug) printf("     dX %g\n",
//				     area*gatgar/(range*range*range*range));

		    /*********************************/
		    /*** Add AG/R^4 to sum           */
                    /*********************************/

		    meas->value+=area*gatgar/(range*range*range*range);

                    /*********************************/
		    /* Goto next box                  */
		    /*********************************/
//                   float gt;
//		   if(debug2){
//		     beam->GetPowerGain((look1+look2)/2.0,(azi1+azi2)/2.0,&gt);
//		   }
//    Vector3 vector;
//    vector.SphericalSet(1.0,40.785*dtr,-0.36*dtr);
//    TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
//				 vector, &tip);
//    Vector3 gc_vector=antenna_frame_to_gc.Forward(vector);
//    EarthPosition r_target = earth_intercept(orbit_state->rsat, gc_vector);

//		   xarray[a*40+look_num]=gatgar;
		    lk+=look_scan_dir*looktol;
		    look_num++;
		  }
		}

//		if(debug2)fwrite(&xarray[0],sizeof(float),40*20,stdout);
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
