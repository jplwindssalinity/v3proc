//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "Instrument.h"


//===============//
// InstrumentSim //
//===============//

InstrumentSim::InstrumentSim()
:	_priPerBeam(0.0), _beamBTimeOffset(0.0), _event(NONE), _eventTime(0)
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

//----------------------------------//
// InstrumentSim::SimulateNextEvent //
//----------------------------------//

int
InstrumentSim::SimulateNextEvent(
	Instrument*		instrument)
{
	//-----------------------------------------//
	// determine the next event and event time //
	//-----------------------------------------//

	int pri_cycle;

	switch(_event)
	{
	case NONE:
		_event = SCATTEROMETER_BEAM_A_MEASUREMENT;
		break;
	case SCATTEROMETER_BEAM_A_MEASUREMENT:
		_event = SCATTEROMETER_BEAM_B_MEASUREMENT;
		pri_cycle = (int)((_eventTime + _beamBTimeOffset) / _priPerBeam);
		_eventTime = _priPerBeam * (double)pri_cycle + _beamBTimeOffset;
		break;
	case SCATTEROMETER_BEAM_B_MEASUREMENT:
		pri_cycle = (int)(_eventTime / _priPerBeam);
		_event = SCATTEROMETER_BEAM_A_MEASUREMENT;
		_eventTime = _priPerBeam * (double)(pri_cycle + 1);
		break;
	}

	//------------------//
	// update the orbit //
	//------------------//

	orbitSim.UpdateOrbit(_eventTime, &(instrument->orbit));

	//-----------------------------//
	// update the antenna position //
	//-----------------------------//

	antennaSim.UpdatePosition(_eventTime, &(instrument->antenna));

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

    l0->time = _eventTime;
 
    l0->gcAltitude = instrument->orbit.gcAltitude;
    l0->gcLongitude = instrument->orbit.gcLongitude;
    l0->gcLatitude = instrument->orbit.gcLatitude;
    instrument->orbit.gcVector.Get(0, &(l0->gcX));
    instrument->orbit.gcVector.Get(1, &(l0->gcY));
    instrument->orbit.gcVector.Get(2, &(l0->gcZ));
    instrument->orbit.velocityVector.Get(0, &(l0->velX));
    instrument->orbit.velocityVector.Get(1, &(l0->velY));
    instrument->orbit.velocityVector.Get(2, &(l0->velZ));
 
    l0->antennaPosition = instrument->antenna.azimuthAngle;
	switch(_event)
	{
		case SCATTEROMETER_BEAM_A_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_A;
			break;
		case SCATTEROMETER_BEAM_B_MEASUREMENT:
			l0->beam = L0::SCATTEROMETER_BEAM_B;
			break;
	}

	//------------------------------//
	// write telemetry if necessary //
	//------------------------------//

	l0->WriteDataRec();

	return(1);
}
