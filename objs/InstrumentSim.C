//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"
#include "LookGeom.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	_priPerBeam(0.0), _beamBTimeOffset(0.0)
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
	Event*		event)
{
	int pri_cycle;

	switch(event->eventId)
	{
	case NONE:
		event->eventId = Event::SCATTEROMETER_BEAM_A_MEASUREMENT;
		break;
	case Event::SCATTEROMETER_BEAM_A_MEASUREMENT:
		event->eventId = Event::SCATTEROMETER_BEAM_B_MEASUREMENT;
		pri_cycle = (int)((event->time + _beamBTimeOffset) / _priPerBeam);
		event->time = _priPerBeam * (double)pri_cycle + _beamBTimeOffset;
		break;
	case Event::SCATTEROMETER_BEAM_B_MEASUREMENT:
		event->eventId = Event::SCATTEROMETER_BEAM_A_MEASUREMENT;
		pri_cycle = (int)(event->time / _priPerBeam);
		event->time = _priPerBeam * (double)(pri_cycle + 1);
		break;
	default:
		event->eventId = Event::UNKNOWN;
		return(0);
		break;
	}
	return(1);
}

//------------------------------//
// InstrumentSim::SimulateEvent //
//------------------------------//

int
InstrumentSim::SimulateEvent(
	Instrument*		instrument,
	Event*			event,
	WindField*		wf,
	GMF*			gmf)
{
	instrument->time = event->time;
	switch(event->eventId)
	{
	case Event::UPDATE_ORBIT:
		spacecraftSim.UpdateOrbit(instrument->time, &(instrument->spacecraft));
		break;
	case Event::SCATTEROMETER_BEAM_A_MEASUREMENT:
	case Event::SCATTEROMETER_BEAM_B_MEASUREMENT:
		spacecraftSim.UpdateOrbit(instrument->time, &(instrument->spacecraft));
		antennaSim.UpdatePosition(instrument->time, &(instrument->antenna));

		//--------------------------//
		// determine the beam index //
		//--------------------------//

		int beam_idx;
		switch(event->eventId)
		{
		case Event::SCATTEROMETER_BEAM_A_MEASUREMENT:
			beam_idx = 0;
			break;
		case Event::SCATTEROMETER_BEAM_B_MEASUREMENT:
			beam_idx = 1;
			break;
		}

		//--------------------------------//
		// generate the coordinate switch //
		//--------------------------------//

		Spacecraft* sc = &(instrument->spacecraft);
		Antenna* antenna = &(instrument->antenna);

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
		velocity_frame(sc->gcVector, sc->velocityVector, &sc_xv, &sc_yv,
			&sc_zv);
		CoordinateSwitch sc_geocent_to_geovel(sc_xv, sc_yv, sc_zv);

		CoordinateSwitch total;

		//------------------------------------------------//
		// calculate the beam vector in the antenna frame //
		//------------------------------------------------//

		Beam* beam = &(antenna->beam[beam_idx]);

		double total_beam_azimuth = antenna->azimuthAngle + beam->azimuthAngle;
		Vector3 beam_orientation;
		beam_orientation.SphericalSet(1.0, 0, 0);

//		beam_orientation.SphericalSet(1.0, beam->lookAngle,
//			total_beam_azimuth);

		//--------------------------------------//
		// calculate the earth intercept vector //
		//--------------------------------------//

		Spacecraft* spacecraft = &(instrument->spacecraft);

		EarthPosition rspot = earth_intercept(spacecraft->gcVector,
			spacecraft->velocityVector, spacecraft->attitude,
			antenna->antennaFrame, beam->beamFrame, beam_orientation);

		//------------------------------------//
		// get sigma-0 for the earth location //
		//------------------------------------//

		Vector3 alt_lat_lon = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);
		double lat, lon;
		alt_lat_lon.Get(0, &lat);
		alt_lat_lon.Get(1, &lon);
		WindVector* wv = wf->NearestWindVector(lon, lat);
		Measurement meas;
		meas.pol = beam->polarization;
		meas.incidenceAngle = 0.0;		// how to calculate?
		meas.scAzimuth = total_beam_azimuth;

		gmf;

		break;
	}

	return(1);
}

//---------------------------//
// InstrumentSim::GenerateL0 //
//---------------------------//

int
InstrumentSim::GenerateL0(
	Instrument*		instrument,
	L0*				l0)
{
	//-----------------------------//
	// update telemetry parameters //
	//-----------------------------//

    l0->time = instrument->time;
 
    l0->gcAltitude = instrument->spacecraft.gcAltitude;
    l0->gcLongitude = instrument->spacecraft.gcLongitude;
    l0->gcLatitude = instrument->spacecraft.gcLatitude;
    instrument->spacecraft.gcVector.Get(0, &(l0->gcX));
    instrument->spacecraft.gcVector.Get(1, &(l0->gcY));
    instrument->spacecraft.gcVector.Get(2, &(l0->gcZ));
    instrument->spacecraft.velocityVector.Get(0, &(l0->velX));
    instrument->spacecraft.velocityVector.Get(1, &(l0->velY));
    instrument->spacecraft.velocityVector.Get(2, &(l0->velZ));
 
    l0->antennaPosition = instrument->antenna.azimuthAngle;
	switch(instrument->event.eventId)
	{
		case Event::SCATTEROMETER_BEAM_A_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_A;
			break;
		case Event::SCATTEROMETER_BEAM_B_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_B;
			break;
	}

	//------------------------------//
	// write telemetry if necessary //
	//------------------------------//

	l0->WriteDataRec();

	return(1);
}
