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
	return;
}

Grid::~Grid()
{
	return;
}

//----------------//
// Grid::Allocate //
//----------------//

int
Grid::Allocate(
	int		cross_track_bins,
	int		along_track_bins)
{
	_grid = (MeasList ***)malloc(cross_track_bins * sizeof(MeasList **));
	if (_grid == NULL)
		return(0);

	for (int i = 0; i < cross_track_bins; i++)
	{
		MeasList** ptr =
			(MeasList **)malloc(along_track_bins * sizeof(MeasList *));
		if (ptr == NULL)
			return(0);
		*(_grid + i) = ptr;
		for (int j = 0; j < along_track_bins; j++)
		{
			MeasList* ml = new MeasList();
			if (ml == NULL)
				return(0);
			*(*(_grid + i) + j) = ml;
		}
	}
	return(1);
}
