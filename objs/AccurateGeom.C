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
#include "Array.h"

#define OUTPUT_DETAILED_INFO 0

#define SPECTRAL_RESPONSE_DELTA_FREQ   8314.0  //Hz
#define SPECTRAL_RESPONSE_FREQ_TOL     0.5     //Hz
#define SPECTRAL_RESPONSE_TOLERANCE    3e-8    
#define SPECTRAL_RESPONSE_NUM_LOOK_STEPS 4
#define INTEGRATE_SLICE_FREQ_TOL       8.0     //Hz
//----------------//
// IntegrateSlice //
//----------------//

int
IntegrateSlice(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    int          num_look_steps_per_slice,
    double        azimuth_integration_range,
    double        azimuth_step_size,
    int          range_gate_clipping,
    float*       X)
{
     int retval;
	//----------------------------------------//
	// determine the baseband frequency range //
	//----------------------------------------//

     float f1, bw;

     int slice_count = qscat->ses.GetTotalSliceCount();
     int slice_idx;
     if (! rel_to_abs_idx(meas->startSliceIdx,slice_count,&slice_idx))
       {
        fprintf(stderr,"IntegrateSlice: Bad slice number\n");
        exit(1);
       }
     qscat->ses.GetSliceFreqBw(slice_idx, &f1, &bw);

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
        attitude, antenna, antenna->txCenterAzimuthAngle);

     //---------------------------------------------//
     // Determine look vector to centroid of slice  //
     //---------------------------------------------//

     Vector3 look_vector=meas->centroid - spacecraft->orbitState.rsat;
     look_vector=antenna_frame_to_gc.Backward(look_vector);
     double centroid_look, centroid_azimuth, dummy;
     look_vector.SphericalGet(&dummy, &centroid_look, &centroid_azimuth);

     double X_doub;
     retval=IntegrateFrequencyInterval(spacecraft,qscat,f1,centroid_look,
				centroid_azimuth,
				bw,num_look_steps_per_slice,
				azimuth_integration_range,
				azimuth_step_size,range_gate_clipping, 
				INTEGRATE_SLICE_FREQ_TOL,
				&X_doub);
     *X=X_doub;
     return(retval);
}

//----------------------------//
// IntegrateFrequencyInterval //
//----------------------------//

int
IntegrateFrequencyInterval(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    double       f1,
    double       centroid_look,
    double       centroid_azimuth,
    double       bw,
    int          num_look_steps_per_slice,
    double       azimuth_integration_range,
    double       azimuth_step_size,
    int          range_gate_clipping,
    double        ftol,
    double*       X)
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
		attitude, antenna, antenna->txCenterAzimuthAngle);


        //-------------------------------//
        // Get ending frequency          //
        //-------------------------------//

        double f2 = f1 + bw;


	if(! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
			    (f1+f2)/2.0,ftol,&centroid_look,centroid_azimuth))
	  return(0);

	//-------------------------------------//
	// loop through azimuths and integrate //
	//-------------------------------------//

	double azimin=centroid_azimuth-azimuth_integration_range/2.0;
	int numazi=(int)(azimuth_integration_range/azimuth_step_size);
	for(int a=0; a<numazi;a++)
    {
        double azi=a*azimuth_step_size+azimin;

	    double start_look=centroid_look;
	    double end_look=centroid_look;

        //--------------------------//
	// find starting look angle //
        //--------------------------//


	    if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
            f1, ftol, &end_look,azi))
        {
            fprintf(stderr,
                "IntegrateSlice: Cannot find ending look angle\n");
            fprintf(stderr, "Probably means earth_intercept not found\n");
            return(0);
	    }

        //------------------------//
        // find ending look angle //
        //------------------------//

	    if (! FindLookAtFreq(&antenna_frame_to_gc, spacecraft, qscat,
            f2, ftol, &start_look,azi))
        {
            fprintf(stderr, "IntegrateSlice: Cannot find starting look angle\n");
            fprintf(stderr, "Probably means earth_intercept not found\n");
            return(0);
	    }

	    double lk=MIN(start_look,end_look);
	    double looktol=fabs(end_look-start_look)/(double)num_look_steps_per_slice;
	    int look_num=0;
	    while(1){

	      double range, gatgar, area, Pf;
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

	      double look1=lk;
	      double look2=lk+looktol;
	      double azi1=azi;
	      double azi2=azi+azimuth_step_size;


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

            if (! SpatialResponse(&antenna_frame_to_gc, spacecraft, qscat,
                (look1+look2)/2.0, (azi1+azi2)/2.0, &gatgar, 1))
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


	    if(OUTPUT_DETAILED_INFO){
	      double alt, lon, lat;
	      tip.rTarget.GetAltLonGDLat(&alt,&lon,&lat);
	      double report_azim=antenna->groundImpactAzimuthAngle;
	      double rtt=IdealRtt(spacecraft,qscat);
              report_azim += rtt * antenna->spinRate/2.0;
              double gp2;
              int beam_number=qscat->cds.currentBeamIdx;
	      double peak_gain = qscat->sas.antenna.beam[beam_number].peakGain;
              gp2=peak_gain*peak_gain;
	      printf("\nDetailed Info %g %g %g %g %g %g %g %g %g\n",
	          (look1+look2)/2.0*rtd,(azi1+azi2)/2.0*rtd,
	           lat*rtd,lon*rtd,gatgar/gp2, range, tip.dopplerFreq,
	          tip.basebandFreq, report_azim*rtd);
 
	    }
	      /*********************************/
	      /*** Add AGPf/R^4 to sum         */
	      /*********************************/

	      *X+=area*gatgar/(range*range*range*range)*Pf;

	      /*********************************/
	      /* Goto next box                  */
	      /*********************************/
              
              lk+=looktol;
	      look_num++;
	    }
	  }
	return(1);
}

//--------------------------//
// GetPulseFractionReceived //
//--------------------------//

double
GetPulseFractionReceived(
    Qscat*       qscat,
    double        range)
{
  double pulse_width = qscat->ses.txPulseWidth;
  double retval=0.0;
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
    double              look1,
    double              look2,
    double              azi1,
    double              azi2,
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
    double              target_freq,
    double              freq_tol,
    double*             look,
    double              azimuth)
{
  Vector3 vector;
  TargetInfoPackage tip;

  /**** Choose look angles on either side of the target frequency ***/
  double start_look=*look-2*dtr;
  double end_look=*look+2*dtr;
  double mid_look, actual_freq, dlookdfreq;
  double start_freq=target_freq-1;
  double end_freq=target_freq-1;
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

int      
SpectralResponse(
     Spacecraft*          spacecraft, 
     Qscat*               qscat, 
     double                freq, 
     double                azim,
     double                look,
     double*              response)
{
     //--------------------------------//
     // Predigest                      //
     //--------------------------------//
     OrbitState* orbit_state=&(spacecraft->orbitState);
     Attitude* attitude=&(spacecraft->attitude);
     Antenna* antenna=&(qscat->sas.antenna);

     //--------------------------------//
     // generate the coordinate switch //
     //--------------------------------//

     CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->txCenterAzimuthAngle);

     double f1;
     f1=freq-SPECTRAL_RESPONSE_DELTA_FREQ/2.0; 
     // Guess at centroid look using electrical boresight look angle.
     // Integrate FrequencyInterval refines the guess.

     if(!IntegrateFrequencyInterval(spacecraft,qscat,f1,look,azim,
				SPECTRAL_RESPONSE_DELTA_FREQ,
				SPECTRAL_RESPONSE_NUM_LOOK_STEPS,
				qscat->cds.azimuthIntegrationRange,
				qscat->cds.azimuthStepSize,0,
				SPECTRAL_RESPONSE_FREQ_TOL,
                                response)) return(0);
     

     return(1);
}


int
GetPeakSpectralResponse(
    CoordinateSwitch* antenna_frame_to_gc,
    Spacecraft*       spacecraft,
    Qscat*            qscat,
    double*           look,
    double*           azim){

  /*** Temporarily change Doppler and Range flags and initialize commanded Doppler
       and range gate delay to BYU boresight. ****/

  // save old qscat values
  Qscat old_qscat=*qscat;

  qscat->cds.useBYUDop=1;
  qscat->cds.useSpectralDop=0;
  qscat->cds.useBYURange=1;
  qscat->cds.useSpectralRange=0;
  SetDelayAndFrequency(spacecraft,qscat);


  /***** Initialize look and azim to BYU Boresight value **/
  if(! GetBYUBoresight(spacecraft,qscat,look,azim)){
	    return(0);
  }

  /***** Set up arrays to use in NegativeSpectralResponse ***/     
  int ndim = 1;
  double** p = (double**)make_array(sizeof(double),2,ndim+1,ndim+2);
  if (p == NULL)
    {
      printf("Error allocating memory in GetPeakSpectralResponse\n");
      return(0);
    }  
  double freq=0;
  p[0][0] = freq;
  p[1][0] = freq+10000.0;
  p[0][1] = *azim;
  p[1][1] = *azim;
  p[0][2] = *look;
  p[1][2] = *look;

  NegSpecParam other_params;
  other_params.spacecraft=spacecraft;
  other_params.qscat=qscat;
  void* ptr=(void*)(& other_params);

  double rtol = SPECTRAL_RESPONSE_TOLERANCE;
  double ftol = SPECTRAL_RESPONSE_FREQ_TOL;
  downhill_simplex((double**)p, ndim, ndim+2, rtol,
		   NegativeSpectralResponse, ptr,ftol);

  freq = p[0][0];
  float dummy;
  if(!FindPeakResponseAtFreq(antenna_frame_to_gc,spacecraft,qscat,freq,
			     ftol,look,azim,&dummy)) return(0);
  
  free_array(p,2,ndim+1,ndim+2);

  // Reinstate old cds values
  *qscat=old_qscat;
  return(1);
}



//--------------------------//
// NegativeSpectralResponse //
//--------------------------//

//
// NegativeSpectralResponse is the function to be minimized by
// GetPeakSpectralResponse.
// It computes the negative of the SpectralResponse for the inputs given
// in the input vector.  The elements of the input vector are:
//
// x[0] = baseband freq (Hz)  
// x[1] = azimuth angle (rad)
// x[2] = look angle(rad) used to intialize search for starting and ending
//        look angles.            
// other_params = pointer to structure containing:
// pointers to the spacecraft and qscat objects 


double
NegativeSpectralResponse(
	double*		x,
	void*		ptr)
{
        double response;
        NegSpecParam* other_params= (NegSpecParam*)ptr;

        if(!SpectralResponse(other_params->spacecraft, other_params->qscat,
			     x[0],x[1],x[2],&response))
        {
            fprintf(stderr,"Error:NegativeSpectralResponse function failed\n");
            exit(1);
        }	

	return(-(double)response);
}






