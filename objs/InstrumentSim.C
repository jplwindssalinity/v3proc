//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "GenericGeom.h"
#include "InstrumentGeom.h"
#include "Ephemeris.h"
#include "Sigma0.h"
#include "Constants.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	startTime(0.0), l00FrameReady(0), uniformSigmaField(0),
	outputXToStdout(0), useKfactor(0), createXtable(0), _spotNumber(0)
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
	WindField*		windfield,
	GMF*			gmf)
{
	//-------------------------//
	// for each measurement... //
	//-------------------------//

	int slice_count = instrument->GetTotalSliceCount();
	int sliceno = -slice_count/2;
	for (Meas* meas = meas_spot->GetHead(); meas; meas = meas_spot->GetNext())
	{
		//----------------------------------------//
		// get lon and lat for the earth location //
		//----------------------------------------//

		double alt, lat, lon;
		if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
			return(0);

		LonLat lon_lat;
		lon_lat.longitude = lon;
		lon_lat.latitude = lat;

		float sigma0;
		if (uniformSigmaField)
		{
			sigma0=1;
		}
		else{
		        //-----------------//
		        // get wind vector //
		        //-----------------//

		        WindVector wv;
			if (! windfield->InterpolatedWindVector(lon_lat, &wv))
			  {
			    wv.spd = 0.0;
			    wv.dir = 0.0;
			  }

			//-------------------------------------------------------------------//
			// Get the Kpm value appropriate for the current beam and wind speed //
			//-------------------------------------------------------------------//

			double Kpm = GetKpm(instrument,&wv);

			//--------------------------------//
			// convert wind vector to sigma-0 //
			//--------------------------------//

			// chi is defined so that 0.0 means the wind is blowing towards
			// the s/c (the opposite direction as the look vector)
			float chi = wv.dir - meas->eastAzimuth + pi;

			gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
				chi, &sigma0);

			//---------------------------------------------------------------//
			// Fuzz the sigma0 by Kpm to simulate the effects of model function
			// error.  The resulting sigma0 is the 'true' value.
			// It does not map back to the correct wind speed for the
			// current beam and geometry because the model function is
			// not perfect.
			// This Kpm application is UNCORRELATED.
			//---------------------------------------------------------------//

			if (instrument->useKpm == 1)
			{
//				sigma0 *= kpmgrid->GetRV(meas->pol,wv.spd,lon,lat);

				Gaussian rv(Kpm*Kpm,1.0);
				float rv1 = -1.0;
				while (rv1 < 0.0)
				{
					// Do not permit negative sigma0's.
					rv1 = rv.GetNumber();
				}
				sigma0 *= rv1;
			}
		}

		//--------------------------------//
		// generate the coordinate switch //
		//--------------------------------//

		CoordinateSwitch gc_to_antenna = AntennaFrameToGC(
							&(spacecraft->orbitState),
							&(spacecraft->attitude),
							&(instrument->antenna));
		gc_to_antenna=gc_to_antenna.ReverseDirection();

		//-------------------------//
		// convert Sigma0 to Power //
		//-------------------------//

		// Kfactor: either 1.0 or taken from table
		float Kfactor=1.0;

		if (useKfactor)
		{
			float orbit_position = instrument->OrbitFraction();

			Kfactor = kfactorTable.RetrieveByRelativeSliceNumber(
				instrument->antenna.currentBeamIdx,
				instrument->antenna.azimuthAngle,
				orbit_position, sliceno);
		}

		if (! sigma0_to_Esn_slice(&gc_to_antenna, spacecraft, instrument, meas,
				Kfactor, sigma0, &(meas->value), &(meas->XK)))
		{
			return(0);
		}
		sliceno++;
		if(slice_count%2==0 && sliceno==0) sliceno++;
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
	Instrument*		instrument,
	L00Frame*		l00_frame)
{
	Antenna* antenna = &(instrument->antenna);

	//----------//
	// set PtGr //
	//----------//

	l00_frame->ptgr= instrument->transmitPower*instrument->echo_receiverGain;
	l00_frame->ptgr *= (1+ptgrNoise.GetNumber());

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
	L00Frame*		l00_frame)
{
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

		if (outputXToStdout)
			printf("%g ",instrument->antenna.azimuthAngle/dtr);
	}

	//------------------------//
	// set measurement values //
	//------------------------//

	if (! SetMeasurements(spacecraft, instrument, &meas_spot, windfield, gmf))
		return(0);

	//--------------------------------//
	// Output X to Stdout if enabled //
	//--------------------------------//

	if (outputXToStdout)
	{
		for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
		{
			printf("%g ",slice->XK);
		}
		if (instrument->antenna.currentBeamIdx==1)
			printf("\n");
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

	if (! SetL00Science(&meas_spot, instrument, l00_frame))
		return(0);

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
