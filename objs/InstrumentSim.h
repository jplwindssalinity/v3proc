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
#include "L00Frame.h"
#include "WindField.h"
#include "GMF.h"
#include "Ephemeris.h"

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

	//--------------------//
	// simulation control //
	//--------------------//

	int		Initialize(Antenna* antenna);
	int		DetermineNextEvent(Antenna* antenna,
				InstrumentEvent* instrument_event);

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

	int		ScatSim(double time, OrbitState* orbit_state,
				Instrument* instrument, int beam_idx, WindField* windfield,
				GMF* gmf);

	//-----------//
	// variables //
	//-----------//

	AntennaSim		antennaSim;		// the antenna simulator

	//---------------------------//
	// level 0 frame information //
	//---------------------------//

	L00Frame		l00Frame;
	int				l00FrameReady;

protected:

	//-----------//
	// variables //
	//-----------//

	//---------------------------//
	// level 0 frame information //
	//---------------------------//

	int			_spotNumber;
};

#endif
