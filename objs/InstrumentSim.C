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
	InstrumentSim*	instrument_sim,
	Instrument*		instrument,
	int				beam_idx,
	WindField*		windfield,
	GMF*			gmf)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);

	//----------------------//
	// update antenna angle //
	//----------------------//

	instrument_sim->antennaSim.UpdatePosition(time, antenna);

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	Attitude att;
	att.Set(0.0, 0.0, antenna->azimuthAngle, 1, 2, 3);
	CoordinateSwitch ant_ped_to_ant_frame(att);

	att.Set(0.0, 0.0, 0.0, 1, 2, 3);
	CoordinateSwitch sc_body_to_ant_ped(att);

	double sc_roll = 0.0;
	double sc_pitch = 0.0;
	double sc_yaw = 0.0;
	att.Set(sc_roll, sc_pitch, sc_yaw, 2, 1, 3);
	CoordinateSwitch sc_geovel_to_sc_body(att);

	Vector3 sc_xv, sc_yv, sc_zv;
	velocity_frame(orbit_state->rsat, orbit_state->vsat,
		&sc_xv, &sc_yv, &sc_zv);
	CoordinateSwitch sc_geocent_to_geovel(sc_xv, sc_yv, sc_zv);

	CoordinateSwitch total;

	//------------------------------------------------//
	// calculate the beam vector in the antenna frame //
	//------------------------------------------------//

	Beam* beam = &(antenna->beam[beam_idx]);

	double total_beam_azimuth = antenna->azimuthAngle + beam->azimuthAngle;
	Vector3 beam_orientation;
	beam_orientation.SphericalSet(1.0, 0, 0);

//	beam_orientation.SphericalSet(1.0, beam->lookAngle,
//		total_beam_azimuth);

	//--------------------------------------//
	// calculate the earth intercept vector //
	//--------------------------------------//

	EarthPosition rspot = earth_intercept(orbit_state->rsat,
		orbit_state->vsat, att, antenna->antennaFrame, beam->beamFrame,
		beam_orientation);

	//------------------------------------//
	// get sigma-0 for the earth location //
	//------------------------------------//

	Vector3 alt_lat_lon = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);
	double lat, lon;
	alt_lat_lon.Get(0, &lat);
	alt_lat_lon.Get(1, &lon);
//	WindVector* wv = wf->NearestWindVector(lon, lat);
	Measurement meas;
	meas.pol = beam->polarization;
	meas.incidenceAngle = 0.0;		// how to calculate?
	meas.scAzimuth = total_beam_azimuth;

	gmf;
	windfield;

	//----------------------------//
	// update the level 0.0 frame //
	//----------------------------//

/*
	if (_pulseNumber == 0)
	{
		l00Frame.time = time;
		gcAltitude = ???;
		gcLongitude = ???;
		gcLatitude = ???;
		gcX = ???;
		gcY = ???;
		gcZ = ???;
		velX = ???;
		velY = ???;
		velZ = ???;
		firstAntennaPosition = ???;
	}
	else
	{
		antennaPosition[_pulseNumber] = ???;
	}

	sigma0[_pulseNumber] = ???;
*/

	return(1);
}
