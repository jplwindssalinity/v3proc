//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L15TOL17_H
#define L15TOL17_H

static const char rcs_id_l15tol17_h[] =
	"@(#) $Id$";

#include "L15.h"
#include "Grid.h"
#include "Antenna.h"
#include "Ephemeris.h"


//======================================================================
// CLASSES
//		L15ToL17
//======================================================================

//======================================================================
// CLASS
//		L15ToL17
//
// DESCRIPTION
//		The L15ToL17 object is used to convert between Level 1.5 data
//		and Level 1.7 data.  It performs co-location or grouping of the
//		measurements.
//======================================================================

class L15ToL17
{
public:

	//--------------//
	// construction //
	//--------------//

	L15ToL17();
	~L15ToL17();

	//------------//
	// conversion //
	//------------//

	int		Group(Grid* grid);
};

#endif
