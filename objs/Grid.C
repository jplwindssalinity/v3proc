//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_grid_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Grid.h"


//======//
// Grid //
//======//

Grid::Grid()
{
	_grid = NULL;
	_ephemeris = NULL;
	return;
}

Grid::~Grid()
{
	if (_grid != NULL)
	{
		free_array(_grid,2,_alongtrack_bins,_crosstrack_bins);
	}
	return;
}

int
Grid::SetEphemeris(Ephemeris *ephemeris)
{
	_ephemeris = ephemeris;
	return(1);
}

int
Grid::SetStartTime(double start_time)
{
	_start_time = start_time;
	return(1);
}

//----------------//
// Grid::Allocate //
//----------------//

int
Grid::Allocate(
double	crosstrack_res,
double	alongtrack_res,
double	crosstrack_size,
double	alongtrack_size)
{
	_alongtrack_res = alongtrack_res;
	_crosstrack_res = crosstrack_res;
	_alongtrack_size = alongtrack_size;
	_crosstrack_size = crosstrack_size;
    _alongtrack_bins = (int) (_alongtrack_size / _alongtrack_res);
    _crosstrack_bins = (int) (_crosstrack_size / _crosstrack_res);

	//
	// Set circular buffer tracking parameters to the beginning of the
	// virtual grid.
	//

	_ati_start = 0;
	_ati_offset = 0;

	_grid = (MeasList**)make_array(sizeof(MeasList),2,
		_alongtrack_bins,_crosstrack_bins);
	if (_grid == NULL) return(0);

	return(1);
}

//
// Grid::Add
//
// Add one measurment to the grid of measurements.
//
// meas = pointer to the Meas object to grid.
// meas_time = time at which the measurement was made.
//

int
Grid::Add(Meas *meas, double meas_time)
{

EarthPosition rground(meas->center.latitude,meas->center.longitude,
                      EarthPosition::GEODETIC);
float ctd,atd;
_ephemeris->GetSubtrackCoordinates(rground,_start_time,meas_time,&ctd,&atd);

//
// Compute grid indices, noting that the cross track grid starts on the left
// side at cti = 0.
//

int cti = (int) ((ctd + _crosstrack_size/2.0)/_crosstrack_res);
int vati = (int) (atd/_alongtrack_res);	// virtual along track index.

if ((cti >= _crosstrack_bins) || (cti < 0))
{
	printf("Error: crosstrack index = %d out of range in Grid::Add\n",cti);
	return(0);
}

//
// Note that reverse shifting is not implemented.
// Thus, if vati falls before the earliest row in memory, it is considered
// out of range instead of trying to back up the buffer.  This limitation
// is imposed by BufferedList.
//

if (vati < _ati_offset)
{
	printf("Error: alongtrack index = %d out of range in Grid::Add\n",vati);
	return(0);
}

//
// Determine if the along track index is in memory or not.
// If not, read and write rows until it is in range.
//

while (vati - _ati_offset >= _alongtrack_bins)
{	// vati is beyond latest row, so need to shift the grid buffer
	ShiftForward();
}

//
// Convert the along track index into the virtual buffer (vati) to an
// index into the grid in memory (ati).
//

int ati = vati - _ati_offset + _ati_start;
if (ati >= _alongtrack_bins) ati -= _alongtrack_bins;

// Add the measurement to the appropriate grid cell measurement list.
_grid[ati][cti].Append(meas);

return(1);

}

//
// Grid::ShiftForward
//
// Shift the grid of measurements by one row.
// The earliest row is written out to the output file and its storage
// is reset to receive another row.  The circular buffer tracking indices
// are updated appropriately so that the newly freed row is mapped to
// the next along track index.
//

int
Grid::ShiftForward()
{

// Construct level 1.7 frames from each grid cell

// Write out the earliest row of measurement lists.
for (int i=0; i < _crosstrack_bins; i++)
{
l17->frame.measList = _grid[_ati_start][i];
l17->WriteDataRec();
_grid[_ati_start][i].FreeContents();	// prepare for new data
}

// Update buffer indices.
_ati_start++;
if (_ati_start >= _alongtrack_bins) _ati_start = 0;
_ati_offset++;

return(1);

}
