//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ANTENNA_H
#define ANTENNA_H

static const char rcs_id_antenna_h[] =
	"@(#) $Id$";

#include "Beam.h"

#define MAX_NUMBER_OF_BEAMS		2

//======================================================================
// CLASSES
//		Antenna
//======================================================================

//======================================================================
// CLASS
//		Antenna
//
// DESCRIPTION
//		The Antenna object contains antenna information.  It represents
//		the actual state of the antenna.
//======================================================================

class Antenna
{
public:

	//--------------//
	// construction //
	//--------------//

	Antenna();
	~Antenna();

	//-----------//
	// variables //
	//-----------//

	int		numberOfBeams;
	Beam	beam[MAX_NUMBER_OF_BEAMS];

	double	azimuthAngle;	// antenna azimuth angle
};

#endif
