//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GRID_H
#define GRID_H

static const char rcs_id_grid_h[] =
	"@(#) $Id$";

#include "Measurement.h"


//======================================================================
// CLASSES
//		GridRow, Grid
//======================================================================

//======================================================================
// CLASS
//		GridRow
//
// DESCRIPTION
//		The GridRow object holds a row of MeasurementLists.
//======================================================================

class GridRow
{
public:

	//--------------//
	// construction //
	//--------------//

	GridRow();
	~GridRow();

	Allocate(int size);

	//---------------------//
	// adding measurements //
	//---------------------//

	int		Add(Measurement* measurement, int cti);

	//--------------//
	// input/output //
	//--------------//

	int		Write(int ofd);

	//-----------//
	// variables //
	//-----------//

	int					ati;		// the along track index for the row
	MeasurementList*	row;
};

//======================================================================
// CLASS
//		Grid
//
// DESCRIPTION
//		The Grid object is a list of GridRow.
//======================================================================

class Grid : public List<GridRow>
{
public:

	//--------------//
	// construction //
	//--------------//

	Grid();
	~Grid();

	//---------------------//
	// adding measurements //
	//---------------------//

	int			Add(Measurement* measurement);

	GridRow*	FindRow(int ati);
	GridRow*	AddRow(int ati);

	//--------------//
	// input/output //
	//--------------//

	GetCoordinates(double time, float longitude, float latitude, float* ctd,
		float* atd);

	//-----------//
	// variables //
	//-----------//

	float	crossTrackResolution;
	float	alongTrackResolution;
	int		bufferRows;				// the number of rows to buffer
	int		outputFd;				// the output file
};

#endif
