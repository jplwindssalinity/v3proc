//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L1TOL15_H
#define L1TOL15_H

static const char rcs_id_l1tol15_h[] =
	"@(#) $Id$";

#include "L1.h"
#include "L15.h"

//======================================================================
// CLASSES
//		L1ToL15
//======================================================================


//======================================================================
// CLASS
//		L1ToL15
//
// DESCRIPTION
//		The L1ToL15 object is used to convert Level 1 data to Level 1.5
//		data
//======================================================================

class L1ToL15
{
public:

	//--------------//
	// construction //
	//--------------//

	L1ToL15();
	~L1ToL15();

	//------------//
	// conversion //
	//------------//

	int		Convert(L1* l1, L15* l15);
};

#endif
