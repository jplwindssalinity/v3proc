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
:	value(0.0), valuedB(0.0), incidenceAngle(0.0), centerLongitude(0.0),
	centerLatitude(0.0)
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

//-----------------------------//
// MeasurementList::GetAverage //
//-----------------------------//

double
MeasurementList::GetAverage()
{
	double sum = 0.0;
	unsigned int count = 0;
	for (Measurement* meas = GetHead(); meas; meas = GetNext())
	{
		sum += meas->value;
		count++;
	}
	double avg = sum / (double)count;
	return(avg);
}
