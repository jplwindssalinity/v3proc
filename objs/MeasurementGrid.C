//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_measurementgrid_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Measurement.h"
#include "MeasurementGrid.h"
#include "Constants.h"

//
// MeasurementGrid
//

//
// Initialize with a set of grid parameters.
//

MeasurementGrid::MeasurementGrid(
double crosstrack_res,
double alongtrack_res,
double swath_width,
double alongtrack_length,
double start_time
)

{

MeasurementGrid::Set(
crosstrack_res,
alongtrack_res,
swath_width,
alongtrack_length,
start_time
);

}

//
// Default constructor which does no initialization (except to set the
// grid data pointer to NULL).
//

MeasurementGrid::MeasurementGrid()
{
_grid = NULL;
return;
}

//
// Destructor
//

MeasurementGrid::~MeasurementGrid()
{

// Free the grid storage.
if (_grid != NULL)
{
	free_pmatrix(_grid,_min_crosstrack_index,_max_crosstrack_index,
		_min_alongtrack_index,_max_alongtrack_index);
}

return;
}

//
// Set grid parameters and allocate the resulting grid.
// Deallocate an existing grid if present.
//

void MeasurementGrid::Set(
double crosstrack_res,
double alongtrack_res,
double swath_width,
double alongtrack_length,
double start_time
)

{

// Deallocate an already existing grid.
if (_grid != NULL)
{
	free_pmatrix(_grid,_min_crosstrack_index,_max_crosstrack_index,
		_min_alongtrack_index,_max_alongtrack_index);
}

_crosstrack_resolution = crosstrack_res;
_alongtrack_resolution = alongtrack_res;
_swath_width = swath_width;
_alongtrack_length = alongtrack_length;
_start_time = start_time;

// Compute index ranges
_max_crosstrack_index = _swath_width/2/_crosstrack_resolution;
_min_crosstrack_index = -_max_crosstrack_index;
_max_alongtrack_index = _alongtrack_length/_alongtrack_resolution - 0.5;
_min_alongtrack_index = 0;

// The grid is a matrix of pointers to MeasurementList's.

_grid = pmatrix(_min_crosstrack_index,_max_crosstrack_index,
		_min_alongtrack_index,_max_alongtrack_index);

}

//
// Fill a grid with the appropriate measurement objects in the input list.
// Those measurement objects that belong in the grid are snipped out of the
// list, and appended to another list at the corresponding grid location.
// Thus, the input list will have all of the gridded objects removed.  Any
// measurement objects left in the input list do not belong in the grid.
// They will likely end up in the next grid filled.  It is the responsibility
// of the calling routine to supply all of the measurement objects that
// belong in the grid.  This can be done with more than one call to Fill
// because Fill will append the objects to the existing grid.
//
// INPUTS:
//
//   mlist = a List object for the input list of measurement objects to grid.
//   ephemeris = object that contains info about the orbit to use.
//

void MeasurementGrid::Fill(MeasurementList mlist, Ephemeris ephemeris)

{

// Start at the beginning of the input list.
Measurement *current_measurement = mlist.GetHead();

double crosstrack,alongtrack;
int index_crosstrack,index_alongtrack;

while (current_measurement)
{	// Traverse the list, looking for objects that belong in the grid.

	// Determine the subtrack coordinates of the current measurement.
	current_measurement -> centerPosition.GetSubtrackCoordinates(ephemeris,
		_start_time, &crosstrack, &alongtrack);

	// Compute grid indices.
	index_crosstrack = crosstrack / _crosstrack_resolution;
	index_alongtrack = alongtrack / _alongtrack_resolution;

	// If on the grid (coordinates in range) then append the current
	// measurement to the grid.
	if ((index_crosstrack <= _max_crosstrack_index) &&
		(index_crosstrack >= _min_crosstrack_index) &&
	    (index_alongtrack <= _max_alongtrack_index) &&
	    (index_alongtrack >= _min_alongtrack_index))
	{
		// snip out of the input list
		current_measurement = mlist.RemoveCurrent();
		// append to the appropriate grid measurement list
		_grid[index_crosstrack][index_alongtrack].Append(current_measurement);
	}

	// Move to next object in the input list.
	current_measurement = mlist.GetNext();
}

}
