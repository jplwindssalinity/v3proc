//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_grid_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Grid.h"
#include "Meas.h"
#include "Sigma0.h"


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
		free_array(_grid, 2, _crosstrack_bins, _alongtrack_bins);
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

	_grid = (OffsetListList**)make_array(sizeof(OffsetListList), 2,
		_crosstrack_bins, _alongtrack_bins);
	if (_grid == NULL)
		return(0);

	// Make an empty OffsetListList object,
	// and use to initialize each grid element
	OffsetListList empty_list;
	for (int i=0; i < _crosstrack_bins; i++)
	{
		for (int j=0; j < _alongtrack_bins; j++)
		{
			_grid[i][j] = empty_list;
		}
	}

	return(1);
}

//-----------//
// Grid::Add //
//-----------//
// Add the measurement to the grid using an offset list.
// meas = pointer to the Meas object to grid
// meas_time = time at which the measurement was made
// spot_id = unique integer for each measurement spot - used for compositing.
// do_composite = 1 if slices should be composited, 0 if not.

int
Grid::Add(
	Meas*		meas,
	double		meas_time,
	long		spot_id,
	int			do_composite)
{
	//----------------------------------//
	// calculate the subtrack distances //
	//----------------------------------//

	float ctd, atd;
	if (ephemeris.GetSubtrackCoordinates(meas->centroid, _start_time,
		meas_time, &ctd, &atd) == 0)
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
//		delete meas;
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
	{
		// vati is beyond latest row, so need to shift the grid buffer
		ShiftForward(do_composite);
	}

	//
	// Convert the along track index into the virtual buffer (vati) to an
	// index into the grid in memory (ati).
	//

	int ati = (vati - _ati_offset + _ati_start) % _alongtrack_bins;

	// convert the measurement to an offset
	
	long* offset = new long;
	*offset = meas->offset;

    OffsetList* offsetlist = _grid[cti][ati].GetHead();

	if (do_composite == 1)
	{	// Composite slices that fall in the same grid location and spot.

	    // Scan for an offset list with the same spot id.
    	while (offsetlist != NULL)
    	{
        	if (offsetlist->spotId == spot_id)
        	{
            	break;
        	}
        	offsetlist = _grid[cti][ati].GetNext();
    	}

    	if (offsetlist == NULL)
    	{
        	// Append a new offsetlist (with the new offset) for the new spot
			offsetlist = new OffsetList;
			offsetlist->Append(offset);
			offsetlist->spotId = spot_id;
			_grid[cti][ati].Append(offsetlist);
    	}
		else
		{
			// Append the offset in the list with the matching spot id.
			offsetlist->Append(offset);
		}
	}
	else
	{	// Put all slices in the first (and only) offset list at this grid loc.
    	if (offsetlist == NULL)
    	{	// Need to create a sublist and attach to the grid square
			offsetlist = new OffsetList;
			if (offsetlist == NULL)
			{
				printf("Error allocating memory in Grid::Add\n");
				exit(-1);
			}
			_grid[cti][ati].Append(offsetlist);
    	}
		offsetlist->Append(offset);
	}

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
// If requested, slices are composited before output.
// The slices to be composited should all be together in a sublist of their
// grid location.
//
// Inputs:
//	do_composite = flag set to 1 if compositing is desired, 0 otherwise.

int
Grid::ShiftForward(int do_composite)
{
	//---------------------------//
	// remember the l15 location //
	//---------------------------//

	FILE* fp = l15.GetFp();
	long offset = ftell(fp);
	if (offset == -1)
		return(0);

	MeasList spot_measList;

	// Write out the earliest row of measurement lists.
	for (int i=0; i < _crosstrack_bins; i++)
	{
		//----------------------------------------//
		// convert each offset list to a MeasList //
		//----------------------------------------//

		l17.frame.measList.FreeContents();

		if (do_composite == 1)
		{
        	for (OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
             	offsetlist;
             	offsetlist = _grid[i][_ati_start].GetNext())
        	{   // each sublist is composited before output
            	offsetlist->MakeMeasList(fp, &spot_measList);
				Meas* meas = new Meas;
				composite(&spot_measList,meas);
				spot_measList.FreeContents();
				if (! l17.frame.measList.Append(meas))
				{
					printf("Error forming list for output in Grid::ShiftForward\n");
					delete meas;
					return(0);
				}
        	}
		}
		else
		{
        	OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
			if (offsetlist != NULL)
			{
            	offsetlist->MakeMeasList(fp, &(l17.frame.measList));
			}
		}

		//----------------------------------//
		// complete and write the l17 frame //
		//----------------------------------//

		l17.frame.rev = 0;
		l17.frame.cti = i;
		l17.frame.ati = _ati_offset;
		if (l17.frame.measList.GetHead() != NULL)
		{
			l17.WriteDataRec();
		}

		//----------------------//
		// free the offset list //
		//----------------------//

		_grid[i][_ati_start].FreeContents();
	}

	// Update buffer indices.
	_ati_start = (_ati_start + 1) % _alongtrack_bins;
	_ati_offset++;

	//----------------------//
	// restore l15 location //
	//----------------------//

	if (fseek(fp, offset, SEEK_SET) == -1)
		return(0);

	return(1);
}

//
// Grid::Flush
//
// Write out all the grid rows in memory.
//

int
Grid::Flush(int do_composite)
{
	for (int i = 0; i < _alongtrack_bins; i++)
	{
		ShiftForward(do_composite);
	}

	return(1);
}
