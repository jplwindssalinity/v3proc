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

	double	lookAngle;			// mounted look angle
	double	azimuthAngle;		// mounted azimuth angle
};

#endif
