//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include <unistd.h>
#include "Measurement.h"
#include "Beam.h"


//=============//
// Measurement //
//=============//

Measurement::Measurement()
:	pol(NONE), value(0.0), incidenceAngle(0.0), scAzimuth(0.0),
	eastAzimuth(0.0), centerLongitude(0.0), centerLatitude(0.0),
	estimatedKp(1.0)
{
	return;
}

Measurement::~Measurement()
{
	return;
}

//-----------------------------//
// Measurement::WriteL17Format //
//-----------------------------//

int
Measurement::WriteL17Format(
	int		ofd)
{
	if (write(ofd, &pol, sizeof(PolE)) != sizeof(PolE) ||
		write(ofd, &value, sizeof(double)) != sizeof(double) ||
		write(ofd, &incidenceAngle, sizeof(double)) != sizeof(double) ||
		write(ofd, &scAzimuth, sizeof(double)) != sizeof(double) ||
		write(ofd, &eastAzimuth, sizeof(double)) != sizeof(double) ||
		write(ofd, &centerLongitude, sizeof(double)) != sizeof(double) ||
		write(ofd, &centerLatitude, sizeof(double)) != sizeof(double))
	{
		return(0);
	}
	return(1);
}

//----------------------------//
// Measurement::ReadL17Format //
//----------------------------//

int
Measurement::ReadL17Format(
	int		ifd)
{
	if (read(ifd, &pol, sizeof(PolE)) != sizeof(PolE) ||
		read(ifd, &value, sizeof(double)) != sizeof(double) ||
		read(ifd, &incidenceAngle, sizeof(double)) != sizeof(double) ||
		read(ifd, &scAzimuth, sizeof(double)) != sizeof(double) ||
		read(ifd, &eastAzimuth, sizeof(double)) != sizeof(double) ||
		read(ifd, &centerLongitude, sizeof(double)) != sizeof(double) ||
		read(ifd, &centerLatitude, sizeof(double)) != sizeof(double))
	{
		return(0);
	}
	return(1);
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

//---------------------------------//
// MeasurementList::WriteL17Format //
//---------------------------------//

int
MeasurementList::WriteL17Format(
	int		ofd)
{
	int count = NodeCount();
	if (write(ofd, &count, sizeof(int)) != sizeof(int))
		return(0);

	for (Measurement* meas = GetHead(); meas; meas = GetNext())
	{
		if (! meas->WriteL17Format(ofd))
			return(0);
	}
	return(1);
}

//--------------------------------//
// MeasurementList::ReadL17Format //
//--------------------------------//

int
MeasurementList::ReadL17Format(
	int		ifd)
{
	int count;
	if (read(ifd, &count, sizeof(int)) != sizeof(int))
		return(0);

	for (int i = 0; i < count; i++)
	{
		Measurement* new_meas = new Measurement();
		if (! new_meas)
			return(0);

		if (! new_meas->ReadL17Format(ifd) ||
			! Append(new_meas))
		{
			delete new_meas;
			return(0);
		}
	}
	return(1);
}
