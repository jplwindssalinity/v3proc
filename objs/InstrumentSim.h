//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTSIM_H
#define INSTRUMENTSIM_H

static const char rcs_id_instrumentsim_h[] =
	"@(#) $Id$";

#include "ConfigList.h"
#include "Instrument.h"
#include "AntennaSim.h"
#include "OrbitSim.h"

//======================================================================
// CLASSES
//		InstrumentSim
//======================================================================


//======================================================================
// CLASS
//		InstrumentSim
//
// DESCRIPTION
//		The InstrumentSim object contains the information necessary to
//		simulate the instrument by operating on the subsystem
//		simulators.  It is used to set members of an Instrument object
//		and to command subsystem simulators as if the instrument were
//		funcionting.
//======================================================================

class InstrumentSim
{
public:
	//------//
	// enum //
	//------//

	enum SimEventE { NONE, BEAM_A, BEAM_B };

	//-------------//
	// contruction //
	//-------------//

	InstrumentSim();
	~InstrumentSim();

	//----------------//
	// initialization //
	//----------------//

	int		InitByConfig(ConfigList* config_list);
	int		Config(ConfigList* config_list);
	int		ConfigOrbitSim(ConfigList* config_list);
	int		ConfigAntennaSim(ConfigList* config_list);

	//--------------------//
	// simulation control //
	//--------------------//

	int		SimulateNextEvent(double* time, SimEventE* event);

	//-----------//
	// variables //
	//-----------//

	Instrument		instrument;		// the instrument state

	AntennaSim		antennaSim;		// the antenna simulator
	OrbitSim		orbitSim;		// the orbit simulator

protected:

	//-----------//
	// variables //
	//-----------//

	double		_priPerBeam;			// seconds
	double		_beamBTimeOffset;		// seconds

	SimEventE	_event;					// the last/current event
	double		_eventTime;				// the last/current event time
};

//--------------------------------//
// Instrument Simulation Keywords //
//--------------------------------//

#define PRI_PER_BEAM_KEYWORD			"PRI_PER_BEAM"
#define BEAM_B_TIME_OFFSET_KEYWORD		"BEAM_B_TIME_OFFSET"

//---------------------------//
// Orbit Simulation Keywords //
//---------------------------//

#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD		"LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_KEYWORD			"MEAN_ANOMALY"

//-----------------------------//
// Antenna Simulation Keywords //
//-----------------------------//

#define SPIN_RATE_KEYWORD				"SPIN_RATE"

#endif
