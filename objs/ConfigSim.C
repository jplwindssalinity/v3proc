//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_configsim_c[] =
	"@(#) $Id$";

#include "ConfigSim.h"
#include "InstrumentSim.h"
#include "OrbitSim.h"
#include "Misc.h"


//------------------//
// ConfigInstrument //
//------------------//

int
ConfigInstrument(
	Instrument*		instrument,
	ConfigList*		config_list)
{
	//--------------------------//
	// configure the instrument //
	//--------------------------//

	// nothing to do here (yet)

	//-----------------------//
	// configure the antenna //
	//-----------------------//

	if (! ConfigAntenna(&(instrument->antenna), config_list))
		return(0);

	return(1);
}

//---------------//
// ConfigAntenna //
//---------------//

int
ConfigAntenna(
	Antenna*		antenna,
	ConfigList*		config_list)
{
	//-----------------------//
	// configure the antenna //
	//-----------------------//

	int number_of_beams;
	if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
		return(0);
	antenna->numberOfBeams = number_of_beams;

	//---------------------//
	// configure each beam //
	//---------------------//

	for (int beam_number = 0; beam_number < antenna->numberOfBeams;
		beam_number++)
	{
		if (! ConfigBeam((antenna->beam + beam_number), beam_number,
			config_list))
		{
			return(0);
		}
	}

	return(1);
}

//------------//
// ConfigBeam //
//------------//

int
ConfigBeam(
	Beam*			beam,
	int				beam_number,
	ConfigList*		config_list)
{
	char keyword[1024];
	char number[8];
	double tmp_double;

	sprintf(number, "%d", beam_number);

	substitute_string(BEAM_x_LOOK_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->lookAngle = tmp_double;

	substitute_string(BEAM_x_AZIMUTH_ANGLE_KEYWORD, "x", number, keyword);
	if (! config_list->GetDouble(keyword, &tmp_double))
		return(0);
	beam->azimuthAngle = tmp_double;

	return(1);
}

//---------------------//
// ConfigInstrumentSim //
//---------------------//

int
ConfigInstrumentSim(
	InstrumentSim*	instrument_sim,
	ConfigList*		config_list)
{
	//------------------------------------//
	// configure the instrument simulator //
	//------------------------------------//

	double tmp_double;

	if (! config_list->GetDouble(PRI_PER_BEAM_KEYWORD, &tmp_double))
		return(0);
	instrument_sim->SetPriPerBeam(tmp_double);
	
	if (! config_list->GetDouble(BEAM_B_TIME_OFFSET_KEYWORD, &tmp_double))
		return(0);
	instrument_sim->SetBeamBTimeOffset(tmp_double);

	//-------------------------------//
	// configure the orbit simulator //
	//-------------------------------//

	if (! ConfigOrbitSim(&(instrument_sim->orbitSim), config_list))
		return(0);

	//---------------------------------//
	// configure the antenna simulator //
	//---------------------------------//

	if (! ConfigAntennaSim(&(instrument_sim->antennaSim), config_list))
		return(0);

	return(1);
}

//----------------//
// ConfigOrbitSim //
//----------------//

int
ConfigOrbitSim(
	OrbitSim*		orbit_sim,
	ConfigList*		config_list)
{
	//-------------------------------//
	// configure the orbit simulator //
	//-------------------------------//
 
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
 
	orbit_sim->DefineOrbit(semi_major_axis, eccentricity, inclination,
		long_of_asc_node, arg_of_perigee, mean_anomaly);
 
	return(1);
}

//------------------//
// ConfigAntennaSim //
//------------------//

int
ConfigAntennaSim(
	AntennaSim*		antenna_sim,
	ConfigList*		config_list)
{
	//---------------------------------//
	// configure the antenna simulator //
	//---------------------------------//
 
	double spin_rate;
	if (! config_list->GetDouble(SPIN_RATE_KEYWORD, &spin_rate))
		return(0);
	antenna_sim->SetSpinRate(spin_rate);

	return(1);
}

//----------//
// ConfigL0 //
//----------//

int
ConfigL0(
	L0*				l0,
	ConfigList*		config_list)
{
	//-------------------------//
	// configure the l0 object //
	//-------------------------//

	char* l0_file = config_list->Get(L0_FILE_KEYWORD);
	if (l0_file == NULL)
		return(0);
	l0->OpenForWriting(l0_file);

	return(1);
}
