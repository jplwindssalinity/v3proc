//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l2b_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <string.h>
#include "L2B.h"
#include "TlmHdfFile.h"
#include "NoTimeTlmFile.h"
#include "ParTab.h"
#include "L1AExtract.h"
#include "WindVector.h"
#include "Wind.h"
#include "Misc.h"
#include "Sds.h"
#include "hdf.h"
#include "mfhdf.h"

#define HDF_ACROSS_BIN_NO    76
#define HDF_NUM_AMBIGUITIES  4

//===========//
// L2BHeader //
//===========//

L2BHeader::L2BHeader()
:   crossTrackResolution(0.0), alongTrackResolution(0.0), zeroIndex(0),
    inclination(0.0)
{
    return;
}

L2BHeader::~L2BHeader()
{
    return;
}

//-----------------//
// L2BHeader::Read //
//-----------------//

int
L2BHeader::Read(
    FILE*  fp)
{
    if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------------//
// L2BHeader::Write //
//------------------//

int
L2BHeader::Write(
    FILE*  fp)
{
    if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//-----------------------//
// L2BHeader::WriteAscii //
//-----------------------//

int
L2BHeader::WriteAscii(
    FILE*  fp)
{
    fprintf(fp, "############################################\n");
    fprintf(fp, "##                L2B DataFile            ##\n");
    fprintf(fp, "############################################\n");
    fprintf(fp,"\n\nCrossTrackRes %g AlongTrackRes %g ZeroIndex %d\n\n",
    crossTrackResolution,alongTrackResolution,zeroIndex);
    return(1);
}

//==========//
// L2BFrame //
//==========//

L2BFrame::L2BFrame()
{
    return;
}

L2BFrame::~L2BFrame()
{
    return;
}

//=====//
// L2B //
//=====//

L2B::L2B()
:   _status(OK)
{
    return;
}

L2B::~L2B()
{
    return;
}

//-----------//
// L2B::Read //
//-----------//

int
L2B::Read(
    const char*  filename)
{
    SetInputFilename(filename);
    if (! OpenForReading())
    {
        fprintf(stderr, "L2B::Read: error opening file %s for reading\n",
            filename);
        return(0);
    }
    if (! ReadHeader())
    {
        fprintf(stderr, "L2B::Read: error reading header from file %s\n",
            filename);
        return(0);
    }
    if (! ReadDataRec())
    {
        fprintf(stderr, "L2B::Read: error reading data record from file %s\n",
            filename);
        return(0);
    }
    return(1);
}

//------------------//
// L2B::ReadPureHdf //
//------------------//

#define ROW_WIDTH  76
#define AMBIGS      4

int
L2B::ReadPureHdf(
    const char*  filename,
    int          unnormalize_mle_flag)
{
    //---------//
    // prepare //
    //---------//

    frame.swath.DeleteEntireSwath();

    //----------------------------------------//
    // some generic HDF start and edge arrays //
    //----------------------------------------//

    // the HDF read routine should only access as many dimensions as needed
    int32 generic_start[3] = { 0, 0, 0 };
    int32 generic_edges[3] = { 1, ROW_WIDTH, AMBIGS };

    //--------------------------//
    // start access to the file //
    //--------------------------//

    int32 sd_id = SDstart(filename, DFACC_READ);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDstart\n");
        return(0);
    }

    //-------------------------------------//
    // determine the actual number of rows //
    //-------------------------------------//

    int32 attr_index_l2b_actual_wvc_rows = SDfindattr(sd_id,
        "l2b_actual_wvc_rows");
    if (attr_index_l2b_actual_wvc_rows == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDfindattr\n");
        return(0);
    }

    char data[1024];
    if (SDreadattr(sd_id, attr_index_l2b_actual_wvc_rows, data) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDreadattr\n");
        return(0);
    }

    int l2b_actual_wvc_rows = 0;
    if (sscanf(data, " %*[^\n] %*[^\n] %d", &l2b_actual_wvc_rows) != 1)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error parsing header\n");
        return(0);
    }

    //----------//
    // allocate //
    //----------//

    if (! frame.swath.Allocate(ROW_WIDTH, l2b_actual_wvc_rows))
        return(0);

    //-------------------------------//
    // get all the necessary SDS IDs //
    //-------------------------------//

    int32 wvc_row_sds_id = SDnametoid(sd_id, "wvc_row");
    int32 wvc_lat_sds_id = SDnametoid(sd_id, "wvc_lat");
    int32 wvc_lon_sds_id = SDnametoid(sd_id, "wvc_lon");
    int32 wvc_index_sds_id = SDnametoid(sd_id, "wvc_index");
    int32 num_in_fore_sds_id = SDnametoid(sd_id, "num_in_fore");
    int32 num_in_aft_sds_id = SDnametoid(sd_id, "num_in_aft");
    int32 num_out_fore_sds_id = SDnametoid(sd_id, "num_out_fore");
    int32 num_out_aft_sds_id = SDnametoid(sd_id, "num_out_aft");
    int32 wvc_quality_flag_sds_id = SDnametoid(sd_id, "wvc_quality_flag");
//    int32 atten_corr_sds_id = SDnametoid(sd_id, "atten_corr");
    int32 model_speed_sds_id = SDnametoid(sd_id, "model_speed");
    int32 model_dir_sds_id = SDnametoid(sd_id, "model_dir");
    int32 num_ambigs_sds_id = SDnametoid(sd_id, "num_ambigs");
    int32 wind_speed_sds_id = SDnametoid(sd_id, "wind_speed");
    int32 wind_dir_sds_id = SDnametoid(sd_id, "wind_dir");
    int32 max_likelihood_est_sds_id = SDnametoid(sd_id, "max_likelihood_est");
    int32 wvc_selection_sds_id = SDnametoid(sd_id, "wvc_selection");
    int32 wind_speed_selection_sds_id = SDnametoid(sd_id,
        "wind_speed_selection");
    int32 wind_dir_selection_sds_id = SDnametoid(sd_id, "wind_dir_selection");
    int32 mp_rain_probability_sds_id = SDnametoid(sd_id,
        "mp_rain_probability");

    //--------------//
    // for each row //
    //--------------//

    for (int ati = 0; ati < l2b_actual_wvc_rows; ati++)
    {
        //--------------//
        // read the row //
        //--------------//

        generic_start[0] = ati;

        int16 wvc_row;
        if (SDreaddata(wvc_row_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)&wvc_row) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_row)\n");
            return(0);
        }
        if (wvc_row != ati + 1)
        {
            fprintf(stderr, "L2B::ReadPureHdf: mismatched index and row\n");
            fprintf(stderr, "    ati = %d, wvc_row = %d\n", ati, wvc_row);
            return(0);
        }

        int16 wvc_lat[ROW_WIDTH];
        if (SDreaddata(wvc_lat_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_lat) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_lat)\n");
            return(0);
        }

        uint16 wvc_lon[ROW_WIDTH];
        if (SDreaddata(wvc_lon_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_lon) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_lon)\n");
            return(0);
        }

        int8 wvc_index[ROW_WIDTH];
        if (SDreaddata(wvc_index_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_index) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_index)\n");
            return(0);
        }

        int8 num_in_fore[ROW_WIDTH];
        if (SDreaddata(num_in_fore_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_in_fore) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_in_fore)\n");
            return(0);
        }

        int8 num_in_aft[ROW_WIDTH];
        if (SDreaddata(num_in_aft_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_in_aft) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_in_aft)\n");
            return(0);
        }

        int8 num_out_fore[ROW_WIDTH];
        if (SDreaddata(num_out_fore_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_out_fore) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_out_fore)\n");
            return(0);
        }

        int8 num_out_aft[ROW_WIDTH];
        if (SDreaddata(num_out_aft_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_out_aft) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_out_aft)\n");
            return(0);
        }

        uint16 wvc_quality_flag[ROW_WIDTH];
        if (SDreaddata(wvc_quality_flag_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_quality_flag) == FAIL)
        {
            fprintf(stderr,
              "L2B::ReadPureHdf: error with SDreaddata (wvc_quality_flag)\n");
            return(0);
        }

/*
        int16 atten_corr[ROW_WIDTH];
        if (SDreaddata(atten_corr_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)atten_corr) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (atten_corr)\n");
            return(0);
        }
*/

        int16 model_speed[ROW_WIDTH];
        if (SDreaddata(model_speed_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)model_speed) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (model_speed)\n");
            return(0);
        }

        uint16 model_dir[ROW_WIDTH];
        if (SDreaddata(model_dir_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)model_dir) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (model_dir)\n");
            return(0);
        }

        int8 num_ambigs[ROW_WIDTH];
        if (SDreaddata(num_ambigs_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)num_ambigs) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (num_ambigs)\n");
            return(0);
        }

        int16 wind_speed[ROW_WIDTH][AMBIGS];
        if (SDreaddata(wind_speed_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_speed) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wind_speed)\n");
            return(0);
        }

        uint16 wind_dir[ROW_WIDTH][AMBIGS];
        if (SDreaddata(wind_dir_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_dir) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wind_dir)\n");
            return(0);
        }

        int16 max_likelihood_est[ROW_WIDTH][AMBIGS];
        if (SDreaddata(max_likelihood_est_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)max_likelihood_est) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (max_likelihood_est)\n");
            return(0);
        }

        int8 wvc_selection[ROW_WIDTH];
        if (SDreaddata(wvc_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_selection) == FAIL)
        {
            fprintf(stderr,
                "L2B::ReadPureHdf: error with SDreaddata (wvc_selection)\n");
            return(0);
        }

        int16 wind_speed_selection[ROW_WIDTH];
        if (SDreaddata(wind_speed_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_speed_selection) == FAIL)
        {
            fprintf(stderr,
          "L2B::ReadPureHdf: error with SDreaddata (wind_speed_selection)\n");
            return(0);
        }

        uint16 wind_dir_selection[ROW_WIDTH];
        if (SDreaddata(wind_dir_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wind_dir_selection) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (wind_dir_selection)\n");
            return(0);
        }

        int16 mp_rain_probability[ROW_WIDTH];
        if (SDreaddata(mp_rain_probability_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)mp_rain_probability) == FAIL)
        {
            fprintf(stderr,
            "L2B::ReadPureHdf: error with SDreaddata (mp_rain_probability)\n");
            return(0);
        }

        //---------------------------------//
        // assemble into wind vector cells //
        //---------------------------------//

        for (int cti = 0; cti < ROW_WIDTH; cti++)
        {
            // if there are no ambiguities, wind was not retrieved
            if (num_ambigs[cti] == 0)
                continue;

            //----------------//
            // create the WVC //
            //----------------//

            WVC* wvc = new WVC();

            //--------------------------------//
            // set the longitude and latitude //
            //--------------------------------//

            wvc->lonLat.longitude = wvc_lon[cti] * HDF_WVC_LON_SCALE * dtr;
            wvc->lonLat.latitude = wvc_lat[cti] * HDF_WVC_LAT_SCALE * dtr;

            //----------------------//
            // set the nudge vector //
            //----------------------//

            wvc->nudgeWV = new WindVectorPlus();
            if (wvc->nudgeWV == NULL)
            {
                fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                return(0);
            }

            float nudge_edir =
                gs_deg_to_pe_rad(model_dir[cti] * HDF_MODEL_DIR_SCALE);
            float nudge_speed = model_speed[cti] * HDF_MODEL_SPEED_SCALE;

            wvc->nudgeWV->SetSpdDir(nudge_speed * NWP_SPEED_CORRECTION,
                nudge_edir);

            //--------------------------------//
            // set the number of measurements //
            //--------------------------------//

            wvc->numInFore = (unsigned char)num_in_fore[cti];
            wvc->numInAft = (unsigned char)num_in_aft[cti];
            wvc->numOutFore = (unsigned char)num_out_fore[cti];
            wvc->numOutAft = (unsigned char)num_out_aft[cti];

            //---------------------------//
            // create the ambiguity list //
            //---------------------------//

            int sigma0_count = num_in_fore[cti] + num_in_aft[cti]
                + num_out_fore[cti] + num_out_aft[cti];

            for (int ambig_idx = 0; ambig_idx < num_ambigs[cti]; ambig_idx++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                if (wvp == NULL)
                {
                    fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                    return(0);
                }

                float edir = gs_deg_to_pe_rad(wind_dir[cti][ambig_idx]
                    * HDF_WIND_DIR_SCALE);
                float spd = wind_speed[cti][ambig_idx] * HDF_WIND_SPEED_SCALE;
                wvp->SetSpdDir(spd, edir);

                wvp->obj = max_likelihood_est[cti][ambig_idx]
                    * HDF_MAX_LIKELIHOOD_EST_SCALE;
                if (unnormalize_mle_flag)
                {
                    wvp->obj *= sigma0_count;
                }

                if (! wvc->ambiguities.Append(wvp))
                {
                    fprintf(stderr, "L2B::ReadPureHdf: error appending\n");
                    return(0);
                }
            }

            //----------------------------//
            // set the selected ambiguity //
            //----------------------------//

            wvc->selected = wvc->ambiguities.GetByIndex(wvc_selection[cti]
                - 1);
            wvc->selected_allocated = 0;

            //------------------------//
            // set the special vector //
            //------------------------//

            WindVector* wv = new WindVector();
            if (wv == NULL)
            {
                fprintf(stderr, "L2B::ReadPureHdf: error allocating\n");
                return(0);
            }

            float special_dir = gs_deg_to_pe_rad(wind_dir_selection[cti]
                * HDF_WIND_DIR_SELECTION_SCALE);
            float special_spd = wind_speed_selection[cti]
                * HDF_WIND_SPEED_SELECTION_SCALE;
            wv->SetSpdDir(special_spd, special_dir);
            wvc->specialVector = wv;

            //----------------------------//
            // set the rain "probability" //
            //----------------------------//

            wvc->rainProb = mp_rain_probability[cti]
                * HDF_MP_RAIN_PROBABILITY_SCALE;
            wvc->rainFlagBits = (char)((0x7000 & wvc_quality_flag[cti]) >> 12);

            //------------------//
            // add WVC to swath //
            //------------------//

            if (! frame.swath.Add(cti, ati, wvc))
            {
                fprintf(stderr, "L2B::ReadPureHdf: error adding WVC\n");
                return(0);
            }
        }
    }

    if (SDendaccess(wvc_row_sds_id) == FAIL ||
        SDendaccess(wvc_lat_sds_id) == FAIL ||
        SDendaccess(wvc_lon_sds_id) == FAIL ||
        SDendaccess(wvc_index_sds_id) == FAIL ||
        SDendaccess(num_in_fore_sds_id) == FAIL ||
        SDendaccess(num_in_aft_sds_id) == FAIL ||
        SDendaccess(num_out_fore_sds_id) == FAIL ||
        SDendaccess(num_out_aft_sds_id) == FAIL ||
        SDendaccess(wvc_quality_flag_sds_id) == FAIL ||
//        SDendaccess(atten_corr_sds_id) == FAIL ||
        SDendaccess(model_speed_sds_id) == FAIL ||
        SDendaccess(model_dir_sds_id) == FAIL ||
        SDendaccess(num_ambigs_sds_id) == FAIL ||
        SDendaccess(wind_speed_sds_id) == FAIL ||
        SDendaccess(wind_dir_sds_id) == FAIL ||
        SDendaccess(max_likelihood_est_sds_id) == FAIL ||
        SDendaccess(wvc_selection_sds_id) == FAIL ||
        SDendaccess(wind_speed_selection_sds_id) == FAIL ||
        SDendaccess(wind_dir_selection_sds_id) == FAIL ||
        SDendaccess(mp_rain_probability_sds_id) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDendaccess\n");
        return(0);
    }

    //------------------------//
    // end access to the file //
    //------------------------//

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "L2B::ReadPureHdf: error with SDend\n");
        return(0);
    }

    // success!
    return(1);
}

//---------------//
// L2B::WriteHdf //
//---------------//

// attributes
Attribute* l2b_actual_wvc_rows = new Attribute("l2b_actual_wvc_rows", "int",
    "1", "<missing>");

// attribute table
Attribute* g_l2b_attribute_table[] =
{
    l2b_actual_wvc_rows,
    NULL
};

// dimension sizes
int32 l2b_dim_sizes_frame[] = { SD_UNLIMITED, 76, 4 };

// dimension names
const char* l2b_dim_names_frame[] = { "Wind_Vector_Cell_Row",
    "Wind_Vector_Cell", "Ambiguity" };

// SDS's
SdsInt16* wvc_row = new SdsInt16("wvc_row", 1, l2b_dim_sizes_frame, "counts",
    1.0, 0.0, l2b_dim_names_frame, 1624, 1);
SdsInt16* wvc_lat = new SdsInt16("wvc_lat", 2, l2b_dim_sizes_frame, "degrees",
    0.01, 0.0, l2b_dim_names_frame, 9000, -9000);
SdsUInt16* wvc_lon = new SdsUInt16("wvc_lon", 2, l2b_dim_sizes_frame, "degrees",
    0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt8* wvc_index = new SdsInt8("wvc_index", 2, l2b_dim_sizes_frame, "counts",
    1.0, 0.0, l2b_dim_names_frame, 76, 1);
SdsInt8* num_in_fore = new SdsInt8("num_in_fore", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_in_aft = new SdsInt8("num_in_aft", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_out_fore = new SdsInt8("num_out_fore", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsInt8* num_out_aft = new SdsInt8("num_out_aft", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 127, 0);
SdsUInt16* wvc_quality_flag = new SdsUInt16("wvc_quality_flag", 2,
    l2b_dim_sizes_frame, "n/a", 1.0, 0.0, l2b_dim_names_frame, 32643, 0);
SdsInt16* model_speed = new SdsInt16("model_speed", 2, l2b_dim_sizes_frame,
    "m/s", 0.01, 0.0, l2b_dim_names_frame, 7000, 0);
SdsUInt16* model_dir = new SdsUInt16("model_dir", 2, l2b_dim_sizes_frame,
    "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt8* num_ambigs = new SdsInt8("num_ambigs", 2, l2b_dim_sizes_frame,
    "counts", 1.0, 0.0, l2b_dim_names_frame, 4, 0);
SdsInt16* wind_speed = new SdsInt16("wind_speed", 3, l2b_dim_sizes_frame,
    "m/s", 0.01, 0.0, l2b_dim_names_frame, 5000, 0);
SdsUInt16* wind_dir = new SdsUInt16("wind_dir", 3, l2b_dim_sizes_frame,
    "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt16* max_likelihood_est = new SdsInt16("max_likelihood_est", 3,
    l2b_dim_sizes_frame, "n/a", 0.001, 0.0, l2b_dim_names_frame, -30000, 0);
SdsInt8* wvc_selection = new SdsInt8("wvc_selection", 2, l2b_dim_sizes_frame,
    "n/a", 1.0, 0.0, l2b_dim_names_frame, 4, 0);
SdsInt16* wind_speed_selection = new SdsInt16("wind_speed_selection", 2,
    l2b_dim_sizes_frame, "m/s", 0.01, 0.0, l2b_dim_names_frame, 7000, 0);
SdsUInt16* wind_dir_selection = new SdsUInt16("wind_dir_selection", 2,
    l2b_dim_sizes_frame, "deg", 0.01, 0.0, l2b_dim_names_frame, 35999, 0);
SdsInt16* mp_rain_probability = new SdsInt16("mp_rain_probability", 2,
    l2b_dim_sizes_frame, "n/a", 0.001, 0.0, l2b_dim_names_frame, 1000, -3000);

// SDS table
Sds* g_l2b_sds_table[] =
{
    wvc_row,
    wvc_lat,
    wvc_lon,
    wvc_index,
    num_in_fore,
    num_in_aft,
    num_out_fore,
    num_out_aft,
    wvc_quality_flag,
    model_speed,
    model_dir,
    num_ambigs,
    wind_speed,
    wind_dir,
    max_likelihood_est,
    wvc_selection,
    wind_speed_selection,
    wind_dir_selection,
    mp_rain_probability,
    NULL
};

int
L2B::WriteHdf(
    const char*  filename,
    int          unnormalize_mle_flag)
{
    WindSwath* swath = &(frame.swath);

    //----------------------//
    // open sds for writing //
    //----------------------//

    int32 sds_output_file_id = SDstart(filename, DFACC_CREATE);
    if (sds_output_file_id == FAIL)
    {
        fprintf(stderr, "L2B::WriteHdf: error with SDstart\n");
        return(0);
    }

    //-----------------------//
    // create the attributes //
    //-----------------------//

    for (int idx = 0; g_l2b_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_l2b_attribute_table[idx])->Write(sds_output_file_id))
        {
            return(0);
        }
    }

    //------------------//
    // create the SDS's //
    //------------------//

    for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_l2b_sds_table[idx];
        if (! sds->Create(sds_output_file_id))
        {
            fprintf(stderr, "L2B::WriteHdf: error creating SDS %d\n", idx);
            return(0);
        }
    }

    //-----------------------//
    // set up the attributes //
    //-----------------------//

    char buffer[1024];

    int at_max = swath->GetAlongTrackBins();
    sprintf(buffer, "%d", at_max);
    l2b_actual_wvc_rows->ReplaceContents(buffer);

    //----------------------//
    // write the attributes //
    //----------------------//

    for (int idx = 0; g_l2b_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_l2b_attribute_table[idx])->Write(sds_output_file_id))
        {
            return(0);
        }
    }

    for (int ati = 0; ati < at_max; ati++)
    {
        //--------------------//
        // set the swath data //
        //--------------------//

        int16 wvc_row_value = ati + 1;
        wvc_row->SetWithInt16(&wvc_row_value);

        float wvc_lat_value[HDF_ACROSS_BIN_NO];
        float wvc_lon_value[HDF_ACROSS_BIN_NO];
        int8 wvc_index_value[HDF_ACROSS_BIN_NO];
        int8 num_in_fore_value[HDF_ACROSS_BIN_NO];
        int8 num_in_aft_value[HDF_ACROSS_BIN_NO];
        int8 num_out_fore_value[HDF_ACROSS_BIN_NO];
        int8 num_out_aft_value[HDF_ACROSS_BIN_NO];
        uint16 wvc_quality_flag_value[HDF_ACROSS_BIN_NO];
        float model_speed_value[HDF_ACROSS_BIN_NO];
        float model_dir_value[HDF_ACROSS_BIN_NO];
        int8 num_ambigs_value[HDF_ACROSS_BIN_NO];
        float wind_speed_value[HDF_ACROSS_BIN_NO * HDF_NUM_AMBIGUITIES];
        float wind_dir_value[HDF_ACROSS_BIN_NO * HDF_NUM_AMBIGUITIES];
        float max_likelihood_est_value[HDF_ACROSS_BIN_NO*HDF_NUM_AMBIGUITIES];
        int8 wvc_selection_value[HDF_ACROSS_BIN_NO];
        float wind_speed_selection_value[HDF_ACROSS_BIN_NO];
        float wind_dir_selection_value[HDF_ACROSS_BIN_NO];
        float mp_rain_probability_value[HDF_ACROSS_BIN_NO];
        for (int cti = 0; cti < HDF_ACROSS_BIN_NO; cti++)
        {
            // init in case there is no WVC
            wvc_lat_value[cti] = 0.0;
            wvc_lon_value[cti] = 0.0;
            num_in_fore_value[cti] = 0;
            num_in_aft_value[cti] = 0;
            num_out_fore_value[cti] = 0;
            num_out_aft_value[cti] = 0;
            wvc_quality_flag_value[cti] = 32387;
            model_speed_value[cti] = 0.0;
            model_dir_value[cti] = 0.0;
            num_ambigs_value[cti] = 0;
            for (int idx = 0; idx < HDF_NUM_AMBIGUITIES; idx++)
            {
                wind_speed_value[cti * HDF_NUM_AMBIGUITIES + idx] = 0.0;
                wind_dir_value[cti * HDF_NUM_AMBIGUITIES + idx] = 0.0;
                max_likelihood_est_value[cti*HDF_NUM_AMBIGUITIES + idx] = 0.0;
            }
            wvc_selection_value[cti] = 0;
            wind_speed_selection_value[cti] = 0.0;
            wind_dir_selection_value[cti] = 0.0;
            mp_rain_probability_value[cti] = 0.0;

            // set the obvious
            wvc_index_value[cti] = cti + 1;

            // get the wvc
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            // set the rest
            wvc_lat_value[cti] = wvc->lonLat.latitude * rtd;
            wvc_lon_value[cti] = wvc->lonLat.longitude * rtd;
            num_in_fore_value[cti] = wvc->numInFore;
            num_in_aft_value[cti] = wvc->numInAft;
            num_out_fore_value[cti] = wvc->numOutFore;
            num_out_aft_value[cti] = wvc->numOutAft;
            wvc_quality_flag_value[cti] = wvc->rainFlagBits << 12;
            if (wvc->nudgeWV != NULL)
            {
                 model_speed_value[cti] = wvc->nudgeWV->spd
                     / NWP_SPEED_CORRECTION;
                 model_dir_value[cti] = pe_rad_to_gs_deg(wvc->nudgeWV->dir);
            }
            num_ambigs_value[cti] = wvc->ambiguities.NodeCount();

            int num_meas = wvc->numInFore + wvc->numInAft + wvc->numOutFore
                + wvc->numOutAft;
            int idx = 0;
            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                wind_speed_value[cti * HDF_NUM_AMBIGUITIES + idx] = wvp->spd;
                wind_dir_value[cti * HDF_NUM_AMBIGUITIES + idx] =
                    pe_rad_to_gs_deg(wvp->dir);
                float mle = wvp->obj;
                if (unnormalize_mle_flag)
                {
                    mle /= num_meas;
                }
                max_likelihood_est_value[cti * HDF_NUM_AMBIGUITIES + idx] =
                    mle;
                idx++;
            }

            wvc_selection_value[cti] =
                wvc->ambiguities.GetIndexOf(wvc->selected) + 1;
            wind_speed_selection_value[cti] = wvc->selected->spd;
            wind_dir_selection_value[cti] =
                pe_rad_to_gs_deg(wvc->selected->dir);
            mp_rain_probability_value[cti] = wvc->rainProb;
        }

        // set the sds
        wvc_lat->SetFromFloat(wvc_lat_value);
        wvc_lon->SetFromFloat(wvc_lon_value);
        wvc_index->SetWithChar(wvc_index_value);
        num_in_fore->SetWithChar(num_in_fore_value);
        num_in_aft->SetWithChar(num_in_aft_value);
        num_out_fore->SetWithChar(num_out_fore_value);
        num_out_aft->SetWithChar(num_out_aft_value);
        wvc_quality_flag->SetWithUnsignedShort(wvc_quality_flag_value);
        model_speed->SetFromFloat(model_speed_value);
        model_dir->SetFromFloat(model_dir_value);
        num_ambigs->SetWithChar(num_ambigs_value);
        wind_speed->SetFromFloat(wind_speed_value);
        wind_dir->SetFromFloat(wind_dir_value);
        max_likelihood_est->SetFromFloat(max_likelihood_est_value);
        wvc_selection->SetWithChar(wvc_selection_value);
        wind_speed_selection->SetFromFloat(wind_speed_selection_value);
        wind_dir_selection->SetFromFloat(wind_dir_selection_value);
        mp_rain_probability->SetFromFloat(mp_rain_probability_value);

        //-----------------//
        // write the SDS's //
        //-----------------//

        for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
        {
            Sds* sds = g_l2b_sds_table[idx];
            if (! sds->Write(ati))
            {
                fprintf(stderr, "L1AH::WriteSDSs: error writing SDS %s\n",
                    sds->GetName());
                return(0);
            }
        }
    }

    //----------------//
    // end sds access //
    //----------------//

    for (int idx = 0; g_l2b_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_l2b_sds_table[idx];
        if (! sds->EndAccess())
        {
            fprintf(stderr, "L1AH::EndSDSOutput: error ending SDS access\n");
            return(0);
        }
    }
    if (SDend(sds_output_file_id) != SUCCEED)
    {
        fprintf(stderr, "L1AH::EndSDSOutput: error with SDend\n");
        return(0);
    }

    return(1);
}

//--------------------//
// L2B::InsertPureHdf //
//--------------------//
// right now, this just inserts the selection

int
L2B::InsertPureHdf(
    const char*  input_filename,
    const char*  output_filename,
    int          unnormalize_mle_flag)
{
    //----------------------------------------//
    // copy the input file to the output file //
    //----------------------------------------//

    FILE* ifp = fopen(input_filename, "r");
    if (ifp == NULL)
        return(0);

    FILE* ofp = fopen(output_filename, "w");
    if (ofp == NULL)
        return(0);

    char c;
    while (fread(&c, sizeof(char), 1, ifp) == 1)
    {
        fwrite(&c, sizeof(char), 1, ofp);
    }
    if (! feof(ifp))
        return(0);    // error, not EOF

    fclose(ifp);

    //----------------------------------------//
    // some generic HDF start and edge arrays //
    //----------------------------------------//
    // the HDF write routines should only access as many dimensions as needed
    int32 generic_start[3] = { 0, 0, 0 };
    int32 generic_edges[3] = { 1, ROW_WIDTH, AMBIGS };

    //--------------------------//
    // start access to the file //
    //--------------------------//

    int32 sd_id = SDstart(output_filename, DFACC_WRITE);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDstart\n");
        return(0);
    }

    //-------------------------------//
    // get all the necessary SDS IDs //
    //-------------------------------//

    int32 wvc_selection_sds_id = SDnametoid(sd_id, "wvc_selection");

    //--------------//
    // for each row //
    //--------------//

    for (int ati = 0; ati < frame.swath.GetAlongTrackBins(); ati++)
    {
        //-----------------//
        // prepare the row //
        //-----------------//

        generic_start[0] = ati;

        //----------------------------------------//
        // assemble wind vector cells into arrays //
        //----------------------------------------//

        int8 wvc_selection[ROW_WIDTH];

        for (int cti = 0; cti < frame.swath.GetCrossTrackBins(); cti++)
        {
            //-------------------------------//
            // initialize the array elements //
            //-------------------------------//

            wvc_selection[cti] = 0;

            //-------------//
            // get the WVC //
            //-------------//

            WVC* wvc = frame.swath.GetWVC(ati, cti);
            if (wvc == NULL)
            {
                continue;
            }

            //----------------------------//
            // set the selected ambiguity //
            //----------------------------//

            int index = wvc->ambiguities.GetIndexOf(wvc->selected);
            if (index != -1)
                wvc_selection[cti] = index;
        }

        //---------------------------//
        // write the selection array //
        //---------------------------//

        if (SDwritedata(wvc_selection_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)wvc_selection) == FAIL)
        {
            fprintf(stderr, "L2B::InsertPureHdf: error with SDwritedata\n");
            return(0);
        }
    }

    if (SDendaccess(wvc_selection_sds_id) == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDendaccess\n");
        return(0);
    }

    //------------------------//
    // end access to the file //
    //------------------------//

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "L2B::InsertPureHdf: error with SDend\n");
        return(0);
    }

    // success!
    return(1);
}

//----------------//
// L2B::WriteVctr //
//----------------//

int
L2B::WriteVctr(
    const char*  filename,
    const int    rank)
{
    return(frame.swath.WriteVctr(filename, rank));
}

//-----------------//
// L2B::WriteAscii //
//-----------------//

int
L2B::WriteAscii()
{
    if (! header.WriteAscii(_outputFp))
        return(0);
    return(frame.swath.WriteAscii(_outputFp));
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    int  unnormalize_mle)
{
    //------//
    // read //
    //------//

    if (! ReadHDF(_inputFilename, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    const char*  filename,
    int          unnormalize_mle)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    //------//
    // read //
    //------//

    if (! ReadHDF(&l2b_hdf_file, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    TlmHdfFile*  tlmHdfFile,
    int          unnormalize_mle)
{
    printf("NOTICE: If you would like to try a cleaner Level 2B HDF\n");
    printf("        reader (i.e. not based on Sally's EA code), then\n");
    printf("        change L2B::ReadHDF to L2B::ReadPureHdf in your code\n");

    WindSwath* swath = &(frame.swath);
    swath->DeleteEntireSwath();    // just in case

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    // determine the inclination from the header (this is such a hack!)
    const char* filename = tlmHdfFile->GetFileName();
    int32 sd_id = SDstart(filename, DFACC_RDONLY);
    int32 attr_index = SDfindattr(sd_id, "orbit_inclination");
    if (attr_index == SUCCEED)
    {
        char attr_name[512];
        int32 adata_type, count;
        SDattrinfo(sd_id, attr_index, attr_name, &adata_type, &count);
        char inc[32];
        SDreadattr(sd_id, attr_index, inc);
        float inclination;
        char* ptr = strchr(inc, (int)'\n');
        ptr = strchr(ptr+1, (int)'\n');
        sscanf(ptr, " %f", &inclination);
        header.inclination = inclination * dtr;
    }
    SDend(sd_id);

    // continue on with whatever...
    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    // open all needed datasets
    if (! _OpenHdfDataSets(tlmHdfFile))
        return(0);

    //--------------//
    // set up swath //
    //--------------//

    int along_track_bins = dataLength;
    int cross_track_bins = HDF_ACROSS_BIN_NO;

// hack in high resolution
#ifdef HIRES12
    cross_track_bins = 152;
#endif

    if (! swath->Allocate(cross_track_bins, along_track_bins))
        return(0);

    //---------------//
    // create arrays //
    //---------------//

    unsigned char*  numambigArray = new unsigned char[cross_track_bins];
    float*          lonArray = new float[cross_track_bins];
    float*          rainArray = new float[cross_track_bins];
    float*          latArray = new float[cross_track_bins];
    float*          speedArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          dirArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          mleArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    char*           selectArray = new char[cross_track_bins];
    float*          modelSpeedArray = (float *) new float[cross_track_bins];
    float*          modelDirArray = (float *) new float[cross_track_bins];
    char*           tmpArray = new char[cross_track_bins];
    int*            numArray = (int *) new int[cross_track_bins];
    unsigned short int*  qualArray =
                      (unsigned short int *) new int[cross_track_bins];
    int32           sdsIds[1];

    for (int32 ati = 0; ati < along_track_bins; ati++)
    {
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] = 0;

        sdsIds[0] = _lonSdsId;
        if (ExtractData2D_76_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            lonArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _latSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            latArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _speedSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            speedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _dirSdsId;
        if (ExtractData3D_76_4_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            dirArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _mleSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            mleArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _selectSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, selectArray) == 0)
            return(0);

        sdsIds[0] = _numambigSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1,
            numambigArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelSpeedSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelSpeedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelDirSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelDirArray) == 0)
        {
            return(0);
        }
        sdsIds[0] = _mpRainProbSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            rainArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _qualSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1,
            qualArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _numInForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numInAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        for (int cti = 0; cti < cross_track_bins; cti++)
        {
            WVC* wvc = new WVC();
            wvc->rainProb=rainArray[cti];
            wvc->rainFlagBits=char((0x7000 & qualArray[cti])>>12);
            wvc->lonLat.longitude = lonArray[cti] * dtr;
            wvc->lonLat.latitude = latArray[cti] * dtr;
            wvc->nudgeWV = new WindVectorPlus();
            float nudge_edir = gs_deg_to_pe_rad(modelDirArray[cti]);
            wvc->nudgeWV->SetSpdDir(modelSpeedArray[cti] * NWP_SPEED_CORRECTION,
                nudge_edir);

            for (int k = 0; k < numambigArray[cti]; k++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                float edir = gs_deg_to_pe_rad(dirArray[cti
                    * HDF_NUM_AMBIGUITIES + k]);
                wvp->SetSpdDir(speedArray[cti * HDF_NUM_AMBIGUITIES + k], edir);
                if (unnormalize_mle)
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k] *
                        numArray[cti];
                }
                else
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k];
                }
                wvc->ambiguities.Append(wvp);
            }
            if (selectArray[cti] > 0 && numambigArray[cti] > 0)
            {
                wvc->selected =
                    wvc->ambiguities.GetByIndex(selectArray[cti] - 1);
                swath->Add(cti, ati, wvc);
            }
            else
            {
                delete wvc;
            }
        }
    }

    delete [] modelDirArray;
    delete [] modelSpeedArray;
    delete [] numambigArray;
    delete [] lonArray;
    delete [] latArray;
    delete [] speedArray;
    delete [] dirArray;
    delete [] mleArray;
    delete [] selectArray;
    delete [] numArray;
    delete [] tmpArray;

    // close all needed datasets
    _CloseHdfDataSets();

    swath->nudgeVectorsRead = 1;
    header.crossTrackResolution = 25.0;
    header.alongTrackResolution = 25.0;
    header.zeroIndex = 38;

    return(1);
}

//---------------------------------//
// L2B::ReadNudgeVectorsFromHdfL2B //
//---------------------------------//

int
L2B::ReadNudgeVectorsFromHdfL2B(
    const char*  filename)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    //------//
    // read //
    //------//

    if (! ReadNudgeVectorsFromHdfL2B(&l2b_hdf_file))
        return(0);

    return(1);
}

//---------------------------------//
// L2B::ReadNudgeVectorsFromHdfL2B //
//---------------------------------//

int
L2B::ReadNudgeVectorsFromHdfL2B(
    TlmHdfFile*  tlmHdfFile)
{
    WindSwath* swath = &(frame.swath);

    int crossTrackBins = HDF_ACROSS_BIN_NO;
#ifdef HIRES12
    crossTrackBins = 152;
#endif

    if (crossTrackBins != swath->GetCrossTrackBins())
    {
        fprintf(stderr,
            "ReadNudgeVectorsFromHdfL2B: crosstrackbins mismatch\n");
        return(0);
    }

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    int alongTrackBins = dataLength;

    // For now do not handle case in which WVC rows are missing
    if (alongTrackBins != swath->GetAlongTrackBins())
    {
        fprintf(stderr, "Unable to process missing WVC rows in HDF file\n");
        return(0);
    }

    // Open Nudge Vector Data Sets
    if ((_modelSpeedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_SPEED)) == 0)
    {
        return(0);
    }
    if ((_modelDirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_DIR)) == 0)
    {
        return(0);
    }

    float* modelSpeedArray=(float*) new float[crossTrackBins];
    float* modelDirArray=(float*) new float[crossTrackBins];

    int32 sdsIds[1];
    for (int32 ati = 0; ati < alongTrackBins; ati++)
    {
        sdsIds[0] = _modelSpeedSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelSpeedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelDirSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelDirArray) == 0)
        {
             return(0);
        }
        for (int cti = 0; cti < crossTrackBins; cti++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (! wvc)
                continue;
            wvc->nudgeWV = new WindVectorPlus();
            float nudge_edir = gs_deg_to_pe_rad(modelDirArray[cti]);
            wvc->nudgeWV->SetSpdDir(modelSpeedArray[cti] * NWP_SPEED_CORRECTION,
                nudge_edir);
        }
    }
    delete [] modelDirArray;
    delete [] modelSpeedArray;

    // close datasets
    (void)SDendaccess(_modelSpeedSdsId); _modelSpeedSdsId = HDF_FAIL;
    (void)SDendaccess(_modelDirSdsId); _modelDirSdsId = HDF_FAIL;

    swath->nudgeVectorsRead = 1;

    return(1);
}

//-----------------------------------//
// L2B::GetArraysForUpdatingDirthHdf //
//-----------------------------------//

int
L2B::GetArraysForUpdatingDirthHdf(
    float**  spd,
    float**  dir,
    int**    num_ambig)
{
    WindSwath* swath = &(frame.swath);
    int along_track_bins = swath->GetAlongTrackBins();
    int cross_track_bins = swath->GetCrossTrackBins();

    for (int cti = 0; cti < cross_track_bins; cti++)
    {
        for (int ati = 0; ati < along_track_bins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (! wvc)
            {
                num_ambig[ati][cti] = 0;
            }
            else
            {
                int k = 0;
                num_ambig[ati][cti] = wvc->ambiguities.NodeCount();
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    spd[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->spd;
                    dir[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->dir;
                    k++;
                }
            }
        }
    }
    return(1);
}

//-----------------------//
// L2B::_OpenHdfDataSets //
//-----------------------//

int
L2B::_OpenHdfDataSets(
    TlmHdfFile*  tlmHdfFile)
{
    if ((_numambigSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_AMBIGS)) == 0)
    {
        return(0);
    }
    if ((_lonSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LON)) == 0)
    {
        return(0);
    }
    if ((_latSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LAT)) == 0)
    {
        return(0);
    }
    if ((_speedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_SPEED)) == 0)
    {
        return(0);
    }
    if ((_dirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_DIR)) == 0)
    {
        return(0);
    }
    if ((_modelSpeedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_SPEED)) == 0)
    {
        return(0);
    }
    if ((_modelDirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_DIR)) == 0)
    {
        return(0);
    }
    if ((_mleSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MAX_LIKELIHOOD_EST)) == 0)
    {
        return(0);
    }
    if ((_selectSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WVC_SELECTION)) == 0)
    {
        return(0);
    }
    if ((_numInForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_FORE)) == 0)
    {
        return(0);
    }
    if ((_numInAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_AFT)) == 0)
    {
        return(0);
    }
    if ((_numOutForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_FORE)) == 0)
    {
        return(0);
    }
    if ((_numOutAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_AFT)) == 0)
    {
        return(0);
    }
    if ((_mpRainProbSdsId = _OpenOneHdfDataSetCorrectly(tlmHdfFile,
        "mp_rain_probability")) == 0)
    {
        return(0);
    }

    if ((_qualSdsId = _OpenOneHdfDataSetCorrectly(tlmHdfFile,
        "wvc_quality_flag")) == 0)
    {
        return(0);
    }
    return(1);
}

//-------------------------//
// L2B::_OpenOneHdfDataSet //
//-------------------------//

int
L2B::_OpenOneHdfDataSet(
    TlmHdfFile*  tlmHdfFile,
    SourceIdE    source,
    ParamIdE     param)
{
    const char* sdsName = ParTabAccess::GetSdsNames(source, param);
    if (sdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);
}

//-------------------//
// L2B::ReadHDFDIRTH //
//-------------------//

int
L2B::ReadHDFDIRTH(
    const char*  filename)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    // Open Data Sets
    int32 dirId;
    int32 spdId;
    if ((dirId = _OpenOneHdfDataSetCorrectly(&l2b_hdf_file,
        "wind_dir_selection")) == 0)
    {
        return(0);
    }
    if ((spdId = _OpenOneHdfDataSetCorrectly(&l2b_hdf_file,
        "wind_speed_selection")) == 0)
    {
        return(0);
    }

    int along_track_bins=frame.swath.GetAlongTrackBins();
    int cross_track_bins=frame.swath.GetCrossTrackBins();

    // create arrays
    float* speed = new float[cross_track_bins];
    float* dir = new float[cross_track_bins];

    for (int32 i = 0; i < along_track_bins; i++)
    {
        if (ExtractData2D_76_int2_float(&l2b_hdf_file, &spdId, i, 1, 1,
            speed) == 0)
        {
            return(0);
        }
        if (ExtractData2D_76_uint2_float(&l2b_hdf_file, &dirId, i, 1, 1,
            dir) == 0)
        {
            return(0);
        }
        for (int j = 0; j < cross_track_bins; j++)
        {
            WVC* wvc= frame.swath.GetWVC(j, i);
            if (! wvc)
                continue;
            if (! wvc->selected)
                continue;
            wvc->selected->spd = speed[j];
            float edir = gs_deg_to_pe_rad(dir[j]);
            wvc->selected->dir = edir;
        }
    }
    delete [] speed;
    delete [] dir;
    (void)SDendaccess(dirId);
    (void)SDendaccess(spdId);
    return(1);
}

//----------------------------------//
// L2B::_OpenOneHdfDataSetCorrectly //
//----------------------------------//

int
L2B::_OpenOneHdfDataSetCorrectly(
    TlmHdfFile*  tlmHdfFile,
    const char*  sdsName)
{
    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType, dataStartIndex,
        dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);
}

//------------------------//
// L2B::_CloseHdfDataSets //
//------------------------//

void
L2B::_CloseHdfDataSets(void)
{
    (void)SDendaccess(_numambigSdsId); _numambigSdsId = HDF_FAIL;
    (void)SDendaccess(_lonSdsId); _lonSdsId = HDF_FAIL;
    (void)SDendaccess(_latSdsId); _latSdsId = HDF_FAIL;
    (void)SDendaccess(_speedSdsId); _speedSdsId = HDF_FAIL;
    (void)SDendaccess(_dirSdsId); _dirSdsId = HDF_FAIL;
    (void)SDendaccess(_mleSdsId); _mleSdsId = HDF_FAIL;
    (void)SDendaccess(_selectSdsId); _selectSdsId = HDF_FAIL;
    (void)SDendaccess(_numInForeSdsId); _numInForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numInAftSdsId); _numInAftSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutForeSdsId); _numOutForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutAftSdsId); _numOutAftSdsId = HDF_FAIL;
    (void)SDendaccess(_modelSpeedSdsId); _modelSpeedSdsId = HDF_FAIL;
    (void)SDendaccess(_modelDirSdsId); _modelDirSdsId = HDF_FAIL;
    (void)SDendaccess(_mpRainProbSdsId); _mpRainProbSdsId = HDF_FAIL;
    (void)SDendaccess(_qualSdsId); _qualSdsId = HDF_FAIL;
    return;
}

//--------------------------//
// L2B::GetNumCellsSelected //
//--------------------------//

int
L2B::GetNumCellsSelected()
{
    return(frame.swath.GetNumCellsSelected());
}

//---------------------------------//
// L2B::GetNumCellsWithAmbiguities //
//---------------------------------//

int
L2B::GetNumCellsWithAmbiguities()
{
    return(frame.swath.GetNumCellsWithAmbiguities());
}
