//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef BEAM_H
#define BEAM_H

static const char rcs_id_beam_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Beam
//======================================================================

//======================================================================
// CLASS
//		Beam
//
// DESCRIPTION
//		The Beam object contains beam state information.
//======================================================================

#include "Attitude.h"

enum PolE { NONE, V_POL, H_POL };

class Beam
{
public:

	//--------------//
	// construction //
	//--------------//

	Beam();
	~Beam();

	//-----------//
	// variables //
	//-----------//

	double	lookAngle;			// mounted look angle relative to antenna
	double	azimuthAngle;		// mounted azimuth angle relative to antenna
	PolE	polarization;

	// Note: the beamFrame needs to be formed using the lookAngle and
	// azimuthAngle above.
	Attitude beamFrame;			// beam frame relative to antenna frame
};

#endif
