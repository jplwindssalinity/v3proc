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
:	_priPerBeam(0.0), _beamBTimeOffset(0.0), l00FrameReady(0), _pulseNumber(0)
{
	return;
}

InstrumentSim::~InstrumentSim()
{
	return;
}

//-----------------------//
// InstrumentSim::SetXxx //
//-----------------------//

int InstrumentSim::SetPriPerBeam(double pri_per_beam)
{
	_priPerBeam = pri_per_beam;
	return(1);
}

int InstrumentSim::SetBeamBTimeOffset(double beam_b_time_offset)
{
	_beamBTimeOffset = beam_b_time_offset;
	return(1);
}

//-----------------------------------//
// InstrumentSim::DetermineNextEvent //
//-----------------------------------//

int
InstrumentSim::DetermineNextEvent(
	InstrumentEvent*	instrument_event)
{
	//------------------------------------//
	// initialize next time of each event //
	//------------------------------------//

	static double scat_a_time = 0.0;
	static double scat_b_time = _beamBTimeOffset;

	//----------------------------------------//
	// find minimum time from possible events //
	//----------------------------------------//

	if (scat_a_time <= scat_b_time)
	{
		instrument_event->eventId =
			InstrumentEvent::SCATTEROMETER_BEAM_A_MEASUREMENT;
		instrument_event->time = scat_a_time;
		scat_a_time = (double)(int)(instrument_event->time / _priPerBeam +
			1.5) * _priPerBeam;
	}
	else if (scat_b_time <= scat_a_time)
	{
		instrument_event->eventId =
			InstrumentEvent::SCATTEROMETER_BEAM_B_MEASUREMENT;
		instrument_event->time = scat_b_time;
		scat_b_time = (double)((int)(instrument_event->time / _priPerBeam) +
			1) * _priPerBeam + _beamBTimeOffset;
	}
	else
		return(0);

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
	Attitude att;
	att.Set(0.0, beam_look_angle, beam_azimuth_angle, 1, 2, 3);
	CoordinateSwitch ant_frame_to_beam_frame(att);

	// antenna frame in antenna pedestal frame
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
