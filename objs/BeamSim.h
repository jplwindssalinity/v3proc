//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef BEAMSIM_H
#define BEAMSIM_H

static const char rcs_id_beamsim_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		BeamSim
//======================================================================

//======================================================================
// CLASS
//		BeamSim
//
// DESCRIPTION
//		The BeamSim object contains the information necessary to
//		simulate a beam by operating on a Beam object.  It is used
//		to set members of the Beam object as if the instrument were
//		functioning.
//======================================================================

class BeamSim
{
public:

	//--------------//
	// construction //
	//--------------//

	BeamSim();
	~BeamSim();
};

#endif
