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
	SimEvent*		event)
{
	int pri_cycle;

	switch(event->event)
	{
	case NONE:
		event->event = SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT;
		break;
	case SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT:
		event->event = SimEvent::SCATTEROMETER_BEAM_B_MEASUREMENT;
		pri_cycle = (int)((event->time + _beamBTimeOffset) / _priPerBeam);
		event->time = _priPerBeam * (double)pri_cycle + _beamBTimeOffset;
		break;
	case SimEvent::SCATTEROMETER_BEAM_B_MEASUREMENT:
		event->event = SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT;
		pri_cycle = (int)(event->time / _priPerBeam);
		event->time = _priPerBeam * (double)(pri_cycle + 1);
		break;
	default:
		event->event = SimEvent::UNKNOWN;
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
	SimEvent*		event)
{
	instrument->time = event->time;
	switch(event->event)
	{
	case SimEvent::UPDATE_ORBIT:
		spacecraftSim.UpdateOrbit(instrument->time, &(instrument->spacecraft));
		break;
	case SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT:
	case SimEvent::SCATTEROMETER_BEAM_B_MEASUREMENT:
		spacecraftSim.UpdateOrbit(instrument->time, &(instrument->spacecraft));
		antennaSim.UpdatePosition(instrument->time, &(instrument->antenna));

		//--------------------------//
		// determine the beam index //
		//--------------------------//

		int beam_idx;
		switch(event->event)
		{
		case SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT:
			beam_idx = 0;
			break;
		case SimEvent::SCATTEROMETER_BEAM_B_MEASUREMENT:
			beam_idx = 1;
			break;
		}

		//--------------------------------//
		// calculate the beam orientation //
		//--------------------------------//

		Antenna* antenna = &(instrument->antenna);
		Beam* beam = &(antenna->beam[beam_idx]);

		Vector3 beam_orientation;
		beam_orientation.SphericalSet(1.0, pi/2.0 - beam->lookAngle,
			antenna->azimuthAngle + beam->azimuthAngle);

		//--------------------------------------//
		// calculate the earth intercept vector //
		//--------------------------------------//

		Spacecraft* spacecraft = &(instrument->spacecraft);

		EarthPosition rspot = earth_intercept(spacecraft->gcVector,
			spacecraft->velocityVector, spacecraft->attitude,
			antenna->antennaFrame, beam_orientation);

		//------------------------------------//
		// get sigma-0 for the earth location //
		//------------------------------------//

		Vector3 alt_lat_lon = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);

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
		case SimEvent::SCATTEROMETER_BEAM_A_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_A;
			break;
		case SimEvent::SCATTEROMETER_BEAM_B_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_B;
			break;
	}

	//------------------------------//
	// write telemetry if necessary //
	//------------------------------//

	l0->WriteDataRec();

	return(1);
}
