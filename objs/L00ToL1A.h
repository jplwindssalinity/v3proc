//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L00TOL1A_H
#define L00TOL1A_H

static const char rcs_id_l00tol1a_h[] =
	"@(#) $Id$";

#include "L00.h"
#include "L1A.h"
#include <string.h>


//======================================================================
// CLASSES
//		L00ToL1A
//======================================================================

//======================================================================
// CLASS
//		L00ToL1A
//
// DESCRIPTION
//		The L00ToL1A object is used to convert between Level 0.0 data
//		and Level 1A data.  It performs all of the engineering unit
//		conversions.
//======================================================================

class L00ToL1A
{
public:

	//--------------//
	// construction //
	//--------------//

	L00ToL1A();
	~L00ToL1A();

	//------------//
	// conversion //
	//------------//

	int		Convert(L00* l00, L1A* l1a);
};

#endif
