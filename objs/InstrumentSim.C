//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "LookGeom.h"
#include "Ephemeris.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	l00FrameReady(0), _pulseNumber(0)
{
	return;
}

InstrumentSim::~InstrumentSim()
{
	return;
}

double g_scat_beam_time[MAX_NUMBER_OF_BEAMS];

//---------------------------//
// InstrumentSim::Initialize //
//---------------------------//

int
InstrumentSim::Initialize(
	Antenna*	antenna)
{
	for (int i = 0; i < antenna->numberOfBeams; i++)
	{
		g_scat_beam_time[i] = antenna->beam[i].timeOffset;
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
	double min_time = g_scat_beam_time[0];
	for (int i = 1; i < antenna->numberOfBeams; i++)
	{
		if (g_scat_beam_time[i] < min_time)
		{
			min_idx = i;
			min_time = g_scat_beam_time[i];
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
		g_scat_beam_time[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.5) * antenna->priPerBeam;
	}
	else
	{
		g_scat_beam_time[min_idx] = (double)(int)(min_time /
			antenna->priPerBeam + 1.0) * antenna->priPerBeam +
			antenna->beam[min_idx].timeOffset;
	}

	return(1);
}

//------------------------//
// InstrumentSim::ScatSim //
//------------------------//

int
InstrumentSim::ScatSim(
	double			time,
	OrbitState*		orbit_state,
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
	double beam_look_angle = beam->lookAngle;
	double beam_azimuth_angle = beam->azimuthAngle;

	//----------------------//
	// update antenna angle //
	//----------------------//

	antennaSim.UpdatePosition(time, antenna);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	// desired beam boresight in antenna frame
	CoordinateSwitch ant_frame_to_beam_frame(beam->beamFrame);

	// antenna frame in antenna pedestal frame
	Attitude att;
	att.Set(0.0, 0.0, antenna->azimuthAngle, 1, 2, 3);
	CoordinateSwitch ant_ped_to_ant_frame(att);

	// antenna pedestal frame in s/c body frame (zero for now)
	att.Set(0.0, 0.0, 0.0, 1, 2, 3);
	CoordinateSwitch sc_body_to_ant_ped(att);

	// s/c body frame in s/c velocity frame (zero for now)
	double sc_roll = 0.0;
	double sc_pitch = 0.0;
	double sc_yaw = 0.0;
	att.Set(sc_roll, sc_pitch, sc_yaw, 2, 1, 3);
	CoordinateSwitch sc_geovel_to_sc_body(att);

	// s/c velocity frame in geocentric earth frame
	Vector3 sc_xv, sc_yv, sc_zv;
	velocity_frame(orbit_state->rsat, orbit_state->vsat,
		&sc_xv, &sc_yv, &sc_zv);
	CoordinateSwitch sc_geocent_to_geovel(sc_xv, sc_yv, sc_zv);

	// total transformation (forward)
	CoordinateSwitch total;
	total = sc_geocent_to_geovel;
	total.Append(&sc_geovel_to_sc_body);
	total.Append(&sc_body_to_ant_ped);
	total.Append(&ant_ped_to_ant_frame);
	total.Append(&ant_frame_to_beam_frame);
	total.ReverseDirection();

	//---------------------------------------------------//
	// calculate the look vector in the geocentric frame //
	//---------------------------------------------------//

	Vector3 rlook_beam;
	rlook_beam.SphericalSet(1.0, beam_look_angle, beam_azimuth_angle);
	Vector3 rlook_geo = total.Forward(rlook_beam);

	//-------------------------------//
	// calculate the earth intercept //
	//-------------------------------//

	EarthPosition rspot = earth_intercept(rlook_geo, orbit_state->rsat);

	//----------------------------------------//
	// get wind vector for the earth location //
	//----------------------------------------//

	Vector3 alt_lat_lon = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);
	double lat, lon;
	alt_lat_lon.Get(0, &lat);
	alt_lat_lon.Get(1, &lon);
	WindVector* wv = windfield->NearestWindVector(lon, lat);

	//---------------------------//
	// generate measurement data //
	//---------------------------//

	Meas meas;
	meas.pol = beam->polarization;
	meas.eastAzimuth = 0.0;			// generate local coords, calc az.
	meas.scAzimuth = 0.0;			// calc az from s/c vel and look angle
	meas.incidenceAngle = 0.0;		// use local normal to calc inc.

	//--------------------------------//
	// convert wind vector to sigma-0 //
	//--------------------------------//

	double chi = meas.eastAzimuth - wv->dir;	// do the wrapping thing?
	double value;
	gmf->GetInterpolatedValue(meas.pol, meas.incidenceAngle, wv->spd, chi,
		&value);

	//----------------------------//
	// update the level 0.0 frame //
	//----------------------------//

	if (_pulseNumber == 0)
	{
		l00FrameReady = 0;
		l00Frame.time = time;
		Vector3 sc_all;
		sc_all = orbit_state->rsat.get_alt_lat_lon(EarthPosition::GEODETIC);
		l00Frame.gcAltitude = sc_all.get(0);
		l00Frame.gcLongitude = sc_all.get(1);
		l00Frame.gcLatitude = sc_all.get(2);
		l00Frame.gcX = orbit_state->rsat.get(0);
		l00Frame.gcY = orbit_state->rsat.get(1);
		l00Frame.gcZ = orbit_state->rsat.get(2);
		l00Frame.velX = orbit_state->vsat.get(0);
		l00Frame.velY = orbit_state->vsat.get(1);
		l00Frame.velZ = orbit_state->vsat.get(2);
	}
	l00Frame.antennaPosition[_pulseNumber] = antenna->azimuthAngle;
	l00Frame.sigma0[_pulseNumber] = value;
	_pulseNumber++;

	//-----------------------------//
	// determine if frame is ready //
	//-----------------------------//

	if (_pulseNumber >= PULSES_PER_L00_FRAME)
		l00FrameReady = 1;

	return(1);
}
