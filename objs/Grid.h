//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GRID_H
#define GRID_H

static const char rcs_id_grid_h[] =
	"@(#) $Id$";

#include "Array.h"
#include "L17.h"
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

	int Allocate(
	double  crosstrack_res,
	double  alongtrack_res,
	double  crosstrack_size,
	double  alongtrack_size);

	int SetStartTime(double start_time);

	//---------------------//
	// adding measurements //
	//---------------------//

	int Add(Meas *meas, double meas_time);

	//--------------//
	// input/output //
	//--------------//

	int		ShiftForward();
	int		Flush();

	//-----------//
	// variables //
	//-----------//

	L17			l17;	// handles output to a level 1.7 data file.
	// The ephemeris object that defines the grid location.
	Ephemeris	ephemeris;

protected:

	// resolution and sizes are in km
	double _crosstrack_res;
	double _alongtrack_res;
	double _crosstrack_size;
	double _alongtrack_size;

	int _crosstrack_bins;
	int _alongtrack_bins;

	//
	// start_time specifies the zero point of the along track axis.
	//

	double _start_time;

	//
	// _ati_start tracks the current earliest point in the circular buffer
	// of grid rows stored in _grid.  Thus, _ati_start = n means that
	// _grid[n] is the first row in memory, and _grid[n-1] is the latest
	// row in memory. (Using arithmetic modulus the length of the grid
	// in memory.)
	// _ati_offset gives the number of rows that the grid in memory is
	// displaced from the virtual grid specifed by the ephemeris object
	// and the grid size parameters. Thus, _ati_offset = n means that
	// _grid[_ati_start] is actually row n of the virtual grid
    // (also zero offset).
	//

	int _ati_start;
	int _ati_offset;

	MeasList** _grid;
};

#endif
