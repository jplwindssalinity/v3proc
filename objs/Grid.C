//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_grid_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Grid.h"


//=========//
// GridRow //
//=========//

GridRow::GridRow()
{
	return;
}

GridRow::~GridRow()
{
	return;
}

//======//
// Grid //
//======//

Grid::Grid()
{
	return;
}

Grid::~Grid()
{
	return;
}

//-----------//
// Grid::Add //
//-----------//

int
Grid::Add(
	Measurement*	measurement)
{
	//-----------------------------------------------------//
	// determine the cross track and along track distances //
	//-----------------------------------------------------//

	float ctd, atd;
	GetCoordinates(measurement->time, measurement->centerLongitude,
		measurement->centerLatitude, &ctd, &atd);

	//--------------------------------//
	// determine the grid coordinates //
	//--------------------------------//

	int cti = (int)(ctd / crossTrackResolution);
	int ati = (int)(atd / alongTrackResolution);

	//----------------------//
	// find appropriate row //
	//----------------------//

	GridRow* row = FindRow(ati);

	//-------------------------------//
	// allocate new row if necessary //
	//-------------------------------//

	if (! row)
	{
		row = AddRow(ati);
		if (row == NULL)
			return(0);
	}

	//------------------------//
	// add measurement to row //
	//------------------------//

	row->Add(measurement, cti);

	//----------------------------------------//
	// write and deallocate rows if necessary //
	//----------------------------------------//

	do
	{
		row = GetTail();
		if (row->ati <= ati - bufferRows)
		{
			if (! row->Write(outputFd))
				return(0);
			RemoveCurrent();
		}
		else
			break;
	} while (1);

	//--------------------------------------//
	// deallocate ephemerities if necessary //
	//--------------------------------------//

//	ephemeris.FreeBefore(measurement->time - bufferTime);

	return(1);
}
