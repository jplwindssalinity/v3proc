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
#include "GSparameters.h"
#include "Misc.h"
#include "Array.h"
#include "LonLat.h"

#include "L1AExtract.h"
#include "ParTab.h"
#include "NoTimeTlmFile.h"

#define HDF_ACROSS_BIN_NO    76
#define HDF_NUM_AMBIGUITIES  4

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
	fprintf(fp, "Spd=%g Dir=%g(%g) Obj=%g\n", spd, dir * rtd, dir, obj);
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
:	selected(NULL), selected_allocated(0)
{
	return;
}

WVC::~WVC()
{
	WindVectorPlus* wvp;
	ambiguities.GotoHead();
        int selected_allocated=0;
        
	  while ((wvp=ambiguities.RemoveCurrent()) != NULL){
		delete wvp;
		if (wvp==selected) selected_allocated=0;
	  }
	  if(selected_allocated) delete selected;
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
    if ( ! lonLat.WriteAscii(fp))
        return(0);

	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		if (! wvp->WriteAscii(fp))
			return(0);
	}

    if (selected)
    {
        fprintf(fp, "Selected: ");
		if (! selected->WriteAscii(fp))
			return(0);
    }
	return(1);
}

//------------------//
// WVC::WriteFlower //
//------------------//

int
WVC::WriteFlower(
	FILE*	fp)
{
	//-------------------//
	// sort by direction //
	//-------------------//

	if (! SortByDir())
		return(0);

	//------------------------//
	// find maximum obj value //
	//------------------------//

	float max_obj = 0.0;
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		if (wvp->obj > max_obj)
			max_obj = wvp->obj;
	}

	//-----------------//
	// generate flower //
	//-----------------//

	float max_dist = 10.0;	// 10 km
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		float dlat = max_dist * sin(wvp->dir) / (r1_earth * max_obj);
		float dlon = max_dist * cos(wvp->dir) /
						(r1_earth * max_obj * cos(lonLat.latitude));
		dlat *= wvp->obj;
		dlon *= wvp->obj;

		LonLat lon_lat;
		lon_lat.latitude = lonLat.latitude + dlat;
		lon_lat.longitude = lonLat.longitude + dlon;
		if (! lon_lat.WriteOtln(fp))
			return(0);
	}

	//------------//
	// close path //
	//------------//

	WindVectorPlus* wvp2 = ambiguities.GetHead();
	float dlat2 = max_dist * sin(wvp2->dir) / (r1_earth * max_obj);
	float dlon2 = max_dist * cos(wvp2->dir) /
					(r1_earth * max_obj * cos(lonLat.latitude));
	dlat2 *= wvp2->obj;
	dlon2 *= wvp2->obj;

	LonLat lon_lat2;
	lon_lat2.latitude = lonLat.latitude + dlat2;
	lon_lat2.longitude = lonLat.longitude + dlon2;
	if (! lon_lat2.WriteOtln(fp))
		return(0);

	//-----//
	// end //
	//-----//

	LonLat inf;
	inf.longitude = (float)HUGE_VAL;
	inf.latitude = (float)HUGE_VAL;
	if (! inf.WriteOtln(fp))
		return(0);

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

//----------------//
// WVC::SortByDir //
//----------------//
// uses idiot-sort (based on the stupidity and laziness of JNH)
// sorts in ascending order (the lowest dir is first)

int
WVC::SortByDir()
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
				if (next_wvp->dir < wvp->dir)
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
// if max_rank is zero it is ignored

WindVectorPlus*
WVC::GetNearestToDirection(
	float	dir,
	int		max_rank)
{
	WindVectorPlus* nearest = NULL;
	float min_dif = two_pi;

	int rank = 0;
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		if (max_rank)
		{
			rank++;
			if (rank > max_rank)
				break;
		}

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

//--------------------------//
// WVC::Rank_Wind_Solutions //
//--------------------------//

int
WVC::Rank_Wind_Solutions()
{

//
// Translated from GS module: Rank_Wind_Solutions2.F
// - Final sorting omitted.
//!File Name:	Rank_Wind_Solutions.F
//
//!Description:
//		This routine eliminates 
//			(1) redundant wind solutions
//			(2) out-of-range
//		and ranks the "final" wind solutions to be
//		used by the ambiguity-removal processor. 
//
//!Input Parameters:
//	wind_speed_delta		-	speed tolerance value (0.1 m/s).
//	wind_dir_delta			-	direction tolerance value (5 degrees).
//	wind_likelihood_delta	-	MLE tolerance value (0.5).
//
//!Input/Output Parameters: 
//	wr_num_ambigs		- number of ambigous wind solutions.
//	wr_mle				- value of maximum likelihood estimation.
//	wr_wind_speed		- speeds of ambiguous wind solutions. 
//	wr_wind_dir			- directions of ambiguous wind solutions. 
//	wr_wind_speed_err	- errors associated with wind speed.
//	wr_wind_dir_err		- errors associated with wind direction. 
//

	float	wr_wind_speed[wind_max_solutions];
	float	wr_wind_dir[wind_max_solutions];
	float	wr_mle[wind_max_solutions];

	// Copy data into wr_ arrays. (convert radians to degrees)
	int i = 0;
	for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
		wvp = ambiguities.GetNext())
	{
		wr_wind_speed[i] = wvp->spd;
		wr_wind_dir[i] = rtd*wvp->dir;
		wr_mle[i] = wvp->obj;
		i++;
		if (i >= wind_max_solutions)
			break;
	}
	int wr_num_ambigs = i;

// Local Declarations
	int		j;
	int		start_j;
	int		ambig;
	int		speed_50_flag[wind_max_solutions];
	int		speed_50_counter;
	int		i_speed_processed;
	int		i_twin_processed;
	int		twin_flag[wind_max_solutions];
	int		twin_counter;
	int		num_ambigs;
	float	diff_speed;
	float	diff_dir;
	float	diff_mle;

	int		num_sorted;
//	int		max_index;
//	float	max_mle;

//	Initialize. 

	speed_50_counter	= 0;
	twin_counter		= 0;
	num_sorted			= 0;
 
	for (j=0; j < wind_max_solutions; j++)
	{
		speed_50_flag[j]	= 0;
		twin_flag[j]		= 0;
	}

	for (ambig=0; ambig < wr_num_ambigs; ambig++)
	{

//	Wind direction:
//		1. Constrain values between 0 and 360 degrees.
//		2. Conform to SEAWIND's oceanographic convention.

		if (wr_wind_dir[ambig] < 0.0)
		{
			wr_wind_dir[ambig] += 360.;
		}

		if (wr_wind_dir[ambig] > 360.)
		{
			wr_wind_dir[ambig] -= 360.; // change made here for the sign: Kyung
		}

// Set high wind speed flag and count up if larger than 50 m/s.

		if (wr_wind_speed[ambig] > 50.)
		{
			speed_50_flag[ambig] = 1;
			speed_50_counter++;
		}
	}

// Eliminate wind solutions with speed > 50 m/s. 

	if (speed_50_counter > 0)
	{
		num_ambigs = wr_num_ambigs - speed_50_counter;
		start_j = 0;
 
		for (i=0; i < num_ambigs; i++)
		{
            i_speed_processed = 0;

			for (j=start_j; j < wr_num_ambigs; j++)
			{
               if (!  i_speed_processed && speed_50_flag[j] == 0)
				{
                   wr_wind_speed[i]    = wr_wind_speed[j];
                   wr_wind_dir[i]      = wr_wind_dir[j];
                   wr_mle[i]           = wr_mle[j];
                   start_j = j + 1;
                   i_speed_processed = 1;
				}
 
			}
		}
        wr_num_ambigs = num_ambigs;
	}

// Identify and eliminate possible "twin" solutions. Search 
// the entire solution space, tag twin solutions with smaller MLE value. 
      
//        write(97,*) wr_wind_speed,wr_wind_dir,wr_mle
	for (i=0; i < wr_num_ambigs; i++)
	{
          twin_flag[i] = 0;
	}
      twin_counter = 0;
  
	for (i=0; i < wr_num_ambigs-1; i++)
	for (j=i+1; j < wr_num_ambigs; j++)
	{

            diff_speed = fabs (wr_wind_speed[i] - wr_wind_speed[j]);
            diff_dir   = fabs (wr_wind_dir[i] - wr_wind_dir[j]);
            diff_mle   = fabs (wr_mle[i] - wr_mle[j]);

            if (diff_dir>180.0) diff_dir=360.-diff_dir;

            if (diff_speed <= wind_speed_delta    &&
               diff_dir   < wind_dir_delta      &&
               diff_mle   < wind_likelihood_delta )
			{
                if (twin_flag[j] == 0)
				{
                   twin_flag[j] = 1;
                   twin_counter = twin_counter + 1;

                   if (wr_mle[j]>wr_mle[i])
					{
                    wr_wind_speed[i] = wr_wind_speed[j];
                    wr_wind_dir[i] = wr_wind_dir[j];
                    wr_mle[i] = wr_mle[j];
					}
				}
			}
	}

// Eliminate tagged twin solutions. 

	if  (twin_counter > 0)
	{
//        write(97,*) 'number of twins found ',twin_counter
         num_ambigs =  wr_num_ambigs  -  twin_counter;
         start_j  = 0;
		for (i=0; i < num_ambigs; i++)
		{
            i_twin_processed = 0;

			for (j=start_j; j < wr_num_ambigs; j++)
			{
				if (! i_twin_processed)
				{
					if  (twin_flag[j] == 0)
					{
                     wr_wind_speed[i] = wr_wind_speed[j];
                     wr_wind_dir[i] = wr_wind_dir[j];
                     wr_mle[i] = wr_mle[j];
                     start_j = j + 1;
                     i_twin_processed  = 1; 
					}
				}

			}
		}
//        write(97,*) wr_wind_speed,wr_wind_dir,wr_mle
         wr_num_ambigs = num_ambigs;
	}

    // Copy data from wr_ arrays. (convert to radians)
	WindVectorPlus* wvp = ambiguities.GetHead();
	for (i = 0; i < wr_num_ambigs; i++)
    {
        wvp->spd = wr_wind_speed[i];
        wvp->dir = wr_wind_dir[i]*dtr;
        wvp->obj = wr_mle[i];
		wvp = ambiguities.GetNext();
    }
    while (wvp != NULL)
    {
        wvp = ambiguities.RemoveCurrent();
        delete(wvp);
    }

    return(1);

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

//---------------------------//
// WindField::ReadEcmwfLoRes //
//---------------------------//

int
WindField::ReadEcmwfLoRes(
        const char*             filename)
{
        if (_field != NULL)
          return(2);

        //-----------//
        // open file //
        //-----------//

        FILE* fp = fopen(filename, "r");
        if (fp == NULL)
                return(0);

        //------------//
        // read field //
        //------------//

        short head[ECMWF_LORES_LON_DIM];
        short tmp_u[ECMWF_LORES_LAT_DIM][ECMWF_LORES_LON_DIM];
        short tmp_v[ECMWF_LORES_LAT_DIM][ECMWF_LORES_LON_DIM];

        int head_size = ECMWF_LORES_LON_DIM * sizeof(short);
        int uvsize = ECMWF_LORES_LON_DIM * ECMWF_LORES_LAT_DIM * sizeof(short);

        if (fread((void *)&head, head_size, 1, fp) != 1 ||
                fread((void *)tmp_u, uvsize, 1, fp) != 1 ||
                fread((void *)tmp_v, uvsize, 1, fp) != 1)
        {
                fclose(fp);
                return(0);
        }

        //------------//
        // close file //
        //------------//

        fclose(fp);

        //---------//
        // convert //
        //---------//

        float u[ECMWF_LORES_LAT_DIM][ECMWF_LORES_LON_DIM];
        float v[ECMWF_LORES_LAT_DIM][ECMWF_LORES_LON_DIM];
        float scale = float(ECMWF_LORES_SCALE_FACTOR);

        //-------------------------------//
        // transfer to wind field format //
        //-------------------------------//

        _lon.SpecifyWrappedCenters(-180.0 * dtr, 180.0 * dtr, ECMWF_LORES_LON_DIM);
        _lat.SpecifyCenters(-90.0 * dtr, 90.0 * dtr, ECMWF_LORES_LAT_DIM);

  
        if (! _Allocate())
                return(0);


        for (int lon_idx = 0; lon_idx < ECMWF_LORES_LON_DIM; lon_idx++)
        {
                for (int lat_idx = 0; lat_idx < ECMWF_LORES_LAT_DIM; lat_idx++)
                {
                         
                        u[lat_idx][lon_idx] = float( tmp_u[lat_idx][lon_idx] ) / scale;
                        v[lat_idx][lon_idx] = float( tmp_v[lat_idx][lon_idx] ) / scale;

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


//----------------------//
// WindField::ReadNSCAT //
//----------------------//

int
WindField::ReadNSCAT(
        const char*             filename)
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

        float u[NSCAT_LAT_DIM][NSCAT_LON_DIM];
        float v[NSCAT_LAT_DIM][NSCAT_LON_DIM];

        int uv_size = NSCAT_LON_DIM * NSCAT_LAT_DIM * sizeof(float);

        if (fread((void *)u, uv_size, 1, fp) != 1 ||
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

        _lon.SpecifyWrappedCenters(0.0 * dtr, 360.0 * dtr, NSCAT_LON_DIM);
        _lat.SpecifyCenters(-75.0 * dtr, 75.0 * dtr, NSCAT_LAT_DIM);

        if (! _Allocate())
                return(0);

        for (int lon_idx = 0; lon_idx < NSCAT_LON_DIM; lon_idx++)
        {
                for (int lat_idx = 0; lat_idx < NSCAT_LAT_DIM; lat_idx++)
                {

                  if (u[lat_idx][lon_idx] == NSCAT_LAND_VALUE ||
                      v[lat_idx][lon_idx] == NSCAT_LAND_VALUE)
                    {
                      u[lat_idx][lon_idx] = 0.;
                      v[lat_idx][lon_idx] = 0.;
                    }
                  
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
    const char*  filename,
    int          extra_time_flag)
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

    if (extra_time_flag)
    {
        if (fwrite((void *)&ymd, int_size, 1, fp) != 1 ||
            fwrite((void *)&hms, int_size, 1, fp) != 1 ||
            fwrite((void *)u, uv_size, 1, fp) != 1 ||
            fwrite((void *)&ymd, int_size, 1, fp) != 1 ||
            fwrite((void *)&hms, int_size, 1, fp) != 1 ||
            fwrite((void *)v, uv_size, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
    }
    else
    {
        if (fwrite((void *)&ymd, int_size, 1, fp) != 1 ||
            fwrite((void *)&hms, int_size, 1, fp) != 1 ||
            fwrite((void *)u, uv_size, 1, fp) != 1 ||
            fwrite((void *)v, uv_size, 1, fp) != 1)
        {
            fclose(fp);
            return(0);
        }
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
        else if (strcasecmp(type, ECMWF_LORES_TYPE) == 0)
        {
                return(ReadEcmwfLoRes(filename));
        }
        else if (strcasecmp(type, NSCAT_TYPE) == 0)
        {
                return(ReadNSCAT(filename));
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
		if (! _lon.GetLinearCoefsClipped(lon, lon_idx, lon_coef))
			return(0);
	}

	// find latitude indicies
	int lat_idx[2];
	float lat_coef[2];
	if (! _lat.GetLinearCoefsClipped(lon_lat.latitude, lat_idx, lat_coef))
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

//---------------------------//
// WindField::FakeEcmwfHiRes //
//---------------------------//

int
WindField::FakeEcmwfHiRes(
    float  speed)
{
	//-------------------------------//
	// specify ECMWF hi-res sampling //
	//-------------------------------//

	_lon.SpecifyWrappedCenters(-180.0 * dtr, 180.0 * dtr, ECMWF_HIRES_LON_DIM);
	_lat.SpecifyCenters(-90.0 * dtr, 90.0 * dtr, ECMWF_HIRES_LAT_DIM);

	if (! _Allocate())
		return(0);

	for (int lon_idx = 0; lon_idx < ECMWF_HIRES_LON_DIM; lon_idx++)
	{
        float dir = 8.0 * (float)lon_idx * two_pi / (float)ECMWF_HIRES_LON_DIM;
        while (dir > two_pi)
            dir -= two_pi;

		for (int lat_idx = 0; lat_idx < ECMWF_HIRES_LAT_DIM; lat_idx++)
		{
			WindVector* wv = new WindVector;
			if (! wv)
				return(0);

            wv->SetSpdDir(speed, dir);
			*(*(_field + lon_idx) + lat_idx) = wv;
		}
	}

	_wrap = 1;
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
	for (int i = 0; i < lon_count; i++)
	{
		for (int j = 0; j < lat_count; j++)
		{
			if (_field[i][j]) delete _field[i][j];
		}
	}

	free_array((void *)_field, 2, lon_count, lat_count);

	_field = NULL;
	return(1);
}


//===========//
// WindSwath //
//===========//

WindSwath::WindSwath()
:	swath(0),  _crossTrackBins(0), _alongTrackBins(0), _validCells(0)
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
		// Quietly dump out of range cells.
		delete wvc;
		return(1);

//        fprintf(stderr, "WindSwath::Add: out of range\n");
//        fprintf(stderr, "  cti = %d (Max = %d), ati = %d (Max = %d)\n",
//            cti, _crossTrackBins, ati, _alongTrackBins);
//		return(0);	// out of range
	}

	if (swath[cti][ati])
    {
        fprintf(stderr, "WindSwath::Add: attempted cell replacement\n");
        fprintf(stderr, "  cti = %d, ati = %d\n", cti, ati);
		return(0);	// already a cell there
    }

	swath[cti][ati] = wvc;
	_validCells++;
	return(1);
}

//---------------------------------//
// WindSwath::GetMaxAmbiguityCount //
//---------------------------------//

int
WindSwath::GetMaxAmbiguityCount()
{
	int max_count = 0;
	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			WVC* wvc = *(*(swath + i) + j);
			if (wvc == NULL)
				continue;

			int count = wvc->ambiguities.NodeCount();
			if (count > max_count)
				max_count = count;
		}
	}
	return(max_count);
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

//----------------------------//
// WindSwath::DeleteLatitudes //
//----------------------------//

int
WindSwath::DeleteLatitudesOutside(
    float  low_lat,
    float  high_lat)
{
    int count = 0;
	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			WVC* wvc = *(*(swath + i) + j);
			if (wvc == NULL)
				continue;

            float lat = wvc->lonLat.latitude;
            if (lat < low_lat || lat > high_lat)
            {
                delete wvc;
                *(*(swath + i) + j) = NULL;
                count++;
                _validCells--;
            }
		}
	}
	return(count);
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

//-----------------------//
// WindSwath::ReadHdfL2B //
//-----------------------//

int
WindSwath::ReadHdfL2B(
	TlmHdfFile*	tlmHdfFile)
{
	DeleteEntireSwath();		// in case

    // cross bin number is fixed
    _crossTrackBins = HDF_ACROSS_BIN_NO;

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    int32 dataType=0, dataStartIndex=0, dataLength=0, numDimensions=0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
                             dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    _alongTrackBins = dataLength;

    // all cells are valid here
    _validCells = _alongTrackBins * _crossTrackBins;

    // open all needed datasets
    if ( ! _OpenHdfDataSets(tlmHdfFile))
        return(0);

	_Allocate();

    float* lonArray = new float[_crossTrackBins];
    float* latArray = new float[_crossTrackBins];
    float* speedArray = (float*)new float[_crossTrackBins*HDF_NUM_AMBIGUITIES];
    float* dirArray = (float*)new float[_crossTrackBins*HDF_NUM_AMBIGUITIES];
    float* mleArray = (float*)new float[_crossTrackBins*HDF_NUM_AMBIGUITIES];
    char*  selectArray = new char[_crossTrackBins];
    int32 sdsIds[1];
	for (int32 i = 0; i < _alongTrackBins; i++)
    {
        sdsIds[0] = _lonSdsId;
        if (ExtractData2D_76_uint2_float(tlmHdfFile, sdsIds, 
                                           i, 1, 1, lonArray) == 0)
            return(0);

        sdsIds[0] = _latSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, 
                                           i, 1, 1, latArray) == 0)
            return(0);

        sdsIds[0] = _speedSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, 
                                           i, 1, 1, speedArray) == 0)
            return(0);

        sdsIds[0] = _dirSdsId;
        if (ExtractData3D_76_4_uint2_float(tlmHdfFile, sdsIds, 
                                           i, 1, 1, dirArray) == 0)
            return(0);

        sdsIds[0] = _mleSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, 
                                           i, 1, 1, mleArray) == 0)
            return(0);

        sdsIds[0] = _selectSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, i, 1, 1, selectArray) == 0)
            return(0);

	    for (int j = 0; j < _crossTrackBins; j++)
	    {
		    WVC* wvc = new WVC();
            wvc->lonLat.longitude = lonArray[j];
            wvc->lonLat.latitude = latArray[j];
            for (int k=0; k < HDF_NUM_AMBIGUITIES; k++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                wvp->SetSpdDir(speedArray[j * HDF_NUM_AMBIGUITIES + k],
                               dirArray[j * HDF_NUM_AMBIGUITIES + k]);
                wvp->obj = mleArray[j * HDF_NUM_AMBIGUITIES + k];
                wvc->ambiguities.Append(wvp);
            }
            if (selectArray[j] > 0)
                wvc->selected = wvc->ambiguities.GetByIndex(selectArray[j]-1);

		    *(*(swath + j) + i) = wvc;
        }
	}
    delete [] lonArray;
    delete [] latArray;
    delete [] speedArray;
    delete [] dirArray;
    delete [] mleArray;
    delete [] selectArray;

    // close all needed datasets
    _CloseHdfDataSets();

	return(1);
}
//-----------------------//
// WindSwath::ReadHdfL2B //
//-----------------------//

int
WindSwath::ReadHdfL2B(
	const char*		filename)
{
    // open the L2B HDF file
    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile hdfL2BFile(filename, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

	if (! ReadHdfL2B(&hdfL2BFile))
		return(0);

	return(1);

}//WindSwath::ReadHdfL2B

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

//------------------------//
// WindSwath::WriteFlower //
//------------------------//

int
WindSwath::WriteFlower(
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

	char* hdr = OTLN_HEADER;
	if (fwrite((void *)hdr, 4, 1, fp) != 1)
		return(0);

	//---------------//
	// write flowers //
	//---------------//

	for (int ati = 0; ati < _alongTrackBins; ati++)
	{
		for (int cti = 0; cti < _crossTrackBins; cti++)
		{
			WVC* wvc = *(*(swath + cti) + ati);
			if (wvc == NULL)
				continue;

			if (! wvc->WriteFlower(fp))
				return(0);
		}
	}

	fclose(fp);
	return(1);
}

//------------------------//
// WindSwath::WriteAscii  //
//------------------------//

int
WindSwath::WriteAscii(
	const char*		filename)
{
	//-----------//
	// open file //
	//-----------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

    int rc = WriteAscii(fp);
    fclose(fp);
    return(rc);

} // WriteAscii

//------------------------//
// WindSwath::WriteAscii  //
//------------------------//

int
WindSwath::WriteAscii(
	FILE*     fp)
{
	//---------------//
	// write ASCII   //
	//---------------//

    fprintf(fp, "Total Along Track Bins: %d\nTotal Cross Track Bins: %d\n",
                             _alongTrackBins, _crossTrackBins);
	for (int ati = 0; ati < _alongTrackBins; ati++)
	{
		for (int cti = 0; cti < _crossTrackBins; cti++)
		{
            fprintf(fp, "Along Track Bin: %d, Cross Track Bin: %d\n", ati, cti);
			WVC* wvc = *(*(swath + cti) + ati);
			if (wvc == NULL)
				continue;

			if (! wvc->WriteAscii(fp))
				return(0);
		}
	}

	return(1);

} // WriteAscii

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

//------------------//
// WindSwath::Nudge //
//------------------//

int
WindSwath::Nudge(
	WindField*	nudge_field,
	int max_rank)
{
	int count = 0;
	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;

			WindVector nudge_wv;
			if (! nudge_field->InterpolatedWindVector(wvc->lonLat, &nudge_wv))
				continue;

			wvc->selected = wvc->GetNearestToDirection(nudge_wv.dir, max_rank);
			count++;
		}
	}
	return(count);
}

//-----------------------//
// WindSwath::SmartNudge //
//-----------------------//

int
WindSwath::SmartNudge(
    WindField*  nudge_field)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            WindVector nudge_wv;
            if (! nudge_field->InterpolatedWindVector(wvc->lonLat, &nudge_wv))
                continue;

            WindVectorPlus* nearest = NULL;
            float min_dif = two_pi;

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                 wvp = wvc->ambiguities.GetNext())
            {
                if (wvp->obj != 1.0)
                    continue;

                float dif = ANGDIF(wvp->dir, nudge_wv.dir);
                if (dif < min_dif)
                {
                    min_dif = dif;
                    nearest = wvp;
                }
            }

            wvc->selected = nearest;
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
	int		max_passes,
	int		weight_flag)
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
	while (pass < max_passes)
	{
		int flips = MedianFilterPass(half_window, new_selected, change,
						weight_flag);
		pass++;
		if (flips == 0)
			break;
	} 

	free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
	free_array(change, 2, _crossTrackBins, _alongTrackBins);
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
	char**				change,
	int					weight_flag)
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

			float min_vector_dif_sum = (float)HUGE_VAL;

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

				//------------------------------//
				// apply weighting if necessary //
				//------------------------------//

				if (weight_flag)
				{
					if (wvp->obj == 0.0)
						vector_dif_sum = (float)HUGE_VAL;
					else
						vector_dif_sum /= wvp->obj;
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

//---------------------------//
// WindSwath::WriteDirErrMap //
//---------------------------//

int
WindSwath::WriteDirErrMap(
	WindField*	truth,
        FILE*           ofp)
{
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

			float dif = fabs(ANGDIF(wvc->selected->dir, 
						true_wv.dir))*rtd;
			float lon=wvc->lonLat.longitude*rtd;
			float lat=wvc->lonLat.latitude*rtd;
			fwrite((void*)&dif,sizeof(float),1,ofp);
			fwrite((void*)&lon,sizeof(float),1,ofp);
			fwrite((void*)&lat,sizeof(float),1,ofp);
			
		}
	}

	return(1);
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

//-----------------------------//
// WindSwath::DirectionDensity //
//-----------------------------//

int
WindSwath::DirectionDensity(
    WindField*      truth,
    unsigned int*   swath_density_array,
    unsigned int*   field_density_array,
    float           low_speed,
    float           high_speed,
    int             direction_count)
{
    //-------------------------//
    // index direction density //
    //-------------------------//

    Index dir_idx;
    dir_idx.SpecifyWrappedCenters(0.0, two_pi, direction_count);

    //------------------//
    // clear the counts //
    //------------------//

    for (int dir = 0; dir < direction_count; dir++)
    {
        *(swath_density_array + dir) = 0;
        *(field_density_array + dir) = 0; 
    }

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

            //--------------------------------------//
            // determine the S/C velocity direction //
            //--------------------------------------//

            int ati_minus = ati - 1;
            if (ati_minus < 0)
                ati_minus = 0;
			WVC* wvc_minus = swath[cti][ati_minus];
			if (! wvc_minus)
                continue;

            int ati_plus = ati + 1;
            if (ati_plus < 0)
                ati_plus = 0;
			WVC* wvc_plus = swath[cti][ati_plus];
			if (! wvc_plus)
                continue;

            double dlat = wvc_plus->lonLat.latitude -
                wvc_minus->lonLat.latitude;
            double dlon = wvc_plus->lonLat.longitude -
                wvc_minus->lonLat.longitude;
            while (dlon > pi)
                dlon -= two_pi;
            while (dlon < -pi)
                dlon += two_pi;

            double sc_dir = atan2(dlat, dlon);    // ccw from east

            //-------------------------------//
            // determine the wind directions //
            //-------------------------------//

            float ret_dir = wvc->selected->dir;
            float true_dir = true_wv.dir;

            //-----------------------------------------------//
            // determine the relative wind direction (0-360) //
            //-----------------------------------------------//

            float rel_ret_dir = ret_dir - sc_dir;
            while (rel_ret_dir < 0.0)
                rel_ret_dir += two_pi;
            while (rel_ret_dir > two_pi)
                rel_ret_dir -= two_pi;

            float rel_true_dir = true_dir - sc_dir;
            while (rel_true_dir < 0.0)
                rel_true_dir += two_pi;
            while (rel_true_dir > two_pi)
                rel_true_dir -= two_pi;

            //---------------------//
            // determine the index //
            //---------------------//

            int ret_idx, true_idx;
            if (! dir_idx.GetNearestWrappedIndex(rel_ret_dir, &ret_idx) ||
                ! dir_idx.GetNearestWrappedIndex(rel_true_dir, &true_idx))
            {
                return(0);
            }

            ( *(swath_density_array + ret_idx) )++;
            ( *(field_density_array + true_idx) )++;
		}
	}

	return(1);
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

//---------------------//
// WindSwath::DirArray //
//---------------------//

int
WindSwath::DirArray(
    int     number_of_bins,
    float*  dir_array)
{
    //-------------------------//
    // index direction density //
    //-------------------------//

    Index dir_idx;
    dir_idx.SpecifyWrappedCenters(0.0, two_pi, number_of_bins);

    for (int i = 0; i < number_of_bins; i++)
    {
        float value;
        dir_idx.IndexToValue(i, &value);
        *(dir_array + i) = value;
	}

	return(1);
}

//---------------------------//
// WindSwath::AvgNambigVsCti //
//---------------------------//

int
WindSwath::AvgNambigVsCti(
    WindField*  truth,
    float*      avg_nambig,
    float       low_speed,
    float       high_speed)
{
	//----------------------------------------//
	// sum number of ambiguities for each cti //
	//----------------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		int count = 0;
		long sum = 0;
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;

            WindVector true_wv;
            if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

			sum += wvc->ambiguities.NodeCount();
			count++;
		}

		avg_nambig[cti] = (double)sum / (double)count;
	}

	return(1);
}

//---------------------//
// WindSwath::WvcVsCti //
//---------------------//

int
WindSwath::WvcVsCti(
    WindField*     truth,
    unsigned int*  count,
    float          low_speed,
    float          high_speed)
{
	//--------------------------------//
	// sum number of WVC for each cti //
	//--------------------------------//

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
        *(count + cti) = 0;
		for (int ati = 0; ati < _alongTrackBins; ati++)
		{
			WVC* wvc = swath[cti][ati];
			if (! wvc)
				continue;

            WindVector true_wv;
            if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

			(*(count + cti))++;
		}
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


//---------------------------------//
// WindSwath::GetProbabilityArray  //
//---------------------------------//
int
WindSwath::GetProbabilityArray(
WindField*           truth,
float***              prob, 
int**          num_samples,
float**        widths,
int          true_dir_bins,
int          delta_dir_bins )
{

  float true_dir_step_size=two_pi/true_dir_bins;
  float delta_dir_step_size=two_pi/delta_dir_bins;

  //------------------------------------------------------//
  // Initialize prob and num_samples arrays               //
  //------------------------------------------------------//
  
  for(int ctd=0; ctd< _crossTrackBins; ctd++){
    for(int td=0; td < true_dir_bins; td++){
      num_samples[ctd][td]=0;
      widths[ctd][td]=0.0;
      for(int dd=0; dd < delta_dir_bins; dd++){
	prob[ctd][td][dd]=0.0;
      }
    }
  }

  //------------------------------------------------------//
  // Accumulate probability info and number of samples    //
  //------------------------------------------------------//
  for(int cti=0;cti<_crossTrackBins;cti++){
    for(int ati=0;ati<_alongTrackBins;ati++){
      // Calculate true direction index 
      WVC* wvc = swath[cti][ati];
      if (! wvc)
	continue;

      WindVector true_wv;
      if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
	continue;   
      while(true_wv.dir<0) true_wv.dir+=two_pi;
      while(true_wv.dir>two_pi) true_wv.dir-=two_pi;

      int true_dir_idx=(int)(true_wv.dir/true_dir_step_size +0.5);
      if(true_dir_idx==true_dir_bins) true_dir_idx=0;

      // Increment number of samples;
      num_samples[cti][true_dir_idx]++;
      float width=1, obj_sum=0;
      for(WindVectorPlus* wvp=wvc->ambiguities.GetHead();wvp;
	  wvp=wvc->ambiguities.GetNext()){
	obj_sum+=wvp->obj;
	if(obj_sum < 0.80) width++;
	// Determine delta direction index			
	float near_angle =
	  wrap_angle_near(wvc->selected->dir, true_wv.dir);
	float dif = near_angle - true_wv.dir;
	int delta_dir_idx=(int)((dif+pi)/delta_dir_step_size +0.5);
        if(delta_dir_idx==delta_dir_bins) delta_dir_idx=0;
        // accumulate prob array
	prob[cti][true_dir_idx][delta_dir_idx]+=wvp->obj;
      }
      widths[cti][true_dir_idx]+=width*delta_dir_step_size*rtd;
    }
  }

  // divide accumulated sums of probabilities by number of samples
  for(int cti=0;cti<_crossTrackBins;cti++){
    for(int tdi=0;tdi<true_dir_bins;tdi++){
      if(num_samples[cti][tdi]!=0){
	for(int ddi=0;ddi<delta_dir_bins;ddi++)
	  prob[cti][tdi][ddi]/=num_samples[cti][tdi];
	widths[cti][tdi]/=num_samples[cti][tdi];
      }
    }
  }
  return(1);
}
void WindSwath::operator-=(const WindSwath& w){
	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			WVC* wvc1 = *(*(swath + i) + j);
			if (wvc1 == NULL)
				continue;
			WVC* wvc2 = *(*(w.swath + i) + j);
			if (wvc2 == NULL)
				continue;

			WindVectorPlus* wvp_sel=new WindVectorPlus;
                        float u1, v1, u2, v2;
			wvc1->selected->GetUV(&u1,&v1);
			wvc2->selected->GetUV(&u2,&v2);
			wvp_sel->SetUV(u1-u2,v1-v2);
			wvc1->selected=wvp_sel;
			wvc1->selected_allocated=1;
                        WindVectorPlus* wvp1=wvc1->ambiguities.GetHead();
			WindVectorPlus* wvp2=wvc2->ambiguities.GetHead();
			  while(wvp1 && wvp2){
                            wvp1->GetUV(&u1,&v1);
                            wvp2->GetUV(&u2,&v2);
			    wvp1->SetUV(u1-u2,v1-v2);
			    wvp1=wvc1->ambiguities.GetNext();
			    wvp2=wvc2->ambiguities.GetNext();
			  }
			while(wvp1){
			  wvp1=wvc1->ambiguities.RemoveCurrent();
			  delete wvp1;
			  wvp1=wvc1->ambiguities.GetCurrent();
			}

		}
	}
}
int WindSwath::DifferenceFromTruth(WindField* truth){
	for (int i = 0; i < _crossTrackBins; i++)
	{
		for (int j = 0; j < _alongTrackBins; j++)
		{
			WVC* wvc = *(*(swath + i) + j);
			if (wvc == NULL)
				continue;

			WindVector true_wv;
			if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
				continue;
			
                        WindVectorPlus* wvp=wvc->ambiguities.GetHead();
			  while(wvp){
			    float u1, v1, u2, v2;
                            wvp->GetUV(&u1,&v1);
                            true_wv.GetUV(&u2,&v2);
			    wvp->SetUV(u1-u2,v1-v2);
			    wvp=wvc->ambiguities.GetNext();
			  }

		}
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

//----------------------------------//
// WindSwath::DirectionDensityVsCti //
//----------------------------------//

int
WindSwath::DirectionDensityVsCti(
    WindField*      truth,
    unsigned int**  swath_density_array,
    unsigned int**  field_density_array,
    float           low_speed,
    float           high_speed,
    int             direction_count)
{
    //-------------------------//
    // index direction density //
    //-------------------------//

    Index dir_idx;
    dir_idx.SpecifyWrappedCenters(0.0, two_pi, direction_count);

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        //------------------------------//
        // clear the count for this cti //
        //------------------------------//

        for (int dir = 0; dir < direction_count; dir++)
        {
            *( *(swath_density_array + cti) + dir) = 0;
            *( *(field_density_array + cti) + dir) = 0; 
        }

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

            //--------------------------------------//
            // determine the S/C velocity direction //
            //--------------------------------------//

            int ati_minus = ati - 1;
            if (ati_minus < 0)
                ati_minus = 0;
			WVC* wvc_minus = swath[cti][ati_minus];
			if (! wvc_minus)
                continue;

            int ati_plus = ati + 1;
            if (ati_plus >= _alongTrackBins)
                ati_plus = _alongTrackBins - 1;
			WVC* wvc_plus = swath[cti][ati_plus];
			if (! wvc_plus)
                continue;

            double dlat = wvc_plus->lonLat.latitude -
                wvc_minus->lonLat.latitude;
            double dlon = wvc_plus->lonLat.longitude -
                wvc_minus->lonLat.longitude;
            while (dlon > pi)
                dlon -= two_pi;
            while (dlon < -pi)
                dlon += two_pi;

            double sc_dir = atan2(dlat, dlon);    // ccw from east

            //-------------------------------//
            // determine the wind directions //
            //-------------------------------//

            float ret_dir = wvc->selected->dir;
            float true_dir = true_wv.dir;

            //-----------------------------------------------//
            // determine the relative wind direction (0-360) //
            //-----------------------------------------------//

            float rel_ret_dir = ret_dir - sc_dir;
            while (rel_ret_dir < 0.0)
                rel_ret_dir += two_pi;
            while (rel_ret_dir > two_pi)
                rel_ret_dir -= two_pi;

            float rel_true_dir = true_dir - sc_dir;
            while (rel_true_dir < 0.0)
                rel_true_dir += two_pi;
            while (rel_true_dir > two_pi)
                rel_true_dir -= two_pi;

            //---------------------//
            // determine the index //
            //---------------------//

            int ret_idx, true_idx;
            dir_idx.GetNearestIndex(rel_ret_dir, &ret_idx);
            dir_idx.GetNearestIndex(rel_true_dir, &true_idx);

            ( *( *(swath_density_array + cti) + ret_idx) )++;
            ( *( *(field_density_array + cti) + true_idx) )++;
		}
	}

	return(1);
}

//-------------------------------------//
// WindSwath::ComponentCovarianceVsCti //
//-------------------------------------//

int
WindSwath::ComponentCovarianceVsCti(
    WindField*      truth,
    float*          cc_array,
    int*            count_array,
    float           low_speed,
    float           high_speed, 
    COMPONENT_TYPE  component1, 
    COMPONENT_TYPE  component2)
{
        // In all this:
        // c1 is the value of component1
        // c2 is the value of component2
	// x is (c1*c2)
       
        //---------------------------------//
        // Allocate Mean Arrays            //
        //---------------------------------//
 
        float* mean_c1_array= new float[_crossTrackBins];
        float* mean_c2_array= new float[_crossTrackBins];

	for (int cti = 0; cti < _crossTrackBins; cti++)
	{
		*(cc_array + cti) = 0.0;
		*(count_array + cti) = 0;
                *(mean_c1_array +cti)=0.0;
                *(mean_c2_array +cti)=0.0;

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
			//-------------------//
			// Find Components   //
			//-------------------//
			float u=0,v=0;
                        float c1=0,c2=0;
			switch(component1){
			case UTRUE:
			  true_wv.GetUV(&u,&v);
			  c1=u;
			  break;
			case VTRUE:
			  true_wv.GetUV(&u,&v);
			  c1=v;
			  break;
			case UMEAS:
			  wvc->selected->GetUV(&u,&v);
			  c1=u;
			  break;
			case VMEAS:
			  wvc->selected->GetUV(&u,&v);
			  c1=v;
			  break;
			default:
			  fprintf(stderr,"ComponentCovariance: Bad component1\n");
			  return(0);
			}
			switch(component2){
			case UTRUE:
			  true_wv.GetUV(&u,&v);
			  c2=u;
			  break;
			case VTRUE:
			  true_wv.GetUV(&u,&v);
			  c2=v;
			  break;
			case UMEAS:
			  wvc->selected->GetUV(&u,&v);
			  c2=u;
			  break;
			case VMEAS:
			  wvc->selected->GetUV(&u,&v);
			  c2=v;
			  break;
			default:
			  fprintf(stderr,"ComponentCovariance: Bad component2\n");
			  return(0);
			}
                        //-------------------//
			// Update Arrays     //
			//-------------------//
			float x = c1*c2;
			*(cc_array + cti) += x;
                        *(mean_c1_array +cti) +=c1;
                        *(mean_c2_array +cti) +=c2;
			(*(count_array + cti))++;
		}

		if (*(count_array + cti) < 2)
			continue;

		//----------------------------------//
		// calculate the covariance         //
		//----------------------------------//

		*(mean_c1_array + cti) /= (float)*(count_array + cti);
		*(mean_c2_array + cti) /= (float)*(count_array + cti);
		*(cc_array + cti) /= (float)*(count_array + cti);
                *(cc_array +cti) -= (*(mean_c1_array +cti))*(*(mean_c2_array +cti));

	}

	delete(mean_c1_array);
        delete(mean_c2_array);
	return(1);
}
int
WindSwath::VectorCorrelationVsCti(
        WindField* truth,
	float* vc_array,
	int* count_array,
	float low_speed,
	float high_speed){
  //---------------------------------------------//
  // Allocate Component Covariance  Arrays       //
  // Nomenclature s[component_id][component_id]  //
  // Component IDs                               //
  // u1: u-component of true vector              //
  // v1: v-component of true vector              //
  // u2: u-component of selected vector          //
  // v2: v-component of selected vector          //
  //---------------------------------------------//

  float* su1u1_array= new float[_crossTrackBins];
  float* su2u2_array= new float[_crossTrackBins];
  float* sv1v1_array= new float[_crossTrackBins];
  float* sv2v2_array= new float[_crossTrackBins];
  float* su1u2_array= new float[_crossTrackBins];
  float* sv1v2_array= new float[_crossTrackBins];
  float* su1v1_array= new float[_crossTrackBins];
  float* su1v2_array= new float[_crossTrackBins];
  float* su2v1_array= new float[_crossTrackBins];
  float* su2v2_array= new float[_crossTrackBins];

  //----------------------------------------------//
  // Calculate Component Covariances              //
  //----------------------------------------------//
  if(! ComponentCovarianceVsCti(truth, su1u1_array, count_array, low_speed, 
		      high_speed, UTRUE, UTRUE))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su2u2_array, count_array, low_speed, 
		      high_speed, UMEAS, UMEAS))
    return(0);
  if(! ComponentCovarianceVsCti(truth, sv1v1_array, count_array, low_speed, 
		      high_speed, VTRUE, VTRUE))
    return(0);
  if(! ComponentCovarianceVsCti(truth, sv2v2_array, count_array, low_speed, 
		      high_speed, VMEAS, VMEAS))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su1u2_array, count_array, low_speed, 
		      high_speed, UTRUE, UMEAS))
    return(0);
  if(! ComponentCovarianceVsCti(truth, sv1v2_array, count_array, low_speed, 
		      high_speed, VTRUE, VMEAS))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su1v1_array, count_array, low_speed, 
		      high_speed, UTRUE, VTRUE))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su1v2_array, count_array, low_speed, 
		      high_speed, UTRUE, VMEAS))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su2v1_array, count_array, low_speed, 
		      high_speed, UMEAS, VTRUE))
    return(0);
  if(! ComponentCovarianceVsCti(truth, su2v2_array, count_array, low_speed, 
		      high_speed, UMEAS, VMEAS))
    return(0);
  
  //-----------------------------------------------//
  // Compute Vector Correlation Array              //
  //-----------------------------------------------//

  for(int cti=0;cti<_crossTrackBins;cti++){
    float su1u1=*(su1u1_array +cti);
    float su2u2=*(su2u2_array +cti);
    float sv1v1=*(sv1v1_array +cti);
    float sv2v2=*(sv2v2_array +cti);
    float su1u2=*(su1u2_array +cti);
    float sv1v2=*(sv1v2_array +cti);
    float su1v1=*(su1v1_array +cti);
    float su1v2=*(su1v2_array +cti);
    float su2v1=*(su2v1_array +cti);
    float su2v2=*(su2v2_array +cti);


    *(vc_array +cti)= su1u1*(su2u2*sv1v2*sv1v2+sv2v2*su2v1*su2v1)+
      sv1v1*(su2u2*su1v2*su1v2+sv2v2*su1u2*su1u2)+
      2.0*(su1v1*su1v2*su2v1*su2v2)+2.0*(su1v1*su1u2*sv1v2*su2v2)-
      2.0*(su1u1*su2v1*sv1v2*su2v2)-2.0*(su2u2*su1v1*su1v2*sv1v2)-
      2.0*(sv2v2*su1v1*su1u2*su2v1)-2.0*(sv1v1*su1u2*su1v2*su2v2);

    (*(vc_array +cti)) /=(su1u1*sv1v1-(su1v1*su1v1))*(su2u2*sv2v2-su2v2*su2v2);
  }
  //----------------------------------------------//
  // deallocate arrays                            //
  //----------------------------------------------//
  delete(su1u1_array);
  delete(su2u2_array);
  delete(sv1v1_array);
  delete(sv2v2_array);
  delete(su1u2_array);
  delete(sv1v2_array);
  delete(su1v1_array);
  delete(su1v2_array);
  delete(su2v1_array);
  delete(su2v2_array);

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

//-------------------------------//
// WindSwath::_OpenOneHdfDataSet //
//-------------------------------//

int
WindSwath::_OpenOneHdfDataSet(
TlmHdfFile*   tlmHdfFile,
SourceIdE     source,
ParamIdE      param)
{
    const char* sdsName = ParTabAccess::GetSdsNames(source, param);
    if (sdsName == 0)
        return(0);

    int32 dataType=0, dataStartIndex=0, dataLength=0, numDimensions=0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType,
                             dataStartIndex, dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);

} // WindSwath::_OpenOneHdfDataSet

//-----------------------------//
// WindSwath::_OpenHdfDataSets //
//-----------------------------//

int
WindSwath::_OpenHdfDataSets(
TlmHdfFile*     tlmHdfFile)
{
    if ((_lonSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LON)) == 0)
        return(0);
    if ((_latSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LAT)) == 0)
        return(0);
    if ((_speedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
                                                     WIND_SPEED)) == 0)
        return(0);
    if ((_dirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WIND_DIR)) == 0)
        return(0);
    if ((_mleSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
                                                     MAX_LIKELIHOOD_EST)) == 0)
        return(0);
    if ((_selectSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
                                                     WVC_SELECTION)) == 0)
        return(0);

    return(1);

} // WindSwath::_OpenHdfDataSets

//------------------------------//
// WindSwath::_CloseHdfDataSets //
//------------------------------//

void
WindSwath::_CloseHdfDataSets(void)
{
    (void)SDendaccess(_lonSdsId); _lonSdsId = HDF_FAIL;
    (void)SDendaccess(_latSdsId); _latSdsId = HDF_FAIL;
    (void)SDendaccess(_speedSdsId); _speedSdsId = HDF_FAIL;
    (void)SDendaccess(_dirSdsId); _dirSdsId = HDF_FAIL;
    (void)SDendaccess(_mleSdsId); _mleSdsId = HDF_FAIL;
    (void)SDendaccess(_selectSdsId); _selectSdsId = HDF_FAIL;
    return;

} // WindSwath::_OpenHdfDataSets
