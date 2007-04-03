//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "WindVector.h"
#include "Wind.h"
#include "GSparameters.h"
#include "Array.h"

//================//
// WindVectorPlus //
//================//

WindVectorPlus::WindVectorPlus()
:   obj(0.0)
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
    FILE*  fp)
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
    FILE*  fp)
{
    fprintf(fp, "Spd=%g Dir=%g(%g) Obj=%g\n", spd, dir * rtd, dir, obj);
    return(1);
}

//-------------------------//
// WindVectorPlus::ReadL2B //
//-------------------------//

int
WindVectorPlus::ReadL2B(
    FILE*  fp)
{
    if (fread((void *)&spd, sizeof(float), 1, fp) != 1 ||
        fread((void *)&dir, sizeof(float), 1, fp) != 1 ||
        fread((void *)&obj, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//=================//
// WindVectorField //
//=================//

WindVectorField::WindVectorField()
{
    return;
}

WindVectorField::~WindVectorField()
{
    lon.Free();
    lat.Free();
    dir.Free();
    spd.Free();
    return;
}

//---------------------------//
// WindVectorField::ReadVctr //
//---------------------------//

int
WindVectorField::ReadVctr(
    const char*  filename)
{
    //-----------//
    // open file //
    //-----------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    //----------------//
    // allocate 1 rev //
    //----------------//

    int revsize = 16000;
    lon.Allocate(revsize);
    lat.Allocate(revsize);
    spd.Allocate(revsize);
    dir.Allocate(revsize);

    //------------//
    // read field //
    //------------//

    float llon, llat, lspd, ldir;
    int idx=0;

    // first read
    if (fread((void *)&ldir, sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }


    if (fread((void *)&llon, sizeof(float), 1, fp) != 1 ||
        fread((void *)&llat, sizeof(float), 1, fp) != 1 ||
        fread((void *)&lspd, sizeof(float), 1, fp) != 1 ||
        fread((void *)&ldir, sizeof(float), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }


    while(! feof(fp))
    {
        idx++;

        lon.SetElement(idx - 1, (double)llon);
        lat.SetElement(idx - 1, (double)llat);
        spd.SetElement(idx - 1, (double)lspd);
        dir.SetElement(idx - 1, (double)ldir);

        //-----------//
        // next read //
        //-----------//

        if (fread((void *)&llon, sizeof(float), 1, fp) != 1 ||
            fread((void *)&llat, sizeof(float), 1, fp) != 1 ||
            fread((void *)&lspd, sizeof(float), 1, fp) != 1 ||
            fread((void *)&ldir, sizeof(float), 1, fp) != 1)
        {
            fclose(fp);
            break;
        }
    }


    fclose(fp);

    return(idx);
}

//-----------------------------------------//
// WindVectorField::InterpolateVectorField //
//-----------------------------------------//

int
WindVectorField::InterpolateVectorField(
    LonLat       lon_lat,
    WindVector*  nwv,
    int          idx)
{
    // find size of windfield

    int sz = dir.GetSize();

    // get earth position of lon_lat

    EarthPosition cell25;
    double alt = 0;
    double bound = 50.;

    cell25.SetAltLonGCLat(alt, (double)lon_lat.longitude,
        (double)lon_lat.latitude);

    // loop through and find position closest

    double distance;
    int good_idx = -1;

    for (int i = 0; i < sz; i++)
    {
        EarthPosition cell50;
        double llat, llon;

        lon.GetElement(i, &llon);
        lat.GetElement(i, &llat);
        cell50.SetAltLonGCLat(alt, llon, llat);

        distance = cell50.SurfaceDistance(cell25);
        if (i == 0)
          fprintf(stderr,"Surface Distance: %g\n",distance);

        if (distance < bound)
        {
            double dir_value, spd_value;
            dir.GetElement(i, &dir_value);
            spd.GetElement(i, &spd_value);
            nwv->SetSpdDir(spd_value, dir_value);
            good_idx = i;
        }
    }

    if (good_idx == -1)
    {
        fprintf(stderr,"Couldn't find 50km cell\n");
        return(0);
    }

    return(1);
}

//=====//
// WVC //
//=====//

WVC::WVC()
:   nudgeWV(NULL), selected(NULL), selected_allocated(0), specialVector(NULL),
    rainProb(0.0), rainFlagBits(0)
{
    return;
}

WVC::~WVC()
{
    WindVectorPlus* wvp;
    ambiguities.GotoHead();

    while ((wvp = ambiguities.RemoveCurrent()) != NULL)
    {
        if (wvp == selected)
            selected_allocated = 0;
        delete wvp;
    }
    if (selected_allocated) {
        if (selected != NULL)
            delete selected;
    }
    if (specialVector != NULL) {
        delete specialVector;
    }
        
    if (nudgeWV)
    {
        delete nudgeWV;
        nudgeWV = NULL;
    }
    return;
}

//---------------//
// WVC::WriteL2B //
//---------------//

int
WVC::WriteL2B(
    FILE*  fp)
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
        if(selected_allocated) count++;
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

    if (selected_allocated)
    {
        selected_idx = ambiguities.NodeCount();
        if (! selected->WriteL2B(fp))
            return(0);
    }

    //----------------------//
    // write selected index //
    //----------------------//

    if (fwrite((void *)&selected_idx, sizeof(char), 1, fp) != 1)
        return(0);

    //--------------------//
    // write nudge vector //
    //--------------------//

    char nudgeWV_allocated= nudgeWV ? 1 : 0;
    if (fwrite((void *)&nudgeWV_allocated, sizeof(char), 1, fp) != 1)
        return(0);
    if (nudgeWV_allocated)
    {
        if (! nudgeWV->WriteL2B(fp))
            return(0);
    }

    //-----------------------//
    // Write directionRanges //
    //-----------------------//

    if (! directionRanges.Write(fp))
        return(0);

    return(1);
}

//--------------//
// WVC::ReadL2B //
//--------------//

int
WVC::ReadL2B(
    FILE*  fp)
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

    if (selected_idx < 0)
        selected = NULL;
    else
        selected = ambiguities.GetByIndex((int)selected_idx);

    //-------------------//
    // read nudge vector //
    //-------------------//

    char nudgeWV_allocated;
    if (fread((void *)&nudgeWV_allocated, sizeof(char), 1, fp) != 1)
        return(0);
    if (nudgeWV_allocated)
    {
        nudgeWV = new WindVectorPlus;
        if (! nudgeWV->ReadL2B(fp))
            return(0);
    }

    //----------------------//
    // Read directionRanges //
    //----------------------//

    if (! directionRanges.Read(fp))
        return(0);

    return(1);
}

//----------------//
// WVC::WriteVctr //
//----------------//

int
WVC::WriteVctr(
    FILE*      fp,
    const int  rank)
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
    FILE*  fp)
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

    //-----------------------//
    // Write directionRanges //
    //-----------------------//

    if (! directionRanges.WriteAscii(fp))
        return(0);

    return(1);
}

//------------------//
// WVC::WriteFlower //
//------------------//

int
WVC::WriteFlower(
    FILE*  fp)
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

    float max_dist = 10.0;    // 10 km
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
            wvp = ambiguities.RemoveCurrent();    // next becomes current
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
    float  dir,
    int    max_rank)
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


WindVectorPlus*
WVC::GetNearestVector(
    float  dir,
    float  spd,
    int    max_rank)
{
    WindVectorPlus* nearest = NULL;
    float min_dif = 1000000;
    float u=spd*cos(dir);
    float v=spd*sin(dir);
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
        float u2=wvp->spd*cos(wvp->dir);
        float v2=wvp->spd*sin(wvp->dir);
        float dif = (u2-u)*(u2-u)+(v2-v)*(v2-v);
        if (dif < min_dif)
        {
            min_dif = dif;
            nearest = wvp;
        }
    }
    return(nearest);
}

//---------------------------------//
// WVC::GetNearestRangeToDirection //
//---------------------------------//
// Returns the nearest ambiguity which is included in the S3 probability
// threshold, other ambiguities those which sum to less than 1-threshold, and
// thus have 0 width are excluded

WindVectorPlus*
WVC::GetNearestRangeToDirection(
    float  dir)
{
    WindVectorPlus* nearest = NULL;
    float min_dif = two_pi;

    AngleInterval* range=directionRanges.GetHead();
    for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
        wvp = ambiguities.GetNext())
    {
        if (range->left!=range->right)
        {
            float dif = ANGDIF(wvp->dir, dir);
            if (dif < min_dif)
            {
                min_dif = dif;
                nearest = wvp;
            }
        }
        range=directionRanges.GetNext();
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

#define MAXIMUM_ACCEPTABLE_SPEED 100

int
WVC::Rank_Wind_Solutions()
{
    // Translated from GS module: Rank_Wind_Solutions2.F
    // - Final sorting omitted.
    //!File Name:    Rank_Wind_Solutions.F
    //
    //!Description:
    //        This routine eliminates
    //            (1) redundant wind solutions
    //            (2) out-of-range
    //        and ranks the "final" wind solutions to be
    //        used by the ambiguity-removal processor.
    //
    //!Input Parameters:
    //    WIND_SPEED_DELTA        -    speed tolerance value (0.1 m/s).
    //    WIND_DIR_DELTA            -    direction tolerance value (5 degrees).
    //    WIND_LIKELIHOOD_DELTA    -    MLE tolerance value (0.5).
    //
    //!Input/Output Parameters:
    //    wr_num_ambigs        - number of ambigous wind solutions.
    //    wr_mle                - value of maximum likelihood estimation.
    //    wr_wind_speed        - speeds of ambiguous wind solutions.
    //    wr_wind_dir            - directions of ambiguous wind solutions.
    //    wr_wind_speed_err    - errors associated with wind speed.
    //    wr_wind_dir_err        - errors associated with wind direction.
    //

    float wr_wind_speed[WIND_MAX_SOLUTIONS];
    float wr_wind_dir[WIND_MAX_SOLUTIONS];
    float wr_mle[WIND_MAX_SOLUTIONS];

    // Copy data into wr_ arrays. (convert radians to degrees)
    int i = 0;
    for (WindVectorPlus* wvp = ambiguities.GetHead(); wvp;
        wvp = ambiguities.GetNext())
    {
        wr_wind_speed[i] = wvp->spd;
        wr_wind_dir[i] = rtd*wvp->dir;
        wr_mle[i] = wvp->obj;
        i++;
        if (i >= WIND_MAX_SOLUTIONS)
            break;
    }
    int wr_num_ambigs = i;

    // Local Declarations
    int    j;
    int    start_j;
    int    ambig;
    int    speed_large_flag[WIND_MAX_SOLUTIONS];
    int    speed_large_counter;
    int    i_speed_processed;
    int    i_twin_processed;
    int    twin_flag[WIND_MAX_SOLUTIONS];
    int    twin_counter;
    int    num_ambigs;
    float  diff_speed;
    float  diff_dir;
    float  diff_mle;

    int    num_sorted;
//  int    max_index;
//  float  max_mle;

//    Initialize.

    speed_large_counter = 0;
    twin_counter = 0;
    num_sorted = 0;

    for (j = 0; j < WIND_MAX_SOLUTIONS; j++)
    {
        speed_large_flag[j] = 0;
        twin_flag[j] = 0;
    }

    for (ambig = 0; ambig < wr_num_ambigs; ambig++)
    {
//    Wind direction:
//        1. Constrain values between 0 and 360 degrees.
//        2. Conform to SEAWIND's oceanographic convention.

        if (wr_wind_dir[ambig] < 0.0)
        {
            wr_wind_dir[ambig] += 360.0;
        }

        if (wr_wind_dir[ambig] > 360.0)
        {
            // change made here for the sign: Kyung
            wr_wind_dir[ambig] -= 360.0;
        }

// Set high wind speed flag and count up if larger than MAXIMUM_ACCEPTABLE_SPEED.

        if (wr_wind_speed[ambig] > MAXIMUM_ACCEPTABLE_SPEED)
        {
            speed_large_flag[ambig] = 1;
            speed_large_counter++;
        }
    }

// Eliminate wind solutions with speed > MAXIMUM_ACCEPTABLE_SPEED.

    if (speed_large_counter > 0)
    {
        num_ambigs = wr_num_ambigs - speed_large_counter;
        start_j = 0;

        for (i = 0; i < num_ambigs; i++)
        {
            i_speed_processed = 0;
            for (j = start_j; j < wr_num_ambigs; j++)
            {
                if (! i_speed_processed && speed_large_flag[j] == 0)
                {
                    wr_wind_speed[i] = wr_wind_speed[j];
                    wr_wind_dir[i] = wr_wind_dir[j];
                    wr_mle[i] = wr_mle[j];
                    start_j = j + 1;
                    i_speed_processed = 1;
                }
            }
        }
        wr_num_ambigs = num_ambigs;
    }

// Identify and eliminate possible "twin" solutions. Search
// the entire solution space, tag twin solutions with smaller MLE value.

// write(97,*) wr_wind_speed,wr_wind_dir,wr_mle
    for (i = 0; i < wr_num_ambigs; i++)
    {
        twin_flag[i] = 0;
    }
    twin_counter = 0;

    for (i = 0; i < wr_num_ambigs - 1; i++)
    {
        for (j = i + 1; j < wr_num_ambigs; j++)
        {
            diff_speed = fabs (wr_wind_speed[i] - wr_wind_speed[j]);
            diff_dir = fabs (wr_wind_dir[i] - wr_wind_dir[j]);
            diff_mle = fabs (wr_mle[i] - wr_mle[j]);

            if (diff_dir > 180.0)
                diff_dir = 360.0 - diff_dir;

            if (diff_speed <= WIND_SPEED_DELTA &&
                diff_dir < WIND_DIR_DELTA &&
                diff_mle < WIND_LIKELIHOOD_DELTA)
            {
                if (twin_flag[j] == 0)
                {
                    twin_flag[j] = 1;
                    twin_counter = twin_counter + 1;

                    if (wr_mle[j] > wr_mle[i])
                    {
                        wr_wind_speed[i] = wr_wind_speed[j];
                        wr_wind_dir[i] = wr_wind_dir[j];
                        wr_mle[i] = wr_mle[j];
                    }
                }
            }
        }
    }

// Eliminate tagged twin solutions.

    if (twin_counter > 0)
    {
//      write(97,*) 'number of twins found ',twin_counter
        num_ambigs = wr_num_ambigs - twin_counter;
        start_j = 0;
        for (i = 0; i < num_ambigs; i++)
        {
            i_twin_processed = 0;
            for (j = start_j; j < wr_num_ambigs; j++)
            {
                if (! i_twin_processed)
                {
                    if (twin_flag[j] == 0)
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
//      write(97,*) wr_wind_speed,wr_wind_dir,wr_mle
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

//------------------------------//
// WVC::GetEstimatedSquareError //
//------------------------------//

float
WVC::GetEstimatedSquareError()
{
    float* prob = new float[ambiguities.NodeCount()];
    WindVectorPlus* wvp = ambiguities.GetHead();
    float scale = wvp->obj;
    int idx = 0;
    float sum = 0;
    while (wvp)
    {
        prob[idx] = exp((wvp->obj - scale) / 2.0);
        sum += prob[idx];
        idx++;
        wvp = ambiguities.GetNext();
    }
    for (int c = 0; c < ambiguities.NodeCount(); c++)
        prob[c] /= sum;

    wvp = ambiguities.GetHead();
    idx = 0;
    float SE = 0;
    while (wvp)
    {
        if (selected != wvp)
        {
            float x1, x2, y1, y2;
            selected->GetUV(&x1, &y1);
            wvp->GetUV(&x2, &y2);
            float dx = x1 - x2;
            float dy = y1 - y2;
            SE += (dx*dx + dy*dy) * prob[idx];
        }
        idx++;
        wvp = ambiguities.GetNext();
    }
    delete[] prob;
    return(SE);
}

//-----------------------//
// WVC::RedistributeObjs //
//-----------------------//

int
WVC::RedistributeObjs()
{
    if (directionRanges.bestObj == NULL)
        return(0);

    int nd = directionRanges.dirIdx.GetBins();
    float step = directionRanges.dirIdx.GetStep();
    int nc = ambiguities.NodeCount();
    float* new_obj = new float[nc];
    for (int c = 0; c < nc; c++)
    {
        new_obj[c] = 0.0;
    }
    for (int c = 0; c < nd; c++)
    {
        float dir = c * step;
        // Find Closest Ambiguity
        WindVectorPlus* closest = GetNearestToDirection(dir);
        WindVectorPlus* wvp = ambiguities.GetHead();
        int idx = 0;
        while (wvp != closest)
        {
            wvp = ambiguities.GetNext();
            idx++;
        }
        float tmp = directionRanges.bestObj[c] - closest->obj;
        tmp /= 2;
        tmp = exp(tmp);
        new_obj[idx] += tmp;
    }
    WindVectorPlus* wvp = ambiguities.GetHead();
    float old_sum = 0, new_sum = 0;
    for (int c = 0; c < nc; c++, wvp = ambiguities.GetNext())
    {
        old_sum += wvp->obj;
        wvp->obj += 2.0 * log(new_obj[c]);
        new_sum += wvp->obj;
    }
    float scale = old_sum / new_sum;
    for (wvp = ambiguities.GetHead(); wvp; wvp = ambiguities.GetNext())
        wvp->obj *= scale;
    delete[] new_obj;
    SortByObj();
    return(1);
}

//===========//
// WindField //
//===========//

WindField::WindField()
:   _wrap(0), _useFixedSpeed(0), _fixedSpeed(0.0),
    _useFixedDirection(0), _fixedDirection(0.0), _useRandomDirection(0), _field(0)
{
    return;
}

WindField::~WindField()
{
    _Deallocate();
    return;
}

//--------------------//
// WindField::ReadSV  //
// windfield from     //
// Svetla Veleva      //
//--------------------//

int
WindField::ReadSV(
    const char*  filename)
{
    //-----------//
    // open file //
    //-----------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    int binByte1, binByte2, dataByte1, dataByte2;
    int lonDim, latDim;

    if (fread((void *)&binByte1, sizeof(int), 1, fp) != 1 ||
        fread((void *)&lonDim, sizeof(int), 1, fp) != 1 ||
        fread((void *)&latDim, sizeof(int), 1, fp) != 1 ||
        fread((void *)&binByte2, sizeof(int), 1, fp) != 1 ||
        fread((void *)&dataByte1, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    //------------//
    // read field //
    //------------//

    float u[latDim][lonDim];
    float v[latDim][lonDim];

    int size = lonDim * latDim * sizeof(float);
    if (fread((void *)u, size, 1, fp) != 1 ||
        fread((void *)v, size, 1, fp) != 1)
    {
        fclose(fp);
        return(0);
    }

    if (fread((void *)&dataByte2, sizeof(int), 1, fp) != 1)
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

    _lon.SpecifyWrappedCenters(lon_min * dtr, lon_max * dtr, lonDim);
    _lat.SpecifyCenters(lat_min * dtr, lat_max * dtr, latDim);

    if (! _Allocate())
        return(0);

    for (int lon_idx = 0; lon_idx < lonDim; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < latDim; lat_idx++)
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

//--------------------//
// WindField::ReadVap //
//--------------------//

int
WindField::ReadVap(
    const char*  filename)
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
    const char*  filename)
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
    const char*  filename)
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
    int uv_size = ECMWF_LORES_LON_DIM * ECMWF_LORES_LAT_DIM * sizeof(short);

    if (fread((void *)&head, head_size, 1, fp) != 1 ||
        fread((void *)tmp_u, uv_size, 1, fp) != 1 ||
        fread((void *)tmp_v, uv_size, 1, fp) != 1)
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

    _lon.SpecifyWrappedCenters(0.0 * dtr, 360.0 * dtr, ECMWF_LORES_LON_DIM);
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
    const char*  filename)
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

//----------------------//
// WindField::ReadNCEP1 //
//----------------------//

int
WindField::ReadNCEP1(
    const char*  filename)
{
    if (_field != NULL)
        return(2);

    //-------------------------//
    // determine the file size //
    //-------------------------//

    struct stat buf;
    if (stat(filename, &buf) != 0)
    {
        fprintf(stderr, "WindField::ReadNCEP1: can't stat file %s\n",
            filename);
        return(0);
    }

    int use_int = 0;
    if (buf.st_size == 522720)
        use_int = 1;

    //-----------//
    // open file //
    //-----------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "WindField::ReadNCEP1: error opening file %s\n",
            filename);
        return(0);
    }

    //------------//
    // read field //
    //------------//

    int int_head[NCEP1_LON_DIM];
    int int_u[NCEP1_LAT_DIM][NCEP1_LON_DIM];
    int int_v[NCEP1_LAT_DIM][NCEP1_LON_DIM];

    short short_head[NCEP1_LON_DIM];
    short short_u[NCEP1_LAT_DIM][NCEP1_LON_DIM];
    short short_v[NCEP1_LAT_DIM][NCEP1_LON_DIM];

    void* head_ptr = NULL;
    void* u_ptr = NULL;
    void* v_ptr = NULL;
    int head_size = 0;
    int uv_size = 0;
    if (use_int)
    {
        head_ptr = int_head;
        u_ptr = int_u;
        v_ptr = int_v;
        head_size = NCEP1_LON_DIM * sizeof(int);
        uv_size = NCEP1_LON_DIM * NCEP1_LAT_DIM * sizeof(int);
    }
    else
    {
        head_ptr = short_head;
        u_ptr = short_u;
        v_ptr = short_v;
        head_size = NCEP1_LON_DIM * sizeof(short);
        uv_size = NCEP1_LON_DIM * NCEP1_LAT_DIM * sizeof(short);
    }

    if (fread((void *)head_ptr, head_size, 1, fp) != 1 ||
        fread((void *)u_ptr, uv_size, 1, fp) != 1 ||
        fread((void *)v_ptr, uv_size, 1, fp) != 1)
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

    float u[NCEP1_LAT_DIM][NCEP1_LON_DIM];
    float v[NCEP1_LAT_DIM][NCEP1_LON_DIM];
    float scale = float(NCEP1_SCALE_FACTOR);

    //-------------------------------//
    // transfer to wind field format //
    //-------------------------------//

    _lon.SpecifyWrappedCenters(0.0 * dtr, 360.0 * dtr, NCEP1_LON_DIM);
    _lat.SpecifyCenters(-90.0 * dtr, 90.0 * dtr, NCEP1_LAT_DIM);

    if (! _Allocate())
        return(0);

    for (int lon_idx = 0; lon_idx < NCEP1_LON_DIM; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < NCEP1_LAT_DIM; lat_idx++)
        {
            if (use_int)
            {
                u[lat_idx][lon_idx] = (float)int_u[lat_idx][lon_idx] / scale;
                v[lat_idx][lon_idx] = (float)int_v[lat_idx][lon_idx] / scale;
            }
            else
            {
                u[lat_idx][lon_idx] = (float)short_u[lat_idx][lon_idx] / scale;
                v[lat_idx][lon_idx] = (float)short_v[lat_idx][lon_idx] / scale;
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

//----------------------//
// WindField::ReadNCEP2 //
//----------------------//

int
WindField::ReadNCEP2(
    const char*  filename)
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

    short head[NCEP2_LON_DIM];
    short tmp_u[NCEP2_LAT_DIM][NCEP2_LON_DIM];
    short tmp_v[NCEP2_LAT_DIM][NCEP2_LON_DIM];

    int head_size = NCEP2_LON_DIM * sizeof(short);
    int uv_size = NCEP2_LON_DIM * NCEP2_LAT_DIM * sizeof(short);

    if (fread((void *)&head, head_size, 1, fp) != 1 ||
        fread((void *)tmp_u, uv_size, 1, fp) != 1 ||
        fread((void *)tmp_v, uv_size, 1, fp) != 1)
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

    float u[NCEP2_LAT_DIM][NCEP2_LON_DIM];
    float v[NCEP2_LAT_DIM][NCEP2_LON_DIM];
    float scale = float(NCEP2_SCALE_FACTOR);

    //-------------------------------//
    // transfer to wind field format //
    //-------------------------------//

    _lon.SpecifyWrappedCenters(0.0 * dtr, 360.0 * dtr, NCEP2_LON_DIM);
    _lat.SpecifyCenters(-90.0 * dtr, 90.0 * dtr, NCEP2_LAT_DIM);

    if (! _Allocate())
        return(0);

    for (int lon_idx = 0; lon_idx < NCEP2_LON_DIM; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < NCEP2_LAT_DIM; lat_idx++)
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

//--------------------------//
// WindField::ReadHurricane //
//--------------------------//

int
WindField::ReadHurricane(
    const char*  filename)
{
    if (_field != NULL)
        return(2);

    //-----------//
    // open file //
    //-----------//

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    //-----------------//
    // Read Boundaries //
    //-----------------//

    float latmin,latmax,lonmin,lonmax;
    char string[80];
    fscanf(fp, "%s", string);
    latmin = atof(string) * dtr;
    fscanf(fp, "%s", string);
    latmax = atof(string) * dtr;
    fscanf(fp, "%s", string);
    lonmin = atof(string) * dtr;
    fscanf(fp, "%s", string);
    lonmax = atof(string) * dtr;

    //-------------------------------//
    // Read size of arrays, allocate //
    //-------------------------------//

    fscanf(fp, "%s", string);
    int num_lat_bins = atoi(string);
    fscanf(fp, "%s", string);
    int num_lon_bins = atoi(string);

    if (num_lon_bins < 1 || num_lat_bins < 1)
        return(0);

    float** tmp_spd = (float**)make_array(sizeof(float), 2, num_lat_bins,
        num_lon_bins);
    float** tmp_dir = (float**)make_array(sizeof(float), 2, num_lat_bins,
        num_lon_bins);

    //------------//
    // read field //
    //------------//

    for (int c = 0; c < num_lat_bins; c++)
    {
        for (int d = 0; d < num_lon_bins; d++)
        {
            fscanf(fp, "%s", string);
            fscanf(fp, "%s", string);
            fscanf(fp, "%s", string);
            tmp_spd[c][d] = atof(string);
            fscanf(fp, "%s", string);
            tmp_dir[c][d] = (450 - atof(string)) * dtr;
            if (tmp_dir[c][d] < 0)
                tmp_dir[c][d] += two_pi;
            if (tmp_dir[c][d] > two_pi)
                tmp_dir[c][d] -= two_pi;
        }
    }
    fclose(fp);

    //-------------------------------//
    // transfer to wind field format //
    //-------------------------------//

    _lon.SpecifyCenters(lonmin, lonmax, num_lon_bins);
    _lat.SpecifyCenters(latmin, latmax, num_lat_bins);

    if (! _Allocate())
        return(0);

    for (int lon_idx = 0; lon_idx < num_lon_bins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < num_lat_bins; lat_idx++)
        {
            WindVector* wv = new WindVector;
            if (! wv)
                return(0);

            wv->spd = tmp_spd[lat_idx][lon_idx];
            wv->dir = tmp_dir[lat_idx][lon_idx];
            *(*(_field + lon_idx) + lat_idx) = wv;
        }
    }
    _wrap = 0;
    free_array((void*)tmp_spd, 2, num_lat_bins, num_lon_bins);
    free_array((void*)tmp_dir, 2, num_lat_bins, num_lon_bins);
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
    const char*  filename,
    const char*  type)
{
    if (strcasecmp(type, SV_TYPE) == 0)
    {
        return(ReadSV(filename));
    }
    else if (strcasecmp(type, VAP_TYPE) == 0)
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
    else if (strcasecmp(type, NCEP1_TYPE) == 0)
    {
        return(ReadNCEP1(filename));
    }
    else if (strcasecmp(type, NCEP2_TYPE) == 0)
    {
        return(ReadNCEP2(filename));
    }
    else
    {
        fprintf(stderr,
            "WindField::ReadType: can't determine windfield type (%s)\n",
            type);
        return(0);
    }
}

//----------------------//
// WindField::WriteVctr //
//----------------------//

int
WindField::WriteVctr(
    const char*  filename)
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
    WindField*    windfield,
    float        lon_res,
    float        lat_res)
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
    LonLat       lon_lat,
    WindVector*  wv)
{
    // put longitude in range (hopefully)
    float lon_min = _lon.GetMin();
    int wrap_factor = (int)ceil((lon_min - lon_lat.longitude) / two_pi);
    float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

    // convert to longitude index
    int lon_idx;
    if (! _lon.GetNearestIndexStrict(lon, &lon_idx))
        return(0);

    // convert to latitude index
    int lat_idx;
    if (! _lat.GetNearestIndexStrict(lon_lat.latitude, &lat_idx))
        return(0);

    *wv = *(*(*(_field + lon_idx) + lat_idx));

    if (_useFixedSpeed)
    {
        wv->spd = _fixedSpeed;
    }

    return(1);
}

//-----------------------------------//
// WindField::InterpolatedWindVector //
//-----------------------------------//
// Bryan Stiles changed Clipped to Strict Interpolation for _wrap=0 and
// latitude, as he could not think of any case in which you would want
// to extrapolate the wind field

int
WindField::InterpolatedWindVector(
    LonLat       lon_lat,
    WindVector*  wv)
{
    // put longitude in range (hopefully)
    float lon_min = _lon.GetMin();
    int wrap_factor = (int)ceil((lon_min - lon_lat.longitude) / two_pi);
    float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

    // find longitude indices
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

    float u =    lon_coef[0] * lat_coef[0] * corner_u[0][0] +
                lon_coef[0] * lat_coef[1] * corner_u[0][1] +
                lon_coef[1] * lat_coef[0] * corner_u[1][0] +
                lon_coef[1] * lat_coef[1] * corner_u[1][1];

    float v =    lon_coef[0] * lat_coef[0] * corner_v[0][0] +
                lon_coef[0] * lat_coef[1] * corner_v[0][1] +
                lon_coef[1] * lat_coef[0] * corner_v[1][0] +
                lon_coef[1] * lat_coef[1] * corner_v[1][1];

    wv->SetUV(u, v);

    if (_useFixedSpeed)
    {
        wv->spd = _fixedSpeed;
    }

    if (_useFixedDirection)
    {
        wv->dir = _fixedDirection;
    }

    if (_useRandomDirection)
    {
        Uniform ranDir(pi, 0.0);
        wv->dir = ranDir.GetNumber();
    }

    return(1);
}

//---------------------//
// WindField::FixSpeed //
//---------------------//

int
WindField::FixSpeed(
    float  speed)
{
    _useFixedSpeed = 1;
    _fixedSpeed = speed;
    return(1);
}

//-------------------------//
// WindField::FixDirection //
//-------------------------//

int
WindField::FixDirection(
    float  direction)
{
    _useFixedDirection = 1;
    _fixedDirection = direction;
    return(1);
}

//----------------------------//
// WindField::RandomDirection //
//----------------------------//

int
WindField::RandomDirection(
    int  randomFlag)
{
    _useRandomDirection = randomFlag;
    return(1);
}

//-------------------------//
// WindField::SetAllSpeeds //
//-------------------------//

int
WindField::SetAllSpeeds(
    float  speed)
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

//-------------------------//
// WindField::ScaleSpeed   //
//-------------------------//

int
WindField::ScaleSpeed(
    float  scale)
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
                wv->spd *= scale;
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
