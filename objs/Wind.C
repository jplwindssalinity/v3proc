//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_wind_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
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
:	longitude(0.0), latitude(0.0), selected(NULL)
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

/*
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
*/

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

	if (fwrite((void *)&longitude, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&latitude, sizeof(float), 1, fp) != 1)
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

	if (fread((void *)&longitude, sizeof(float), 1, fp) != 1 ||
		fread((void *)&latitude, sizeof(float), 1, fp) != 1)
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

	if (! _Allocate())
		return(0);

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
	if (_field == NULL)
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
	free_array((void *)_field, 2, _lonCount, _latCount);
	_field = NULL;
	return(1);
}


//===========//
// WindSwath //
//===========//

WindSwath::WindSwath()
:	swath(0), _crossTrackSize(0), _alongTrackSize(0), _validCells(0)
{
	return;
}

WindSwath::~WindSwath()
{
	DeleteEntireSwath();
	return;
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
	if (cti < 0 || cti >= _crossTrackSize ||
		ati < 0 || ati >= _alongTrackSize)
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
	for (int i = 0; i < _alongTrackSize; i++)
	{
		for (int j = 0; j < _crossTrackSize; j++)
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
	if (fwrite((void *)&_alongTrackSize, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&_crossTrackSize, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&_validCells, sizeof(int), 1, fp) != 1)
	{
		return(0);
	}

	for (int cti = 0; cti < _crossTrackSize; cti++)
	{
		for (int ati = 0; ati < _alongTrackSize; ati++)
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

	if (fread((void *)&_alongTrackSize, sizeof(int), 1, fp) != 1 ||
		fread((void *)&_crossTrackSize, sizeof(int), 1, fp) != 1 ||
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
			_crossTrackSize, _alongTrackSize);

	//-------------------------//
	// create a new change map //
	//-------------------------//

	char** change = (char**)make_array(sizeof(char), 2,
		_crossTrackSize, _alongTrackSize);
		
	//--------------------//
	// prep for filtering //
	//--------------------//

	int half_window = window_size / 2;

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
	for (int cti = 0; cti < _crossTrackSize; cti++)
	{
		int cti_min = cti - half_window;
		int cti_max = cti + half_window + 1;
		if (cti_min < 0)
			cti_min = 0;
		if (cti_max > _crossTrackSize)
			cti_max = _crossTrackSize;
		for (int ati = 0; ati < _alongTrackSize; ati++)
		{
			int ati_min = ati - half_window;
			int ati_max = ati + half_window + 1;
			if (ati_min < 0)
				ati_min = 0;
			if (ati_max > _alongTrackSize)
				ati_max = _alongTrackSize;

			//------------------------------//
			// initialize the new selection //
			//------------------------------//

			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;
			new_selected[cti][ati] = NULL;

			//-------------------//
			// check for changes //
			//-------------------//

			for (int i = cti_min; i < cti_max; i++)
			{
				for (int j = ati_min; j < ati_max; j++)
				{
					if (change[cti][ati])
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
						if (i == ati && j == cti)
							continue;		// don't check central vector

						WVC* other_wvc = swath[i][j];
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

	for (int cti = 0; cti < _crossTrackSize; cti++)
	{
		for (int ati = 0; ati < _alongTrackSize; ati++)
		{
			change[cti][ati] = 0;
			if (new_selected[ati][cti])
			{
				if (new_selected[ati][cti] != swath[cti][ati]->selected)
				{
					swath[cti][ati]->selected = new_selected[ati][cti];
					change[cti][ati] = 1;
					flips++;
				}
			}
		}
	}

	return(flips);
}

//------------------//
// WindSwath::Skill //
//------------------//

int
WindSwath::Skill(
	WindField*	truth,
	int*		skill_sum_array,
	int*		total_sum_array)
{
	for (int cti = 0; cti < _crossTrackSize; cti++)
	{
		for (int ati = 0; ati < _alongTrackSize; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc || ! wvc->selected)
				continue;

			WindVector* true_wv = truth->NearestWindVector(wvc->longitude,
				wvc->latitude);
			if (true_wv == NULL)
				continue;

			WindVectorPlus* nearest =
				wvc->GetNearestToDirection(true_wv->dir);
			if (nearest == NULL)
				continue;

			if (nearest == wvc->selected)
				(*(skill_sum_array + cti))++;

			(*(total_sum_array + cti))++;
		}
	}
	return(1);
}

//----------------------//
// WindSwath::_Allocate //
//----------------------//

int
WindSwath::_Allocate()
{
	if (swath == NULL)
		return(0);

	swath = (WVC ***)make_array(sizeof(WVC *), 2, _crossTrackSize,
		_alongTrackSize);

	if (swath == NULL)
		return(0);

	for (int i = 0; i < _crossTrackSize; i++)
	{
		for (int j = 0; j < _alongTrackSize; j++)
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

	free_array((void *)swath, 2, _crossTrackSize, _alongTrackSize);
	swath = NULL;
	_crossTrackSize = 0;
	_alongTrackSize = 0;
	return(1);
}
