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

//======================================================================
// CLASSES
//		InstrumentSim
//======================================================================

#define POINTS_PER_SPOT_OUTLINE		18

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
	int		UpdateAntennaPosition(double time, Instrument* instrument);

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

	int		LocateSlices(double time, Spacecraft* spacecraft,
				Instrument* instrument, MeasSpot* meas_spot);
	int		LocateSpot(double time, Spacecraft* spacecraft,
				Instrument* instrument, MeasSpot* meas_spot);
	int		SetMeasurements(Spacecraft* spacecraft, 
				Instrument* instrument, 
				MeasSpot* meas_spot, WindField* windfield,
				GMF* gmf);
	int		SetL00Spacecraft(Spacecraft* spacecraft, L00Frame* l00_frame);
	int		SetL00Science(MeasSpot* meas_spot, Instrument* instrument,
				L00Frame* l00_frame);
	int		ScatSim(double time, Spacecraft* spacecraft,
				Instrument* instrument, WindField* windfield, GMF* gmf,
				L00Frame* l00_frame);

	//-----------//
	// variables //
	//-----------//

	int				slicesPerSpot;
	double			startTime;
	AntennaSim		antennaSim;		// the antenna simulator

	//---------------------------//
	// level 0 frame information //
	//---------------------------//

	int			l00FrameReady;

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

#endif
