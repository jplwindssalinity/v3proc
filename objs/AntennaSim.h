//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ANTENNASIM_H
#define ANTENNASIM_H

static const char rcs_id_antennasim_h[] =
	"@(#) $Id$";

#include "Antenna.h"

//======================================================================
// CLASSES
//		AntennaSim
//======================================================================


//======================================================================
// CLASS
//		AntennaSim
//
// DESCRIPTION
//		The AntennaSim object contains the information necessary to
//		simulate an antenna by operating on an Antenna object.  It is
//		used to set members of the Antenna object as if the instrument
//		were functioning.
//======================================================================

class AntennaSim
{
public:

	//--------------//
	// construction //
	//--------------//

	AntennaSim();
	~AntennaSim();

	//------------------//
	// antenna position //
	//------------------//

	int		UpdatePosition(double time, Antenna* antenna);

        //-------------------//
        // variables         //
        //-------------------//

	double startTime;
	double startAzimuth;
};

#endif


