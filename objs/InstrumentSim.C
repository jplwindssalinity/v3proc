//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "GenericGeom.h"
#include "AccurateGeom.h"
#include "InstrumentGeom.h"
#include "Ephemeris.h"
#include "Sigma0.h"
#include "Constants.h"
#include "Kpm.h"
#include "CheckFrame.h"

//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	startTime(0.0), l00FrameReady(0), uniformSigmaField(0),
	outputXToStdout(0), useKfactor(0), useBYUXfactor(0), createXtable(0),
	rangeGateClipping(0), applyDopplerError(0), simVs1BCheckfile(NULL),
	_spotNumber(0)
{
	return;
}

InstrumentSim::~InstrumentSim()
{
	return;
}

//---------------------------//
// InstrumentSim::Initialize //
//---------------------------//

int
InstrumentSim::Initialize(
	Antenna*	antenna)
{
	for (int i = 0; i < antenna->numberOfBeams; i++)
	{
		_scatBeamTime[i] = startTime + antenna->beam[i].timeOffset;
	}
	return(1);
}

//-----------------------------------//
// InstrumentSim::DetermineNextEvent //
//-----------------------------------//

int
InstrumentSim::DetermineNextEvent(
	Antenna*			antenna,
	InstrumentEvent*	instrument_event)
{
	//----------------------------------------//
	// find minimum time from possible events //
	//----------------------------------------//

	int min_idx = 0;
	double min_time = _scatBeamTime[0];
	for (int i = 1; i < antenna->numberOfBeams; i++)
	{
		if (_scatBeamTime[i] < min_time)
		{
			min_idx = i;
			min_time = _scatBeamTime[i];
		}
	}

	//-----------------------//
	// set event information //
	//-----------------------//

	instrument_event->time = min_time;
	instrument_event->eventId =
		InstrumentEvent::SCATTEROMETER_MEASUREMENT;
	instrument_event->beamIdx = min_idx;

	//----------------------------//
	// update next time for event //
	//----------------------------//

	double cycle_start_time = min_time - antenna->beam[min_idx].timeOffset;
	int cycle_idx = (int)((cycle_start_time - startTime) /
			antenna->priPerBeam + 0.5);
	_scatBeamTime[min_idx] = startTime +
		(double)(cycle_idx + 1) * antenna->priPerBeam +
		antenna->beam[min_idx].timeOffset;

	return(1);
}

//--------------------------------------//
// InstrumentSim::UpdateAntennaPosition //
//--------------------------------------//

int
InstrumentSim::UpdateAntennaPosition(
	Instrument*		instrument)
{
	antennaSim.UpdatePosition(instrument->time, &(instrument->antenna));
	return(1);
}

//--------------------------------//
// InstrumentSim::SetMeasurements //
//--------------------------------//

int
InstrumentSim::SetMeasurements(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	MeasSpot*		meas_spot,
	CheckFrame*		cf,
	WindField*		windfield,
	GMF*			gmf,
	Kp*				kp,
	KpmField*		kpmField)
{
	//-------------------------//
	// for each measurement... //
	//-------------------------//

	int slice_count = instrument->GetTotalSliceCount();
	int slice_i = 0;
	int sliceno = -slice_count/2;
        Meas* meas=meas_spot->GetHead();
	while(meas)
	{
		meas->startSliceIdx = sliceno;

		//----------------------------------------//
		// get lon and lat for the earth location //
		//----------------------------------------//

		double alt, lat, lon;
		if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
			return(0);

		LonLat lon_lat;
		lon_lat.longitude = lon;
		lon_lat.latitude = lat;

                // Compute Land Flag
                meas->landFlag=landMap.IsLand(lon,lat);

		float sigma0;
                if(meas->landFlag==1){
		  // Set sigma0 to average NSCAT land sigma0 for appropriate
                  // incidence angle and polarization
		        if (meas->pol==H_POL) sigma0=0.085;
			else sigma0=0.1;
			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = sigma0;
				cf->wv[slice_i].spd = 0.0;
				cf->wv[slice_i].dir = 0.0;
			}		  
		}
		else if (uniformSigmaField)
		{
			sigma0=1;
			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = 1.0;
				cf->wv[slice_i].spd = 0.0;
				cf->wv[slice_i].dir = 0.0;
			}
		}
		else
		{
			//-----------------//
			// get wind vector //
			//-----------------//

			WindVector wv;
			if (! windfield->InterpolatedWindVector(lon_lat, &wv))
			{
				wv.spd = 0.0;
				wv.dir = 0.0;
			}

			//--------------------------------//
			// convert wind vector to sigma-0 //
			//--------------------------------//

			// chi is defined so that 0.0 means the wind is blowing towards
			// the s/c (the opposite direction as the look vector)
			float chi = wv.dir - meas->eastAzimuth + pi;

			gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
				chi, &sigma0);

			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = sigma0;
				cf->wv[slice_i].spd = wv.spd;
				cf->wv[slice_i].dir = wv.dir;
			}

			//---------------------------------------------------------------//
			// Fuzz the sigma0 by Kpm to simulate the effects of model function
			// error.  The resulting sigma0 is the 'true' value.
			// It does not map back to the correct wind speed for the
			// current beam and geometry because the model function is
			// not perfect.
			//---------------------------------------------------------------//

			// Uncorrelated component.
			if (instrument->simUncorrKpmFlag == 1)
			{
				double kpm_value;
				if (! kp->kpm.GetKpm(meas->pol,wv.spd,&kpm_value))
				{
					printf("Error: Bad Kpm value in InstrumentSim::SetMeas\n");
					exit(-1);
				}
				Gaussian gaussianRv(1.0,0.0);
				float rv1 = gaussianRv.GetNumber();
				float RV = rv1*kpm_value + 1.0;
			    if (RV < 0.0)
    			{
        			RV = 0.0;   // Do not allow negative sigma0's.
    			}
				sigma0 *= RV;
			}

			// Correlated component.
			if (instrument->simCorrKpmFlag == 1)
			{
				sigma0 *= kpmField->GetRV(instrument->corrKpm, lon_lat);
			}
		}

		//-------------------------//
		// convert Sigma0 to Power //
		//-------------------------//

		// Kfactor: either 1.0, taken from table, or X is computed
        // directly
        float Xfactor=0;
        float Kfactor=1.0;
        float true_Es,true_En,var_esn_slice;
		CoordinateSwitch gc_to_antenna;

		if (computeXfactor || useBYUXfactor){
                  // If you cannot calculate X it probably means the
                  // slice is partially off the earth.
                  // In this case remove it from the list and go on
                  // to next slice
          if(computeXfactor){
		    if(!ComputeXfactor(spacecraft,instrument,meas,&Xfactor)){
		      meas=meas_spot->RemoveCurrent();
		      delete meas;
		      meas=meas_spot->GetCurrent();
		      slice_i++;
		      sliceno++;
		      if(slice_count%2==0 && sliceno==0) sliceno++;
		      continue;
		    }
		  }
		  else if(useBYUXfactor){
		      Xfactor=BYUX.GetXTotal(spacecraft,instrument,meas);
		  }
		  if (! sigma0_to_Esn_slice_given_X(instrument, meas,
				Xfactor, sigma0, &(meas->value),
                &true_Es, &true_En, &var_esn_slice))
		    {
		      return(0);
		    }
		  meas->XK=Xfactor;
		}
        else{
		  Kfactor=1.0;  // default to use if no Kfactor specified.
		  if (useKfactor)
		    {
		      float orbit_position = instrument->OrbitFraction();

		      Kfactor = kfactorTable.RetrieveByRelativeSliceNumber(
				 instrument->antenna.currentBeamIdx,
				 instrument->antenna.azimuthAngle,
				 orbit_position, sliceno);
		    }

		  //--------------------------------//
		  // generate the coordinate switch //
		  //--------------------------------//

		  gc_to_antenna = AntennaFrameToGC(
							  &(spacecraft->orbitState),
							  &(spacecraft->attitude),
							  &(instrument->antenna));
		  gc_to_antenna=gc_to_antenna.ReverseDirection();

		  if (! sigma0_to_Esn_slice(&gc_to_antenna,spacecraft,instrument,meas,
				Kfactor, sigma0, &(meas->value), &(meas->XK),
                &true_Es, &true_En, &var_esn_slice))
		    {
		      return(0);
		    }
		}

		if (simVs1BCheckfile)
		{
		    FILE* fptr = fopen(simVs1BCheckfile,"a");
            if (fptr == NULL)
            {
	          fprintf(stderr,"Error opening %s\n",simVs1BCheckfile);
              exit(-1);
            }

            double lambda = speed_light_kps / instrument->transmitFreq;
            Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
            cf->R[slice_i] = (float)rlook.Magnitude();
		    if (computeXfactor || useBYUXfactor)
            {
              // Antenna gain is not computed when using BYU X factor because
              // the X factor already includes the normalized patterns.
              // Thus, to see what it actually is, we need to do the geometry
              // work here that is normally done in radar_X() when using
              // the K-factor approach.
		      gc_to_antenna = AntennaFrameToGC(
							      &(spacecraft->orbitState),
							      &(spacecraft->attitude),
							      &(instrument->antenna));
		      gc_to_antenna=gc_to_antenna.ReverseDirection();
              double roundTripTime = 2.0 * cf->R[slice_i] / speed_light_kps;
              int ib = instrument->antenna.currentBeamIdx;
              Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
              double r, theta, phi;
              rlook_antenna.SphericalGet(&r,&theta,&phi);
              if(! instrument->antenna.beam[ib].GetPowerGainProduct(theta, phi,
                     roundTripTime,instrument->antenna.actualSpinRate,
                     &(cf->GatGar[slice_i])))
              {
                cf->GatGar[slice_i] = 1.0;	// set a dummy value. 
              }

            }
            else
            {
              cf->GatGar[slice_i] = meas->XK / Kfactor *
                (64*pi*pi*pi * cf->R[slice_i]*
                   cf->R[slice_i]*cf->R[slice_i]*cf->R[slice_i] *
                   instrument->systemLoss) /
                (instrument->transmitPower * instrument->echo_receiverGain *
                   lambda*lambda);
            }

            cf->var_esn_slice[slice_i] = var_esn_slice;
            cf->true_Es[slice_i] = true_Es;
            cf->true_En[slice_i] = true_En;
			cf->XK[slice_i] = meas->XK;
			cf->centroid[slice_i] = meas->centroid;
			cf->azimuth[slice_i] = meas->eastAzimuth;
			cf->incidence[slice_i] = meas->incidenceAngle;
			cf->AppendSliceRecord(fptr, slice_i, lon, lat);
                fclose(fptr);
		}

		sliceno++;
		slice_i++;
		if(slice_count%2==0 && sliceno==0) sliceno++;
		meas=meas_spot->GetNext();
	}

	return(1);
}

//---------------------------------//
// InstrumentSim::SetL00Spacecraft //
//---------------------------------//

int
InstrumentSim::SetL00Spacecraft(
	Spacecraft*		spacecraft, L00Frame* l00_frame)
{
	OrbitState* orbit_state = &(spacecraft->orbitState);

	double alt, lon, lat;
	if (! orbit_state->rsat.GetAltLonGDLat(&alt, &lon, &lat))
		return(0);

	l00_frame->gcAltitude = alt;
	l00_frame->gcLongitude = lon;
	l00_frame->gcLatitude = lat;
	l00_frame->gcX = orbit_state->rsat.Get(0);
	l00_frame->gcY = orbit_state->rsat.Get(1);
	l00_frame->gcZ = orbit_state->rsat.Get(2);
	l00_frame->velX = orbit_state->vsat.Get(0);
	l00_frame->velY = orbit_state->vsat.Get(1);
	l00_frame->velZ = orbit_state->vsat.Get(2);

	return(1);
}

//------------------------------//
// InstrumentSim::SetL00Science //
//------------------------------//

int
InstrumentSim::SetL00Science(
	MeasSpot*		meas_spot,
	CheckFrame*		cf,
	Instrument*		instrument,
	L00Frame*		l00_frame)
{
	Antenna* antenna = &(instrument->antenna);

	//----------//
	// set PtGr //
	//----------//
	// Only "noise it up" if simKpriFlag is set

	l00_frame->ptgr= instrument->transmitPower*instrument->echo_receiverGain;
	if (simVs1BCheckfile)
	{
		cf->ptgr = l00_frame->ptgr;
	}

	if (instrument->simKpriFlag)
		l00_frame->ptgr *= (1 + ptgrNoise.GetNumber(instrument->time));

	//----------------------//
	// set antenna position //
	//----------------------//

	l00_frame->antennaPosition[_spotNumber] =
		(unsigned short)antenna->GetEncoderValue();

	//-------------------------//
	// for each measurement... //
	//-------------------------//

	int slice_number = _spotNumber * l00_frame->slicesPerSpot;
	for (Meas* meas = meas_spot->GetHead(); meas;
		meas = meas_spot->GetNext())
	{
		//----------------------------//
		// update the level 0.0 frame //
		//----------------------------//

		l00_frame->science[slice_number] = meas->value;
		slice_number++;
	}

	// Compute the spot noise measurement.
	sigma0_to_Esn_noise(instrument,meas_spot,
		&(l00_frame->spotNoise[_spotNumber]));

	_spotNumber++;

	return(1);
}

//------------------------//
// InstrumentSim::ScatSim //
//------------------------//

int
InstrumentSim::ScatSim(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	WindField*		windfield,
	GMF*			gmf,
	Kp*				kp,
	KpmField*		kpmField,
	L00Frame*		l00_frame)
{
	CheckFrame cf;
	if (simVs1BCheckfile)
	{
		if (!cf.Allocate(instrument->scienceSlicesPerSpot))
		{
            fprintf(stderr,"Error allocating a CheckFrame\n");
			return(0);
		}
	}

	MeasSpot meas_spot;

	//----------------------------------------//
	// compute frame header info if necessary //
	//----------------------------------------//

	if (_spotNumber == 0)
	{
		//----------------------//
		// frame initialization //
		//----------------------//

		if (! SetL00Spacecraft(spacecraft,l00_frame))
			return(0);
		l00_frame->time = instrument->time;
		l00_frame->orbitTicks = instrument->orbitTicks;
		l00_frame->instrumentTicks = instrument->instrumentTicks;
		l00_frame->priOfOrbitTickChange = 255;		// flag value
	}

	//-------------------------------------------------------------//
	// command the range delay, range width, and Doppler frequency //
	//-------------------------------------------------------------//

	SetRangeAndDoppler(spacecraft, instrument);
        if(applyDopplerError){
	  instrument->SetCommandedDoppler(instrument->commandedDoppler+dopplerBias);
	}
	//---------------------//
	// locate measurements //
	//---------------------//

	if (instrument->scienceSlicesPerSpot <= 1)
	{
		if (! LocateSpot(spacecraft, instrument, &meas_spot))
			return(0);
	}
	else
	{
		if (! LocateSliceCentroids(spacecraft, instrument, &meas_spot))
		    return(0);

	}

	//------------------------//
	// set measurement values //
	//------------------------//

	if (! SetMeasurements(spacecraft, instrument, &meas_spot, &cf,
		windfield, gmf, kp, kpmField))
	{
		return(0);
	}


	//---------------------------------------//
	// Output X values to X table if enabled //
	//---------------------------------------//

	if (createXtable)
	{
		int sliceno = 0;
		for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			float orbit_position = instrument->OrbitFraction();

			if (!xTable.AddEntry(slice->XK,
				instrument->antenna.currentBeamIdx,
				instrument->antenna.azimuthAngle,
				orbit_position, sliceno))
			{
				return(0);
			}
			sliceno++;
		}
	}

	//---------------------------------//
	// Add Spot Specific Info to Frame //
	//---------------------------------//

	if (! SetL00Science(&meas_spot, &cf, instrument, l00_frame))
		return(0);

	//--------------------------------//
	// Output X to Stdout if enabled //
	//--------------------------------//

	
	if (outputXToStdout)
	{		
	        float XK_max=0;
	        for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			if(XK_max < slice->XK) XK_max=slice->XK;
		}
	        float total_spot_X=0;
                float total_spot_power=0;
 		for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		  {   
		    int slice_count = instrument->GetTotalSliceCount();     
		    int slice_idx;
		    if(!rel_to_abs_idx(slice->startSliceIdx,slice_count,&slice_idx)){
		      fprintf(stderr,"ScatSim: Bad slice number\n");
		      exit(1);
		    }
		    float dummy, freq;
		    instrument->GetSliceFreqBw(slice_idx, &freq, &dummy);

		    float gain=10*log10(slice->XK/XK_max);
                    float lambda=speed_light_kps/instrument->transmitFreq;
                    float kdb=instrument->transmitPower*instrument->echo_receiverGain*lambda*lambda/(64*pi*pi*pi*instrument->systemLoss);
		    kdb=10*log10(kdb);
                    if(instrument->antenna.currentBeamIdx==0)
		      kdb+=2*G0H;
		    else kdb+=2*G0V;
		    float XKdb=10*log10(slice->XK);
		    double range=(spacecraft->orbitState.rsat - 
 				     slice->centroid).Magnitude();
		    float rtt=2.0*range/speed_light_kps;
		    float pf=GetPulseFractionReceived(instrument,range);
		    //		    printf("%d %g %g %g %g %g %g %g\n",
		    //   instrument->antenna.currentBeamIdx,
		    //   instrument->antenna.azimuthAngle*rtd,freq,
		    //   gain, rtt, pf,slice->XK, slice->value);

		    printf("%g ",XKdb-kdb); //HACK
                    float delta_freq=BYUX.GetDeltaFreq(spacecraft,instrument);
                    //printf("%g ", delta_freq);
		    total_spot_power+=slice->value;
		    total_spot_X+=slice->XK;
			                
		  }
		printf("\n"); //HACK
		RangeTracker* rt= &(instrument->antenna.beam[instrument->antenna.currentBeamIdx].rangeTracker);
			

		unsigned short orbit_step=rt->OrbitTicksToStep(instrument->orbitTicks,
				   instrument->orbitTicksPerOrbit);

		//		printf("TOTALS %d %d %g %g %g %g\n",(int)orbit_step,
		//       instrument->antenna.currentBeamIdx,
		//      instrument->antenna.azimuthAngle*rtd,
		//      instrument->commandedRxGateDelay,
		//      total_spot_X,total_spot_power);
		fflush(stdout);
	}

	//---------------------------------//
	// set orbit tick change indicator //
	//---------------------------------//

	if (instrument->orbitTicks != l00_frame->orbitTicks)
		l00_frame->priOfOrbitTickChange = _spotNumber;

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	if (_spotNumber >= l00_frame->spotsPerFrame)
	{
		l00FrameReady = 1;	// indicate frame is ready
		_spotNumber = 0;	// prepare to start a new frame
	}
	else
	{
		l00FrameReady = 0;	// indicate frame is not ready
	}

    //--------------------------------//
    // Output data if enabled //
    //--------------------------------//


    if (simVs1BCheckfile)
    {
        FILE* fptr = fopen(simVs1BCheckfile,"a");
        if (fptr == NULL)
        {
            fprintf(stderr,"Error opening %s\n",simVs1BCheckfile);
            exit(-1);
        }
		cf.time = instrument->time;
		cf.rsat = spacecraft->orbitState.rsat;
		cf.vsat = spacecraft->orbitState.vsat;
		cf.attitude = spacecraft->attitude;
        cf.orbit_frac = instrument->OrbitFraction();
        cf.antenna_azi = instrument->antenna.azimuthAngle;

        // Compute earth intercept of electrical boresight (one-way).
	    Antenna* antenna = &(instrument->antenna);
	    Beam* beam = antenna->GetCurrentBeam();
        CoordinateSwitch antenna_to_gc = AntennaFrameToGC(
					      &(spacecraft->orbitState),
					      &(spacecraft->attitude),
					      antenna);
        double look,azim;
        beam->GetElectricalBoresight(&look,&azim);
        Vector3 boresight;
        boresight.SphericalSet(1.0, look, azim);
        Vector3 boresight_gc = antenna_to_gc.Forward(boresight);
        EarthPosition r_boresight;
        if (earth_intercept(spacecraft->orbitState.rsat, boresight_gc,
                       &r_boresight) != 1)
        {
          fprintf(stderr,"Error computing boresight position for checkframe\n");
          return(0);
        }
        cf.boresight_position = r_boresight;

		cf.AppendRecord(fptr);
        fclose(fptr);
    }

	return(1);
}

//---------------------------------//
// InstrumentSim::ComputeKfactor   //
//---------------------------------//
int
InstrumentSim::ComputeKfactor(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
        Meas*                   meas,
	float* Kf){
  
        if(!IntegrateSlice(spacecraft,instrument,meas,
			      numLookStepsPerSlice,azimuthIntegrationRange,
			      azimuthStepSize, rangeGateClipping, Kf))
	  return(0);	
	Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
        CoordinateSwitch gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState), 
							  &(spacecraft->attitude), &(instrument->antenna));
        gc_to_antenna=gc_to_antenna.ReverseDirection();
	double R = rlook.Magnitude();
	double roundTripTime = 2.0 * R / speed_light_kps;

	int ib = instrument->antenna.currentBeamIdx;
	Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
	double r, theta, phi;
	rlook_antenna.SphericalGet(&r,&theta,&phi);
	float GatGar;
	if(instrument->antenna.beam[ib].GetPowerGainProduct(theta, phi, roundTripTime,
	  instrument->antenna.actualSpinRate, &GatGar)!=1){
	  fprintf(stderr,"ComputeKfactor: Cannot calculate GatGar at centroid\n");
	  return(0);
	}


        (*Kf)*=R*R*R*R/GatGar;
        return(1);
}

//---------------------------------//
// InstrumentSim::ComputeXfactor   //
//---------------------------------//
int
InstrumentSim::ComputeXfactor(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
        Meas*                   meas,
	float* X){

  	double lambda = speed_light_kps / instrument->transmitFreq;
        if(!IntegrateSlice(spacecraft,instrument,meas,
			      numLookStepsPerSlice,azimuthIntegrationRange,
			      azimuthStepSize, rangeGateClipping, X))
	  return(0);	

        (*X)*=instrument->transmitPower * instrument->echo_receiverGain 
	        * lambda*lambda /(64*pi*pi*pi * instrument->systemLoss);
        return(1);
}

//--------------------//
// SetRangeAndDoppler //
//--------------------//

int
SetRangeAndDoppler(
	Spacecraft*		spacecraft,
	Instrument*		instrument)
{
	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();

	//-------------------------------------//
	// command the rx gate width and delay //
	//-------------------------------------//

	float residual_delay = 0.0;
	if (beam->useRangeTracker)
	{
		beam->rangeTracker.SetInstrument(instrument, &residual_delay);
	}
	else
	{
		//-------//
		// width //
		//-------//

		instrument->commandedRxGateWidth = beam->rxGateWidth;

		//-------//
		// delay //
		//-------//

		double rtt = IdealRtt(spacecraft, instrument);
		if (! RttToCommandedReceiverDelay(instrument, rtt))
			return(0);
	}

	//-------------------------------//
	// command the Doppler frequency //
	//-------------------------------//

	if (beam->useDopplerTracker)
	{
		beam->dopplerTracker.SetInstrument(instrument, residual_delay);
	}
	else
	{
		if (! IdealCommandedDoppler(spacecraft, instrument))
			return(0);
	}

	return(1);
}
