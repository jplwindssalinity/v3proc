//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "InstrumentSim.h"


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

//-----------------------------//
// InstrumentSim::InitByConfig //
//-----------------------------//

int
InstrumentSim::InitByConfig(
	ConfigList*		config_list)
{
	if (! Config(config_list))
		return(0);

	if (! ConfigOrbitSim(config_list))
		return(0);

	if (! ConfigAntennaSim(config_list))
		return(0);

	return(1);
}

//-----------------------//
// InstrumentSim::Config //
//-----------------------//

int
InstrumentSim::Config(
	ConfigList*		config_list)
{
	double tmp_double;

	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &tmp_double))
		return(0);
	_priPerBeam = tmp_double;

	if (! config_list->GetDouble(BEAM_B_TIME_OFFSET_KEYWORD, &tmp_double))
		return(0);
	_beamBTimeOffset = tmp_double;

	return(1);
}

//-------------------------------//
// InstrumentSim::ConfigOrbitSim //
//-------------------------------//

int
InstrumentSim::ConfigOrbitSim(
	ConfigList*		config_list)
{
	//------------------------------------//
	// set up orbit simulation parameters //
	//------------------------------------//
 
	double semi_major_axis;
	if (! config_list->GetDouble(SEMI_MAJOR_AXIS_KEYWORD, &semi_major_axis))
		return(0);
 
	double eccentricity;
	if (! config_list->GetDouble(ECCENTRICITY_KEYWORD, &eccentricity))
		return(0);
 
	double inclination;
	if (! config_list->GetDouble(INCLINATION_KEYWORD, &inclination))
		return(0);
 
	double long_of_asc_node;
	if (! config_list->GetDouble(LONG_OF_ASC_NODE_KEYWORD, &long_of_asc_node))
		return(0);
 
	double arg_of_perigee;
	if (! config_list->GetDouble(ARGUMENT_OF_PERIGEE_KEYWORD, &arg_of_perigee))
		return(0);
 
	double mean_anomaly;
	if (! config_list->GetDouble(MEAN_ANOMALY_KEYWORD, &mean_anomaly))
		return(0);
 
	orbitSim.DefineOrbit(semi_major_axis, eccentricity, inclination,
		long_of_asc_node, arg_of_perigee, mean_anomaly);
 
	return(1);
}

//---------------------------------//
// InstrumentSim::ConfigAntennaSim //
//---------------------------------//

int
InstrumentSim::ConfigAntennaSim(
	ConfigList*		config_list)
{
	//--------------------------------------//
	// set up antenna simulation parameters //
	//--------------------------------------//
 
	double spin_rate;
	if (! config_list->GetDouble(SPIN_RATE_KEYWORD, &spin_rate))
		return(0);

	antennaSim.SetSpinRate(spin_rate);
	return(1);
}

//----------------------------------//
// InstrumentSim::SimulateNextEvent //
//----------------------------------//

int
InstrumentSim::SimulateNextEvent(
	double*		time,
	SimEventE*	event)
{
	//--------------------------//
	// determine the next event //
	//--------------------------//

	int pri_cycle;
	switch(_event)
	{
	case NONE:
		_event = BEAM_A;
		break;
	case BEAM_A:
		_event = BEAM_B;
		pri_cycle = (int)((_eventTime + _beamBTimeOffset) / _priPerBeam);
		_eventTime = _priPerBeam * (double)pri_cycle + _beamBTimeOffset;
		break;
	case BEAM_B:
		pri_cycle = (int)(_eventTime / _priPerBeam);
		_event = BEAM_A;
		_eventTime = _priPerBeam * (double)(pri_cycle + 1);
		break;
	}

	//------------------//
	// update the orbit //
	//------------------//

	orbitSim.UpdateOrbit(_eventTime, &(instrument.orbit));

	//-----------------------------//
	// update the antenna position //
	//-----------------------------//

	antennaSim.UpdatePosition(_eventTime, &(instrument.antenna));

	//--------------------------//
	// set event and event time //
	//--------------------------//

	*event = _event;
	*time = _eventTime;

	return(1);
}
