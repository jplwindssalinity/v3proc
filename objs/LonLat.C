//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_lonlat_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "LonLat.h"


//========//
// LonLat //
//========//

LonLat::LonLat()
:	longitude(0.0), latitude(0.0)
{
	return;
}

LonLat::LonLat(EarthPosition r)

{
	this->Set(r);
	return;
}

LonLat::~LonLat()
{
	return;
}

//-------------//
// LonLat::Set //
//-------------//

int
LonLat::Set(EarthPosition r)

{
	double alt,lat,lon;
	r.GetAltLonGDLat(&alt, &lon, &lat);
	longitude = (float)lon;
	latitude = (float)lat;
	return(1);
}

//---------------//
// LonLat::Write //
//---------------//

int
LonLat::Write(
	FILE*	fp)
{
	if (fwrite((void *)&longitude, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&latitude, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//--------------//
// LonLat::Read //
//--------------//

int
LonLat::Read(
	FILE*	fp)
{
	if (fread((void *)&longitude, sizeof(float), 1, fp) != 1 ||
		fread((void *)&latitude, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------//
// LonLat::WriteBvg //
//------------------//

int
LonLat::WriteBvg(
	FILE*	fp)
{
	if (fwrite((void *)&longitude, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&latitude, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//=========//
// Outline //
//=========//

Outline::Outline()
{
	return;
}

Outline::~Outline()
{
	EarthPosition* r;
	GotoHead();
	while ((r=RemoveCurrent()) != NULL)
		delete r;

	return;
}

//----------------//
// Outline::Write //
//----------------//

//
// Oulines are written out as LonLat objects even though they are stored as
// EarthPositions.  Thus, nonzero altitudes will be lost.
//

int
Outline::Write(
	FILE*	fp)
{
	int count = NodeCount();
	if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	LonLat lon_lat;
    for (EarthPosition* r = GetHead(); r; r = GetNext())
	{
		lon_lat.Set(*r);
        if (! lon_lat.Write(fp))
			return(0);
	}
	return(1);
}

//---------------//
// Outline::Read //
//---------------//

int
Outline::Read(
	FILE*	fp)
{
	FreeContents();

	int count;
	if (fread((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	LonLat lon_lat;
	for (int i = 0; i < count; i++)
	{
		EarthPosition *new_r = new EarthPosition();
		if (! lon_lat.Read(fp) ) return(0);
		new_r->SetAltLonGDLat(0.0, lon_lat.longitude, lon_lat.latitude);
		if (! Append(new_r)) return(0);
	}
	return(1);
}

//-------------------//
// Outline::WriteBvg //
//-------------------//

int
Outline::WriteBvg(
	FILE*	fp)
{
	// write the points
	EarthPosition* r;
	LonLat lon_lat;
    for (r = GetHead(); r; r = GetNext())
	{
		lon_lat.Set(*r);
        if (! lon_lat.WriteBvg(fp))
			return(0);
	}

	// close the figure
	r = GetHead();
	lon_lat.Set(*r);
	if (! lon_lat.WriteBvg(fp))
		return(0);

	// indicate done
	LonLat inf;
	inf.longitude = (float)HUGE_VAL;
	inf.latitude = (float)HUGE_VAL;
	if (! inf.WriteBvg(fp))
		return(0);

	return(1);
}

//-----------------------//
// Outline::FreeContents //
//-----------------------//

void
Outline::FreeContents()
{
	EarthPosition* r;
	GotoHead();
	while ((r = RemoveCurrent()) != NULL)
		delete r;
	return;
}
