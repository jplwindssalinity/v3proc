//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_wind_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "Wind.h"
#include "Constants.h"
#include "Misc.h"
#include "Array.h"


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

//-------------------//
// WindVector::GetUV //
//-------------------//

int
WindVector::GetUV(
	float*	u,
	float*	v)
{
	*u = spd * (float)cos((double)dir);
	*v = spd * (float)sin((double)dir);
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

//--------------------------//
// WindVectorPlus::WriteL20 //
//--------------------------//

int
WindVectorPlus::WriteL20(
	FILE*	fp)
{
	if (fwrite((void *)&spd, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&dir, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&obj, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//-------------------------//
// WindVectorPlus::ReadL20 //
//-------------------------//

int
WindVectorPlus::ReadL20(
	FILE*	fp)
{
	if (fread((void *)&spd, sizeof(float), 1, fp) != 1 ||
		fread((void *)&dir, sizeof(float), 1, fp) != 1 ||
		fread((void *)&obj, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}


//=====//
// WVC //
//=====//

WVC::WVC()
:	selected(NULL)
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
	//----------------------------------//
	// write the longitude and latitude //
	//----------------------------------//

	if (fwrite((void *)&(lonLat.longitude), sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&(lonLat.latitude), sizeof(float), 1, fp) != 1)
	{
		return(0);
	}

	//---------------------------//
	// write the number of nodes //
	//---------------------------//

	unsigned char count = ambiguities.NodeCount();
	if (fwrite((void *)&count, sizeof(unsigned char), 1, fp) != 1)
		return(0);

	//-------------//
	// write nodes //
	//-------------//

	char selected_idx = -1;
	char idx = 0;

	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		if (! wvp->WriteL20(fp))
			return(0);

		if (wvp == selected)
			selected_idx = idx;

		idx++;
	}

	//----------------------//
	// write selected index //
	//----------------------//

	if (fwrite((void *)&selected_idx, sizeof(char), 1, fp) != 1)
		return(0);

	return(1);
}

//--------------//
// WVC::ReadL20 //
//--------------//

int
WVC::ReadL20(
	FILE*	fp)
{
	//---------------------------------//
	// read the longitude and latitude //
	//---------------------------------//

	if (fread((void *)&(lonLat.longitude), sizeof(float), 1, fp) != 1 ||
		fread((void *)&(lonLat.latitude), sizeof(float), 1, fp) != 1)
	{
		return(0);
	}

	//--------------------------//
	// read the number of nodes //
	//--------------------------//

	unsigned char count;
	if (fread((void *)&count, sizeof(unsigned char), 1, fp) != 1)
		return(0);

	//------------//
	// read nodes //
	//------------//

	for (int i = 0; i < count; i++)
	{
		WindVectorPlus* wvp = new WindVectorPlus();

		if (! wvp->ReadL20(fp))
			return(0);

		if (! ambiguities.Append(wvp))
			return(0);
	}

	//---------------------//
	// read selected index //
	//---------------------//

	char selected_idx;
	if (fread((void *)&selected_idx, sizeof(char), 1, fp) != 1)
		return(0);

	//----------------------//
	// set selected pointer //
	//----------------------//

	selected = ambiguities.GetNodeWithIndex((int)selected_idx);

	return(1);
}

//----------------//
// WVC::WriteVctr //
//----------------//

int
WVC::WriteVctr(
	FILE*		fp,
	const int	rank)
{
	WindVectorPlus* write_me = NULL;
	if (rank == 0)
		write_me = selected;
	else
		write_me = ambiguities.GetNodeWithIndex(rank - 1);

	if (! write_me)
		return(1);

	if (fwrite((void *)&(lonLat.longitude), sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&(lonLat.latitude), sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&(write_me->spd), sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&(write_me->dir), sizeof(float), 1, fp) != 1)
	{
		return(0);
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
	int need_sorting;
	do
	{
		need_sorting = 0;
		for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
			wvp = ambiguities.GetNext())
		{
			WindVectorPlus* next_wvp = ambiguities.GetNext();
			if (next_wvp)
			{
				ambiguities.GotoPrev();
				if (next_wvp->obj > wvp->obj)
				{
					ambiguities.SwapCurrentAndNext();
					need_sorting = 1;
				}
			}
		}
	} while (need_sorting);
	return(1);
}

//----------------------------//
// WVC::GetNearestToDirection //
//----------------------------//

WindVectorPlus*
WVC::GetNearestToDirection(
	float	dir)
{
	WindVectorPlus* nearest = NULL;
	float min_dif = two_pi;
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		float dif = ANGDIF(wvp->dir, dir);
		if (dif < min_dif)
		{
			min_dif = dif;
			nearest = wvp;
		}
	}
	return(nearest);
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

int
WindField::ReadVap(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	//------------//
	// read field //
	//------------//

	float u[VAP_LAT_DIM][VAP_LON_DIM];
	float v[VAP_LAT_DIM][VAP_LON_DIM];

	int size = VAP_LON_DIM * VAP_LAT_DIM * sizeof(float);
	if (fread((void *)u, size, 1, fp) != 1 ||
		fread((void *)v, size, 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//------------//
	// close file //
	//------------//

	fclose(fp);

	//-------------------------------//
	// transfer to wind field format //
	//-------------------------------//

	_lonCount = VAP_LON_DIM;
	_lonMin = 0.0 * dtr;
	_lonMax = 359.0 * dtr;
	_lonStep = 1.0 * dtr;

	_latCount = VAP_LAT_DIM;
	_latMin = -60.0 * dtr;
	_latMax = 60.0 * dtr;
	_latStep = 1.0 * dtr;

	if (! _Allocate())
		return(0);

	for (int lon_idx = 0; lon_idx < VAP_LON_DIM; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < VAP_LAT_DIM; lat_idx++)
		{
			WindVector* wv = new WindVector;
			if (! wv)
				return(0);

			wv->SetUV(u[lat_idx][lon_idx], v[lat_idx][lon_idx]);
			*(*(_field + lon_idx) + lat_idx) = wv;
		}
	}

	return(1);
}

//---------------------------//
// WindField::ReadEcmwfHiRes //
//---------------------------//

int
WindField::ReadEcmwfHiRes(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	//------------//
	// read field //
	//------------//

	int ymd, hms;
	float u[ECMWF_HIRES_LAT_DIM][ECMWF_HIRES_LON_DIM];
	float v[ECMWF_HIRES_LAT_DIM][ECMWF_HIRES_LON_DIM];

	int int_size = sizeof(int);
	int uv_size = ECMWF_HIRES_LON_DIM * ECMWF_HIRES_LAT_DIM * sizeof(float);

	if (fread((void *)&ymd, int_size, 1, fp) != 1 ||
		fread((void *)&hms, int_size, 1, fp) != 1 ||
		fread((void *)u, uv_size, 1, fp) != 1 ||
		fread((void *)v, uv_size, 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//------------//
	// close file //
	//------------//

	fclose(fp);

	//-------------------------------//
	// transfer to wind field format //
	//-------------------------------//

	_lonCount = ECMWF_HIRES_LON_DIM;
	_lonMin = -180.0 * dtr;
	_lonMax = 179.4375 * dtr;
	_lonStep = 0.5625 * dtr;

	_latCount = ECMWF_HIRES_LAT_DIM;
	_latMin = -90.0 * dtr;
	_latMax = 90.0 * dtr;
	_latStep = 0.5625 * dtr;

	if (! _Allocate())
		return(0);

	for (int lon_idx = 0; lon_idx < ECMWF_HIRES_LON_DIM; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < ECMWF_HIRES_LAT_DIM; lat_idx++)
		{
			WindVector* wv = new WindVector;
			if (! wv)
				return(0);

			wv->SetUV(u[lat_idx][lon_idx], v[lat_idx][lon_idx]);
			*(*(_field + lon_idx) + lat_idx) = wv;
		}
	}

	return(1);
}

//---------------------//
// WindField::ReadType //
//---------------------//

int
WindField::ReadType(
	const char*		filename,
	const char*		type)
{
	if (strcasecmp(type, VAP_TYPE) == 0)
	{
		return(ReadVap(filename));
	}
	else if (strcasecmp(type, ECMWF_HIRES_TYPE) == 0)
	{
		return(ReadEcmwfHiRes(filename));
	}
	else
		return(0);
}

//----------------------//
// WindField::WriteVctr //
//----------------------//

int
WindField::WriteVctr(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

	//--------------//
	// write header //
	//--------------//

	char* hdr = VCTR_HEADER;
	if (fwrite((void *)hdr, 4, 1, fp) != 1)
		return(0);

	//-------//
	// write //
	//-------//

	for (int lon_idx = 0; lon_idx < _lonCount; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < _latCount; lat_idx++)
		{
			WindVector* wv = _field[lon_idx][lat_idx];
			if (wv)
			{
				// calculate longitude and latitude
				float longitude = _lonMin + lon_idx * _lonStep;
				float latitude = _latMin + lat_idx * _latStep;
				if (fwrite((void *)&longitude, sizeof(float), 1, fp) != 1 ||
					fwrite((void *)&latitude, sizeof(float), 1, fp) != 1 ||
					fwrite((void *)&(wv->spd), sizeof(float), 1, fp) != 1 ||
					fwrite((void *)&(wv->dir), sizeof(float), 1, fp) != 1)
				{
					return(0);
				}
			}
		}
	}

	//------------//
	// close file //
	//------------//

	fclose(fp);

	return(1);
}

//------------------------------//
// WindField::NearestWindVector //
//------------------------------//

int
WindField::NearestWindVector(
	LonLat			lon_lat,
	WindVector*		wv)
{
	// put longitude in range (hopefully)
	int wrap_factor = (int)ceil((_lonMin - lon_lat.longitude) / 360.0);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// convert to longitude index
	int lon_idx = (int)((lon - _lonMin) / _lonStep + 0.5);
	if (lon_idx < 0 || lon_idx >= _lonCount)
		return(0);

	// convert to latitude index
	int lat_idx = (int)((lon_lat.latitude - _latMin) / _latStep + 0.5);
	if (lat_idx < 0 || lat_idx >= _latCount)
		return(0);

	*wv = *(*(*(_field + lon_idx) + lat_idx));

	return(1);
}

//-----------------------------------//
// WindField::InterpolatedWindVector //
//-----------------------------------//

int
WindField::InterpolatedWindVector(
	LonLat			lon_lat,
	WindVector*		wv)
{
	// put longitude in range (hopefully)
	int wrap_factor = (int)ceil((_lonMin - lon_lat.longitude) / 360.0);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// find lower longitude index
	int lon_idx_1 = (int)((lon - _lonMin) / _lonStep);
	if (lon_idx_1 < 0 || lon_idx_1 >= _lonCount)
		return(0);
	float lon_1 = _lonMin + lon_idx_1 * _lonStep;

	// find upper longitude index
	int lon_idx_2 = lon_idx_1 + 1;		// try the quick easy way
	if (lon_idx_2 >= _lonCount)
	{
		// must do the long hard way
		lon = lon_lat.longitude + _lonStep;
		wrap_factor = (int)ceil((_lonMin - lon) / 360.0);
		lon += (float)wrap_factor * two_pi;

		lon_idx_2 = (int)((lon - _lonMin) / _lonStep);
		if (lon_idx_2 >= _lonCount)
			return(0);
	}
	if (lon_idx_2 < 0)
		return(0);
	float lon_2 = _lonMin + lon_idx_2 * _lonStep;

	// find lower latitude index
	int lat_idx_1 = (int)((lon_lat.latitude - _latMin) / _latStep);
	if (lat_idx_1 < 0 || lat_idx_1 >= _latCount)
		return(0);
	float lat_1 = _latMin + lat_idx_1 * _latStep;

	// find upper latitude index
	int lat_idx_2 = lat_idx_1 + 1;
	if (lat_idx_2 >= _latCount)
		return(0);
	float lat_2 = _latMin + lat_idx_2 * _latStep;

	float p;
	if (lon_idx_1 == lon_idx_2)
		p = 1.0;
	else
		p = (lon_lat.longitude - lon_1) / (lon_2 - lon_1);
	float pn = 1.0 - p;

	float q;
	if (lat_idx_1 == lat_idx_2)
		q = 1.0;
	else
		q = (lon_lat.latitude - lat_1) / (lat_2 - lat_1);
	float qn = 1.0 - q;

	WindVector* wv1 = *(*(_field + lon_idx_1) + lat_idx_1);
	WindVector* wv2 = *(*(_field + lon_idx_2) + lat_idx_1);
	WindVector* wv3 = *(*(_field + lon_idx_1) + lat_idx_2);
	WindVector* wv4 = *(*(_field + lon_idx_2) + lat_idx_2);

	float u1, u2, u3, u4, v1, v2, v3, v4;
	wv1->GetUV(&u1, &v1);
	wv2->GetUV(&u2, &v2);
	wv3->GetUV(&u3, &v3);
	wv4->GetUV(&u4, &v4);

	float u = pn * qn * u1 + p * qn * u2 + p * q * u3 + pn * q * u4;
	float v = pn * qn * v1 + p * qn * v2 + p * q * v3 + pn * q * v4;

	wv->SetUV(u, v);
	return(1);
}

//----------------------//
// WindField::_Allocate //
//----------------------//

int
WindField::_Allocate()
{
	if (_field != NULL)
		return(0);

	_field = (WindVector ***)make_array(sizeof(WindVector *), 2, _lonCount,
		_latCount);

	if (_field == NULL)
		return(0);

	for (int i = 0; i < _lonCount; i++)
	{
		for (int j = 0; j < _latCount; j++)
		{
			_field[i][j] = NULL;
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
	if (_field == NULL)
		return(1);

	free_array((void *)_field, 2, _lonCount, _latCount);

	_field = NULL;
	return(1);
}


//===========//
// WindSwath //
//===========//

WindSwath::WindSwath()
:	swath(0), _crossTrackBins(0), _alongTrackBins(0), _validCells(0)
{
	return;
}

WindSwath::~WindSwath()
{
	DeleteEntireSwath();
	return;
}

//---------------------//
// WindSwath::Allocate //
//---------------------//

int
WindSwath::Allocate(
	int		cross_track_bins,
	int		along_track_bins)
{
	_crossTrackBins = cross_track_bins;
	_alongTrackBins = along_track_bins;
	return(_Allocate());
}

//----------------//
// WindSwath::Add //
//----------------//

int
WindSwath::Add(
	int		cti,
	int		ati,
	WVC*	wvc)
{
	if (cti < 0 || cti >= _crossTrackBins ||
		ati < 0 || ati >= _alongTrackBins)
	{
		return(0);	// out of range
	}

	if (swath[cti][ati])
		return(0);	// already a cell there

	swath[cti][ati] = wvc;
	_validCells++;
	return(1);
}

//-----------------------//
// WindSwath::DeleteWVCs //
//-----------------------//

int
WindSwath::DeleteWVCs()
{
	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			WVC* wvc = *(*(swath + i) + j);
			if (wvc == NULL)
				continue;

			delete wvc;
			*(*(swath + i) + j) = NULL;
		}
	}
	_validCells = 0;
	return(1);
}

//------------------------------//
// WindSwath::DeleteEntireSwath //
//------------------------------//

int
WindSwath::DeleteEntireSwath()
{
	if (! DeleteWVCs())
		return(0);

	if (! _Deallocate())
		return(0);

	return(1);
}

//---------------------//
// WindSwath::WriteL20 //
//---------------------//

int
WindSwath::WriteL20(
	FILE*	fp)
{
	if (fwrite((void *)&_crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&_alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&_validCells, sizeof(int), 1, fp) != 1)
	{
		return(0);
	}

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = *(*(swath + cti) + ati);
			if (wvc == NULL)
				continue;

			if (fwrite((void *)&cti, sizeof(int), 1, fp) != 1 ||
				fwrite((void *)&ati, sizeof(int), 1, fp) != 1)
			{
				return(0);
			}

			if (! wvc->WriteL20(fp))
				return(0);
		}
	}
	return(1);
}

//--------------------//
// WindSwath::ReadL20 //
//--------------------//

int
WindSwath::ReadL20(
	FILE*	fp)
{
	DeleteEntireSwath();		// in case

	if (fread((void *)&_crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fread((void *)&_alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fread((void *)&_validCells, sizeof(int), 1, fp) != 1)
	{
		return(0);
	}

	_Allocate();

	for (int i = 0; i < _validCells; i++)
	{
		int cti, ati;
		if (fread((void *)&cti, sizeof(int), 1, fp) != 1 ||
			fread((void *)&ati, sizeof(int), 1, fp) != 1)
		{
			return(0);
		}

		WVC* wvc = new WVC();

		if (! wvc->ReadL20(fp))
			return(0);

		*(*(swath + cti) + ati) = wvc;
	}
	return(1);
}

//--------------------//
// WindSwath::ReadL20 //
//--------------------//

int
WindSwath::ReadL20(
	const char*		filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	if (! ReadL20(fp))
		return(0);

	fclose(fp);
	return(1);
}

//----------------------//
// WindSwath::WriteVctr //
//----------------------//

int
WindSwath::WriteVctr(
	const char*		filename,
	const int		rank)		// 0 = selected
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

	//--------------//
	// write header //
	//--------------//

	char* hdr = VCTR_HEADER;
	if (fwrite((void *)hdr, 4, 1, fp) != 1)
		return(0);

	//---------------//
	// write vectors //
	//---------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = *(*(swath + cti) + ati);
			if (wvc == NULL)
				continue;

			if (! wvc->WriteVctr(fp, rank))
				return(0);
		}
	}

	fclose(fp);
	return(1);
}

//-------------------------//
// WindSwath::InitWithRank //
//-------------------------//

int
WindSwath::InitWithRank(
	int		rank)
{
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;
			wvc->selected = wvc->ambiguities.GetNodeWithIndex(rank-1);
			count++;
		}
	}
	return(count);
}

//-------------------------//
// WindSwath::MedianFilter //
//-------------------------//
// Returns the number of passes.

int
WindSwath::MedianFilter(
	int		window_size,
	int		max_passes)
{
	//----------------------------//
	// create a new selection map //
	//----------------------------//

	WindVectorPlus*** new_selected =
		(WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2,
			_crossTrackBins, _alongTrackBins);

	//-------------------------//
	// create a new change map //
	//-------------------------//

	char** change = (char**)make_array(sizeof(char), 2,
		_crossTrackBins, _alongTrackBins);

	//--------------------//
	// prep for filtering //
	//--------------------//

	int half_window = window_size / 2;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			change[cti][ati] = 1;
		}
	}

	//--------//
	// filter //
	//--------//

	int pass = 0;
	do
	{
		int flips = MedianFilterPass(half_window, new_selected, change);
		pass++;
		if (flips == 0)
			break;
	} while (pass < max_passes);

	return(pass);
}

//-----------------------------//
// WindSwath::MedianFilterPass //
//-----------------------------//
// Returns the number of vector changes.

int
WindSwath::MedianFilterPass(
	int					half_window,
	WindVectorPlus***	new_selected,
	char**				change)
{
	//-------------//
	// filter loop //
	//-------------//

	int flips = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		int cti_min = cti - half_window;
		int cti_max = cti + half_window + 1;
		if (cti_min < 0)
			cti_min = 0;
		if (cti_max > _crossTrackBins)
			cti_max = _crossTrackBins;
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			int ati_min = ati - half_window;
			int ati_max = ati + half_window + 1;
			if (ati_min < 0)
				ati_min = 0;
			if (ati_max > _alongTrackBins)
				ati_max = _alongTrackBins;

			//------------------------------//
			// initialize the new selection //
			//------------------------------//

			new_selected[cti][ati] = NULL;
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;

			//-------------------//
			// check for changes //
			//-------------------//

			for (int i = cti_min; i < cti_max; i++)
			{
				for (int j = ati_min; j < ati_max; j++)
				{
					if (change[i][j])
					{
						goto change;
						break;
					}
				}
			}
			continue;		// no changes

		change:

			float min_vector_dif_sum = 9e69;

			for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
				wvp = wvc->ambiguities.GetNext())
			{
				float vector_dif_sum = 0.0;
				float x1 = wvp->spd * cos(wvp->dir);
				float y1 = wvp->spd * sin(wvp->dir);

				for (int i = cti_min; i < cti_max; i++)
				{
					for (int j = ati_min; j < ati_max; j++)
					{
						if (i == cti && j == ati)
							continue;		// don't check central vector

						WVC* other_wvc = swath[i][j];
						if (! other_wvc)
							continue;

						WindVectorPlus* other_wvp = other_wvc->selected;
						if (! other_wvp)
							continue;

						float x2 = other_wvp->spd * cos(other_wvp->dir);
						float y2 = other_wvp->spd * sin(other_wvp->dir);

						float dx = x2 - x1;
						float dy = y2 - y1;
						vector_dif_sum += sqrt(dx*dx + dy*dy);
					}
				}

				if (vector_dif_sum < min_vector_dif_sum)
				{
					min_vector_dif_sum = vector_dif_sum;
					new_selected[cti][ati] = wvp;
				}
			}	// done with ambiguities
		}	// done with ati
	}	// done with cti

	//------------------//
	// transfer updates //
	//------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			change[cti][ati] = 0;
			if (new_selected[cti][ati])
			{
				if (new_selected[cti][ati] != swath[cti][ati]->selected)
				{
					swath[cti][ati]->selected = new_selected[cti][ati];
					change[cti][ati] = 1;
					flips++;
				}
			}
		}
	}

	return(flips);
}

//----------------------//
// WindSwath::RmsSpdErr //
//----------------------//

float
WindSwath::RmsSpdErr(
	WindField*	truth)
{
	//----------------------------------//
	// calculate the sum of the squares //
	//----------------------------------//

	double sum = 0.0;
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			double dif = wvc->selected->spd - true_wv.spd;
			sum += (dif * dif);
			count++;
		}
	}

	//-------------------------------//
	// take the mean and square root //
	//-------------------------------//

	float rms_spd_err = (float)sqrt(sum/(double)count);

	return(rms_spd_err);
}

//----------------------//
// WindSwath::RmsDirErr //
//----------------------//

float
WindSwath::RmsDirErr(
	WindField*	truth)
{
	//----------------------------------//
	// calculate the sum of the squares //
	//----------------------------------//

	double sum = 0.0;
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			double dif = ANGDIF(wvc->selected->dir, true_wv.dir);
			sum += (dif * dif);
			count++;
		}
	}

	//-------------------------------//
	// take the mean and square root //
	//-------------------------------//

	float rms_dir_err = (float)sqrt(sum/(double)count);

	return(rms_dir_err);
}

//------------------//
// WindSwath::Skill //
//------------------//

float
WindSwath::Skill(
	WindField*	truth)
{
	int good_count = 0;
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			WindVectorPlus* nearest = wvc->GetNearestToDirection(true_wv.dir);
			if (nearest == wvc->selected)
				good_count++;

			count++;
		}
	}

	float skill = (float)good_count / (float)count;

	return(skill);
}

//--------------------//
// WindSwath::SpdBias //
//--------------------//

float
WindSwath::SpdBias(
	WindField*	truth)
{
	//---------------------------//
	// calculate the summed bias //
	//---------------------------//

	double sum = 0.0;
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			double dif = wvc->selected->spd - true_wv.spd;
			sum += dif;
			count++;
		}
	}

	//-----------//
	// normalize //
	//-----------//

	float spd_bias = (float)(sum / (double)count);

	return(spd_bias);
}

//---------------------//
// WindSwath::CtdArray //
//---------------------//

int
WindSwath::CtdArray(
	float		cross_track_res,
	float*		ctd_array)
{
	for (int i = 0; i < _crossTrackBins; i++)
	{
		float ctd = ((float)i - ((float)_crossTrackBins - 1.0) / 2.0) *
			cross_track_res;
		*(ctd_array + i) = ctd;
	}
	return(1);
}

//---------------------------//
// WindSwath::RmsSpdErrVsCti //
//---------------------------//

int
WindSwath::RmsSpdErrVsCti(
	WindField*	truth,
	float*		rms_spd_err_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	//----------------------------------//
	// calculate the sum of the squares //
	//----------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			if (true_wv.spd < low_speed || true_wv.spd > high_speed)
				continue;

			float spd_err = wvc->selected->spd - true_wv.spd;
			*(rms_spd_err_array + cti) += (spd_err * spd_err);
			(*(count_array + cti))++;
		}
	}

	//-------------------------------//
	// take the mean and square root //
	//-------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		*(rms_spd_err_array + cti) /= (float)*(count_array + cti);
		*(rms_spd_err_array + cti) = sqrt(*(rms_spd_err_array + cti));
	}

	return(1);
}

//---------------------------//
// WindSwath::RmsDirErrVsCti //
//---------------------------//

int
WindSwath::RmsDirErrVsCti(
	WindField*	truth,
	float*		rms_dir_err_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	//----------------------------------//
	// calculate the sum of the squares //
	//----------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			if (true_wv.spd < low_speed || true_wv.spd > high_speed)
				continue;

			float dir_err = ANGDIF(wvc->selected->dir, true_wv.dir);
			*(rms_dir_err_array + cti) += (dir_err * dir_err);
			(*(count_array + cti))++;
		}
	}

	//-------------------------------//
	// take the mean and square root //
	//-------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		*(rms_dir_err_array + cti) /= (float)*(count_array + cti);
		*(rms_dir_err_array + cti) = sqrt(*(rms_dir_err_array + cti));
	}

	return(1);
}

//-----------------------//
// WindSwath::SkillVsCti //
//-----------------------//

int
WindSwath::SkillVsCti(
	WindField*	truth,
	float*		skill_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	//---------------------//
	// calculate the count //
	//---------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		int good_count = 0;
		int count = 0;
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			if (true_wv.spd < low_speed || true_wv.spd > high_speed)
				continue;

			WindVectorPlus* nearest = wvc->GetNearestToDirection(true_wv.dir);
			if (nearest == wvc->selected)
				good_count++;

			count++;
		}

		*(count_array + cti) = count;
		*(skill_array + cti) = (float)good_count / (float)count;
	}

	return(1);
}

//-------------------------//
// WindSwath::SpdBiasVsCti //
//-------------------------//

int
WindSwath::SpdBiasVsCti(
	WindField*	truth,
	float*		spd_bias_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		double sum = 0.0;
		int count = 0;
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			if (true_wv.spd < low_speed || true_wv.spd > high_speed)
				continue;

            double dif = wvc->selected->spd - true_wv.spd;
            sum += dif;
            count++;
		}

		*(count_array + cti) = count;
		*(spd_bias_array + cti) = (float)(sum / (double)count);
	}

	return(1);
}

//----------------------//
// WindSwath::_Allocate //
//----------------------//

int
WindSwath::_Allocate()
{
	if (swath != NULL)
		return(0);

	swath = (WVC ***)make_array(sizeof(WVC *), 2, _crossTrackBins,
		_alongTrackBins);

	if (swath == NULL)
		return(0);

	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			swath[i][j] = NULL;
		}
	}

	return(1);
}

//------------------------//
// WindSwath::_Deallocate //
//------------------------//

int
WindSwath::_Deallocate()
{
	if (swath == NULL)
		return(1);

	free_array((void *)swath, 2, _crossTrackBins, _alongTrackBins);
	swath = NULL;
	_crossTrackBins = 0;
	_alongTrackBins = 0;
	return(1);
}
