//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef LONLAT_H
#define LONLAT_H

static const char rcs_id_lonlat_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "List.h"
#include "EarthPosition.h"


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

	Set(EarthPosition r);

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	int		WriteAscii(FILE* fp);
	int		WriteBvg(FILE* fp);

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
//		The Outline object contains a list of EarthPositions.  Typically used
//		to indicate the outline of a cell on the earth.
//======================================================================

class Outline : public List<EarthPosition>
{
public:

	//--------------//
	// construction //
	//--------------//

	Outline();
	~Outline();

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	int		WriteAscii(FILE* fp);
	int		WriteBvg(FILE* fp);

	double	Area();
	
	//---------//
	// freeing //
	//---------//

	void	FreeContents();
};

#endif
