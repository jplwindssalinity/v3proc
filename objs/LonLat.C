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
	GetHead();
	while ((lon_lat=RemoveCurrent()) != NULL)
		delete lon_lat;

	return;
}
