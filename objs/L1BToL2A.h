//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1BTOL2A_H
#define L1BTOL2A_H

static const char rcs_id_l1btol2a_h[] =
	"@(#) $Id$";

#include "L1B.h"
#include "Grid.h"
#include "Antenna.h"
#include "Ephemeris.h"


//======================================================================
// CLASSES
//		L1BToL2A
//======================================================================

//======================================================================
// CLASS
//		L1BToL2A
//
// DESCRIPTION
//		The L1BToL2A object is used to convert between Level 1B data
//		and Level 2A data.  It performs co-location or grouping of the
//		measurements.
//======================================================================

class L1BToL2A
{
public:

	//--------------//
	// construction //
	//--------------//

	L1BToL2A();
	~L1BToL2A();

	//------------//
	// conversion //
	//------------//

	int		Group(Grid* grid, int do_composite);
};

#endif
