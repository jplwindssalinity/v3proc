//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include "Meas.h"
#include "Beam.h"
#include "Constants.h"
#include "LonLat.h"
#include "GenericGeom.h"


//======//
// Meas //
//======//

Meas::Meas()
:	value(0.0), XK(0.0), EnSlice(0.0), bandwidth(0.0),
	transmitPulseWidth(0.0), pol(NONE), eastAzimuth(0.0), incidenceAngle(0.0),
	A(0.0), B(0.0), C(0.0), offset(0)
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
	// Sanity check on sigma0 and estimated Kp
	if (fabs(value) > 1.0e5)
	{
		printf("Error: Meas::Write encountered invalid sigma0 = %g\n",value);
		exit(-1);
	}

	if (fwrite((void *)&value, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&XK, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&transmitPulseWidth, sizeof(float), 1, fp) != 1 ||
		outline.Write(fp) != 1 ||
		centroid.WriteLonLat(fp) != 1 ||
		fwrite((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fwrite((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&A, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&B, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&C, sizeof(float), 1, fp) != 1)
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
	offset = ftell(fp);
	if (fread((void *)&value, sizeof(float), 1, fp) != 1 ||
		fread((void *)&XK, sizeof(float), 1, fp) != 1 ||
		fread((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
		fread((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
		fread((void *)&transmitPulseWidth, sizeof(float), 1, fp) != 1 ||
		outline.Read(fp) != 1 ||
		centroid.ReadLonLat(fp) != 1 ||
		fread((void *)&pol, sizeof(PolE), 1, fp) != 1 ||
		fread((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		fread((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		fread((void *)&A, sizeof(float), 1, fp) != 1 ||
		fread((void *)&B, sizeof(float), 1, fp) != 1 ||
		fread((void *)&C, sizeof(float), 1, fp) != 1)
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

//-------------------//
// Meas::EstimatedKp //
//-------------------//

float
Meas::EstimatedKp(float sigma0)
{
	float Tp = 0.0015;		// transmit pulse length (need to handle better!)
	float Kpr2 = 0.00512;	// 0.3 dB
	float Kpm2 = 0.03059;	// 0.7 dB

	// if measurement is nonsense, return a Kp of 1.0
        // Since A is set to zero by Er_to_sigma0 if useKpc is zero
        // this should handle that case as well.
	if (A == 0.0 || EnSlice==0)
		return(1.0);

	float snr = sigma0 * XK * Tp/EnSlice;
	if (snr < 0.0)
	{
		fprintf(stderr,
			"Error: Meas::EstimatedKp computed negative SNR = %g\n", snr);
        fprintf(stderr, "S0=%g XK=%g EnSlice=%g\n", sigma0, XK, EnSlice);
		exit(1);
	}
	float Kpc2 = A + B/snr + C/snr/snr;
	float Kp = sqrt(Kpr2 + Kpm2 + Kpc2);
	return(Kp);
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
	EarthPosition sum;
	sum.SetPosition(0.0, 0.0, 0.0);
	for (Meas* meas = GetHead(); meas; meas = GetNext())
	{
		sum += meas->centroid;
	}

	// The center of the earth is at 0,0,0 (geocentric coords)
	EarthPosition earth_center;
	earth_center.SetPosition(0.0, 0.0, 0.0);
	// Find the surface point lying along the averaged direction.
	EarthPosition ravg = earth_intercept(earth_center,sum);

	LonLat lon_lat;
	lon_lat.Set(ravg);
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


//============//
// OffsetList //
//============//

OffsetList::OffsetList()
{
	return;
}

OffsetList::~OffsetList()
{
	FreeContents();
	return;
}

//--------------------------//
// OffsetList::MakeMeasList //
//--------------------------//

int
OffsetList::MakeMeasList(
	FILE*		fp,
	MeasList*	meas_list)
{
	for (long* offset = GetHead(); offset; offset = GetNext())
	{
		Meas* meas = new Meas();
		if (fseek(fp, *offset, SEEK_SET) == -1)
			return(0);

		if (! meas->Read(fp))
			return(0);

		if (! meas_list->Append(meas))
		{
			delete meas;
			return(0);
		}
	}

	return(1);
}

//--------------------------//
// OffsetList::FreeContents //
//--------------------------//

void
OffsetList::FreeContents()
{
	long* offset;
	GotoHead();
	while ((offset = RemoveCurrent()) != NULL)
		delete offset;
	return;
}

//================//
// OffsetListList //
//================//

OffsetListList::OffsetListList()
{
	return;
}

OffsetListList::~OffsetListList()
{
	FreeContents();
	return;
}

//------------------------------//
// OffsetListList::FreeContents //
//------------------------------//

void
OffsetListList::FreeContents()
{
	OffsetList* offsetlist;
	GotoHead();
	while ((offsetlist = RemoveCurrent()) != NULL)
		delete offsetlist;
	return;
}

//==========//
// MeasSpot //
//==========//

MeasSpot::MeasSpot()
:	time(0.0), scOrbitState(), scAttitude()
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
		MeasList::Write(fp) != 1)
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
		MeasList::Read(fp) != 1)
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
