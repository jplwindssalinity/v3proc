//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include "Measurement.h"


//=============//
// Measurement //
//=============//

Measurement::Measurement()
:	pol(SCATTEROMETER_V_POL), value(0.0), valuedB(0.0), incidenceAngle(0.0),
	scAzimuth(0.0), northAzimuth(0.0), centerLongitude(0.0),
	centerLatitude(0.0), estimatedKp(1.0)
{
	return;
}

Measurement::~Measurement()
{
	return;
}


//=================//
// MeasurementList //
//=================//

MeasurementList::MeasurementList()
{
	return;
}

MeasurementList::~MeasurementList()
{
	return;
}
