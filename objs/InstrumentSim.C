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
#define UNIFORM_SIGMA 0 // (If 1 then all sigma0s=1)

//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	slicesPerSpot(0),startTime(0.0), l00FrameReady(0), _spotNumber(0)
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
	int cycle_idx = (int)(cycle_start_time / antenna->priPerBeam + 0.5);
	_scatBeamTime[min_idx] = (double)(cycle_idx + 1) * antenna->priPerBeam +
		antenna->beam[min_idx].timeOffset;

	return(1);
}

//--------------------------------------//
// InstrumentSim::UpdateAntennaPosition //
//--------------------------------------//

int
InstrumentSim::UpdateAntennaPosition(
	double			time,
	Instrument*		instrument)
{
	antennaSim.UpdatePosition(time, &(instrument->antenna));
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

	for (Meas* meas = meas_spot->GetHead(); meas;
		meas = meas_spot->GetNext())
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
		float sigma0;
		gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
			chi, &sigma0);

		if (UNIFORM_SIGMA) sigma0=1;
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

		/************* FOR NOW Kfactor=1.0 *********/
		float Kfactor=1.0;
		if(! sigma0_to_Pr(&gc_to_antenna, spacecraft, instrument, meas,
				Kfactor, sigma0, &(meas->value)))
		{
			return(0);
		}
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
	l00_frame->gcX = orbit_state->rsat.get(0);
	l00_frame->gcY = orbit_state->rsat.get(1);
	l00_frame->gcZ = orbit_state->rsat.get(2);
	l00_frame->velX = orbit_state->vsat.get(0);
	l00_frame->velY = orbit_state->vsat.get(1);
	l00_frame->velZ = orbit_state->vsat.get(2);

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
	_spotNumber++;

	return(1);
}

//------------------------//
// InstrumentSim::ScatSim //
//------------------------//

int
InstrumentSim::ScatSim(
	double			time,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	WindField*		windfield,
	GMF*			gmf,
	L00Frame*		l00_frame)
{
	MeasSpot meas_spot;

	//-----------------------------//
	// If at the start of a frame //
	// Compute Frame header info //
	//-----------------------------//

	if (_spotNumber == 0)
	{	
		if (! SetL00Spacecraft(spacecraft,l00_frame))
			return(0);
		l00_frame->time = time;
	}

	//---------------------//
	// locate measurements //
	//---------------------//

	if (slicesPerSpot <= 1)
	{
		if (! LocateSpot(time, spacecraft, instrument, &meas_spot))
			return(0);
	}
	else
	{
		if (! LocateSlices(time, spacecraft, instrument, slicesPerSpot,
			&meas_spot))
		{
			return(0);
		}
	}

	//------------------------//
	// set measurement values //
	//------------------------//

	if (! SetMeasurements(spacecraft, instrument, &meas_spot, windfield, gmf))
		return(0);


	//--------------------------------//
	// Add Spot Specific Info to Frame //
	//--------------------------------//

	if (! SetL00Science(&meas_spot, instrument, l00_frame))
		return(0);

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
