//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L0TOL1_H
#define L0TOL1_H

static const char rcs_id_l0tol1_h[] =
	"@(#) $Id$";

#include "L0.h"
#include "L1.h"

//======================================================================
// CLASSES
//		L0ToL1
//======================================================================


//======================================================================
// CLASS
//		L0ToL1
//
// DESCRIPTION
//		The L0ToL1 object is used to convert Level 0 data to Level 1
//		data
//======================================================================

class L0ToL1
{
public:

	//--------------//
	// construction //
	//--------------//

	L0ToL1();
	~L0ToL1();

	//------------//
	// conversion //
	//------------//

	int		Convert(L0* l0, L1* l1);
};

#endif
