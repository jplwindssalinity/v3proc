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
:	_crosstrack_res(0.0), _alongtrack_res(0.0),
	_crosstrack_size(0.0), _alongtrack_size(0.0),
	_crosstrack_bins(0), _alongtrack_bins(0),
	_start_time(0.0), _ati_start(0), _ati_offset(0),
	_grid(NULL)
{
	return;
}

Grid::~Grid()
{
	if (_grid != NULL)
	{
		free_array(_grid,2,_crosstrack_bins,_alongtrack_bins);
	}
	return;
}

int
Grid::SetStartTime(double start_time)
{
	_start_time = start_time;
	l17.header.startTime = _start_time;
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

	// Put data into L17 object for use in header.
	l17.header.crossTrackResolution = _crosstrack_res;
	l17.header.alongTrackResolution = _alongtrack_res;
	l17.header.crossTrackBins = _crosstrack_bins;
	l17.header.alongTrackBins = _alongtrack_bins;
	l17.header.zeroIndex = (int) (_crosstrack_size/2.0/_crosstrack_res + 0.5);

	//
	// Set circular buffer tracking parameters to the beginning of the
	// virtual grid.
	//

	_ati_start = 0;
	_ati_offset = 0;

	_grid = (MeasList**)make_array(sizeof(MeasList),2,
		_crosstrack_bins,_alongtrack_bins);
	if (_grid == NULL) return(0);

	// Make an empty MeasList object, and copy it into each grid list.
	MeasList empty_meas_list;

	for (int i=0; i < _crosstrack_bins; i++)
	for (int j=0; j < _alongtrack_bins; j++)
	{
		_grid[i][j] = empty_meas_list;
	}

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
Grid::Add(
	Meas	*meas,
	double	meas_time)
{
	float ctd, atd;
	if (ephemeris.GetSubtrackCoordinates(meas->centroid, _start_time,
		meas_time,&ctd,&atd) == 0)
	{
		return(0);	// Couldn't find a grid position, so dump this measurement.
	}

	//
	// Compute grid indices, noting that the cross track grid starts on the left
	// side at cti = 0.
	//

	int cti = (int) ((ctd + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
	int vati = (int) (atd/_alongtrack_res);	// virtual along track index.

	if ((cti >= _crosstrack_bins) || (cti < 0))
	{
		printf("Error: crosstrack index = %d out of range in Grid::Add\n",cti);
		return(0);
	}

	//
	// Negative vati means the point falls before the defined grid start.
	// For this special case, Add() returns success, but dumps the measurement.
	// If vati falls inside the defined grid, but in a portion that has been
	// already output, then an error message is generated (see the next
	// if-block after this one).
	//

	if (vati < 0)
	{
		delete meas;
		return(1);
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
	_grid[cti][ati].Append(meas);

	//printf("%d %d %f %f\n",cti,vati,ctd,atd);
	return(1);
}

//
// Grid::ShiftForward
//
// Shift the grid of measurements by one alongtrack row.
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
	l17.frame.measList = _grid[i][_ati_start];
	l17.frame.rev = 0;
	l17.frame.cti = i;
	l17.frame.ati = _ati_offset;
	if (l17.frame.measList.GetHead() != NULL)
	{
		// only write a L1.7 frame if it contains some measurements
		l17.WriteDataRec();
	}
	_grid[i][_ati_start].FreeContents();	// prepare for new data
}

// Update buffer indices.
_ati_start++;
if (_ati_start >= _alongtrack_bins) _ati_start = 0;
_ati_offset++;

return(1);
}

//
// Grid::Flush
//
// Write out all the grid rows in memory.
//

int
Grid::Flush()
{
	for (int i = 0; i < _alongtrack_bins; i++)
	{
		ShiftForward();
	}

	return(1);
}
