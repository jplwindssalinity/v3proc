//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L10TOL15_H
#define L10TOL15_H

static const char rcs_id_l10tol15_h[] =
	"@(#) $Id$";

#include "L10.h"
#include "L15.h"


//======================================================================
// CLASSES
//		L10ToL15
//======================================================================

//======================================================================
// CLASS
//		L10ToL15
//
// DESCRIPTION
//		The L10ToL15 object is used to convert between Level 0.0 data
//		and Level 1.0 data.  It performs all of the engineering unit
//		conversions.
//======================================================================

class L10ToL15
{
public:

	//--------------//
	// construction //
	//--------------//

	L10ToL15();
	~L10ToL15();

	//------------//
	// conversion //
	//------------//

	int		Convert(L10* l10, L15* l15);
};

#endif
