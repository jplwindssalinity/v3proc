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
:	_priPerBeam(0.0), _beamBTimeOffset(0.0), _event(NONE)
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
	if (! ConfigInstrument(config_list))
		return(0);

	if (! ConfigOrbit(config_list))
		return(0);

	return(1);
}

//---------------------------------//
// InstrumentSim::ConfigInstrument //
//---------------------------------//

int
InstrumentSim::ConfigInstrument(
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

//----------------------------//
// InstrumentSim::ConfigOrbit //
//----------------------------//

int
InstrumentSim::ConfigOrbit(
	ConfigList*		config_list)
{
	//-------------------------//
	// set up orbit parameters //
	//-------------------------//
 
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

//----------------------------------//
// InstrumentSim::SimulateNextEvent //
//----------------------------------//

int
InstrumentSim::SimulateNextEvent(
	double*		time)
{
	int pri_cycle = (int)(*time / _priPerBeam);
	
	SimEventE current_event;
	switch(_event)
	{
	case NONE:
		current_event = BEAM_A;
		break;
	case BEAM_A:
		current_event = BEAM_B;
		*time = _priPerBeam * (double)pri_cycle + _beamBTimeOffset;
		break;
	case BEAM_B:
		current_event = BEAM_A;
		*time = _priPerBeam * (double)(pri_cycle + 1);
		break;
	}
	_event = current_event;

	orbitSim.UpdateOrbit(*time, &(instrument.orbit));

	return(1);
}
