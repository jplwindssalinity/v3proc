//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_wind_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "Wind.h"
#include "Constants.h"


//============//
// WindVector //
//============//

WindVector::WindVector()
:	spd(0.0), dir(0.0)
{
	return;
}

WindVector::~WindVector()
{
	return;
}

//-----------------------//
// WindVector::SetSpdDir //
//-----------------------//
 
int
WindVector::SetSpdDir(
	float	speed,
	float	direction)
{
	spd = speed;
	dir = direction;
	return(1);
}

//-------------------//
// WindVector::SetUV //
//-------------------//
 
int
WindVector::SetUV(
	float	u,
	float	v)
{
	spd = (float)hypot((double)u, (double)v);
	dir = (float)atan2((double)v, (double)u);
	return(1);
}

//================//
// WindVectorPlus //
//================//

WindVectorPlus::WindVectorPlus()
:	obj(0.0)
{
	return;
}

WindVectorPlus::~WindVectorPlus()
{
	return;
}


//=====//
// WVC //
//=====//

WVC::WVC()
:	selectedIdx(-1)
{
	return;
}

WVC::~WVC()
{
	WindVectorPlus* wvp;
	ambiguities.GotoHead();
	while ((wvp=ambiguities.RemoveCurrent()) != NULL)
		delete wvp;

	return;
}

//---------------//
// WVC::WriteL20 //
//---------------//

int
WVC::WriteL20(
	FILE*	fp)
{
	if (fwrite((void *)&selectedIdx, sizeof(char), 1, fp) != 1)
		return(0);

	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		wvp->WriteL20(fp);
	}
	return(1);
}

//-----------------------//
// WVC::WriteAmbigsAscii //
//-----------------------//

int
WVC::WriteAmbigsAscii(
	FILE*		ofp)
{
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		fprintf(ofp, "Spd:%g Dir:%g Obj:%g\n", wvp->spd, wvp->dir * rtd,
			wvp->obj);
	}
	return(1);
}

//-----------------------//
// WVC::RemoveDuplicates //
//-----------------------//

int
WVC::RemoveDuplicates()
{
	int count = 0;

	//--------------------------------------//
	// use a new WVC to identify duplicates //
	//--------------------------------------//

	WVC new_wvc;
	WindVectorPlus* wvp = ambiguities.GetHead();
	while (wvp)
	{
		int match_found = 0;
		for (WindVectorPlus* wvp_tmp = new_wvc.ambiguities.GetHead();
			wvp_tmp && wvp_tmp != wvp;
			wvp_tmp = new_wvc.ambiguities.GetNext())
		{
			if (wvp_tmp->spd == wvp->spd &&
				wvp_tmp->dir == wvp->dir)
			{
				match_found = 1;
				break;
			}
		}
		if (match_found)
		{
			wvp = ambiguities.RemoveCurrent();	// next becomes current
			delete wvp;
			wvp = ambiguities.GetCurrent();
			count++;
		}
		else
		{
			new_wvc.ambiguities.Append(wvp);
			wvp = ambiguities.GetNext();
		}
	}

	//---------------------//
	// get rid of new list //
	//---------------------//

	wvp = new_wvc.ambiguities.GetHead();
	while (wvp)
		wvp = new_wvc.ambiguities.RemoveCurrent();

	return(count);
}

//----------------//
// WVC::SortByObj //
//----------------//
// uses idiot-sort (based on the stupidity and laziness of JNH)
// sorts in descending order (the highest obj is first)

int
WVC::SortByObj()
{
	int need_sorting = 1;

	while (need_sorting)
	{
		need_sorting = 0;
		for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
			wvp = ambiguities.GetNext())
		{
			WindVectorPlus* next_wvp = ambiguities.GetNext();
			if (next_wvp)
			{
				ambiguities.GetPrev();
				if (next_wvp->obj > wvp->obj)
				{
					ambiguities.SwapCurrentAndNext();
					ambiguities.GetNext();
					need_sorting = 1;
				}
			}
		}
	}
	return(1);
}

//-------------------//
// WVC::FreeContents //
//-------------------//

void
WVC::FreeContents()
{
	WindVectorPlus* wvp;
	ambiguities.GotoHead();
	while ((wvp = ambiguities.RemoveCurrent()))
		delete wvp;
	return;
}


//===========//
// WindField //
//===========//

WindField::WindField()
:	_lonCount(0), _lonMin(0.0), _lonMax(0.0), _lonStep(0.0),
	_latCount(0), _latMin(0.0), _latMax(0.0), _latStep(0.0),
	_field(0)
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
	float	longitude,
	float	latitude)
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


//===============//
// WindFieldPlus //
//===============//

WindFieldPlus::WindFieldPlus()
:	_lonCount(0), _lonMin(0.0), _lonMax(0.0), _lonStep(0.0),
	_latCount(0), _latMin(0.0), _latMax(0.0), _latStep(0.0),
	_field(0)
{
	return;
}

WindFieldPlus::~WindFieldPlus()
{
	_Deallocate();
	return;
}

//--------------------------//
// WindFieldPlus::_Allocate //
//--------------------------//

int
WindFieldPlus::_Allocate()
{
	_field = (WVC ***)malloc(_lonCount * sizeof(WVC **));
	if (_field == NULL)
		return(0);

	for (int i = 0; i < _lonCount; i++)
	{
		WVC** ptr =
			(WVC **)malloc(_latCount * sizeof(WVC *));
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

//----------------------------//
// WindFieldPlus::_Deallocate //
//----------------------------//

int
WindFieldPlus::_Deallocate()
{
	for (int i = 0; i < _lonCount; i++)
	{
		for (int j = 0; j < _latCount; j++)
		{
			WVC* ptr = *(*(_field + i) + j);
			if (ptr)
				delete *(*(_field + i) + j);
		}
		free(*(_field + i));
	}
	free(_field);
	return(1);
}
