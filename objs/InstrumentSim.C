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
#include "Constants.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	startTime(0.0), endTime(0.0), l00FrameReady(0), _spotNumber(0)
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

	if (min_idx == 0)
	{
		_scatBeamTime[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.5) * antenna->priPerBeam;
	}
	else
	{
		_scatBeamTime[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.0) * antenna->priPerBeam +
			antenna->beam[min_idx].timeOffset;
	}

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

//------------------------//
// InstrumentSim::ScatSim //
//------------------------//

int
InstrumentSim::ScatSim(
	double			time,
	OrbitState*		sc_orbit_state,
	Attitude*		sc_attitude,
	Instrument*		instrument,
	int				beam_idx,
	WindField*		windfield,
	GMF*			gmf)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = &(antenna->beam[beam_idx]);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch beam_frame_to_gc = BeamFrameToGC(sc_orbit_state,
		sc_attitude, antenna, beam);

	//---------------------------------------------------//
	// calculate the look vector in the geocentric frame //
	//---------------------------------------------------//

	Vector3 rlook_beam;
	rlook_beam.SphericalSet(1.0, 0.0, 0.0);		// boresight
	Vector3 rlook_gc = beam_frame_to_gc.Forward(rlook_beam);

	//-------------------------------//
	// calculate the earth intercept //
	//-------------------------------//

	EarthPosition spot_on_earth = earth_intercept(sc_orbit_state->rsat,
		rlook_gc);

	//----------------------------------------//
	// get wind vector for the earth location //
	//----------------------------------------//

	double alt, lat, lon;
	if (spot_on_earth.GetAltLatLon(EarthPosition::GEODETIC,
		 &alt, &lat, &lon) == 0)
	{
		printf("Error: ScatSim can't convert spot_on_earth\n");
		return(0);
	}

	LonLat lon_lat;
	lon_lat.longitude = lon;
	lon_lat.latitude = lat;
	WindVector wv = windfield->InterpolatedWindVector(lon_lat);

	//---------------------------//
	// generate measurement data //
	//---------------------------//

	Meas meas;
	meas.pol = beam->polarization;

	// get local measurement azimuth
	CoordinateSwitch gc_to_surface = spot_on_earth.SurfaceCoordinateSystem();
	Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
	double r, theta, phi;
	rlook_surface.SphericalGet(&r, &theta, &phi);
	meas.eastAzimuth = phi;
	
	// get incidence angle
	meas.incidenceAngle = spot_on_earth.IncidenceAngle(rlook_gc);

	//--------------------------------//
	// convert wind vector to sigma-0 //
	//--------------------------------//

	// chi is defined so that 0.0 means the wind is blowing towards
	// the s/c (the opposite direction as the look vector)
	float chi = wv.dir - meas.eastAzimuth + pi;
	float value;
	gmf->GetInterpolatedValue(meas.pol, meas.incidenceAngle, wv.spd, chi,
		&value);

	//----------------------------//
	// update the level 0.0 frame //
	//----------------------------//

	L00Frame* l00_frame = &(l00.frame);
	if (_spotNumber == 0)
	{
		l00FrameReady = 0;
		l00_frame->time = time;
		if (sc_orbit_state->rsat.GetAltLatLon(EarthPosition::GEODETIC,
			 &alt, &lat, &lon) == 0)
		{
			printf("Error: ScatSim can't convert rsat\n");
			return(0);
		}
		l00_frame->gcAltitude = alt;
		l00_frame->gcLongitude = lon;
		l00_frame->gcLatitude = lat;
		l00_frame->gcX = sc_orbit_state->rsat.get(0);
		l00_frame->gcY = sc_orbit_state->rsat.get(1);
		l00_frame->gcZ = sc_orbit_state->rsat.get(2);
		l00_frame->velX = sc_orbit_state->vsat.get(0);
		l00_frame->velY = sc_orbit_state->vsat.get(1);
		l00_frame->velZ = sc_orbit_state->vsat.get(2);
	}
	l00_frame->antennaPosition[_spotNumber] = antenna->GetEncoderValue();
	l00_frame->science[_spotNumber] = value;
	_spotNumber++;

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	if (_spotNumber >= l00_frame->spotsPerFrame)
	{
		l00FrameReady = 1;	// indicate frame is ready
		_spotNumber = 0;	// prepare to start a new frame
	}

	return(1);
}
