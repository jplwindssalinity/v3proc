//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_lonlat_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "LonLat.h"


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

//=========//
// Outline //
//=========//

Outline::Outline()
{
	return;
}

Outline::~Outline()
{
	LonLat* lon_lat;
	GotoHead();
	while ((lon_lat=RemoveCurrent()) != NULL)
		delete lon_lat;

	return;
}

//----------------//
// Outline::Write //
//----------------//

int
Outline::Write(
	FILE*	fp)
{
	int count = NodeCount();
	if (fwrite((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

    for (LonLat* lon_lat = GetHead(); lon_lat; lon_lat = GetNext())
	{
        if (! lon_lat->Write(fp))
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
	int count;
	if (fread((void *)&count, sizeof(int), 1, fp) != 1)
		return(0);

	for (int i = 0; i < count; i++)
	{
		LonLat* new_lon_lat = new LonLat();
		if (! new_lon_lat->Read(fp) ||
			! Append(new_lon_lat))
		{
			return(0);
		}
	}
	return(1);
}
