//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_windfield_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include "WindField.h"


//===========//
// WindField //
//===========//

WindField::WindField()
:	_lonCount(0), _lonMin(0.0), _lonMax(0.0), _lonStep(0.0), _latCount(0),
	_latMin(0.0), _latMax(0.0), _latStep(0.0), _field(NULL)
{
	return;
}

WindField::~WindField()
{
	_Deallocate();
	return;
}

//--------------------//
// WindField::ReadVap //
//--------------------//

#define VAP_LON_DIM		360
#define VAP_LAT_DIM		121

int
WindField::ReadVap(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	int ifd = open(filename, O_RDONLY);
	if (ifd == -1)
		return(0);

	//------------//
	// read field //
	//------------//

	float u[VAP_LON_DIM][VAP_LAT_DIM];
	float v[VAP_LON_DIM][VAP_LAT_DIM];

	int size = VAP_LON_DIM * VAP_LAT_DIM * sizeof(float);
	if (read(ifd, u, size) != size ||
		read(ifd, v, size) != size)
	{
		close(ifd);
		return(0);
	}

	//------------//
	// close file //
	//------------//

	close(ifd);

	//-------------------------------//
	// transfer to wind field format //
	//-------------------------------//

	_lonCount = VAP_LON_DIM;
	_lonMin = 0.0;
	_lonMax = 350.0;
	_lonStep = 1.0;

	_latCount = VAP_LAT_DIM;
	_latMin = -60.0;
	_latMax = 60.0;
	_latStep = 1.0;

	_Allocate();

	for (int lon_idx = 0; lon_idx < VAP_LON_DIM; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < VAP_LAT_DIM; lat_idx++)
		{
			WindVector* wv = new WindVector;
			if (! wv)
				return(0);

			wv->SetUV(u[lon_idx][lat_idx], v[lon_idx][lat_idx]);
			*(*(_field + lon_idx) + lat_idx) = wv;
		}
	}

	return(1);
}

//------------------------------//
// WindField::NearestWindVector //
//------------------------------//

WindVector*
WindField::NearestWindVector(
	double			longitude,
	double			latitude)
{
	int lon_idx = (int)((longitude - _lonMin) / _lonStep + 0.5);
	int lat_idx = (int)((latitude - _latMin) / _latStep + 0.5);
	WindVector* wv = *(*(_field + lon_idx) + lat_idx);

	return(wv);
}

//----------------------//
// WindField::_Allocate //
//----------------------//

int
WindField::_Allocate()
{
	_field = (WindVector ***)malloc(_lonCount * sizeof(WindVector **));
	if (_field == NULL)
		return(0);

	for (int i = 0; i < _lonCount; i++)
	{
		WindVector** ptr =
			(WindVector **)malloc(_latCount * sizeof(WindVector *));
		if (ptr == NULL)
			return(0);

		*(_field + i) = ptr;
		for (int j = 0; j < _latCount; j++)
		{
			*(*(_field + i) + j) = NULL;
		}
	}
	return(1);
}

//------------------------//
// WindField::_Deallocate //
//------------------------//

int
WindField::_Deallocate()
{
	for (int i = 0; i < _lonCount; i++)
	{
		for (int j = 0; j < _latCount; j++)
		{
			WindVector* ptr = *(*(_field + i) + j);
			if (ptr)
				delete *(*(_field + i) + j);
		}
		free(*(_field + i));
	}
	free(_field);
	return(1);
}
