//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTSIM_H
#define INSTRUMENTSIM_H

static const char rcs_id_instrumentsim_h[] =
	"@(#) $Id$";

#include "Antenna.h"
#include "Instrument.h"
#include "Wind.h"
#include "GMF.h"
#include "AntennaSim.h"
#include "L00.h"
#include "Spacecraft.h"
#include "Distributions.h"
#include "XTable.h"
#include "Kpm.h"
#include "CheckFrame.h"

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
	int		UpdateAntennaPosition(Instrument* instrument);

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

	int		SetMeasurements(Spacecraft* spacecraft, Instrument* instrument,
				MeasSpot* meas_spot, CheckFrame* cf, WindField* windfield,
				GMF* gmf, Kp* kp,
				KpmField* kpmField);
	int		SetL00Spacecraft(Spacecraft* spacecraft, L00Frame* l00_frame);
	int		SetL00Science(MeasSpot* meas_spot, CheckFrame* cf,
				Instrument* instrument,
				L00Frame* l00_frame);
	int		ScatSim(Spacecraft* spacecraft, Instrument* instrument,
				WindField* windfield, GMF* gmf, Kp* kp, KpmField* kpmField,
				L00Frame* l00_frame);

	float           ComputeKfactor(Spacecraft* spacecraft, Instrument* instrument,
				       Meas* meas);
	//-----------//
	// variables //
	//-----------//

	TimeCorrelatedGaussian	ptgrNoise;
	double			startTime;
	AntennaSim		antennaSim;		// the antenna simulator

	XTable			kfactorTable;
	XTable			xTable;
	int numLookStepsPerSlice;

	float azimuthIntegrationRange; 
	// Width in azimuth angle (radians) of area used in
	// integration.

        float azimuthStepSize;

	float dopplerBias;

	//---------------------------//
	// level 0 frame information //
	//---------------------------//

	int			l00FrameReady;

	//-------//
	// flags //
	//-------//

	int			uniformSigmaField;	// set all sigma0 values to 1.0
	int			outputXToStdout;	// write X value to stdout
	int			useKfactor;			// read and use K-factor table
	int			computeKfactor;     // compute K-factor
	int			createXtable;		// create an X table
	int			rangeGateClipping;  // simulate range gate clipping
	int			applyDopplerError;  // simulate doppler tracking error

	char*		simVs1BCheckfile;	// output data for cross check with 1B

protected:

	//-----------//
	// variables //
	//-----------//

	double		_scatBeamTime[MAX_NUMBER_OF_BEAMS];


	//---------------------------//
	// level 0 frame information //
	//---------------------------//

	int			_spotNumber;
};

//------------------//
// Helper functions //
//------------------//

int		SetRangeAndDoppler(Spacecraft* spacecraft, Instrument* instrument);

#endif










