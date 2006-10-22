//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef GRID_H
#define GRID_H

static const char rcs_id_grid_h[] =
	"@(#) $Id$";

#include "Array.h"
#include "L1B.h"
#include "L2A.h"
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
        enum GridMethodE{ CENTROID, OVERLAP };
	//--------------//
	// construction //
	//--------------//

	Grid();
	~Grid();

	int		Allocate(double crosstrack_res, double alongtrack_res,
				double crosstrack_size, double alongtrack_size);

	int		SetStartTime(double start_time);
	int		SetEndTime(double end_time);

	//---------------------//
	// adding measurements //
	//---------------------//

	int		Add(Meas *meas, double meas_time, long spot_id, int do_composite);

	//--------------//
	// input/output //
	//--------------//

	int		ShiftForward(int do_composite);
	int		Flush(int do_composite);

	//-----------//
	// variables //
	//-----------//

	L1B			l1b;		// handles input from a level 1B data file.
	L2A			l2a;		// handles output to a level 2A data file.
	Ephemeris	ephemeris;	// defines the grid location

        int meas_length;
        //float gctd[181][361];
        //float gatd[181][361];
        float gctd[361][721];  // 1st index is lat, 2nd is lon
        float gatd[361][721];  // 1st index is lat, 2nd is lon

        double lat_end_time;
        GridMethodE method;
        float overlapFactor;
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
	// end_time specifes the largest along track index that will be gridded.
	// start_position is the nadir point corresponding to start_time.
	//

	EarthPosition _start_position;
	double _start_time;
	double _end_time;

	// index corresponding to _end_time.
	int _max_vati;

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

	int				_ati_start;
	int				_ati_offset;

	double			_orbit_period;

	OffsetListList**	_grid;			// the grid of lists of offset lists
};

#endif
