//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MEASUREMENTGRID_H
#define MEASUREMENTGRID_H

static const char rcs_id_measurmentgrid_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		MeasurementGrid
//======================================================================

class MeasurementGrid;

//======================================================================
// CLASS
//		MeasurementGrid
//
// DESCRIPTION
//		The MeasurementGrid object contains a 2-D array of MeasurementList
//		objects and additional information that specifies the grid spacing
//		and dimensions.  Construction of the grid means grouping measurment
//		data (supplied in one long MeasurementList object) according to
//		some kind of proximity criteria, and storing the resulting lists
//		in the grid locations determined by the subtrack coordinates
//		of the data.  The input MeasurementList is taken apart in this
//	    process.  Thus, after constructing the MeasurementGrid object,
//	 	the input list will	have lost all the elements that were put into
//		the grid.
//======================================================================

class MeasurementGrid
{
public:

//--------------//
// construction //
//--------------//

MeasurementGrid::MeasurementGrid(
double crosstrack_res,
double alongtrack_res,
double swath_width,
double alongtrack_length,
double start_time
);

MeasurementGrid();
~MeasurementGrid();


//-------------------------//
//-------------------------//

void MeasurementGrid::Set(
double crosstrack_res,
double alongtrack_res,
double swath_width,
double alongtrack_length,
double start_time
);
void MeasurementGrid::Fill(MeasurementList mlist, Ephemeris ephemeris);

protected:

//-----------//
// variables //
//-----------//

double _crosstrack_resolution;	// km
double _alongtrack_resolution;	// km
double _swath_width;			// km (total width)
double _alongtrack_length;		// km (starting from nadir point at time_start)
int _max_crosstrack_index;
int _min_crosstrack_index;
int _max_alongtrack_index;
int _min_alongtrack_index;

//
// _start_time specifies the beginning of the subtrack grid.
//

double _start_time;

MeasurementList **_grid;

};

#endif
