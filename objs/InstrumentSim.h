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
#include "L0.h"

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
//		functioning.
//======================================================================

class InstrumentSim
{
public:

	//------//
	// enum //
	//------//

	enum SimEventE { NONE, SCATTEROMETER_BEAM_A_MEASUREMENT,
		SCATTEROMETER_BEAM_B_MEASUREMENT };

	//-------------//
	// contruction //
	//-------------//

	InstrumentSim();
	~InstrumentSim();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetPriPerBeam(double pri_per_beam);
	int		SetBeamBTimeOffset(double beam_b_time_offset);

	double	GetEventTime() { return(_eventTime); };

	//--------------------//
	// simulation control //
	//--------------------//

	int		SimulateNextEvent(Instrument* instrument);
	int		GenerateL0(Instrument* instrument, L0* l0);

	//-----------//
	// variables //
	//-----------//

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

#endif
