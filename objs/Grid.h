//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GRID_H
#define GRID_H

static const char rcs_id_grid_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		Grid
//======================================================================

//======================================================================
// CLASS
//		Grid
//
// DESCRIPTION
//		The Grid object is a grid (or subgrid) of co-located
//		measurements.  It can be used as a circular buffer.
//======================================================================

class Grid
{
public:

	//--------------//
	// construction //
	//--------------//

	Grid();
	~Grid();

	int		Allocate(int cross_track_bins, int along_track_bins);

	//---------------------//
	// adding measurements //
	//---------------------//

	//--------------//
	// input/output //
	//--------------//

protected:

	//-----------//
	// variables //
	//-----------//

	MeasList***		_grid;
};

#endif
