//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_lonlat_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "LonLat.h"
#include "Constants.h"
#include "EarthPosition.h"
#include "List.h"


//========//
// LonLat //
//========//

LonLat::LonLat()
:	longitude(0.0), latitude(0.0)
{
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
	double alt, lat, lon;
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

//--------------------//
// LonLat::WriteAscii //
//--------------------//

int
LonLat::WriteAscii(
	FILE*	fp)
{
	fprintf(fp, "%g %g\n", longitude * rtd, latitude * rtd);
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

//---------------------//
// Outline::WriteAscii //
//---------------------//

int
Outline::WriteAscii(
	FILE*	fp)
{
	// write the points
	EarthPosition* r;
	LonLat lon_lat;
    for (r = GetHead(); r; r = GetNext())
	{
		lon_lat.Set(*r);
        if (! lon_lat.WriteAscii(fp))
			return(0);
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
	int count = NodeCount();

	if (count < 1)
		return(1);

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

//---------------//
// Outline::Area //
//---------------//

//
// Find the area enclosed by the outline on a locally flat surface.
// This method assumes (but does not check) that the points are stored
// in order going around the perimeter.
//

double
Outline::Area()
{
	
	int num = NodeCount();

	if (num <= 2)
	{	// points and lines have no area!
		return(0.0);
	}
	else
	{
		EarthPosition* p1 = GetHead();
		double mag_p1 = p1->Magnitude();

		EarthPosition* p2;
		EarthPosition* p3 = GetNext();
		double area = 0.0;

		for (int i=3; i <= num; i++)
		{	// sum area from all the sub-triangles.

			// move to next triangle (always keeping p1 the same)
			p2 = p3;
			p3 = GetNext();

			// compute position magnitudes (some inefficiency here)
			double mag_p2 = p2->Magnitude();
			double mag_p3 = p3->Magnitude();
			double R1 = (mag_p1 + mag_p2 + mag_p3)/3.0;

			// Side lengths from the earth center angle.
			double s12 = R1*acos((*p1 % *p2) / mag_p1 / mag_p2);
			double s23 = R1*acos((*p2 % *p3) / mag_p2 / mag_p3);
			double s31 = R1*acos((*p3 % *p1) / mag_p3 / mag_p1);
	
			if (s12 != 0.0 & s23 != 0.0 & s31 != 0.0)
			{	// 3 points are distinct, so add area contribution
				// Cosine law
				double A1 = acos(-(s23*s23 - s12*s12 - s31*s31)/s12/s31/2.0);
				area += 0.5*s12*s31*sin(A1);
			}
		}
		return(area);
	}

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
