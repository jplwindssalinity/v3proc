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
#include "SpacecraftSim.h"
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

	//--------------------//
	// simulation control //
	//--------------------//

	int		DetermineNextEvent(Event* event);
	int		SimulateEvent(Instrument* instrument, Event* event);
	int		GenerateL0(Instrument* instrument, L0* l0);

	//-----------//
	// variables //
	//-----------//

	AntennaSim		antennaSim;		// the antenna simulator
	SpacecraftSim	spacecraftSim;	// the spacecraft simulator

protected:

	//-----------//
	// variables //
	//-----------//

	double		_priPerBeam;			// seconds
	double		_beamBTimeOffset;		// seconds
};

#endif
