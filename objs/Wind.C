//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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
// WindVectorPlus::WriteL2B //
//--------------------------//

int
WindVectorPlus::WriteL2B(
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

//----------------------------//
// WindVectorPlus::WriteAscii //
//----------------------------//

int
WindVectorPlus::WriteAscii(
	FILE*	fp)
{
	fprintf(fp, "Spd=%g Dir=%g Obj=%g\n", spd, dir * rtd, obj);
	return(1);
}

//-------------------------//
// WindVectorPlus::ReadL2B //
//-------------------------//

int
WindVectorPlus::ReadL2B(
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
// WVC::WriteL2B //
//---------------//

int
WVC::WriteL2B(
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
		if (! wvp->WriteL2B(fp))
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
// WVC::ReadL2B //
//--------------//

int
WVC::ReadL2B(
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

		if (! wvp->ReadL2B(fp))
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

	selected = ambiguities.GetByIndex((int)selected_idx);

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
		write_me = ambiguities.GetByIndex(rank - 1);

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

//-----------------//
// WVC::WriteAscii //
//-----------------//

int
WVC::WriteAscii(
	FILE*	fp)
{
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		if (! wvp->WriteAscii(fp))
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
:	_wrap(0), _field(0)
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

	_lon.SpecifyWrappedCenters(0.0 * dtr, 360.0 * dtr, VAP_LON_DIM);
	_lat.SpecifyCenters(-60.0 * dtr, 60.0 * dtr, VAP_LAT_DIM);

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

	_wrap = 1;

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

	_lon.SpecifyWrappedCenters(-180.0 * dtr, 180.0 * dtr, ECMWF_HIRES_LON_DIM);
	_lat.SpecifyCenters(-90.0 * dtr, 90.0 * dtr, ECMWF_HIRES_LAT_DIM);

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

	_wrap = 1;
	return(1);
}

//----------------------------//
// WindField::WriteEcmwfHiRes //
//----------------------------//

int
WindField::WriteEcmwfHiRes(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

	//----------------//
	// transfer to uv //
	//----------------//

	int ymd, hms;
	float u[ECMWF_HIRES_LAT_DIM][ECMWF_HIRES_LON_DIM];
	float v[ECMWF_HIRES_LAT_DIM][ECMWF_HIRES_LON_DIM];

	for (int lon_idx = 0; lon_idx < ECMWF_HIRES_LON_DIM; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < ECMWF_HIRES_LAT_DIM; lat_idx++)
		{
			WindVector* wv = *(*(_field + lon_idx) + lat_idx);
			if (wv)
			{
				wv->GetUV(&u[lat_idx][lon_idx], &v[lat_idx][lon_idx]);
			}
		}
	}

	//-------------//
	// write field //
	//-------------//

	int int_size = sizeof(int);
	int uv_size = ECMWF_HIRES_LON_DIM * ECMWF_HIRES_LAT_DIM * sizeof(float);

	if (fwrite((void *)&ymd, int_size, 1, fp) != 1 ||
		fwrite((void *)&hms, int_size, 1, fp) != 1 ||
		fwrite((void *)u, uv_size, 1, fp) != 1 ||
		fwrite((void *)v, uv_size, 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//------------//
	// close file //
	//------------//

	fclose(fp);

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

	int lon_count = _lon.GetBins();
	int lat_count = _lat.GetBins();
	for (int lon_idx = 0; lon_idx < lon_count; lon_idx++)
	{
		float longitude;
		_lon.IndexToValue(lon_idx, &longitude);
		for (int lat_idx = 0; lat_idx < lat_count; lat_idx++)
		{
			float latitude;
			_lat.IndexToValue(lat_idx, &latitude);
			WindVector* wv = _field[lon_idx][lat_idx];
			if (wv)
			{
				// calculate longitude and latitude
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

//-------------------//
// WindField::NewRes //
//-------------------//

int
WindField::NewRes(
	WindField*	windfield,
	float		lon_res,
	float		lat_res)
{
	//----------//
	// re-index //
	//----------//

	int old_bins, new_bins;
	float old_step;

	old_bins = windfield->_lon.GetBins();
	old_step = windfield->_lon.GetStep();
	new_bins = (int)((float)old_bins * old_step / lon_res + 0.5);
	_lon.SpecifyNewBins(&(windfield->_lon), new_bins);

	old_bins = windfield->_lat.GetBins();
	old_step = windfield->_lat.GetStep();
	new_bins = (int)((float)old_bins * old_step / lat_res + 0.5);
	_lat.SpecifyNewBins(&(windfield->_lat), new_bins);

	//----------//
	// allocate //
	//----------//

	if (! _Allocate())
		return(0);

	//--------------------//
	// generate windfield //
	//--------------------//

	LonLat lon_lat;
	int lon_bins = _lon.GetBins();
	int lat_bins = _lat.GetBins();
	for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
	{
		float value;
		if (! _lon.IndexToValue(lon_idx, &value))
			return(0);

		lon_lat.longitude = value;
		for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
		{
			if (! _lat.IndexToValue(lat_idx, &value))
				return(0);

			lon_lat.latitude = value;

			WindVector* wv = new WindVector;
			if (! wv)
				return(0);

			if (! windfield->InterpolatedWindVector(lon_lat, wv))
				return(0);

			*(*(_field + lon_idx) + lat_idx) = wv;
		}
	}
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
	float lon_min = _lon.GetMin();
	int wrap_factor = (int)ceil((lon_min - lon_lat.longitude) / two_pi);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// convert to longitude index
	int lon_idx;
	if (! _lon.GetNearestIndex(lon, &lon_idx))
		return(0);

	// convert to latitude index
	int lat_idx;
	if (! _lat.GetNearestIndex(lon_lat.latitude, &lat_idx))
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
	float lon_min = _lon.GetMin();
	int wrap_factor = (int)ceil((lon_min - lon_lat.longitude) / two_pi);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// find longitude indicies
	int lon_idx[2];
	float lon_coef[2];
	if (_wrap)
	{
		if (! _lon.GetLinearCoefsWrapped(lon, lon_idx, lon_coef))
			return(0);
	}
	else
	{
		if (! _lon.GetLinearCoefsStrict(lon, lon_idx, lon_coef))
			return(0);
	}

	// find latitude indicies
	int lat_idx[2];
	float lat_coef[2];
	if (! _lat.GetLinearCoefsStrict(lon_lat.latitude, lat_idx, lat_coef))
		return(0);

	WindVector* corner_wv[2][2];
	corner_wv[0][0] = *(*(_field + lon_idx[0]) + lat_idx[0]);
	corner_wv[0][1] = *(*(_field + lon_idx[0]) + lat_idx[1]);
	corner_wv[1][0] = *(*(_field + lon_idx[1]) + lat_idx[0]);
	corner_wv[1][1] = *(*(_field + lon_idx[1]) + lat_idx[1]);

	float corner_u[2][2], corner_v[2][2];
	corner_wv[0][0]->GetUV(&corner_u[0][0], &corner_v[0][0]);
	corner_wv[0][1]->GetUV(&corner_u[0][1], &corner_v[0][1]);
	corner_wv[1][0]->GetUV(&corner_u[1][0], &corner_v[1][0]);
	corner_wv[1][1]->GetUV(&corner_u[1][1], &corner_v[1][1]);

	float u =	lon_coef[0] * lat_coef[0] * corner_u[0][0] +
				lon_coef[0] * lat_coef[1] * corner_u[0][1] +
				lon_coef[1] * lat_coef[0] * corner_u[1][0] +
				lon_coef[1] * lat_coef[1] * corner_u[1][1];

	float v =	lon_coef[0] * lat_coef[0] * corner_v[0][0] +
				lon_coef[0] * lat_coef[1] * corner_v[0][1] +
				lon_coef[1] * lat_coef[0] * corner_v[1][0] +
				lon_coef[1] * lat_coef[1] * corner_v[1][1];

	wv->SetUV(u, v);
	return(1);
}

//-------------------------//
// WindField::SetAllSpeeds //
//-------------------------//

int
WindField::SetAllSpeeds(
	float	speed)
{
	int count = 0;
	int lon_count = _lon.GetBins();
	int lat_count = _lat.GetBins();
	for (int lon_idx = 0; lon_idx < lon_count; lon_idx++)
	{
		for (int lat_idx = 0; lat_idx < lat_count; lat_idx++)
		{
			WindVector* wv = *(*(_field + lon_idx) + lat_idx);
			if (wv)
			{
				wv->spd = speed;
				count++;
			}
		}
	}
	return(count);
}

//----------------------//
// WindField::_Allocate //
//----------------------//

int
WindField::_Allocate()
{
	if (_field != NULL)
		return(0);

	int lon_count = _lon.GetBins();
	int lat_count = _lat.GetBins();
	_field = (WindVector ***)make_array(sizeof(WindVector *), 2, lon_count,
		lat_count);

	if (_field == NULL)
		return(0);

	for (int i = 0; i < lon_count; i++)
	{
		for (int j = 0; j < lat_count; j++)
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

	int lon_count = _lon.GetBins();
	int lat_count = _lat.GetBins();
	free_array((void *)_field, 2, lon_count, lat_count);

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
// WindSwath::WriteL2B //
//---------------------//

int
WindSwath::WriteL2B(
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

			if (! wvc->WriteL2B(fp))
				return(0);
		}
	}
	return(1);
}

//--------------------//
// WindSwath::ReadL2B //
//--------------------//

int
WindSwath::ReadL2B(
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

		if (! wvc->ReadL2B(fp))
			return(0);

		*(*(swath + cti) + ati) = wvc;
	}
	return(1);
}

//--------------------//
// WindSwath::ReadL2B //
//--------------------//

int
WindSwath::ReadL2B(
	const char*		filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	if (! ReadL2B(fp))
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

	for (int ati = 0; ati < _alongTrackBins; ati++)
	{
		for (int cti = 0; cti < _crossTrackBins; cti++)
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
			wvc->selected = wvc->ambiguities.GetByIndex(rank-1);
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

//--------------------------//
// WindSwath::SelectNearest //
//--------------------------//

int
WindSwath::SelectNearest(
	WindField*	truth)
{
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;

			wvc->selected = wvc->GetNearestToDirection(true_wv.dir);
			count++;
		}
	}

	return(count);
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
	float*		std_dev_array,
	float*		std_err_array,
	float*		spd_bias_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	// in all of this, x is (sample - true)^2

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		*(rms_spd_err_array + cti) = 0.0;
		*(std_err_array + cti) = 0.0;
		*(std_dev_array + cti) = 0.0;
		*(spd_bias_array + cti) = 0.0;
		*(count_array + cti) = 0;

		//-------------------------//
		// first pass calculations //
		//-------------------------//

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

			float dif = wvc->selected->spd - true_wv.spd;
			float x = dif * dif;
			*(rms_spd_err_array + cti) += x;
			*(spd_bias_array + cti) += dif;
			(*(count_array + cti))++;
		}

		if (*(count_array + cti) < 2)
			continue;

		//--------------------//
		// calculate the bias //
		//--------------------//

		*(spd_bias_array + cti) /= (float)*(count_array + cti);

		//----------------------------------//
		// calculate the mean squared error //
		//----------------------------------//

		*(rms_spd_err_array + cti) /= (float)*(count_array + cti);

		//--------------------------//
		// second pass calculations //
		//--------------------------//

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

			float dif = wvc->selected->spd - true_wv.spd;
			float x = dif * dif;
			float dev = x - *(rms_spd_err_array + cti);
			*(std_dev_array + cti) += (dev * dev);
		}

		//-----------//
		// RMS error //
		//-----------//

		*(rms_spd_err_array + cti) = sqrt(*(rms_spd_err_array + cti));

		*(std_dev_array + cti) /= (float)(*(count_array + cti) - 1);
		*(std_dev_array + cti) = sqrt(*(std_dev_array + cti));
		*(std_dev_array + cti) /= (2.0 * sqrt(*(rms_spd_err_array + cti)));

		*(std_err_array + cti) = *(std_dev_array + cti) /
			sqrt(*(count_array + cti));
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
	float*		std_dev_array,
	float*		std_err_array,
	float*		dir_bias_array,
	int*		count_array,
	float		low_speed,
	float		high_speed)
{
	// in all of this, x is (sample - true)^2

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		*(rms_dir_err_array + cti) = 0.0;
		*(std_err_array + cti) = 0.0;
		*(std_dev_array + cti) = 0.0;
		*(dir_bias_array + cti) = 0.0;
		*(count_array + cti) = 0;

		//-------------------------//
		// first pass calculations //
		//-------------------------//

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

            float near_angle =
                wrap_angle_near(wvc->selected->dir, true_wv.dir);
            float dif = near_angle - true_wv.dir;
			float x = dif * dif;
			*(rms_dir_err_array + cti) += x;
			*(dir_bias_array + cti) += dif;
			(*(count_array + cti))++;
		}

		if (*(count_array + cti) < 2)
			continue;

		//--------------------//
		// calculate the bias //
		//--------------------//

		*(dir_bias_array + cti) /= (float)*(count_array + cti);

		//----------------------------------//
		// calculate the mean squared error //
		//----------------------------------//

		*(rms_dir_err_array + cti) /= (float)*(count_array + cti);

		//--------------------------//
		// second pass calculations //
		//--------------------------//

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

            float near_angle =
                wrap_angle_near(wvc->selected->dir, true_wv.dir);
            float dif = near_angle - true_wv.dir;
			float x = dif * dif;
			float dev = x - *(rms_dir_err_array + cti);
			*(std_dev_array + cti) += (dev * dev);
		}

		//-----------//
		// RMS error //
		//-----------//

		*(rms_dir_err_array + cti) = sqrt(*(rms_dir_err_array + cti));

		*(std_dev_array + cti) /= (float)(*(count_array + cti) - 1);
		*(std_dev_array + cti) = sqrt(*(std_dev_array + cti));
		*(std_dev_array + cti) /= (2.0 * sqrt(*(rms_dir_err_array + cti)));

		*(std_err_array + cti) = *(std_dev_array + cti) /
			sqrt(*(count_array + cti));
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

//------------------------//
// WindSwath::WithinVsCti //
//------------------------//

int
WindSwath::WithinVsCti(
	WindField*	truth,
	float*		skill_array,
	int*		count_array,
	float		low_speed,
	float		high_speed,
	float		within_angle)
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

			float dif = ANGDIF(true_wv.dir, wvc->selected->dir);
			if (dif < within_angle)
				good_count++;

			count++;
		}

		*(count_array + cti) = count;
		*(skill_array + cti) = (float)good_count / (float)count;
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
