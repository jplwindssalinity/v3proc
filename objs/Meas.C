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
#include "Constants.h"
#include "LonLat.h"


//======//
// Meas //
//======//

Meas::Meas()
:	value(0.0), pol(NONE), eastAzimuth(0.0), incidenceAngle(0.0),
	estimatedKp(1.0)
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
	FreeContents();
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

//------------------//
// Meas::WriteAscii //
//------------------//

int
Meas::WriteAscii(
	FILE*	fp)
{
	fprintf(fp, "Pol: %s\n", beam_map[(int)pol]);
	fprintf(fp, "Azi: %g\n", eastAzimuth);
	fprintf(fp, "Inc: %g\n", incidenceAngle);
	fprintf(fp, "Val: %g\n", value);
	return(1);
}

//--------------------//
// Meas::FreeContents //
//--------------------//

void
Meas::FreeContents()
{
	outline.FreeContents();
	return;
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
	FreeContents();

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

//----------------------//
// MeasList::WriteAscii //
//----------------------//

int
MeasList::WriteAscii(
	FILE*	fp)
{
	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		if (! meas->WriteAscii(fp))
			return(0);
	}
	return(1);
}

//-------------------------//
// MeasList::AverageLonLat //
//-------------------------//

LonLat
MeasList::AverageLonLat()
{
	// determine if the 360/0 gap is jumped (eg. points at 1 and 359 lon)
	int above_pi = 0;
	int below_pi = 0;
	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		if (meas->center.longitude > pi)
			above_pi = 1;
		else
			below_pi = 1;
	}

	float lon = 0.0;
	float lat = 0.0;
	int count = 0;
	if (above_pi && below_pi)
	{
		for (Meas* meas = GetHead(); meas; meas = GetNext())
		{
			lon += meas->center.longitude;
			if (meas->center.longitude > pi)
				lon -= two_pi;		// wrapping was necessary
			lat += meas->center.latitude;
			count++;
		}
	}
	else
	{
		for (Meas* meas = GetHead(); meas; meas = GetNext())
		{
			lon += meas->center.longitude;
			lat += meas->center.latitude;
			count++;
		}
	}

	lat /= (float)count;
	lon /= (float)count;
	if (lon < 0.0)
		lon += two_pi;

	LonLat lon_lat;
	lon_lat.longitude = lon;
	lon_lat.latitude = lat;

	return(lon_lat);
}

//------------------------//
// MeasList::FreeContents //
//------------------------//

void
MeasList::FreeContents()
{
	Meas* meas;
	GotoHead();
	while ((meas = RemoveCurrent()) != NULL)
		delete meas;
	return;
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
	FreeContents();

	if (fread((void *)&time, sizeof(double), 1, fp) != 1 ||
		scOrbitState.Read(fp) != 1 ||
		scAttitude.Read(fp) != 1 ||
		slices.Read(fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------------//
// MeasSpot::FreeContents //
//------------------------//

void
MeasSpot::FreeContents()
{
	slices.FreeContents();
	return;
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
	GotoHead();
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
	FreeContents();

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

void
MeasSpotList::FreeContents()
{
	MeasSpot* meas_spot;
	GotoHead();
	while ((meas_spot = RemoveCurrent()) != NULL)
		delete meas_spot;
	return;
}
