//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_c[] =
    "@(#) $Id$";

#include "WindSwath.h"
#include "hdf_support.h"
#include "Distributions.h"
#include "Array.h"
#include "Vect.h"
#include "EarthGeom.h"

#define HDF_NUM_AMBIGUITIES      4
#define S3_USE_MEDIAN_FOR_RANGE  1    // otherwise uses mean filter
#define S3_USE_CLOSEST_VECTOR    0    // otherwise uses closest direction

#define FLIPPING_WITHIN_RANGE_THRESHOLD  (5.0*dtr)

//===========//
// WindSwath //
//===========//

WindSwath::WindSwath()
:   swath(0), useNudgeVectorsAsTruth(0), nudgeVectorsRead(0),
    _crossTrackBins(0), _alongTrackBins(0), _validCells(0)
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
    int  cross_track_bins,
    int  along_track_bins)
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
    int   cti,
    int   ati,
    WVC*  wvc)
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
//        return(0);    // out of range
    }

    if (swath[cti][ati])
    {
        fprintf(stderr, "WindSwath::Add: attempted cell replacement\n");
        fprintf(stderr, "  cti = %d, ati = %d\n", cti, ati);
        return(0);    // already a cell there
    }

    swath[cti][ati] = wvc;
    _validCells++;
    return(1);
}

//-------------------//
// WindSwath::Remove //
//-------------------//

WVC*
WindSwath::Remove(
    int  cti,
    int  ati)
{
    WVC* wvc = NULL;
    if (cti < 0 || cti >= _crossTrackBins ||
        ati < 0 || ati >= _alongTrackBins)
    {
        return(NULL);
    }
    else
    {
        wvc = swath[cti][ati];
        swath[cti][ati] = NULL;
        if (wvc)
            _validCells--;
        return(wvc);
    }
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

//-------------------//
// WindSwath::GetWVC //
//-------------------//

WVC*
WindSwath::GetWVC(
    int  cti,
    int  ati)
{
    if (cti < 0 || cti >= _crossTrackBins ||
        ati < 0 || ati >= _alongTrackBins)
    {
        return(NULL);
    }
    return( *(*(swath + cti) + ati) );
}

//-----------------------//
// WindSwath::GetGoodWVC //
//-----------------------//
// this version won't bother returning a bad WVC
// bad = rain flag doesn't indicate rain-free

WVC*
WindSwath::GetGoodWVC(
    int  cti,
    int  ati)
{
    WVC* wvc = GetWVC(cti, ati);
    if (wvc == NULL)
        return(NULL);

    if (wvc->rainFlagBits & RAIN_FLAG_UNUSABLE ||
        wvc->rainFlagBits & RAIN_FLAG_RAIN)
    {
        // inconclusive or contaminated
        return(NULL);
    }
    return(wvc);
}

//-------------------------//
// WindSwath::ReadFlagFile //
//-------------------------//

int
WindSwath::ReadFlagFile(
    const char*  flag_file)
{
    FILE* ifp = fopen(flag_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr,
            "WindSwath::ReadFlagFile: error opening flag file %s\n",
            flag_file);
        return(0);
    }
    unsigned int size = _alongTrackBins * _crossTrackBins;
    float* flag_value = (float*)malloc(sizeof(float) * size);
    char* flag = (char*)malloc(sizeof(char) * size);
    if (flag_value == NULL || flag == NULL)
    {
        fprintf(stderr,
          "WindSwath::ReadFlagFile: error allocating memory for flag_value\n");
        fclose(ifp);
        return(0);
    }

    if ((fread(flag_value, sizeof(float), size, ifp) != size) ||
        (fread(flag, sizeof(char), size, ifp) != size))
    {
        fprintf(stderr, "Fatal Error: Flag file %s cannot be read\n",
            flag_file);
        exit(1);
    }
    fclose(ifp);
    for (int ati = 0; ati < _alongTrackBins; ati++)
    {
        for (int cti = 0; cti < _crossTrackBins; cti++)
        {
            int offset= ati * _crossTrackBins + cti;
            WVC* wvc = GetWVC(cti, ati);
            if (wvc != NULL)
            {
                wvc->rainProb = flag_value[offset];
                switch ((int)(flag[offset]))
                {
                case 0:
                    wvc->rainFlagBits=0;
                    break;
                case 1:
                    wvc->rainFlagBits=2;
                    break;
                case 2:
                    wvc->rainFlagBits=3;
                    break;
                case 3:
                    wvc->rainFlagBits=4;
                    break;
                case 4:
                    wvc->rainFlagBits=6;
                    break;
                case 5:
                    wvc->rainFlagBits=7;
                    break;
                case 6:
                    wvc->rainFlagBits=7;
                    break;
                default:
                    fprintf(stderr,"Error:ReadFlagFile:Bad Flag\n");
                    exit(1);
                }
            }
        }
    }
    free(flag);
    free(flag_value);
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

//------------------------------//
// WindSwath::DeleteFlaggedData //
//------------------------------//

int
WindSwath::DeleteFlaggedData()
{
    int count = 0;
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL)
                continue;
            if ((RAIN_FLAG_UNUSABLE | RAIN_FLAG_RAIN) & wvc->rainFlagBits)
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

//------------------------------//
// WindSwath::DeleteFlaggedData //
//------------------------------//

int
WindSwath::DeleteFlaggedData(
    const char*  flag_file,
    int          use_thresh,
    float        threshold_both,
    float        threshold_outer)
{
    if (strcasecmp(flag_file, "l2b") == 0)
    {
        if (use_thresh != 0)
        {
            fprintf(stderr,
           "DeleteFlaggedData using threshold in L2B file not implemented!\n");
            exit(0);
        }
        return(DeleteFlaggedData());
    }
    FILE* ifp = fopen(flag_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "Fatal Error: Flag file %s cannot be opened\n",
            flag_file);
        exit(1);
    }
    unsigned int size = _alongTrackBins * _crossTrackBins;
    float* flag_value = (float*)malloc(sizeof(float) * size);
    char* flag = (char*)malloc(sizeof(char) * size);
    if (flag_value == NULL || flag == NULL)
    {
        fprintf(stderr,
            "Fatal Error: Error allocating memory for flag_value\n");
        exit(1);
    }

    if ((fread(flag_value, sizeof(float), size, ifp) != size) ||
        (fread(flag, sizeof(char), size, ifp) != size))
    {
        fprintf(stderr, "Fatal Error: Flag file %s cannot be read\n",
            flag_file);
        exit(1);
    }
    fclose(ifp);

    int count = 0;
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL)
                continue;

            int offset = j * _crossTrackBins + i;
            int not_classified = 0;
            int rain_bit_set = 0;
            float threshold = threshold_both;

            // outer beam only case
            if (flag[offset] >= 3)
                threshold = threshold_outer;
            if (flag[offset] == 2 || flag[offset] >= 5)
                not_classified = 1;
            if (use_thresh == 0 && (flag[offset] == 1 || flag[offset] == 4))
                rain_bit_set = 1;
            if ((use_thresh && (flag_value[offset] > threshold)) ||
                not_classified || rain_bit_set)
            {
                delete wvc;
                *(*(swath + i) + j) = NULL;
                count++;
                _validCells--;
            }
        }
    }

    free(flag_value);
    free(flag);
    return(count);
}

//-----------------------------------//
// WindSwath::DeleteLatitudesOutside //
//-----------------------------------//

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

//------------------------------------//
// WindSwath::DeleteLongitudesOutside //
//------------------------------------//

int
WindSwath::DeleteLongitudesOutside(
    float  low_lon,
    float  high_lon)
{
    int count = 0;
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL)
                continue;

            float lon = wvc->lonLat.longitude;
            if (lon < low_lon || lon > high_lon)
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

//------------------------------------//
// WindSwath::DeleteDirectionOutliers //
//------------------------------------//

int
WindSwath::DeleteDirectionOutliers(
    float       max_dir_err,
    WindField*  truth)
{
    int count = 0;
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (!wvc  || !(wvc->selected))
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            double dir_err = ANGDIF(wvc->selected->dir, true_wv.dir);
            if (dir_err > max_dir_err)
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

//--------------------------------//
// WindSwath::DeleteSpeedOutliers //
//--------------------------------//

int
WindSwath::DeleteSpeedOutliers(
    float       max_spd_err,
    WindField*  truth)
{
    int count = 0;
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (!wvc  || !(wvc->selected))
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            double spd_err = fabs(wvc->selected->spd - true_wv.spd);
            if (spd_err > max_spd_err)
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
    FILE*  fp)
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
    FILE*  fp)
{
    DeleteEntireSwath();    // in case

    //printf("In WindSwath::ReadL2B, L2B version ID: %d.%d\n",
    //       version_id_major, version_id_minor);

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
        
        // AGF added 6/3/2010
        // call method depending on version ID
        if( version_id_major == 1 )
        {
          if (! wvc->ReadL2B(fp)) return(0);
        }
        else if( version_id_major == 2 )
        {
          if (! wvc->ReadL2B_v2(fp)) return(0);
        }
        else if( version_id_major == 3 )
        {
          if (! wvc->ReadL2B_v3(fp)) return(0); // new version BWS 11/29/2010
        }
        else if( version_id_major == 4 )
        {
          if (! wvc->ReadL2B_v4(fp)) return(0); // new version TAW 03/02/2011
        }
        else if( version_id_major == 5 )
        {
          if (! wvc->ReadL2B_v5(fp)) return(0); // new version TAW 03/18/2011
        }
        else if( version_id_major == 6 )
        {
          if (! wvc->ReadL2B_v6(fp)) return(0); // new version TAW 03/18/2011
        }
        else
        {
          fprintf(stderr, "Unknown L2B version ID: %d.%d\n",
                  version_id_major, version_id_minor);
          return(0);
        }
        // End of 6/3/2010 agf mods.
        
        
        *(*(swath + cti) + ati) = wvc;
    }
    return(1);
}

//--------------------//
// WindSwath::ReadL2B //
//--------------------//

int
WindSwath::ReadL2B(
    const char*  filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    if (! ReadL2B(fp))
        return(0);

    fclose(fp);
    return(1);
}

//----------------------------//
// WindSwath::GetSpdDirNumSel //
//----------------------------//

int
WindSwath::GetSpdDirNumSel(
    float**  spd,
    float**  dir,
    int**    num_ambig,
    int**    selected)
{
    for (int j = 0; j < _crossTrackBins; j++)
    {
        for (int i = 0; i < _alongTrackBins; i++)
        {
            WVC* wvc = swath[j][i];
            if (! wvc)
            {
                num_ambig[i][j] = 0;
            }
            else
            {
                int k = 0;
                int idx = 1;
                num_ambig[i][j] = wvc->ambiguities.NodeCount();
                if (num_ambig[i][j] > HDF_NUM_AMBIGUITIES)
                    num_ambig[i][j] = HDF_NUM_AMBIGUITIES;
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    spd[i][j*HDF_NUM_AMBIGUITIES+k] = wvp->spd;
                    dir[i][j*HDF_NUM_AMBIGUITIES+k] = wvp->dir;
                    if (wvp == wvc->selected)
                        selected[i][j] = idx;
                    k++;
                    idx++;
                }
            }
        }
    }
    return(1);
}

//----------------------//
// WindSwath::UpdateHdf //
//----------------------//

int WindSwath::UpdateHdf(
   const char*  filename,
   float **     spd,
   float **     dir,
   int **       num_ambigs,
   int **       selected)
{
    //  Fix Directions
    printf("%d %d\n", _alongTrackBins, _crossTrackBins);
    for (int i = 0; i < _alongTrackBins; i++)
    {
        for (int j = 0; j < _crossTrackBins * HDF_NUM_AMBIGUITIES; j++)
        {
            dir[i][j] = (450.0 - rtd * dir[i][j]);
            if (dir[i][j] > 360.0)
                dir[i][j] -= 360.0;
        }
    }

    // Update DataSets
    for (int32 i = 0; i < _alongTrackBins; i++)
    {
        if (! UpdateDataSet(filename, "wind_speed", i, &spd[i][0]))
            return(0);
        if (! UpdateDataSet(filename, "wind_dir", i, &dir[i][0]))
            return(0);
        if (! UpdateDataSet(filename, "num_ambigs", i, &(num_ambigs[i][0])))
            return(0);
        if (! UpdateDataSet(filename, "wvc_selection", i, &(selected[i][0])))
            return(0);
    }
    return(1);
}

//---------------------------//
// WindSwath::ReadNscatSwv25 //
//---------------------------//

#define RECL  460

int
WindSwath::ReadNscatSwv25(
    const char*  filename)
{
    DeleteEntireSwath();    // in case

    //---------------//
    // open the file //
    //---------------//

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "WindSwath::ReadNscatSwv25: error opening file %s\n",
            filename);
        return(0);
    }

    //-----------------//
    // read the header //
    //-----------------//

    char buffer[RECL];
    int num_hdr_recs = 0;
    int num_output_recs = 0;
    char* ptr = NULL;

    if (fread(buffer, 1, RECL, ifp) != RECL)
    {
        fprintf(stderr, "WindSwath::ReadNscatSwv25: error reading header\n");
        goto close_and_fail;
    }

    ptr = strchr(buffer, '=');
    if (ptr == NULL)
    {
        fprintf(stderr,
            "WindSwath::ReadNscatSwv25: error finding = in first line\n");
        goto close_and_fail;
    }

    if (sscanf(ptr + 1, " %d", &num_hdr_recs) != 1)
    {
        fprintf(stderr,
            "WindSwath::ReadNscatSwv25: error determining record count\n");
        goto close_and_fail;
    }

    for (int i = 1; i < num_hdr_recs; i++)
    {
        ptr = strstr(buffer, "Num_Actual_Output_Records");
        if (ptr != NULL)
        {
            if (sscanf(ptr + strlen("Num_Actual_Output_Records"), " = %d",
                &num_output_recs) != 1)
            {
                fprintf(stderr, "WindSwath::ReadNscatSwv25:");
                fprintf(stderr, " error determining output record count\n");
                goto close_and_fail;
             }
        }

        if (fread(buffer, 1, RECL, ifp) != RECL)
        {
            fprintf(stderr,
                "WindSwath::ReadNscatSwv25: error reading header record\n");
            goto close_and_fail;
        }
    }

    //----------//
    // allocate //
    //----------//

    _crossTrackBins = 24 + 16 + 24;    // left + nadir gap + right
    _alongTrackBins = num_output_recs;
    _validCells = 0;
    _Allocate();

    //--------------//
    // read records //
    //--------------//

    for (int rec_idx = 0; rec_idx < num_output_recs; rec_idx++)
    {
        int ati = rec_idx;

        if (fread(buffer, 1, RECL, ifp) != RECL)
        {
            if (feof(ifp))
                break;

            fprintf(stderr,
                "WindSwath::ReadNscatSwv25: error reading data record %d\n",
                rec_idx);
            goto close_and_fail;
        }

        short wvc_lat[48];
        unsigned short wvc_lon[48];
        unsigned char wvc_flag[48];
        short wvc_spd[48];
        short wvc_dir[48];

        memcpy(wvc_lat, &(buffer[28]), 96);
        memcpy(wvc_lon, &(buffer[124]), 96);
        memcpy(wvc_flag, &(buffer[220]), 48);
        memcpy(wvc_spd, &(buffer[268]), 96);
        memcpy(wvc_dir, &(buffer[364]), 96);

        for (int wvc_idx = 0; wvc_idx < 48; wvc_idx++)
        {
            if (wvc_flag[wvc_idx] > 3)
                continue;

            // this is a hack and should not be here
            if (wvc_lat[wvc_idx] == 0 && wvc_lon[wvc_idx] == 0)
                continue;

            // convert cross track index in file to real cross track index
            int cti = wvc_idx;
            if (cti > 23)
                cti += 16;

            WVC* wvc = new WVC();
            if (wvc == NULL)
            {
                fprintf(stderr,
                    "WindSwath::ReadNscatSwv25: error creaing WVC\n");
                goto close_and_fail;
            }

            wvc->lonLat.Set(wvc_lon[wvc_idx] * 0.01 * dtr,
                wvc_lat[wvc_idx] * 0.01 * dtr);

            WindVectorPlus* wvp = new WindVectorPlus;
            if (wvp == NULL)
            {
                fprintf(stderr,
                    "WindSwath::ReadNscatSwv25: error creaing wvp\n");
                goto close_and_fail;
            }
            wvp->spd = wvc_spd[wvc_idx] * 0.01;
            float angle = wvc_dir[wvc_idx] * 0.01 * dtr;
            wvp->dir = CWNTOCCWE(angle);

            wvc->ambiguities.Append(wvp);
            wvc->selected = wvp;

            *(*(swath + cti) + ati) = wvc;
        }
    }

    fclose(ifp);
    return(1);

    //----------------//
    // close and fail //
    //----------------//

    close_and_fail:

    fclose(ifp);
    return(0);
}

//----------------------//
// WindSwath::WriteVctr //
//----------------------//

int
WindSwath::WriteVctr(
    const char*        filename,
    const int        rank)        // 0 = selected
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
    const char*        filename)
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

//-----------------------//
// WindSwath::WriteAscii //
//-----------------------//

int
WindSwath::WriteAscii(
    const char*        filename)
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

//-----------------------//
// WindSwath::WriteAscii //
//-----------------------//

int
WindSwath::WriteAscii(
    FILE*  fp)
{
    //-------------//
    // write ASCII //
    //-------------//

    fprintf(fp, "Total Along Track Bins: %d\nTotal Cross Track Bins: %d\n",
        _alongTrackBins, _crossTrackBins);
    for (int ati = 0; ati < _alongTrackBins; ati++)
    {
        for (int cti = 0; cti < _crossTrackBins; cti++)
        {
            fprintf(fp, "Along Track Bin: %d, Cross Track Bin: %d\n", ati,
                cti);
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
    int  rank)
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

//--------------------------//
// WindSwath::InitWithNudge //
//--------------------------//

int
WindSwath::InitWithNudge()
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;
            if (wvc->ambiguities.NodeCount() == 1)
            {
                wvc->selected = wvc->ambiguities.GetByIndex(0);
            }
            else
            {
                wvc->selected = wvc->GetNearestToDirection(wvc->nudgeWV->dir);
            }
            count++;
        }
    }
    return(count);
}

//-----------------------//
// WindSwath::InitRandom //
//-----------------------//

int
WindSwath::InitRandom()
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

                        // get random rank
            int nc=wvc->ambiguities.NodeCount();
            Uniform rand(nc/2.0,nc/2.0);
            int idx=(int)rand.GetNumber();

                        // account for possible floating point inaccuracies.
            if(idx<0) idx=0;
                        if(idx>=nc) idx=nc-1;

            wvc->selected = wvc->ambiguities.GetByIndex(idx);
            count++;
        }
    }
    return(count);
}

//----------------------//
// WindSwath::HideSpeed //
//----------------------//

int
WindSwath::HideSpeed(
    float  min_speed,
    float  max_speed)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            WindVectorPlus* wvp = wvc->selected;
            if (wvp->spd > min_speed && wvp->spd < max_speed)
            {
                wvc->selected = NULL;
                count++;
            }
        }
    }
    return(count);
}

//----------------------------//
// WindSwath::GetNudgeVectors //
//----------------------------//

int
WindSwath::GetNudgeVectors(
    WindField*  nudge_field)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;
            /*** Not sure why we have this check
	    /*** Commenting out for now
            /*** May effect coastal retrieval or cause bug
            /*** in HurrSp1 retrieval
            if (! wvc->ambiguities.NodeCount())
	      continue;
	    ***/
            wvc->nudgeWV = new WindVectorPlus;
            if (! nudge_field->InterpolatedWindVector(wvc->lonLat,
						       wvc->nudgeWV))
            {
                delete wvc->nudgeWV;
                wvc->nudgeWV=NULL;
            }
        }
    }
    nudgeVectorsRead = 1;
    return(1);
}


int
WindSwath::GetNudgeVectors(float** spd, float** dir, int nati, int ncti)
{
  if(ncti!=_crossTrackBins){
    fprintf(stderr,"GetNudgeVectors: mismatch between nudge array and swath size\n");
    exit(1);
  }
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;
            /*** Not sure why we have this check
	    /*** Commenting out for now
            /*** May effect coastal retrieval or cause bug
            /*** in HurrSp1 retrieval
            if (! wvc->ambiguities.NodeCount())
	      continue;
	    ***/
            wvc->nudgeWV = new WindVectorPlus;
	    if(ati>=nati){
	      delete wvc->nudgeWV;
	      wvc->nudgeWV=NULL;
	    }
	    float nspd=spd[ati][cti];
	    if(nspd<0){
	      delete wvc->nudgeWV;
	      wvc->nudgeWV=NULL;
	    }
	    else{
	      wvc->nudgeWV->spd=nspd;
	      wvc->nudgeWV->dir=dir[ati][cti];
	    }
        }
    }
    nudgeVectorsRead = 1;
    return(1);
}



int
WindSwath::GetNudgeVectors(
    WindVectorField*  nudge_field)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;
            wvc->nudgeWV = new WindVectorPlus;
            if (! nudge_field->InterpolateVectorField(wvc->lonLat,
						      wvc->nudgeWV,0))
            {
                delete wvc->nudgeWV;
                wvc->nudgeWV=NULL;
            }
        }
    }
    nudgeVectorsRead = 1;
    return(1);
}

//----------------------------//
// WindSwath::GetNudgeVectors //
//----------------------------//

int
WindSwath::GetHurricaneNudgeVectors(
    WindField*      nudge_field,
    EarthPosition*  center,
    float           radius)
{
    if (! nudgeVectorsRead)
    {
        fprintf(stderr, "WindSwath::GetHurricaneNudgeVectors failed 1\n");
        exit(0);
    }
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            EarthPosition cell_pos;

            cell_pos.SetAltLonGDLat(0.0, wvc->lonLat.longitude,
                wvc->lonLat.latitude);
            if (center->SurfaceDistance(cell_pos) < radius)
            {
                if (! wvc->nudgeWV)
                    wvc->nudgeWV = new WindVectorPlus;
                if (! nudge_field->InterpolatedWindVector(wvc->lonLat,
                    wvc->nudgeWV))
                {
                    fprintf(stderr,
                        "WindSwath::GetHurricaneNudgeVectors failed\n");
                    exit(0);
                }
            }
        }
    }
    return(1);
}

//------------------//
// WindSwath::Nudge //
//------------------//

int
WindSwath::Nudge(
    int  max_rank)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

            if (wvc->nudgeWV==NULL)
                continue;

            wvc->selected = wvc->GetNearestToDirection(wvc->nudgeWV->dir,
                max_rank);
            
            // if max_rank >= wvc->numAmbiguities, then we used
            // all ambigs to nudge with.
            wvc->qualFlag = ( wvc->qualFlag & ~L2B_QUAL_FLAG_ALL_AMBIG ) | 
             (wvc->numAmbiguities < max_rank) * L2B_QUAL_FLAG_ALL_AMBIG;
            
            count++;
        }
    }
    return(count);
}

//------------------------//
// WindSwath::StreamNudge //
//------------------------//

int
WindSwath::StreamNudge(
    float  stream_thresh)
{
    printf("WindSwath::StreamT %g\n", stream_thresh);
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

            if (wvc->nudgeWV == NULL)
                continue;
            WindVectorPlus* wvp1 = wvc->ambiguities.GetHead();
            WindVectorPlus* wvp2 = wvc->ambiguities.GetNext();
            if (! wvp2)
                wvc->selected = wvp1;
            else
            {
                float angdif = fabs(ANGDIF(wvp1->dir, wvp2->dir));
                if (angdif < stream_thresh * dtr)
                    wvc->selected = wvp1;
                else
                {
                    wvc->selected =
                        wvc->GetNearestToDirection(wvc->nudgeWV->dir, 2);
                    count++;
                }
            }
        }
    }
    return(count);
}

//---------------------------//
// WindSwath::HurricaneNudge //
//---------------------------//

int
WindSwath::HurricaneNudge(
    int             min_rank,
    EarthPosition*  center,
    float           radius)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

            if (wvc->nudgeWV == NULL)
                continue;

            EarthPosition cell_pos;
            cell_pos.SetAltLonGDLat(0.0, wvc->lonLat.longitude,
                wvc->lonLat.latitude);
            if (center->SurfaceDistance(cell_pos) < radius)
            {
                if (! wvc->nudgeWV)
                {
                    fprintf(stderr, "WindSwath::NudgeHurricane failed\n");
                    exit(0);
                }
                wvc->selected = wvc->GetNearestToDirection(wvc->nudgeWV->dir,
                    min_rank);
                count++;
            }
        }
    }
    return(count);
}

//--------------------//
// WindSwath::S3Nudge //
//--------------------//

int
WindSwath::S3Nudge()
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

            if (wvc->nudgeWV==NULL)
                continue;

            wvc->selected = wvc->GetNearestRangeToDirection(wvc->nudgeWV->dir);
            count++;
        }
    }
    return(count);
}


//-----------------------//
// WindSwath::ThresNudge //
//-----------------------//

int
WindSwath::ThresNudge(
    int    max_rank,
    float  thres[2])
{   
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

            // threshold objective function //

            int rank_idx = 0;

            int w = 0;
            if (cti < 9 || cti > 71) { w = 1; }
              else { w = 0; }

            WindVectorPlus* head = wvc->ambiguities.GetHead();

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead();
                 wvp; wvp = wvc->ambiguities.GetNext()) {
              if ( exp(0.5*(wvp->obj - head->obj)) >= thres[w] )
                rank_idx++;
            }

            /* If all the ambiguities contributed to nudging, note it */
            wvc->qualFlag = ( wvc->qualFlag & ~L2B_QUAL_FLAG_ALL_AMBIG ) | 
             (rank_idx == wvc->numAmbiguities) * L2B_QUAL_FLAG_ALL_AMBIG;

            WindVector nudge_wv;
            if (wvc->nudgeWV==NULL)
              continue;

            wvc->selected = wvc->GetNearestToDirection(wvc->nudgeWV->dir, rank_idx);
            count++;            
        }
    }
    return(count);
}

//-----------------------//
// WindSwath::LoResNudge //
//-----------------------//

int
WindSwath::LoResNudge(
    WindVectorField*   nudge_field,
    int                max_rank)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
      {
        for (int ati = 0; ati < _alongTrackBins; ati++)
          {
            WVC* wvc = swath[cti][ati];
        if (! wvc)
          continue;

        wvc->numAmbiguities = wvc->ambiguities.NodeCount();

        WindVector nudge_wv;

        int idx = (ati * _crossTrackBins + cti)/2;

        if (! nudge_field->InterpolateVectorField(wvc->lonLat, &nudge_wv, idx) )
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

            wvc->numAmbiguities = wvc->ambiguities.NodeCount();

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

//-------------------------------//
// WindSwath::MedianFilter_4Pass //
//-------------------------------//
// Returns the number of passes.
// If Special=0 Standard Median Filter (default)
// if Special=1 Use Range Information  (S3)
// If Special=2 Use Spatial Probability Maximization (S4)

int
WindSwath::MedianFilter_4Pass(
    int  window_size,
    int  max_passes,
    int  bound,
    int  weight_flag,
    int  special,
    int  freeze)
{
    //----------------------------//
    // create a new selection map //
    //----------------------------//

    WindVectorPlus*** new_selected =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2,
        _crossTrackBins, _alongTrackBins);

    //--------------------------------------------------------------------------//
    // create arrays to flag filtering / influence during median filter passes //
    //-------------------------------------------------------------------------//
    char** change    = (char**)make_array(sizeof(char), 2, _crossTrackBins,
        _alongTrackBins);
        
    char** filter    = (char**)make_array(sizeof(char), 2, _crossTrackBins,
        _alongTrackBins);
        
    char** influence = (char**)make_array(sizeof(char), 2, _crossTrackBins,
        _alongTrackBins);        
    
    
    int half_window = window_size / 2;

    int total_passes = 0;
    
    printf("bound: %d\n",bound);
    
    for( int med_filt_iter_cntr = 0; med_filt_iter_cntr < 4; ++med_filt_iter_cntr)
    {
      for (int cti = 0; cti < _crossTrackBins; cti++)
      {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
          WVC* wvc = swath[cti][ati];
          
          if ( ! wvc )  // Trap cells without a wind retrival.
          { 
            filter[cti][ati]    = 0;
            influence[cti][ati] = 0;
            change[cti][ati]    = 0;
            continue;
          }

          // coast || ice kept at no filter, no influence until iter 3
          if( wvc->landiceFlagBits & (LAND_ICE_FLAG_ICE+LAND_ICE_FLAG_COAST) )
          {
            switch(med_filt_iter_cntr)
            {
              case 0:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 0;
                change[cti][ati]    = 0;
                break;              
              case 1:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 0;
                change[cti][ati]    = 0;
                break;              
              case 2:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 0;
                change[cti][ati]    = 0;
                break;
              case 3:
                filter[cti][ati]    = 1;
                influence[cti][ati] = 1;
                change[cti][ati]    = 1;
                break;
              default:
                 printf("In WindSwath::MedianFilter_4Pass, we shouldn't be here!!!\n");
                break;
            }
          }
          // rain && ! ice && ! coast
          else if( (!(wvc->rainFlagBits & RAIN_FLAG_UNUSABLE)) &&  
                (wvc->rainFlagBits & RAIN_FLAG_RAIN) )
          {
            switch(med_filt_iter_cntr)
            {
              case 0:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 0;   
                change[cti][ati]    = 0;
                break;                
              case 1:
                filter[cti][ati]    = 1;
                influence[cti][ati] = 1;
                change[cti][ati]    = 1;
                break;                
              case 2:
                filter[cti][ati]    = 1;
                influence[cti][ati] = 1;
                change[cti][ati]    = 1;
                break;                
              case 3:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 1;
                change[cti][ati]    = 0;
                break;
              default:
                printf("In WindSwath::MedianFilter_4Pass, we shouldn't be here!!!\n");
                break;
            }
          }
          else // rain_free && ! ice && ! coast
          {
            switch(med_filt_iter_cntr)
            {
              case 0:
                filter[cti][ati]    = 1;
                influence[cti][ati] = 1;
                change[cti][ati]    = 1;
                break;                
              case 1:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 1;
                change[cti][ati]    = 0;
                break;                
              case 2:
                filter[cti][ati]    = 1;
                influence[cti][ati] = 1;              
                change[cti][ati]    = 1;
                break;                
              case 3:
                filter[cti][ati]    = 0;
                influence[cti][ati] = 1;
                change[cti][ati]    = 0;
                break;
              default:
                printf("In WindSwath::MedianFilter_4Pass, we shouldn't be here!!!\n");
                break;
            }
          }
        }
      }
      //--------//
      // filter //
      //--------//
    
      int pass = 0;
      while (pass < max_passes)
      {
          int flips = MedianFilter4Pass_Pass( half_window, new_selected, change, 
                    filter, influence, bound, weight_flag );
          pass++;
          if (flips == 0)
              break;
      }      
      
      printf("On median filter pass: %d, number of times filter applied: %d\n",
        med_filt_iter_cntr,pass);
      
      total_passes = total_passes + pass;
      
    }

    free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
    free_array(change,       2, _crossTrackBins, _alongTrackBins);
    free_array(filter,       2, _crossTrackBins, _alongTrackBins);
    free_array(influence,    2, _crossTrackBins, _alongTrackBins);
    
    return(total_passes);
}



//-------------------------//
// WindSwath::MedianFilter //
//-------------------------//
// Returns the number of passes.
// If Special=0 Standard Median Filter (default)
// if Special=1 Use Range Information  (S3)
// If Special=2 Use Spatial Probability Maximization (S4)

int
WindSwath::MedianFilter(
    int  window_size,
    int  max_passes,
    int  bound,
    int  weight_flag,
    int  special,
    int  freeze)
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

    char** change = (char**)make_array(sizeof(char), 2, _crossTrackBins,
        _alongTrackBins);

    //--------------------//
    // prep for filtering //
    //--------------------//

    int half_window = window_size / 2;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            if (freeze == 0 || cti < freeze || cti > _crossTrackBins - freeze)
                change[cti][ati] = 1;
            else
                change[cti][ati] =0;
        }
    }

    //--------//
    // filter //
    //--------//

    int pass = 0;
    while (pass < max_passes)
    {
        int flips = MedianFilterPass(half_window, new_selected, change,
                        bound, weight_flag, special, freeze);
        pass++;
        if (flips == 0)
            break;
    }

    free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
    free_array(change, 2, _crossTrackBins, _alongTrackBins);
    return(pass);
}

#define BK_NUM_PASS 200
#define BK_NUDGE 0
#define NUDGE_DEVIATION 1.0
//-------------------------//
// WindSwath::BestKFilter  //
//-------------------------//

int
WindSwath::BestKFilter(
    int  window_size,
    int  k)
{
    //----------------------------//
    // create a new selection map //
    //----------------------------//

    WindVectorPlus*** new_selected =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2,
            _crossTrackBins, _alongTrackBins);

    //-------------------------//
    // create a probability map//
    //-------------------------//

    float** prob = (float**)make_array(sizeof(float), 2,
        _crossTrackBins, _alongTrackBins);

        //-------------------------//
        // create best prob array  //
        //-------------------------//
        float* best_prob=new float[k];

    //--------------------//
    // prep for filtering //
    //--------------------//

    int half_window = window_size / 2;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            if(swath[cti][ati]){
              swath[cti][ati]->selected = NULL;
              if(!swath[cti][ati]->RedistributeObjs())
                fprintf(stderr,"BestKFilter RedistributeObjs failed.\n");
            }
            new_selected[cti][ati]=NULL;

        }
    }

    //--------//
    // filter //
    //--------//

    int finished=0;
        int pass=0;
    while (!finished && pass<BK_NUM_PASS)
    {
        pass++;
        printf("Pass=%d ",pass);
        finished = BestKFilterPass(half_window,k,new_selected, prob,
                        best_prob);
    }
        int count=0;
        // Remove unselected WVCs and and count them
        for(int cti=0;cti<_crossTrackBins;cti++){
      for(int ati=0; ati<_alongTrackBins;ati++){
        if(swath[cti][ati]==NULL) continue;
            if(swath[cti][ati]->selected==NULL){
          count++;
          WVC* wvc=Remove(cti,ati);
              delete wvc;
        }
      }
    }
        printf("Unselected count=%d\n",count);
    free_array(new_selected, 2, _crossTrackBins, _alongTrackBins);
    free_array(prob, 2, _crossTrackBins, _alongTrackBins);
        delete[] best_prob;
    return(1);
}

//-----------------------------//
// WindSwath::MedianFilterPass //
//-----------------------------//
// Returns the number of vector changes.

int    g_number_needed = 0;
float  g_speed_stopper = 0.0;
float  g_error_ratio_of_best = 1.0;
float  g_error_of_best = 1.0;    // ratio to speed
float  g_rain_flag_threshold = 1.0;
int    g_rain_bit_flag_on = 0;
int**  g_freeze_array = NULL;

int
WindSwath::MedianFilterPass(
    int                half_window,
    WindVectorPlus***  new_selected,
    char**             change,
    int                bound,
    int                weight_flag,
    int                special,
    int                freeze)
{
    int flips = 0;
    float energy = 0.0;

    //-------------//
    // filter loop //
    //-------------//

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        int cti_min = cti - half_window;
        int cti_max = cti + half_window + 1;
        if (cti_min < bound)
            cti_min = bound;
        if (cti_max > _crossTrackBins-bound)
            cti_max = _crossTrackBins-bound;
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
            // check for freeze  //
            // state             //
            //-------------------//
            if (g_freeze_array != NULL)
            {
                if (g_freeze_array[cti][ati] == 1)
                    continue;
            }
            if (freeze != 0 & cti >= freeze & cti <= _crossTrackBins - freeze)
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
            continue;        // no changes

        change:

            float min_vector_dif_sum = (float)HUGE_VAL;
            float min_vector_dif_avg = (float)HUGE_VAL;
            float second_vector_dif_sum = (float)HUGE_VAL;
            int   selected_count = 0;

            //--------------------------------------------//
            // Don't use range information if unavailable //
            // or special= 0                              //
            //--------------------------------------------//

            if (special == 0 ||
                (special == 1 && wvc->directionRanges.NodeCount() == 0))
            {
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    float vector_dif_sum = 0.0;
                    float x1 = wvp->spd * cos(wvp->dir);
                    float y1 = wvp->spd * sin(wvp->dir);

                    selected_count = 0;
                    int available_count = 0;
                    for (int i = cti_min; i < cti_max; i++)
                    {
                        for (int j = ati_min; j < ati_max; j++)
                        {
                            if (i == cti && j == ati)
                                continue;        // don't check central vector

                            WVC* other_wvc = swath[i][j];
                            if (! other_wvc)
                                continue;

                            if ( other_wvc->rainProb > g_rain_flag_threshold)
                                continue;
                            if ( g_rain_bit_flag_on &&
                                ((RAIN_FLAG_UNUSABLE | RAIN_FLAG_RAIN) &
                                other_wvc->rainFlagBits))
                            {
                                continue;
                            }
                            available_count++;    // other wvc exists

                            WindVectorPlus* other_wvp = other_wvc->selected;
                            if (! other_wvp)
                                continue;

                            // special purpose speed threshold
                            if (other_wvp->spd < g_speed_stopper)
                                continue;

                            selected_count++;   // wvc has a valid selection

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

                    if (vector_dif_sum < min_vector_dif_sum &&
                        selected_count >= g_number_needed)
                    {
                        second_vector_dif_sum = min_vector_dif_sum;
                        min_vector_dif_sum = vector_dif_sum;
                        min_vector_dif_avg = vector_dif_sum /
                            (float)selected_count;
                        new_selected[cti][ati] = wvp;
                    }
                    else if (vector_dif_sum < second_vector_dif_sum)
                    {
                        second_vector_dif_sum = vector_dif_sum;
                    }
                }   // done with ambiguities

                // a few propagation checks
                if (wvc->selected == NULL)
                {
                    if (wvc->rainProb > g_rain_flag_threshold)
                        new_selected[cti][ati] = NULL;
                    if (g_rain_bit_flag_on &&
                        ((RAIN_FLAG_UNUSABLE | RAIN_FLAG_RAIN) &
                        wvc->rainFlagBits))
                    {
                        new_selected[cti][ati]=NULL;
                    }

                    // how must does the best beat the second best?
                    if (second_vector_dif_sum > 0.0 &&
                        g_error_ratio_of_best < 1.0)
                    {
                        float ratio = min_vector_dif_sum /
                            second_vector_dif_sum;
                        if (ratio > g_error_ratio_of_best)
                        {
                            new_selected[cti][ati] = NULL;
                        }
                    }
                    // how absolutely good is the best?
                    if (new_selected[cti][ati] != NULL &&
                        g_error_of_best < 1.0)
                    {
                        float avg_vector_dif = min_vector_dif_avg /
                            (float)selected_count;
                        float dif_to_spd = avg_vector_dif /
                            new_selected[cti][ati]->spd;
                        if (dif_to_spd > g_error_of_best)
                            new_selected[cti][ati] = NULL;
                    }
                }
            }
            else if (special == 1)
            {
                //-----------------------------------------------//
                // Special Cases: Use Range Info (S3)            //
                //-----------------------------------------------//

                WindVectorPlus* wvp = new WindVectorPlus;
                // Determine the Median Vector
                if (S3_USE_MEDIAN_FOR_RANGE)
                {
                    if (! GetMedianBySorting(wvp, cti_min, cti_max,
                        ati_min, ati_max))
                    {
                        return(0);
                    }
                }
                else
                {
                    if (! GetWindowMean(wvp, cti_min, cti_max,
                        ati_min, ati_max))
                    {
                        return(0);
                    }
                }
                if (S3_USE_CLOSEST_VECTOR)
                {
                    if (! wvc->directionRanges.GetNearestVector(wvp))
                        return(0);
                }
                else
                {
                    float tmp = wvc->directionRanges.GetNearestValue(wvp->dir);
                    wvp->dir = tmp;
                    wvp->spd = wvc->directionRanges.GetBestSpeed(tmp);
                    wvp->obj = wvc->directionRanges.GetBestObj(tmp);
                }
                new_selected[cti][ati] = wvp;
            }  // Done with special==1 procedure
            else if (special == 2)
            {
                // Spatial Probability Search (special==2)
                WindVectorPlus* wvp = new WindVectorPlus;
                energy += GetMostProbableDir(wvp, cti, ati, cti_min,
                    cti_max, ati_min, ati_max);
                new_selected[cti][ati] = wvp;
            }
            else
            {
                fprintf(stderr, "MedianFilter: Bad Special Value %d\n",
                    special);
                exit(1);
            }
        }    // done with ati
    }    // done with cti

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
                //-----------------------------------------------//
                // Special Procedure if special==1 or special==2 //
                // used                                          //
                //-----------------------------------------------//

                if ((special == 1 &&
                    swath[cti][ati]->directionRanges.NodeCount() != 0) ||
                    special == 2)
                {
                    // replace selected
                    // but set change only if the direction
                    // changes substantially

                    if (swath[cti][ati]->selected_allocated)
                    {
                        double dif = ANGDIF(swath[cti][ati]->selected->dir,
                            new_selected[cti][ati]->dir);
                        if (dif > FLIPPING_WITHIN_RANGE_THRESHOLD)
                            change[cti][ati]=1;
                        else
                            change[cti][ati]=0;

                        delete swath[cti][ati]->selected;
                    }
                    else
                    {
                        swath[cti][ati]->selected_allocated = 1;
                        change[cti][ati] = 1;
                    }
                    swath[cti][ati]->selected = new_selected[cti][ati];
                    flips += change[cti][ati];
                }
                else if (new_selected[cti][ati] != swath[cti][ati]->selected)
                {
                    //------------------------//
                    // IF RANGE INFO NOT USED //
                    //------------------------//

                    if (new_selected[cti][ati]->spd < g_speed_stopper)
                    {
                        continue;
                    }

                    change[cti][ati] = 1;
                    swath[cti][ati]->selected = new_selected[cti][ati];
                    flips += change[cti][ati];
                }
            }
        }
    }
    printf("Flips %d  Energy %g\n", flips, energy);
    fflush(stdout);
    return(flips);
}


//-----------------------------------//
// WindSwath::MedianFilterPass_4Pass //
//-----------------------------------//
// Returns the number of vector changes.


int
WindSwath::MedianFilter4Pass_Pass(
    int                half_window,
    WindVectorPlus***  new_selected,
    char**             change,
    char**             filter,
    char**             influence,
    int                bound,
    int                weight_flag )
{
    int flips = 0;
    float energy = 0.0;

    //-------------//
    // filter loop //
    //-------------//
        
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        int cti_min = cti - half_window;
        int cti_max = cti + half_window + 1;
        if (cti_min < bound)
            cti_min = bound;
        if (cti_max > _crossTrackBins-bound)
            cti_max = _crossTrackBins-bound;
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

            if ( ! wvc )
                continue;  // goto next cell
                
            if( ! filter[cti][ati] )
                continue;  // goto next cell
            
            //------------------------------------------------------//
            // check for changes & influences in window around cell //
            //------------------------------------------------------//
            
            int sum_change = 0;
            int sum_influence = 0;
            
            for (int i = cti_min; i < cti_max; i++)
            {
                for (int j = ati_min; j < ati_max; j++)
                {
                    sum_change    += change[i][j];
                    sum_influence += influence[i][j];
                }
            }
            
            if( sum_change == 0 || sum_influence == 0 )
                continue;
            
            float min_vector_dif_sum = (float)HUGE_VAL;
            float min_vector_dif_avg = (float)HUGE_VAL;
            float second_vector_dif_sum = (float)HUGE_VAL;
            int   selected_count = 0;

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                float vector_dif_sum = 0.0;
                float x1 = wvp->spd * cos(wvp->dir);
                float y1 = wvp->spd * sin(wvp->dir);

                selected_count = 0;
                int available_count = 0;
                for (int i = cti_min; i < cti_max; i++)
                {
                    for (int j = ati_min; j < ati_max; j++)
                    {
                        /*  The F90 official code DOES use the center... AGF 7/20/2009
                        if (i == cti && j == ati)
                            continue;        // don't check central vector
                        */
                        
                        WVC* other_wvc = swath[i][j];
                        
                        if( ! other_wvc )
                            continue;
                        
                        if( ! influence[i][j] )
                            continue;

                        available_count++;    // other wvc exists and will be used 
                                              // to compute filter.

                        WindVectorPlus* other_wvp = other_wvc->selected;
                        if (! other_wvp)
                            continue;

                        // special purpose speed threshold
                        if (other_wvp->spd < g_speed_stopper)
                            continue;

                        selected_count++;   // wvc has a valid selection

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

                if (vector_dif_sum < min_vector_dif_sum &&
                    selected_count >= g_number_needed)
                {
                    second_vector_dif_sum = min_vector_dif_sum;
                    min_vector_dif_sum = vector_dif_sum;
                    min_vector_dif_avg = vector_dif_sum /
                        (float)selected_count;
                    new_selected[cti][ati] = wvp;
                }
                else if (vector_dif_sum < second_vector_dif_sum)
                {
                    second_vector_dif_sum = vector_dif_sum;
                }
            }   // done with ambiguities

            // a few propagation checks
            if (wvc->selected == NULL)
            {
                if (wvc->rainProb > g_rain_flag_threshold)
                    new_selected[cti][ati] = NULL;
                if (g_rain_bit_flag_on &&
                    ((RAIN_FLAG_UNUSABLE | RAIN_FLAG_RAIN) &
                    wvc->rainFlagBits))
                {
                    new_selected[cti][ati]=NULL;
                }

                // how must does the best beat the second best?
                if (second_vector_dif_sum > 0.0 &&
                    g_error_ratio_of_best < 1.0)
                {
                    float ratio = min_vector_dif_sum /
                        second_vector_dif_sum;
                    if (ratio > g_error_ratio_of_best)
                    {
                        new_selected[cti][ati] = NULL;
                    }
                }
                // how absolutely good is the best?
                if (new_selected[cti][ati] != NULL &&
                    g_error_of_best < 1.0)
                {
                    float avg_vector_dif = min_vector_dif_avg /
                        (float)selected_count;
                    float dif_to_spd = avg_vector_dif /
                        new_selected[cti][ati]->spd;
                    if (dif_to_spd > g_error_of_best)
                        new_selected[cti][ati] = NULL;
                }
            }
        }    // done with ati
    }    // done with cti

    //------------------//
    // transfer updates //
    //------------------//
    
    
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            change[cti][ati] = 0;
            if ( new_selected[cti][ati] != NULL )
            {
                if (new_selected[cti][ati] != swath[cti][ati]->selected)
                {
                    if (new_selected[cti][ati]->spd < g_speed_stopper)
                    {
                        continue;
                    }

                    change[cti][ati] = 1;
                    swath[cti][ati]->selected = new_selected[cti][ati];
                    flips++;
                }
            }
        }
    }
    printf("Flips %d  Energy %g\n", flips, energy);
    fflush(stdout);
    return(flips);
}



//----------------------------//
// WindSwath::BestKFilterPass //
//----------------------------//
// Returns 1 if finished 0 otherwise.

#define NUM_BK_SUB_PASSES 100

int
WindSwath::BestKFilterPass(
    int                half_window,
    int                k,
    WindVectorPlus***  new_selected,
    float**            prob,
    float*             best_prob)
{
  // Initialize best probability array
  for(int c=0;c<k;c++) best_prob[c]=0.0;

  for (int cti = 0; cti < _crossTrackBins; cti++)
    {
      for (int ati = 0; ati < _alongTrackBins; ati++)
    {
      if(swath[cti][ati]==NULL) continue;
          if(swath[cti][ati]->selected!=NULL) continue;


      int cti_min=MAX(0,cti-half_window);
      int cti_max=MIN(_crossTrackBins-1,cti+half_window);
      int ati_min=MAX(0,ati-half_window);
      int ati_max=MIN(_alongTrackBins,ati+half_window);

          // Calculate probabilities
      prob[cti][ati]=GetMostProbableAmbiguity(&(new_selected[cti][ati]),
                          cti,ati,cti_min,cti_max,
                          ati_min,ati_max);

          // Update best probability array
      if(prob[cti][ati]>best_prob[0]){
        best_prob[0]=prob[cti][ati];
            int c=1;
            while(best_prob[c]<best_prob[c-1]){
          float tmp=best_prob[c];
              best_prob[c]=best_prob[c-1];
              best_prob[c-1]=tmp;
          c++;
              if(c==k)break;
        }
      }
    }
    }
   int finished=1;
   int num_added=0;
   static int num_wrong=0;
   for (int cti = 0; cti < _crossTrackBins; cti++)
    {
      for (int ati = 0; ati < _alongTrackBins; ati++)
    {
      if(swath[cti][ati]==NULL) continue;
          if(swath[cti][ati]->selected!=NULL) continue;

      int wrong=0;
          WindVectorPlus* closest=NULL;
      if(swath[cti][ati]->nudgeWV){
        closest=
          swath[cti][ati]->GetNearestToDirection(swath[cti][ati]->nudgeWV->dir);}

      if(closest && closest!=new_selected[cti][ati])
        wrong=1;
          num_added++;

          if(prob[cti][ati]!=1){
           if(prob[cti][ati]<best_prob[0]){
         num_added--;
         new_selected[cti][ati]=NULL;
         finished=0;
         wrong=0;
       }
      }
      num_wrong+=wrong;
      swath[cti][ati]->selected=new_selected[cti][ati];
    }
    }
   int subfinished=0;
   int subpass=0;
   printf("Minim Prob. %g Num Added %d Num Wrong %d\n",best_prob[0],num_added,num_wrong);

   //-------------------------//
   // create a new change map //
   //-------------------------//

   char** change = (char**)make_array(sizeof(char), 2,
                      _crossTrackBins, _alongTrackBins);

    //----------------------------//
    // prep for subpass filtering //
    //----------------------------//

   for (int cti = 0; cti < _crossTrackBins; cti++)
     {
       for (int ati = 0; ati < _alongTrackBins; ati++)
     {
       change[cti][ati] = 1;
     }
     }


   while(subpass<NUM_BK_SUB_PASSES && !subfinished){
     subpass++;
     printf("Subpass %d:",subpass);
     subfinished=BestKFilterSubPass(half_window, new_selected,
                    &num_wrong, change);
   }
   if(num_added==0) finished=1;
   free_array(change, 2, _crossTrackBins, _alongTrackBins);
   return(finished);
}

//-------------------------------//
// WindSwath::BestKFilterSubPass //
//-------------------------------//
// Returns 1 if finished 0 otherwise.

int
WindSwath::BestKFilterSubPass(
    int                half_window,
    WindVectorPlus***  new_selected,
    int*               num_wrong,
    char**             change)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            if (swath[cti][ati] == NULL)
                continue;
            if (swath[cti][ati]->selected == NULL)
                continue;

      int cti_min=MAX(0,cti-half_window);
      int cti_max=MIN(_crossTrackBins-1,cti+half_window);
      int ati_min=MAX(0,ati-half_window);
      int ati_max=MIN(_alongTrackBins,ati+half_window);

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
      continue;
          // Get Best Ambiguity
change:      GetMostProbableAmbiguity(&(new_selected[cti][ati]),
                          cti,ati,cti_min,cti_max,
                          ati_min,ati_max);

    }
    }
   int finished=1;
   int flips=0;
   for (int cti = 0; cti < _crossTrackBins; cti++)
    {
      for (int ati = 0; ati < _alongTrackBins; ati++)
    {
          change[cti][ati]=0;
      if(swath[cti][ati]==NULL) continue;
          if(swath[cti][ati]->selected==NULL) continue;
          if(new_selected[cti][ati]!=swath[cti][ati]->selected){
            int new_wrong=0, old_wrong=0;
        WindVectorPlus* closest=NULL;
        if(swath[cti][ati]->nudgeWV){
          closest=swath[cti][ati]->GetNearestToDirection(swath[cti][ati]->nudgeWV->dir);
        }
        if(closest && closest!=swath[cti][ati]->selected)
          old_wrong=1;
        if(closest && closest!=new_selected[cti][ati])
          new_wrong=1;
            *num_wrong+=(new_wrong-old_wrong);
        swath[cti][ati]->selected=new_selected[cti][ati];
        finished=0;
        flips++;
        change[cti][ati]=1;
      }


    }
    }
   printf(" Num_flipped %d Num_wrong %d\n",flips, *num_wrong);
   return(finished);
}

int WindSwath::GetNumCellsSelected(){

  int retval=0;
  for (int cti = 0; cti < _crossTrackBins; cti++){
    for (int ati = 0; ati < _alongTrackBins; ati++){
      WVC* wvc = swath[cti][ati];
      if(wvc){
    if(wvc->selected) retval++;
      }
    }
  }
  return(retval);
}

int WindSwath::GetNumCellsWithAmbiguities(){
  int retval=0;
  for (int cti = 0; cti < _crossTrackBins; cti++){
    for (int ati = 0; ati < _alongTrackBins; ati++){
      WVC* wvc = swath[cti][ati];
      if(wvc){
    if(wvc->ambiguities.NodeCount()) retval++;
      }
    }
  }
  return(retval);
}

//--------------------------//
// WindSwath::GetWindowMean //
//--------------------------//

int
WindSwath::GetWindowMean(
    WindVectorPlus*  wvp,
    int              cti_min,
    int              cti_max,
    int              ati_min,
    int              ati_max)
{
  double x=0,y=0;
  int count=0;
  for (int i = cti_min; i < cti_max; i++)
    {
      for (int j = ati_min; j < ati_max; j++)
    {
      WVC* other_wvc = swath[i][j];
      if (! other_wvc)
        continue;

      WindVectorPlus* other_wvp = other_wvc->selected;
      if (! other_wvp)
        continue;
      count++;
      x+= other_wvp->spd * cos(other_wvp->dir);
      y+= other_wvp->spd * sin(other_wvp->dir);
      count++;
    }
    }
    if(count==0) return(0);
    x/=count;
    y/=count;
    wvp->spd=sqrt(x*x + y*y);
    wvp->dir=acos(x/wvp->spd);
    if(y<0) wvp->dir=two_pi-wvp->dir;
    wvp->obj=-(float)HUGE_VAL;
    return(1);
}

//-------------------------------//
// WindSwath::GetMostProbableDir //
//-------------------------------//

#define CORRELATION_WIDTH  1.0  // Currently in units of Cross Track Index
                                // Should be km!!!!
#define CW_DEVIATION_AMB        1.0  // 1.0 times wind speed
#define CW_DEVIATION            0.25 // 0.25  times speed
#define NUM_DIRECTION_STEPS    45

float
WindSwath::GetMostProbableDir(
    WindVectorPlus*  wvp,
    int              cti,
    int              ati,
    int              cti_min,
    int              cti_max,
    int              ati_min,
    int              ati_max)
{
  float max_prob=-HUGE_VAL;
  float max_dir=0.0;
  WVC* wvc=swath[cti][ati];
  int bins=NUM_DIRECTION_STEPS;
  float step=two_pi/bins;
  for(int s=0;s<bins;s++){
    float dir=step*s;
    float likelihood=0.0;
    float spd=wvc->directionRanges.GetBestSpeed(dir);
    float x1=spd*cos(dir);
    float y1=spd*sin(dir);
    for (int i = cti_min; i < cti_max; i++)
    {
      for (int j = ati_min; j < ati_max; j++)
    {
      WVC* other_wvc = swath[i][j];
      if (! other_wvc)
        continue;

      WindVectorPlus* other_wvp = other_wvc->selected;
      if (! other_wvp)
        continue;
          if(i==cti && j==ati) continue;
          // Add likelihood due to probability of selected being correct
          // Commented out because it is not necessary unless other possible
          // directions for the neighboring vectros are considered.
          // As it is now it just adds a constant value to likelihood for
          // all possible dir values.
          //likelihood+=other_wvc->directionRanges.GetBestObj(other_wvp->dir);
      // Add likelihood due to difference between selected and trial value
          float dist=sqrt(float((cti-i)*(cti-i))+float((ati-j)*(ati-j)));
          float k=CW_DEVIATION*spd/CORRELATION_WIDTH;
          float sigma=k*dist;
          float x2=other_wvp->spd*cos(other_wvp->dir);
          float y2=other_wvp->spd*sin(other_wvp->dir);
          float dx=x1-x2;
          float dy=y1-y2;
      float diff=(dx*dx+dy*dy)/(sigma*sigma);
      likelihood-=diff;
    }
    }
    likelihood+=wvc->directionRanges.GetBestObj(dir);
    if(likelihood>max_prob){
      max_prob=likelihood;
      max_dir=dir;
    }
  }
  wvp->dir=max_dir;
  wvp->spd=wvc->directionRanges.GetBestSpeed(max_dir);
  wvp->obj=wvc->directionRanges.GetBestObj(max_dir);
  return(max_prob);
}

//-------------------------------------//
// WindSwath::GetMostProbableAmbiguity //
//-------------------------------------//

float
WindSwath::GetMostProbableAmbiguity(
    WindVectorPlus**  wvp,
    int               cti,
    int               ati,
    int               cti_min,
    int               cti_max,
    int               ati_min,
    int               ati_max)
{
    float max_prob = -HUGE_VAL;
    int max_prob_num = -1;

    WVC* wvc = swath[cti][ati];
    float* prob = new float[wvc->ambiguities.NodeCount()];

    int amb_num = 0;
    for (WindVectorPlus* amb = wvc->ambiguities.GetHead(); amb;
        amb = wvc->ambiguities.GetNext(), amb_num++)
    {
        float spd = amb->spd;
        float likelihood = 0.0;
        float x1, y1;
        amb->GetUV(&x1, &y1);

    for (int i = cti_min; i < cti_max; i++)
    {
      for (int j = ati_min; j < ati_max; j++)
    {
      WVC* other_wvc = swath[i][j];
      if (! other_wvc)
        continue;

      WindVectorPlus* other_wvp = other_wvc->selected;
      if (! other_wvp)
        continue;
          if(i==cti && j==ati) continue;
          // Add likelihood due to probability of selected being correct
          // Commented out because it is not necessary unless other possible
          // directions for the neighboring vectors are considered.
          // As it is now it just adds a constant value to likelihood for
          // all possible dir values.
          //likelihood+=other_wvc->directionRanges.GetBestObj(other_wvp->dir);
      // Add likelihood due to difference between selected and trial value
          float dist=sqrt(float((cti-i)*(cti-i))+float((ati-j)*(ati-j)));
          // k1: part of sigma due to width of peak and distance
          float k1=CW_DEVIATION_AMB*spd/CORRELATION_WIDTH;

          // Part of sigma due to possible error in neighboring selection
          float k2=other_wvc->GetEstimatedSquareError();
          float sigma=sqrt(k1*k1*dist*dist+k2*k2);
          float x2,y2;
          other_wvp->GetUV(&x2,&y2);
          float dx=x1-x2;
          float dy=y1-y2;
      float diff=(dx*dx+dy*dy)/(sigma*sigma);
      likelihood-=diff;
    }
    }
    likelihood+=amb->obj;
    if(BK_NUDGE){
      float sigma=NUDGE_DEVIATION*spd;
      float x2,y2;
      wvc->nudgeWV->GetUV(&x2,&y2);
      float dx=x1-x2;
      float dy=y1-y2;
      likelihood-=(dx*dx+dy*dy)/(sigma*sigma);
    }
    if(likelihood>max_prob){
      max_prob=likelihood;
      max_prob_num=amb_num;
      *wvp=amb;
    }
    prob[amb_num]=likelihood;
  }

  //Normalize
  float sum=0.0;
  for(int c=0;c<wvc->ambiguities.NodeCount();c++){
    prob[c]-=max_prob;
    prob[c]/=2.0;
    prob[c]=exp(prob[c]);
    sum+=prob[c];
  }
  delete[] prob;
  return(prob[max_prob_num]/sum);
}

//------------------------------------//
// WindSwath::DiscardUnselectedRanges //
//------------------------------------//

int
WindSwath::DiscardUnselectedRanges()
{
   for (int i = 0; i < _crossTrackBins; i++)
    {
      for (int j = 0; j < _alongTrackBins; j++)
    {
      WVC* wvc = swath[i][j];
      if (! wvc)
        continue;

      WindVectorPlus* sel = wvc->selected;
      if (! sel)
        continue;
          AngleInterval* ai=wvc->directionRanges.GetHead();
      WindVectorPlus* wvp=wvc->ambiguities.GetHead();

      while(wvp){
        if(wvp!=sel)
          {
        ai=wvc->directionRanges.RemoveCurrent();
        delete ai;
        wvp=wvc->ambiguities.RemoveCurrent();
                delete wvp;
        wvp=wvc->ambiguities.GetCurrent();
          }
        else{
          ai=wvc->directionRanges.GetNext();
          wvp=wvc->ambiguities.GetNext();
        }
      }
    }
    }
   return(1);
}

//--------------------//
// WindSwath::Shotgun //
//--------------------//

#define SECTOR_COUNT             4
#define MIN_TARGET_SECTOR_COUNT  6

int
WindSwath::Shotgun(
    int  angle_window_size,
    int  blast_window_size)
{
    //---------------------//
    // create a target map //
    //---------------------//

    char** target = (char**)make_array(sizeof(char), 2,
        _crossTrackBins, _alongTrackBins);

    //--------------------//
    // prep for filtering //
    //--------------------//

    int half_angle_window = angle_window_size / 2;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            target[cti][ati] = 0;
        }
    }

    //--------------//
    // find targets //
    //--------------//

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        int cti_min = cti - half_angle_window;
        int cti_max = cti + half_angle_window + 1;
        if (cti_min < 0)
            cti_min = 0;
        if (cti_max > _crossTrackBins)
            cti_max = _crossTrackBins;

        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            int ati_min = ati - half_angle_window;
            int ati_max = ati + half_angle_window + 1;
            if (ati_min < 0)
                ati_min = 0;
            if (ati_max > _alongTrackBins)
                ati_max = _alongTrackBins;

            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            //--------------------//
            // clear accumulators //
            //--------------------//

            int align[SECTOR_COUNT], offset[SECTOR_COUNT];
            for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
            {
                align[sector_idx] = 0;
                offset[sector_idx] = 0;
            }

            //-----------//
            // in window //
            //-----------//

            for (int i = cti_min; i < cti_max; i++)
            {
                for (int j = ati_min; j < ati_max; j++)
                {
                    if (i == cti && j == ati)
                        continue;  // don't check central vector

                    WVC* other_wvc = swath[i][j];
                    if (! other_wvc)
                        continue;

                    WindVectorPlus* other_wvp = other_wvc->selected;
                    if (! other_wvp)
                        continue;

                    int align_idx = (int)(SECTOR_COUNT * other_wvp->dir /
                        two_pi);
                    align[align_idx]++;

                    int offset_idx = (int)(SECTOR_COUNT * other_wvp->dir /
                        two_pi + 0.5) % SECTOR_COUNT;
                    offset[offset_idx]++;
                }
            }

            //-----------------------//
            // check sector coverage //
            //-----------------------//

            int sector_count = 0;
            for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
            {
                if (align[sector_idx] > 0)
                    sector_count++;
                if (offset[sector_idx] > 0)
                    sector_count++;
            }

            //-----------------------------//
            // determine if it is a target //
            //-----------------------------//

            if (sector_count > MIN_TARGET_SECTOR_COUNT)
                target[cti][ati] = 1;
        }
    }

    //--------//
    // SHOOT! //
    //--------//

    int half_blast_window = blast_window_size / 2;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            //---------//
            // target? //
            //---------//

            if (! target[cti][ati])
                continue;

            int cti_min = cti - half_blast_window;
            int cti_max = cti + half_blast_window + 1;
            if (cti_min < 0)
                cti_min = 0;
            if (cti_max > _crossTrackBins)
                cti_max = _crossTrackBins;

            int ati_min = ati - half_blast_window;
            int ati_max = ati + half_blast_window + 1;
            if (ati_min < 0)
                ati_min = 0;
            if (ati_max > _alongTrackBins)
                ati_max = _alongTrackBins;

            for (int i = cti_min; i < cti_max; i++)
            {
                for (int j = ati_min; j < ati_max; j++)
                {
                    WVC* other_wvc = swath[i][j];
                    if (! other_wvc)
                        continue;

                    WindVectorPlus* other_wvp = other_wvc->selected;
                    if (! other_wvp)
                        continue;

                    // init with first ranked
                    other_wvc->selected = NULL;
//                    other_wvc->selected =
//                        other_wvc->ambiguities.GetByIndex(0);
                }
            }
        }
    }

    return(1);
}

//-------------------------------//
// WindSwath::GetMedianBySorting //
//-------------------------------//

int
WindSwath::GetMedianBySorting(
    WindVectorPlus*  wvp,
    int              cti_min,
    int              cti_max,
    int              ati_min,
    int              ati_max)
{
  float* x=new float[(cti_max-cti_min)*(ati_max-ati_min)];
  float* y=new float[(cti_max-cti_min)*(ati_max-ati_min)];
  int count=0;
  for (int i = cti_min; i < cti_max; i++)
    {
      for (int j = ati_min; j < ati_max; j++)
    {
      WVC* other_wvc = swath[i][j];
      if (! other_wvc)
        continue;
      
      WindVectorPlus* other_wvp = other_wvc->selected;
      if (! other_wvp)
        continue;
      
      x[count] = other_wvp->spd * cos(other_wvp->dir);
      y[count] = other_wvp->spd * sin(other_wvp->dir);
      if(isnan(x[count]) || isnan(y[count])){
        fprintf(stderr,"Fatal Error NaN produced in WindSwath::GetMedianBysorting\n");
	exit(1);
        // should never happen
	//  continue;
      }
      count++;
    }
    }
  if(count==0) return(0);
  sort_increasing(x,count);
  sort_increasing(y,count);
  float x_med, y_med;
  if(count%2==1){
    x_med=x[count/2];
    y_med=y[count/2];
  }
  else{
    x_med=0.5*(x[count/2-1] + x[count/2]);
    y_med=0.5*(y[count/2-1] + y[count/2]);
  }
  wvp->spd=sqrt(x_med*x_med + y_med*y_med);
  if(wvp->spd==0) wvp->dir=0;
  else wvp->dir=acos(x_med/wvp->spd);
  if(y_med<0) wvp->dir=two_pi-wvp->dir;
  wvp->obj=-(float)HUGE_VAL;
  delete[] x;
  delete[] y;
  return(1);
}

//--------------------------//
// WindSwath::SelectNearest //
//--------------------------//

int
WindSwath::SelectNearest(
    WindField*    truth)
{
  if(truth==NULL){
    fprintf(stderr,"No truth field... Selecting closest to nudge vectors\n");
    useNudgeVectorsAsTruth=1;
  }
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            WindVector true_wv;
	    if (wvc->nudgeWV ==0)
	      {
		wvc->selected=NULL;
		continue;
	      }
	    if (useNudgeVectorsAsTruth && wvc->nudgeWV)
	      {
		true_wv.dir=wvc->nudgeWV->dir;
		true_wv.spd=wvc->nudgeWV->spd;
	      }
	    else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
	      {
		continue;
	      }
	    
	    
            wvc->selected = wvc->GetNearestToDirection(true_wv.dir);
            count++;
        }
    }
    
    return(count);
}

//------------------------//
// WindSwath::SelectTruth //
//------------------------//
// added by A. Chau to get the truth file output on the same grid as the retrieved speeds 
// modeled after SelectNudge and SelectNearest. 9/23/09
int
WindSwath::SelectTruth(WindField* truth)
{
  if(truth==NULL)
  {
    fprintf(stderr,"No truth field...Selecting nudge vectors instead of truth\n");
    useNudgeVectorsAsTruth=1;
  }
  else
    {
      fprintf(stderr,"Selecting truth field \n");
      useNudgeVectorsAsTruth=0; 
    }
  int count = 0;
  for (int cti = 0; cti < _crossTrackBins; cti++)
    {
      for (int ati = 0; ati < _alongTrackBins; ati++)
	{
	  WVC* wvc = swath[cti][ati];
	  if (! wvc)
	    continue;

	  WindVector true_wv;
	  if (wvc->nudgeWV == 0)
	    {
	      wvc->selected = NULL;
	      //fprintf(stderr,"set selected = NULL\n");
	      continue;
	    }
	  if (useNudgeVectorsAsTruth && wvc->nudgeWV)
	    {
	      true_wv.dir = wvc->nudgeWV->dir;
	      true_wv.spd = wvc->nudgeWV->spd;
	      //fprintf(stderr,"set true_wv.dir as nudge ");
	    }
	  else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
	    {
	      wvc->selected = NULL;
	      fprintf(stderr,"didn't interpolate truth");
	      continue;
	    }
	  wvc->selected->dir = true_wv.dir;
	  wvc->selected->spd = true_wv.spd;
	  count++;
	}
    }
  return(count);
}
		    

//------------------------//
// WindSwath::SelectNudge //
//------------------------//

int
WindSwath::SelectNudge()
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;
            if (! wvc->nudgeWV || wvc->ambiguities.NodeCount()==0)
            {
                wvc->selected=NULL;
                continue;
            }
            if(wvc->nudgeWV->spd == 0)
            {
                wvc->selected=NULL;
                continue;
            }
            wvc->selected = wvc->nudgeWV;
            count++;
        }
    }

    return(count);
}

//--------------------------//
// WindSwath::MatchSelected //
//--------------------------//

int
WindSwath::MatchSelected(
    WindSwath*    source)
{
    int count = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            WindVectorPlus* wvp;
            if (! source->swath[cti][ati])
                wvc->selected = NULL;
            else if (! source->swath[cti][ati]->selected)
                wvc->selected = NULL;
            else
            {
                wvp = source->swath[cti][ati]->selected;
                wvc->selected = wvc->GetNearestToDirection(wvp->dir);
                count++;
            }
        }
    }
    return(count);
}

//----------------------//
// WindSwath::RmsSpdErr //
//----------------------//

float
WindSwath::RmsSpdErr(
    WindField*    truth)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
    WindField*    truth)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
    WindField*  truth,
        FILE*   ofp)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc || ! wvc->selected)
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            float dif = fabs(ANGDIF(wvc->selected->dir,
                        true_wv.dir))*rtd;
            float lon=wvc->lonLat.longitude*rtd;
            float lat=wvc->lonLat.latitude*rtd;
            fwrite((void*)&dif, sizeof(float), 1, ofp);
            fwrite((void*)&lon, sizeof(float), 1, ofp);
            fwrite((void*)&lat, sizeof(float), 1, ofp);
        }
    }

    return(1);
}

//----------------------------------//
// WindSwath::WriteMaxDirErrIndices //
//----------------------------------//

int
WindSwath::WriteMaxDirErrIndices(
    WindField*  truth,
    FILE*       ofp)
{
    int ati_max = 0;
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        float max = 0.0;
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc || ! wvc->selected)
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            float dif = fabs(ANGDIF(wvc->selected->dir, true_wv.dir)) * rtd;
            if (dif > max)
            {
                ati_max = ati;
                max = dif;
            }
        }
        fprintf(ofp, "Err %f cti %d ati %d\n", max, cti, ati_max);
    }

    return(1);
}

//------------------//
// WindSwath::Skill //
//------------------//

float
WindSwath::Skill(
    WindField*    truth)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            WindVectorPlus* nearest = wvc->GetNearestToDirection(true_wv.dir);
            if (nearest == wvc->selected)
                good_count++;

            count++;
        }
    }

    float skill;
    if (count == 0)
        skill = 0;
    else
        skill = (float)good_count / (float)count;

    return(skill);
}

//--------------------//
// WindSwath::SpdBias //
//--------------------//

float
WindSwath::SpdBias(
    WindField*  truth)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
    WindField*     truth,
    unsigned int*  swath_density_array,
    unsigned int*  field_density_array,
    float          low_speed,
    float          high_speed,
    int            direction_count)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
            if (! dir_idx.GetNearestIndexWrapped(rel_ret_dir, &ret_idx) ||
                ! dir_idx.GetNearestIndexWrapped(rel_true_dir, &true_idx))
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
    float   cross_track_res,
    float*  ctd_array)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV){
          true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

            sum += wvc->ambiguities.NodeCount();
            count++;
        }

        if (count == 0)
          avg_nambig[cti] = 0;
        else
          avg_nambig[cti] = (double)sum / (double)count;
    }

    return(1);
}

//---------------------------//
// WindSwath::RmsSpdErrVsCti //
//---------------------------//

int
WindSwath::RmsSpdErrVsCti(
    WindField*  truth,
    float*      rms_spd_err_array,
    float*      std_dev_array,
    float*      std_err_array,
    float*      spd_bias_array,
    int*        count_array,
    float       low_speed,
    float       high_speed)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
    WindField*  truth,
    float*      rms_dir_err_array,
    float*      std_dev_array,
    float*      std_err_array,
    float*      dir_bias_array,
    int*        count_array,
    float       low_speed,
    float       high_speed)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir=wvc->nudgeWV->dir;
                true_wv.spd=wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV){
          true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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

//--------------------------------//
// WindSwath::GetProbabilityArray //
//--------------------------------//

int
WindSwath::GetProbabilityArray(
    WindField*  truth,
    float***    prob,
    int**       num_samples,
    float**     widths,
    int         true_dir_bins,
    int         delta_dir_bins)
{
    float true_dir_step_size = two_pi / true_dir_bins;
    float delta_dir_step_size = two_pi / delta_dir_bins;

    //----------------------------------------//
    // Initialize prob and num_samples arrays //
    //----------------------------------------//

    for (int ctd = 0; ctd < _crossTrackBins; ctd++)
    {
        for (int td = 0; td < true_dir_bins; td++)
        {
            num_samples[ctd][td] = 0;
            widths[ctd][td] = 0.0;
            for (int dd = 0; dd < delta_dir_bins; dd++)
            {
                prob[ctd][td][dd] = 0.0;
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
      if (useNudgeVectorsAsTruth && wvc->nudgeWV){
    true_wv.dir=wvc->nudgeWV->dir;
    true_wv.spd=wvc->nudgeWV->spd;
      }
      else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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

//-----------------------//
// WindSwath::operator-= //
//-----------------------//

void
WindSwath::operator-=(
    const WindSwath&  w)
{
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc1 = *(*(swath + i) + j);
            if (wvc1 == NULL)
                continue;
            WVC* wvc2 = *(*(w.swath + i) + j);
            if (wvc2 == NULL)
            {
                wvc1->selected = NULL;
                continue;
            }
            float u1, v1, u2, v2;
            if (wvc1->selected && wvc2->selected)
            {
                WindVectorPlus* wvp_sel = new WindVectorPlus;
                wvc1->selected->GetUV(&u1, &v1);
                wvc2->selected->GetUV(&u2, &v2);
                wvp_sel->SetUV(u1-u2, v1-v2);
                wvc1->selected = wvp_sel;
                wvc1->selected_allocated = 1;
            }
            else
            {
                wvc1->selected = NULL;
            }
            WindVectorPlus* wvp1 = wvc1->ambiguities.GetHead();
            WindVectorPlus* wvp2 = wvc2->ambiguities.GetHead();
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

//--------------------------------//
// WindSwath::DifferenceFromTruth //
//--------------------------------//

int
WindSwath::DifferenceFromTruth(
    LonLatWind*  truth)
{
    for (int i = 0; i < _crossTrackBins; i++) {
        for (int j = 0; j < _alongTrackBins; j++) {
            WVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL) {
                continue;
            }

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV) {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv)) {
                continue;
            }

            WindVectorPlus* wvp = wvc->ambiguities.GetHead();
            while (wvp) {
                float u1, v1, u2, v2;
                wvp->GetUV(&u1, &v1);
                true_wv.GetUV(&u2, &v2);
                wvp->SetUV(u1 - u2, v1 - v2);
                wvp = wvc->ambiguities.GetNext();
            }
        }
    }
    return(1);
}

//-----------------------------//
// WindSwath::ExcessDifference //
//-----------------------------//

int
WindSwath::ExcessDifference(
    WindField*  truth)
{
    for (int i = 0; i < _crossTrackBins; i++)
    {
        for (int j = 0; j < _alongTrackBins; j++)
        {
            WVC* wvc = *(*(swath + i) + j);
            if (wvc == NULL)
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
            {
                continue;
            }
            float true_u, true_v;
            true_wv.GetUV(&true_u, &true_v);

            WindVectorPlus* nearest_to_truth =
                wvc->GetNearestToDirection(true_wv.dir);
            float near_u, near_v;
            nearest_to_truth->GetUV(&near_u, &near_v);
            float near_udif = true_u - near_u;
            float near_vdif = true_v - near_v;
            float near_mag = sqrt(near_udif*near_udif +
                near_vdif*near_vdif);

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                float u1, v1;
                wvp->GetUV(&u1, &v1);

                float udif = u1 - true_u;
                float vdif = v1 - true_v;

                wvp->SetUV(udif, vdif);    // set vector
                wvp->spd -= near_mag;      // tweak magnitude
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
    WindField*    truth,
    float*        skill_array,
    int*        count_array,
    float        low_speed,
    float        high_speed)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV){
          true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

            WindVectorPlus* nearest = wvc->GetNearestToDirection(true_wv.dir);
            if (nearest == wvc->selected)
                good_count++;

            count++;
        }

        *(count_array + cti) = count;
        if (count == 0)
          *(skill_array + cti) = 0;
        else
          *(skill_array + cti) = (float)good_count / (float)count;
    }

    return(1);
}

//------------------------//
// WindSwath::WithinVsCti //
//------------------------//

int
WindSwath::WithinVsCti(
    WindField*    truth,
    float*        skill_array,
    int*        count_array,
    float        low_speed,
    float        high_speed,
    float        within_angle)
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV){
          true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

            float dif = ANGDIF(true_wv.dir, wvc->selected->dir);
            if (dif < within_angle)
                good_count++;

            count++;
        }

        *(count_array + cti) = count;
        if (count == 0)
          *(skill_array + cti) = 0;
        else
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
            if (useNudgeVectorsAsTruth && wvc->nudgeWV){
          true_wv.dir=wvc->nudgeWV->dir;
              true_wv.spd=wvc->nudgeWV->spd;
        }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
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
            dir_idx.GetNearestIndexStrict(rel_ret_dir, &ret_idx);
            dir_idx.GetNearestIndexStrict(rel_true_dir, &true_idx);

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

    //----------------------//
    // Allocate Mean Arrays //
    //----------------------//

    float* mean_c1_array= new float[_crossTrackBins];
    float* mean_c2_array= new float[_crossTrackBins];

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        *(cc_array + cti) = 0.0;
        *(count_array + cti) = 0;
        *(mean_c1_array +cti) = 0.0;
        *(mean_c2_array +cti) = 0.0;

        //-------------------------//
        // first pass calculations //
        //-------------------------//

        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc || ! wvc->selected)
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue;

            //-------------------//
            // Find Components   //
            //-------------------//

            float u = 0;
            float v = 0;
            float c1 = 0;
            float c2 = 0;
            switch(component1)
            {
            case UTRUE:
                true_wv.GetUV(&u, &v);
                c1=u;
                break;
            case VTRUE:
                true_wv.GetUV(&u, &v);
                c1=v;
                break;
            case UMEAS:
                wvc->selected->GetUV(&u, &v);
                c1=u;
                break;
            case VMEAS:
                wvc->selected->GetUV(&u, &v);
                c1=v;
                break;
            default:
                fprintf(stderr, "ComponentCovariance: Bad component1\n");
                return(0);
            }
            switch(component2){
            case UTRUE:
              true_wv.GetUV(&u, &v);
              c2=u;
              break;
            case VTRUE:
              true_wv.GetUV(&u, &v);
              c2=v;
              break;
            case UMEAS:
              wvc->selected->GetUV(&u, &v);
              c2=u;
              break;
            case VMEAS:
              wvc->selected->GetUV(&u, &v);
              c2=v;
              break;
            default:
              fprintf(stderr, "ComponentCovariance: Bad component2\n");
              return(0);
            }

            //---------------//
            // Update Arrays //
            //---------------//

            float x = c1*c2;
            *(cc_array + cti) += x;
                        *(mean_c1_array +cti) +=c1;
                        *(mean_c2_array +cti) +=c2;
            (*(count_array + cti))++;
        }

        if (*(count_array + cti) < 2)
            continue;

        //--------------------------//
        // calculate the covariance //
        //--------------------------//

        *(mean_c1_array + cti) /= (float)*(count_array + cti);
        *(mean_c2_array + cti) /= (float)*(count_array + cti);
        *(cc_array + cti) /= (float)*(count_array + cti);
                *(cc_array +cti) -= (*(mean_c1_array +cti))*(*(mean_c2_array +cti));

    }

    delete(mean_c1_array);
        delete(mean_c2_array);
    return(1);
}

//-----------------------------------//
// WindSwath::VectorCorrelationVsCti //
//-----------------------------------//

int
WindSwath::VectorCorrelationVsCti(
    WindField*  truth,
    float*      vc_array,
    int*        count_array,
    float       low_speed,
    float       high_speed)
{
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

    //---------------------------------//
    // Calculate Component Covariances //
    //---------------------------------//

    if(! ComponentCovarianceVsCti(truth, su1u1_array, count_array, low_speed,
              high_speed, UTRUE, UTRUE))
    {
        return(0);
    }
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

    float dem = (su1u1*sv1v1-(su1v1*su1v1))*(su2u2*sv2v2-su2v2*su2v2);

    if (dem == 0)
      (*(vc_array +cti)) /= 1;
    else
      (*(vc_array +cti)) /=(su1u1*sv1v1-(su1v1*su1v1))*(su2u2*sv2v2-su2v2*su2v2);
  }

    //-------------------//
    // deallocate arrays //
    //-------------------//

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

//--------------------------------//
// WindSwath::Streamosity         //
//--------------------------------//

int    WindSwath::Streamosity(
    WindField* truth,
    float* stream_array,
    float* good_stream_array,
    float low_speed,
    float high_speed){

  for(int cti=0;cti<_crossTrackBins;cti++){
    int count=0;
    stream_array[cti]=0;
    good_stream_array[cti]=0;
    for(int ati=0;ati<_alongTrackBins;ati++){
       WVC* wvc = swath[cti][ati];
       if (! wvc)
     continue;

       WindVector true_wv;
       if (useNudgeVectorsAsTruth && wvc->nudgeWV){
     true_wv.dir=wvc->nudgeWV->dir;
     true_wv.spd=wvc->nudgeWV->spd;
       }
       else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
     continue;
       count++; // increment count of good wind vector cells
       WindVectorPlus* wvp1=wvc->ambiguities.GetHead();
       WindVectorPlus* wvp2=wvc->ambiguities.GetNext();
       if( !wvp1 || !wvp2) continue;
       float angdif=fabs(ANGDIF(wvp1->dir, wvp2->dir));
       if(angdif < 120*dtr) continue;
       stream_array[cti]++;
       float dir_err1=fabs(ANGDIF(wvp1->dir, true_wv.dir));
       float dir_err2=fabs(ANGDIF(wvp2->dir, true_wv.dir));
       dir_err1=fabs(pi/2 - fabs(pi/2-dir_err1));
       dir_err2=fabs(pi/2 - fabs(pi/2-dir_err2));
       if(dir_err1<30*dtr && dir_err2<30*dtr) good_stream_array[cti]++;
    }
    if(count){
      stream_array[cti]/=count;
      good_stream_array[cti]/=count;
    }
  }
  return(1);
}

//----------------------------//
// WindSwath::FractionNAmbigs //
//----------------------------//

int
WindSwath::FractionNAmbigs(
    WindField*  truth,
    float*      frac_1amb_array,
    float*      frac_2amb_array,
    float*      frac_3amb_array,
    float*      frac_4amb_array,
    float       low_speed,
    float       high_speed)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        int count = 0;
        frac_1amb_array[cti] = 0;
        frac_2amb_array[cti] = 0;
        frac_3amb_array[cti] = 0;
        frac_4amb_array[cti] = 0;
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue;

            WindVector true_wv;
            if (useNudgeVectorsAsTruth && wvc->nudgeWV)
            {
                true_wv.dir = wvc->nudgeWV->dir;
                true_wv.spd = wvc->nudgeWV->spd;
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;
            count++; // increment count of good wind vector cells
            int num = wvc->ambiguities.NodeCount();
            switch(num)
            {
            case 1:
                frac_1amb_array[cti]++;
                break;
            case 2:
                frac_2amb_array[cti]++;
                break;
            case 3:
                frac_3amb_array[cti]++;
                break;
            case 4:
                frac_4amb_array[cti]++;
                break;
            default:
                fprintf(stderr, "Frac_N_Ambigs:: Bad number of ambigs %d\n",
                    num);
                return(0);
            }
        }
        if (count)
        {
            frac_1amb_array[cti] /= count;
            frac_2amb_array[cti] /= count;
            frac_3amb_array[cti] /= count;
            frac_4amb_array[cti] /= count;
        }
    }
    return(1);
}

//-------------------------------//
// WindSwath::NudgeOverrideVsCti //
//-------------------------------//

int
WindSwath::NudgeOverrideVsCti(
    WindField*  truth,
    float*      correction_rate_array,
    float*      change_incorrect_rate_array,
    float*      bad_nudge_rate_array,
    float       low_speed,
    float       high_speed)
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        // Initialize counts and array
        int count = 0;
        int total_count = 0;
        correction_rate_array[cti] = 0;
        change_incorrect_rate_array[cti] = 0;
        bad_nudge_rate_array[cti] = 0;
        /// loop through along track bins
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath[cti][ati];
            if (! wvc)
                continue; // skip bad wind vector cells

            WindVector true_wv;
            // Error Message for useNudgeVectorsAsTruth==1 case
            if (useNudgeVectorsAsTruth)
            {
                fprintf(stderr,
               "NudgeOverride makes no sense with useNudgeVectorsAsTruth=1\n");
                return(0);
            }
            else if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue; // if no truth, skip

            if (true_wv.spd < low_speed || true_wv.spd > high_speed)
                continue; // if out of speed range, skip

            WindVector* nudge_wv = wvc->nudgeWV;
            if (! nudge_wv)
                continue;  // if no nudge vector, skip

            WindVectorPlus* sel = wvc->selected;
            if (! sel)
                continue;  // if no selection skip

            // compute closest ambiguities to truth and nudge field
            WindVectorPlus* nudge_near =
                wvc->GetNearestToDirection(nudge_wv->dir);
            WindVectorPlus* truth_near =
                wvc->GetNearestToDirection(true_wv.dir);

            total_count++;

            // if nudge and truth agree, then skip
            if (nudge_near == truth_near)
                continue;

            // increment count of nudge/truth disagreements;
            count++;

            // increment count of instrument corrections
            if (truth_near == sel)
                correction_rate_array[cti]++;
            // increment count of instrument incorrect changes
            else if (sel != nudge_near)
                change_incorrect_rate_array[cti]++;
        }
        // normalize by counts;
        if (count != 0)
        {
            correction_rate_array[cti] /= count;
            change_incorrect_rate_array[cti] /= count;
        }
        else
        {
            correction_rate_array[cti] =- 1;
            change_incorrect_rate_array[cti] =- 1;
        }
        bad_nudge_rate_array[cti] = (float)(count) / total_count;
    }
    return(1);
}

//------------------------------//
// WindSwath::NearestWindVector //
//------------------------------//

int
WindSwath::NearestWindVector(
    LonLat       lon_lat,
    WindVector*  wv)
{
    return(0);
}

//-----------------------------------//
// WindSwath::InterpolatedWindVector //
//-----------------------------------//
// Calling this method will allocate, and never deallocate, memory.
// Typically, the allocations will be small.

int
WindSwath::InterpolatedWindVector(
    LonLat       lon_lat,
    WindVector*  wv)
{
printf("Target %g %g\n", lon_lat.longitude * rtd, lon_lat.latitude * rtd);
    static LonLat* lonlat_min = NULL;
    static LonLat* lonlat_max = NULL;
    static int section_count = 0;

    //----------------------------------------------------//
    // if this is the first call, break the wind swath up //
    //----------------------------------------------------//

    if (lonlat_min == NULL) {
        section_count = (int)(_alongTrackBins / _crossTrackBins) + 1;
        lonlat_min = new LonLat[section_count];
        lonlat_max = new LonLat[section_count];

        for (int sidx = 0; sidx < section_count; sidx++) {

            int min_ati = sidx * _crossTrackBins - 1;
            min_ati = MAX(0, min_ati);
            int max_ati = (sidx + 1) * _crossTrackBins + 1;
            max_ati = MIN(max_ati, _alongTrackBins);

            lonlat_min[sidx].latitude = two_pi;
            lonlat_min[sidx].longitude = two_pi;
            lonlat_max[sidx].latitude = -two_pi;
            lonlat_max[sidx].longitude = -two_pi;

            for (int ati = min_ati; ati < max_ati; ati++) {
                for (int cti = 0; cti < _crossTrackBins; cti++) {
                    WVC* wvc = GetWVC(cti, ati);
                    if (wvc == NULL) {
                        continue;
                    }
                    lonlat_min[sidx].latitude = MIN(lonlat_min[sidx].latitude,
                        wvc->lonLat.latitude);
                    lonlat_min[sidx].longitude = MIN(
                        lonlat_min[sidx].longitude, wvc->lonLat.longitude);
                    lonlat_max[sidx].latitude = MAX(lonlat_max[sidx].latitude,
                        wvc->lonLat.latitude);
                    lonlat_max[sidx].longitude = MAX(
                        lonlat_max[sidx].longitude, wvc->lonLat.longitude);
                }
            }
        }
    }

    // For now, this is a fairly simply algorithm:
    // First, a tiny search is performed around the last successful indicies.
    // If that fails, a full-on brute force search is performed.

    //--------------------------------//
    // the quick tiny vicinity search //
    //--------------------------------//

    static int last_good_cti = 0;
    static int last_good_ati = 0;
    for (int delta_cti = -1; delta_cti < 2; delta_cti++) {
        int cti = (last_good_cti + delta_cti) % _crossTrackBins;
        for (int delta_ati = -1; delta_ati < 2; delta_ati++) {
            int ati = (last_good_ati + delta_ati) % _alongTrackBins;
            if (_Interpolate(lon_lat, cti, ati, wv)) {
                last_good_ati = ati;
                last_good_cti = cti;
                return(1);
            }
        }
    }

    //------------------------//
    // the brute force search //
    //------------------------//
    // only search promising sections

    for (int sidx = 0; sidx < section_count; sidx++) {
        if (lon_lat.latitude < lonlat_min[sidx].latitude ||
            lon_lat.longitude < lonlat_min[sidx].longitude ||
            lon_lat.latitude > lonlat_max[sidx].latitude ||
            lon_lat.longitude > lonlat_max[sidx].longitude)
        {
            continue;
        }

        int min_ati = sidx * _crossTrackBins - 1;
        min_ati = MAX(0, min_ati);
        int max_ati = (sidx + 1) * _crossTrackBins + 1;
        max_ati = MIN(max_ati, _alongTrackBins);

        for (int cti = 0; cti < _crossTrackBins - 1; cti++) {
            for (int ati = min_ati; ati < max_ati; ati++) {
                if (_Interpolate(lon_lat, cti, ati, wv)) {
                    last_good_cti = cti;
                    last_good_ati = ati;
                    return(1);
                }
            }
        }
    }

    return(0);
}

//-------------------------//
// WindSwath::_Interpolate //
//-------------------------//
// Tries to interpolate this WindSwath to the given longitude and latitude
// using only the 4-neighbors specified by low_cti and low_ati.
// Returns 1 on success; 0 on failure.
// Grid is:
//     D----C
//     |    |
//     |    |
//     A----B
// where AB is the cross track axis and AD is the along track axis
// low_cti, low_ati refers to point A

int
WindSwath::_Interpolate(
    LonLat&      lon_lat,
    int          low_cti,
    int          low_ati,
    WindVector*  wv)
{
    //-----------------------//
    // check for all corners //
    //-----------------------//

    WVC* wvc_a = GetWVC(low_cti, low_ati);
    if (wvc_a == NULL) {
        return(0);
    }
    WVC* wvc_b = GetWVC(low_cti + 1, low_ati);
    if (wvc_b == NULL) {
        return(0);
    }
    WVC* wvc_c = GetWVC(low_cti + 1, low_ati + 1);
    if (wvc_c == NULL) {
        return(0);
    }
    WVC* wvc_d = GetWVC(low_cti, low_ati + 1);
    if (wvc_d == NULL) {
        return(0);
    }

    //-----------------------//
    // create corner vectors //
    //-----------------------//

    Vect a, b, c, d;
    set_alt_lon_gclat(0.0, wvc_a->lonLat.longitude, wvc_a->lonLat.latitude, &a);
    set_alt_lon_gclat(0.0, wvc_b->lonLat.longitude, wvc_b->lonLat.latitude, &b);
    set_alt_lon_gclat(0.0, wvc_c->lonLat.longitude, wvc_c->lonLat.latitude, &c);
    set_alt_lon_gclat(0.0, wvc_d->lonLat.longitude, wvc_d->lonLat.latitude, &d);

    //-------------------------------//
    // make sure p is inside the box //
    //-------------------------------//

    Vect p;
    set_alt_lon_gclat(0.0, lon_lat.longitude, lon_lat.latitude, &p);

    double s1, t1;
    if (! p.Decompose(a, b, d, &s1, &t1)) {
        return(0);
    }
    if (s1 < 0.0 || t1 < 0.0) {
        return(0);
    }

    double s2, t2;
    if (! p.Decompose(c, d, b, &s2, &t2)) {
        return(0);
    }
    if (s2 < 0.0 || t2 < 0.0) {
        return(0);
    }

    //--------------------------------------------//
    // make sure we are on this side of the earth //
    //--------------------------------------------//

    Vect diff;
    diff.Difference(a, p);
    double a_dist = diff.Magnitude();
    if (a_dist > r1_earth) {
        return(0);
    }

    //----------------------------//
    // determine the coefficients //
    //----------------------------//

    diff.Difference(b, p);
    double b_dist = diff.Magnitude();
    diff.Difference(c, p);
    double c_dist = diff.Magnitude();
    diff.Difference(d, p);
    double d_dist = diff.Magnitude();

    double a_top = b_dist * c_dist * d_dist;
    double b_top = a_dist * c_dist * d_dist;
    double c_top = a_dist * b_dist * d_dist;
    double d_top = a_dist * b_dist * c_dist;
    double denom = (a_top + b_top + c_top + d_top);

    double a_coef = a_top / denom;
    double b_coef = b_top / denom;
    double c_coef = c_top / denom;
    double d_coef = d_top / denom;
printf("coefs = %g %g %g %g\n", a_coef, b_coef, c_coef, d_coef);

    //------------------------//
    // decompose into u and v //
    //------------------------//

    float u_sum = 0.0;
    float v_sum = 0.0;
    float u, v;
    WindVectorPlus* wvp;
    wvp = wvc_a->selected;
    if (wvp == NULL) {
        return(0);
    }
    wvp->GetUV(&u, &v);
    u_sum += (a_coef * u);
    v_sum += (a_coef * v);

    wvp = wvc_b->selected;
    if (wvp == NULL) {
        return(0);
    }
    wvp->GetUV(&u, &v);
    u_sum += (b_coef * u);
    v_sum += (b_coef * v);

    wvp = wvc_c->selected;
    if (wvp == NULL) {
        return(0);
    }
    wvp->GetUV(&u, &v);
    u_sum += (c_coef * u);
    v_sum += (c_coef * v);

    wvp = wvc_d->selected;
    if (wvp == NULL) {
        return(0);
    }
    wvp->GetUV(&u, &v);
    u_sum += (d_coef * u);
    v_sum += (d_coef * v);
    
    //-----------------------------//
    // set the interpolated vector //
    //-----------------------------//

    wv->SetUV(u_sum, v_sum);
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
