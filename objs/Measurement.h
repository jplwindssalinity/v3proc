//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

static const char rcs_id_measurement_h[] =
	"@(#) $Id$";

#include "List.h"


//======================================================================
// CLASSES
//		Measurement, MeasurementList
//======================================================================

//======================================================================
// CLASS
//		Measurement
//
// DESCRIPTION
//		The Measurement object is a general purpose object for
//		holding and manipulating sigma-0 and brightness temperature
//		data.
//======================================================================

class Measurement
{
public:

	//--------------//
	// construction //
	//--------------//

	Measurement();
	~Measurement();

	//-----------//
	// variables //
	//-----------//

	double		value;			// sigma-0 or temperature measurement...
	double		valuedB;		// ...and in dB
	double		incidenceAngle;
	double		scAzimuth;		// az. angle relative to s/c
	double		northAzimuth;	// az. angle relative to north

	double		centerLongitude;
	double		centerLatitude;
};

//======================================================================
// CLASS
//		MeasurementList
//
// DESCRIPTION
//		The MeasurementList object is a list of Measurements.  This is
//		the output of the sigma-0 grouper and is fed into the wind
//		retriever.
//======================================================================

class MeasurementList : List<Measurement>
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasurementList();
	~MeasurementList();

	//----------//
	// analysis //
	//----------//

	double		GetAverage();
};

#endif
