//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef LONLAT_H
#define LONLAT_H

static const char rcs_id_lonlat_h[] =
	"@(#) $Id$";

#include "List.h"


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

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

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

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	int		WriteBvg(FILE* fp);

	//---------//
	// freeing //
	//---------//

	void	FreeContents();
};

#endif
