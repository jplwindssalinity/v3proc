//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_accurategeom_c[] =
    "@(#) $Id$";

#include "Spacecraft.h"
#include "Meas.h"
#include "InstrumentGeom.h"
#include "AccurateGeom.h"
#include "Qscat.h"
#include "Misc.h"

//-----------------//
// IntegrateSlices //
//-----------------//

int
IntegrateSlices(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    MeasSpot*    meas_spot,
    int          num_look_steps_per_slice,
    float        azimuth_integration_range,
    float        azimuth_step_size)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(qscat->sas.antenna);
	Beam* beam = qscat->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = qscat->cds.time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//-----------------------------------------------//
	// command the range delay and Doppler frequency //
	//-----------------------------------------------//

/*
    if (qscat->cds.useTracking)
    {
        // normal range and Doppler tracking
        qscat->cds.CmdRangeAndDoppler(&(qscat->sas), &(qscat->ses));
    }
    else
    {
        // ideal range and Doppler tracking
        fprintf(stderr,
            "Need to implement ideal range and Doppler tracking\n");
        exit(1);
    }
*/

	//------------------//
	// find beam center //
	//------------------//

	double look, azimuth;

	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);


	//-------------------//
	// for each slice... //
	//-------------------//

	int total_slices = qscat->ses.GetTotalSliceCount();
	for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
	{
		//-------------------------//
		// create a new measurment //
		//-------------------------//

		Meas* meas = new Meas();
		meas->pol = beam->polarization;

//		int debug=0, debug2=0;
//		if (slice_idx==0 && beam->polarization==H_POL) debug2=0;
//		if (debug) printf("\n\nAntennaAzimuth %g\n",
//		 antenna->azimuthAngle/dtr);

		//----------------------------------------//
		// determine the baseband frequency range //
		//----------------------------------------//

		float f1, bw, f2;
		qscat->ses.GetSliceFreqBw(slice_idx, &f1, &bw);
		f2 = f1 + bw;

		//----------------------------------//
		// find the centroid of the slice	//
		// We'll simply use the peak gain	//
		// point at the central frequency	//
		//----------------------------------//

		EarthPosition centroid;
		Vector3 look_vector;

		float centroid_freq=(f1+f2)/2.0;

		// guess at a reasonable slice frequency tolerance of 8 Hz
		float ftol = 8.0;
		double centroid_look=look;
		double centroid_azimuth=azimuth;
		float dummy;

		if (! FindPeakGainAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
            centroid_freq, ftol, &centroid_look, &centroid_azimuth, &dummy))
        {
            return(0);
        }

		look_vector.SphericalSet(1.0, centroid_look, centroid_azimuth);

		TargetInfoPackage tip;
		if (! TargetInfo(&antenna_frame_to_gc, spacecraft, qscat, look_vector,
            &tip))
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
		if (fabs(f2)>fabs(f1))
		{
			high_gain_freq=f1;
			low_gain_freq=f2;
			if (f1<f2) look_scan_dir=-1;
			else look_scan_dir=+1;
		}
		else
		{
			high_gain_freq=f2;
			low_gain_freq=f1;
			if (f1>f2) look_scan_dir=-1;
			else look_scan_dir=+1;
		}

		//-------------------------------------//
		// loop through azimuths and integrate //
		//-------------------------------------//

		for(int a=0; a<numazi;a++)
		{
            float azi=a*azimuth_step_size+azimin;

            float start_look=centroid_look;
            float end_look=centroid_look;

//		    if (debug) printf("For Azimuth %g ....\n",azi);

            //--------------------------//
            // find starting look angle //
            //--------------------------//

            if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
                high_gain_freq, ftol, &start_look,azi))
            {
                return(0);
            }

            //------------------------//
            // find ending look angle //
            //------------------------//

            if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
                low_gain_freq, ftol, &end_look,azi))
            {
                return(0);
            }

            float lk=start_look;
            looktol=fabs(end_look-start_look)/(float)num_look_steps_per_slice;
            int look_num=0;

            while(1)
            {
                // get integration box corners
                Outline box;
                float look1=lk;
                float look2=lk+looktol*look_scan_dir;
                float azi1=azi;
                float azi2=azi+azimuth_step_size;

                if (! FindBoxCorners(&antenna_frame_to_gc, spacecraft, qscat,
                    look1, look2, azi1, azi2, &box))
                {
                    return(0);
                }

		    //**************************//
		    //**** Determine Center of box ****//
		    //**************************//
		    Vector3 box_center;
                    box_center.SphericalSet(1.0, (look1+look2)/2.0,
					    (azi1+azi2)/2.0);

            //---------------------------------------------//
            // check to see if look angle scan is finished //
            //---------------------------------------------//

		    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, qscat,
                box_center, &tip))
            {
                return(0);
            }

		    if (look_num>=num_look_steps_per_slice)
		      break;

		    float gatgar, range, area;

		    /******************************/
                    /** Calculate Box Area        */
                    /******************************/
		    area=box.Area();
		    range=tip.slantRange;
            if (! PowerGainProduct(&antenna_frame_to_gc, spacecraft, qscat,
                (look1+look2)/2.0, (azi1+azi2)/2.0, &gatgar))
            {
                return(0);
            }

		    /*********************************/
		    /*** Add AG/R^4 to sum           */
                    /*********************************/

		    meas->value+=area*gatgar/(range*range*range*range);

                    /*********************************/
		    /* Goto next box                  */
		    /*********************************/
		    lk+=look_scan_dir*looktol;
		    look_num++;
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

//----------------//
// IntegrateSlice //
//----------------//

int
IntegrateSlice(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    int          num_look_steps_per_slice,
    float        azimuth_integration_range,
    float        azimuth_step_size,
    int          range_gate_clipping,
    float*       X)
{
        *X=0.0;

    //-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(qscat->sas.antenna);
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//----------------------------------------//
	// determine the baseband frequency range //
	//----------------------------------------//

	float f1, bw, f2;

	int slice_count = qscat->ses.GetTotalSliceCount();
    int slice_idx;
	if (! rel_to_abs_idx(meas->startSliceIdx,slice_count,&slice_idx))
    {
        fprintf(stderr,"IntegrateSlice: Bad slice number\n");
        exit(1);
	}
	qscat->ses.GetSliceFreqBw(slice_idx, &f1, &bw);
	f2 = f1 + bw;

	//------------------------------------------//
	// Choose high gain side of slice			//
	// and direction of look angle increment	//
	//------------------------------------------//

	float high_gain_freq, low_gain_freq;
	int look_scan_dir;
	if (fabs(f2)>fabs(f1))
    {
	    high_gain_freq=f1;
	    low_gain_freq=f2;
	    if (f1<f2) look_scan_dir=-1;
	    else look_scan_dir=+1;
    }
	else
    {
	    high_gain_freq=f2;
	    low_gain_freq=f1;
	    if (f1>f2) look_scan_dir=-1;
	    else look_scan_dir=+1;
    }

	//---------------------------------------------//
	// Determine look vector to centroid of slice  //
	//---------------------------------------------//

	Vector3 look_vector=meas->centroid - spacecraft->orbitState.rsat;
	look_vector=antenna_frame_to_gc.Backward(look_vector);
	double centroid_look, centroid_azimuth, dummy;
	look_vector.SphericalGet(&dummy, &centroid_look, &centroid_azimuth);

	//-------------------------------------//
	// loop through azimuths and integrate //
	//-------------------------------------//

	float azimin=centroid_azimuth-azimuth_integration_range/2.0;
	int numazi=(int)(azimuth_integration_range/azimuth_step_size);
	for(int a=0; a<numazi;a++)
    {
        float azi=a*azimuth_step_size+azimin;

	    float start_look=centroid_look;
	    float end_look=centroid_look;

        //--------------------------//
	    // find starting look angle //
        //--------------------------//

	    // guess at a reasonable slice frequency tolerance of 8 Hz
	    float ftol = 8.0;

	    if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
            high_gain_freq, ftol, &start_look,azi))
        {
            fprintf(stderr,
                "IntegrateSlice: Cannot find starting look angle\n");
            fprintf(stderr, "Probably means earth_intercept not found\n");
            return(0);
	    }

        //------------------------//
	    // find ending look angle //
        //------------------------//

	    if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
            low_gain_freq, ftol, &end_look,azi))
        {
            fprintf(stderr, "IntegrateSlice: Cannot find ending look angle\n");
            fprintf(stderr, "Probably means earth_intercept not found\n");
            return(0);
	    }

	    float lk=start_look;
	    float looktol=fabs(end_look-start_look)/(float)num_look_steps_per_slice;
	    int look_num=0;
	    while(1){

	      float range, gatgar, area, Pf;
	      /******************************/
	      /** Check to see if look angle */
	      /** scan is finished          */
	      /******************************/
	      if (look_num>=num_look_steps_per_slice)
		break;


	      //*******************************//
	      //* Determine box corners in look*//
	      //* and azimuth.                 *//
	      //********************************//

	      float look1=lk;
	      float look2=lk+looktol*look_scan_dir;
	      float azi1=azi;
	      float azi2=azi+azimuth_step_size;


	      //*******************************//
	      //* find location of box corners *//
	      //* on the ground.               *//
	      //********************************//
	      Outline box;

            if (! FindBoxCorners(&antenna_frame_to_gc, spacecraft, qscat,
                look1, look2, azi1, azi2, &box))
            {
                fprintf(stderr,"IntegrateSlice: Cannot find box corners\n");
                fprintf(stderr,"Probably means earth_intercept not found\n");
                return(0);
            }

	      //*******************************************//
	      //**** Determine Center of box and range ****//
	      //*******************************************//
	      Vector3 box_center;
	      box_center.SphericalSet(1.0, (look1+look2)/2.0,
					    (azi1+azi2)/2.0);
	      TargetInfoPackage tip;
	      if (! TargetInfo(&antenna_frame_to_gc, spacecraft, qscat,
              box_center, &tip))
          {
              fprintf(stderr,"IntegrateSlice: Cannot find box range\n");
              fprintf(stderr,"Probably means earth_intercept not found\n");
              return(0);
	      }
	      range=tip.slantRange;

	      /******************************/
	      /** Calculate Box Area        */
	      /******************************/
	      area=box.Area();

	      /******************************/
	      /* Calculate two-way gain     */
	      /******************************/

            if (! PowerGainProduct(&antenna_frame_to_gc, spacecraft, qscat,
                (look1+look2)/2.0, (azi1+azi2)/2.0, &gatgar))
            {
                fprintf(stderr, "IntegrateSlice: Cannot find box gain\n");
                fprintf(stderr,"Probably means earth_intercept not found\n");
                return(0);
            }

	      /*********************************/
	      /** Calculate portion of pulse   */
              /** received                     */
              /*********************************/

              Pf=1.0;
            if (range_gate_clipping)
                Pf = GetPulseFractionReceived(qscat, range);

	      /*********************************/
	      /*** Add AGPf/R^4 to sum         */
	      /*********************************/

	      *X+=area*gatgar/(range*range*range*range)*Pf;

	      /*********************************/
	      /* Goto next box                  */
	      /*********************************/

	      lk+=look_scan_dir*looktol;
	      look_num++;
	    }
	  }
	return(1);
}

//--------------------------//
// GetPulseFractionReceived //
//--------------------------//

float
GetPulseFractionReceived(
    Qscat*       qscat,
    float        range)
{
  double pulse_width = qscat->ses.txPulseWidth;
  float retval=0.0;
  double round_trip_time = 2.0 * range / speed_light_kps;

  double leading_edge_return_time=round_trip_time;
  double lagging_edge_return_time=round_trip_time+pulse_width;

    SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
    double start_gate_time = qscat->ses.rxGateDelay;
    double end_gate_time = qscat->ses.rxGateDelay + ses_beam_info->rxGateWidth;

  //*************************//
  // Case 1: Whole pulse is  //
  // in the gate.            //
  //*************************//

  if (leading_edge_return_time>start_gate_time){
    if (lagging_edge_return_time < end_gate_time) retval=1.0;

    //*************************//
    // Case 2: Pulse overlaps  //
    // end of gate.            //
    //*************************//
    else if (leading_edge_return_time<end_gate_time)
      retval=(end_gate_time-leading_edge_return_time)/pulse_width;

    //*************************//
    // Case 3: Whole Pulse is  //
    // past end of gate.       //
    //*************************//

    else retval=0;
  }

  //*************************//
  // Case 4: Pulse overlaps  //
  // start of gate.          //
  //*************************//
  else if (lagging_edge_return_time>start_gate_time){
    if (lagging_edge_return_time<end_gate_time)
      retval=(lagging_edge_return_time-start_gate_time)/pulse_width;

    //**************************//
    // Case 5: Whole range gate //
    // is within the pulse      //
    //**************************//
    else retval=(end_gate_time-start_gate_time)/pulse_width;
  }

  //*************************//
  // Case 6: Whole Pulse is  //
  // before start of gate.   //
  //*************************//
  else retval=0.0;

  return(retval);
}

//----------------//
// FindBoxCorners //
//----------------//

int
FindBoxCorners(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    Qscat*             qscat,
    float              look1,
    float              look2,
    float              azi1,
    float              azi2,
    Outline*           box)
{
  Vector3 vector;
  EarthPosition* corner;
  TargetInfoPackage tip;

  //----------------------------//
  // Deallocate old box         //
  //----------------------------//
  box->FreeContents();

    //----------------------------------------//
    // calculate first corner, add to outline //
    //----------------------------------------//

    vector.SphericalSet(1.0,look1,azi1);
    if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
        return(0);

  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

    //-----------------------------------------//
    // calculate second corner, add to outline //
    //-----------------------------------------//

    vector.SphericalSet(1.0,look1,azi2);
    if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
        return(0);

    corner=new EarthPosition;
    *corner=tip.rTarget;
    box->Append(corner);

  //---------------------------------------------//
  // Calculate third corner, add to Outline      //
  //---------------------------------------------//

    vector.SphericalSet(1.0,look2,azi2);
    if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
        return(0);

    corner=new EarthPosition;
    *corner=tip.rTarget;
    box->Append(corner);

  //---------------------------------------------//
  // Calculate fourth corner, add to Outline      //
  //---------------------------------------------//

    vector.SphericalSet(1.0,look2,azi1);
    if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
        return(0);

  corner=new EarthPosition;
  *corner=tip.rTarget;
  box->Append(corner);

  return(1);
}

//----------------//
// FindLookAtFreq //
//----------------//

int
FindLookAtFreq(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    Qscat*             qscat,
    float              target_freq,
    float              freq_tol,
    float*             look,
    float              azimuth)
{
  Vector3 vector;
  TargetInfoPackage tip;

  /**** Choose look angles on either side of the target frequency ***/
  float start_look=*look-2*dtr;
  float end_look=*look+2*dtr;
  float mid_look, actual_freq, dlookdfreq;
  float start_freq=target_freq-1;
  float end_freq=target_freq-1;
    while(1)
    {
        vector.SphericalSet(1.0,start_look,azimuth);
        if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
            return(0);

        start_freq=tip.basebandFreq;

        vector.SphericalSet(1.0,end_look,azimuth);
        if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
            return(0);

        end_freq=tip.basebandFreq;

        if (target_freq < start_freq && target_freq > end_freq) break;

        start_look -= dtr;
        end_look += dtr;
    }

    do
    {
        dlookdfreq=(end_look-start_look)/(end_freq-start_freq);
        mid_look=start_look + dlookdfreq*(target_freq-start_freq);
        vector.SphericalSet(1.0,mid_look,azimuth);
        if (! TargetInfo(antenna_frame_to_gc, spacecraft, qscat, vector, &tip))
            return(0);

        actual_freq=tip.basebandFreq;
        if (actual_freq < target_freq){
          end_look=mid_look;
          end_freq=actual_freq;
        }
        else{
          start_look=mid_look;
          start_freq= actual_freq;
        }
    } while(fabs(actual_freq-target_freq)>freq_tol);
  *look=mid_look;
  return(1);
}
