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

//---------------//
// Outline::Area //
//---------------//

//
// Find the area enclosed by the trapezoidal area on a spherical surface.
// This method assumes (but does not check) that the area is trapezoidal,
// and the points are stored in order going around the perimeter.
//

double
Outline::Area()
{
	
	EarthPosition* p1 = GetHead();
	EarthPosition* p2 = GetNext();
	EarthPosition* p3 = GetNext();
	EarthPosition* p4 = GetNext();

	double mag_p1 = p1->Magnitude();
	double mag_p2 = p2->Magnitude();
	double mag_p3 = p3->Magnitude();
	double mag_p4 = p4->Magnitude();

	// Divide into two spherical triangles and compute their areas.

	// Average spherical radius to use in each spherical triangle.
	double R1 = (mag_p1 + mag_p2 + mag_p3)/3.0;
	double R2 = (mag_p1 + mag_p3 + mag_p4)/3.0;

	// Side lengths expressed as the cosine,sine of the earth center angle.
	double cos_s12 = (*p1 % *p2) / mag_p1 / mag_p2;
	double cos_s23 = (*p2 % *p3) / mag_p2 / mag_p3;
	double cos_s31 = (*p3 % *p1) / mag_p3 / mag_p1;
	double cos_s34 = (*p3 % *p4) / mag_p3 / mag_p4;
	double cos_s41 = (*p4 % *p1) / mag_p4 / mag_p1;

	double sin_s12 = sqrt(1.0 - cos_s12*cos_s12);
	double sin_s23 = sqrt(1.0 - cos_s23*cos_s23);
	double sin_s31 = sqrt(1.0 - cos_s31*cos_s31);
	double sin_s34 = sqrt(1.0 - cos_s34*cos_s34);
	double sin_s41 = sqrt(1.0 - cos_s41*cos_s41);

	// Triangle 1 between points 1,2,3.
	// (Angles A1 and A3 for this triangle only)

	// Cosine law for spherical triangles
	// A1 is the angle of vertex 1 of the spherical triangle on the surface
	double cos_A1 = (cos_s23 - cos_s12*cos_s31) / (sin_s12 * sin_s31);
	double A1 = acos(cos_A1);
	double sin_A1 = sin(A1);
	// Sine Law to get the other angles.
	double A2 = asin(sin_s31 * sin_A1 / sin_s23);
	double A3 = asin(sin_s12 * sin_A1 / sin_s23);

	double area1 = (A1 + A2 + A3 - pi)*R1*R1;

	// Triangle 2 between points 1,3,4.
	// (Angles A1 and A3 for this triangle only)

	// Cosine law for spherical triangles
	// A1 is the angle of vertex 1 of the spherical triangle on the surface
	cos_A1 = (cos_s34 - cos_s41*cos_s31) / (sin_s41 * sin_s31);
	A1 = acos(cos_A1);
	sin_A1 = sin(A1);
	// Sine Law to get the other angles.
	A3 = asin(sin_s41 * sin_A1 / sin_s34);
	double A4 = asin(sin_s31 * sin_A1 / sin_s34);

	double area2 = (A1 + A3 + A4 - pi)*R2*R2;

	return(area1 + area2);

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
