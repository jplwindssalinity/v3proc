//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef KP_H
#define KP_H

static const char rcs_id_kp_h[] =
	"@(#) $Id$";

#include "PiscTable.h"


//======================================================================
// CLASSES
//		Kp
//======================================================================

//======================================================================
// CLASS
//		Kp
//
// DESCRIPTION
//		The Kp object holds estimates of total Kp as a function of
//		polarization, incidence angle, wind speed, and relative wind
//		direction (chi).
//======================================================================

class Kp : public PiscTable
{
public:

	//--------------//
	// construction //
	//--------------//

	Kp();
	~Kp();
};

#endif
