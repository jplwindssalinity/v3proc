//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include "Meas.h"
#include "Beam.h"


//======//
// Meas //
//======//

Meas::Meas()
:	value(0.0), pol(NONE), eastAzimuth(0.0), incidenceAngle(0.0),
	estimatedKp(0.0)
{
	return;
}

Meas::~Meas()
{
	return;
}

//-------------//
// Meas::Write //
//-------------//

int
Meas::Write(
	FILE*	fp)
{
	if (fwrite((void *)&value, sizeof(float), 1, fp) != 1 ||
		outline.Write(fp) != 1 ||
		center.Write(fp) != 1 ||
		fwrite((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fwrite((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&estimatedKp, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------//
// Meas::Read //
//------------//

int
Meas::Read(
	FILE*	fp)
{
	if (fread((void *)&value, sizeof(float), 1, fp) != 1 ||
		outline.Read(fp) != 1 ||
		center.Read(fp) != 1 ||
		fread((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fread((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fread((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fread((void *)&estimatedKp, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//==========//
// MeasList //
//==========//

MeasList::MeasList()
{
	return;
}

MeasList::~MeasList()
{
	FreeContents();
	return;
}

//-----------------//
// MeasList::Write //
//-----------------//

int
MeasList::Write(
	FILE*	fp)
{
	int count = NodeCount();
	if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		if (! meas->Write(fp))
			return(0);
	}
	return(1);
}

//----------------//
// MeasList::Read //
//----------------//

int
MeasList::Read(
	FILE*	fp)
{
	int count;
	if (fread((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	for (int i = 0; i < count; i++)
	{
		Meas* new_meas = new Meas();
		if (! new_meas->Read(fp) ||
			! Append(new_meas))
		{
			return(0);
		}
	}
	return(1);
}

//------------------------//
// MeasList::FreeContents //
//------------------------//

int
MeasList::FreeContents()
{
	Meas* meas;
	GetHead();
	while ((meas = RemoveCurrent()) != NULL)
		delete meas;
	return(1);
}


//==========//
// MeasSpot //
//==========//

MeasSpot::MeasSpot()
:	time(0.0), scOrbitState(), scAttitude(), slices()
{
	return;
}

MeasSpot::~MeasSpot()
{
	return;
}

//-----------------//
// MeasSpot::Write //
//-----------------//

int
MeasSpot::Write(
	FILE*	fp)
{
	if (fwrite((void *)&time, sizeof(double), 1, fp) != 1 ||
		scOrbitState.Write(fp) != 1 ||
		scAttitude.Write(fp) != 1 ||
		slices.Write(fp) != 1)
	{
		return(0);
	}
	return(1);
}

//----------------//
// MeasSpot::Read //
//----------------//

int
MeasSpot::Read(
	FILE*	fp)
{
	if (fread((void *)&time, sizeof(double), 1, fp) != 1 ||
		scOrbitState.Read(fp) != 1 ||
		scAttitude.Read(fp) != 1 ||
		slices.Read(fp) != 1)
	{
		return(0);
	}
	return(1);
}


//==============//
// MeasSpotList //
//==============//

MeasSpotList::MeasSpotList()
{
	return;
}

MeasSpotList::~MeasSpotList()
{
	MeasSpot* meas_spot;
	GetHead();
	while ((meas_spot=RemoveCurrent()) != NULL)
		delete meas_spot;

	return;
}

//---------------------//
// MeasSpotList::Write //
//---------------------//

int
MeasSpotList::Write(
	FILE*	fp)
{
	int count = NodeCount();
	if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	for (MeasSpot* meas_spot = GetHead(); meas_spot; meas_spot = GetNext())
	{
		if (! meas_spot->Write(fp))
			return(0);
	}
	return(1);
}

//--------------------//
// MeasSpotList::Read //
//--------------------//

int
MeasSpotList::Read(
	FILE*	fp)
{
	int count;
	if (fread((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	for (int i = 0; i < count; i++)
	{
		MeasSpot* new_meas_spot = new MeasSpot();
		if (! new_meas_spot->Read(fp) ||
			! Append(new_meas_spot))
		{
			return(0);
		}
	}
	return(1);
}

//----------------------------//
// MeasSpotList::FreeContents //
//----------------------------//

int
MeasSpotList::FreeContents()
{
	MeasSpot* meas_spot;
	GetHead();
	while ((meas_spot = RemoveCurrent()) != NULL)
		delete meas_spot;
	return(1);
}
