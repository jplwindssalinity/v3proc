//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef LONLAT_H
#define LONLAT_H

static const char rcs_id_lonlat_h[] =
	"@(#) $Id$";

#include "List.h"
#include "List.C"


//======================================================================
// CLASSES
//		LonLat, Outline
//======================================================================

//======================================================================
// CLASS
//		LonLat
//
// DESCRIPTION
//		The LonLat object contains a longitude and a latitude.
//======================================================================

class LonLat
{
public:

	//--------------//
	// construction //
	//--------------//

	LonLat();
	~LonLat();

	//-----------//
	// variables //
	//-----------//

	float	longitude;
	float	latitude;
};

//======================================================================
// CLASS
//		Outline
//
// DESCRIPTION
//		The Outline object contains a list of LonLat.  Typically used
//		to indicate the outline of a cell on the earth.
//======================================================================

class Outline : public List<LonLat>
{
public:

	//--------------//
	// construction //
	//--------------//

	Outline();
	~Outline();
};

#endif
