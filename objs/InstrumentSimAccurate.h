//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTSIMACCURATE_H
#define INSTRUMENTSIMACCURATE_H

static const char rcs_id_instrumentsimaccurate_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		InstrumentSimAccurate
//======================================================================

//======================================================================
// CLASS
//		InstrumentSimAccurate
//
// DESCRIPTION
//		A derived class of InstrumentSim which allows for more
//              accurate but slower Instrument Simulation.
//======================================================================


#include"InstrumentSim.h"

class InstrumentSimAccurate: public InstrumentSim
{
public:

	//-------------//
	// construction //
	//-------------//

	InstrumentSimAccurate();
	~InstrumentSimAccurate();

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

	int		SetMeasurements(Instrument* instrument, 
				MeasSpot* meas_spot, WindField* windfield,
				GMF* gmf);

	int		ScatSim(Spacecraft* spacecraft, Instrument* instrument,
				WindField* windfield, GMF* gmf, L00Frame* l00_frame);

	// integration parameters and other constants

	int numLookStepsPerSlice;

	float azimuthIntegrationRange; 
	// Width in azimuth angle (radians) of area used in
	// integration.

	float azimuthStepSize;

	
};


#endif

