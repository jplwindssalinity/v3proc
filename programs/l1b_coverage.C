//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1b_coverage
//
// SYNOPSIS
//    l1b_coverage <land_map> <l1b_file> <output_array>
//
// DESCRIPTION
//    Reads in a land map and an L1B file and generates an array
//    suitable for conversion into a jpeg. The values in the array
//    are as follows:
//      0 = missed ocean
//      1 = missed land
//      2 = covered ocean
//      3 = covered land
//
// OPTIONS
//    The following options are supported:
//      none.
//
// OPERANDS
//    The following operands are supported:
//      <land_map>      A land map.
//      <l1b_file>      An L1B file.
//      <output_array>  The output array file
//
// EXAMPLES
//    An example of a command line is:
//      % l1b_coverage /home/sim/data/landmap.dat L1B.dat output.arr
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    James N. Huddleston <mailto:James.N.Huddleston@jpl.nasa.gov>
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <string.h>
#include "hdf.h"
#include "mfhdf.h"
#include "LandMap.h"
#include "Index.h"
#include "Constants.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

#define LON_BINS  720
#define LON_MIN   0.0
#define LON_MAX   360.0

#define LAT_BINS  360
#define LAT_MIN   -90.0
#define LAT_MAX   90.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

const char* no_path(const char* string);
void usage(const char* command, const char* option_array[],
    const int exit_value);
int32 SDnametoid(int32 sd_id, char* sds_name);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<land_map>", "<l1b_file>", "<output_array>",
    0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    extern char* optarg;
    extern int optind;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 3)
        usage(command, usage_array, 1);

    const char* land_map_file = argv[optind++];
    const char* l1b_file = argv[optind++];
    const char* output_file = argv[optind++];

    //-------------------//
    // read the land map //
    //-------------------//

    LandMap land_map;
    if (! land_map.Initialize((char *)land_map_file, 1))
    {
        fprintf(stderr, "%s: error reading land map file %s\n", command,
            land_map_file);
        exit(1);
    }

    //----------------------//
    // initialize the array //
    //----------------------//

    Index lon_index;
    lon_index.SpecifyWrappedCenters(LON_MIN, LON_MAX, LON_BINS);
    Index lat_index;
    lat_index.SpecifyEdges(LAT_MIN, LAT_MAX, LAT_BINS);
    float array[LON_BINS][LAT_BINS];
    for (int lon_idx = 0; lon_idx < LON_BINS; lon_idx++)
    {
        float lon;
        lon_index.IndexToValue(lon_idx, &lon);
        lon *= dtr;
        for (int lat_idx = 0; lat_idx < LAT_BINS; lat_idx++)
        {
            float lat;
            lat_index.IndexToValue(lat_idx, &lat);
            lat *= dtr;

            if (land_map.IsLand(lon, lat))
            {
                array[lon_idx][lat_idx] = 1;
            }
            else
            {
                array[lon_idx][lat_idx] = 0;
            }
        }
    }

    //-------------------//
    // read the L1B file //
    //-------------------//

    int32 sd_id = SDstart(l1b_file, DFACC_READ);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "%s: error opening HDF file %s for reading\n",
            command, l1b_file);
        exit(1);
    }

    int32 attr_index_l1b_actual_frame = SDfindattr(sd_id, "l1b_actual_frames");

    char data[1024];
    if (SDreadattr(sd_id, attr_index_l1b_actual_frame, data) == FAIL)
    {
        fprintf(stderr,
            "%s: error reading attribute for l1b_actual_frames\n", command);
        exit(1);
    }

    int frame_count = 0;
    if (sscanf(data, " %*[^\n] %*[^\n] %d", &frame_count) != 1)
    {
        fprintf(stderr, "%s: error parsing l1b_actual_frame attribute\n",
            command);
        fprintf(stderr, "%s\n", data);
        exit(1);
    }

    int32 frame_err_status_sds_id = SDnametoid(sd_id, "frame_err_status");
    int32 frame_qual_flag_sds_id = SDnametoid(sd_id, "frame_qual_flag");
    int32 cell_lat_sds_id = SDnametoid(sd_id, "cell_lat");
    int32 cell_lon_sds_id = SDnametoid(sd_id, "cell_lon");

    int32 start[3] = { 0, 0 };
    int32 edges[3] = { 1, 100 };

    for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
    {
        start[0] = frame_idx;

        uint32 frame_err_status;
        if (SDreaddata(frame_err_status_sds_id, start,
            NULL, edges, (VOIDP)&frame_err_status) == FAIL)
        {
            fprintf(stderr,
                "%s: error reading SD data for frame_err_status (ID=%d)\n",
                command, (int)frame_err_status_sds_id);
            exit(1);
        }

        //--------------------
        // good frames have a frame_err_status of zero
        //--------------------

        if (frame_err_status != 0)
        {
            //--------------------
            // There is something evil in this frame. Tell the
            // user and go on to the next frame.
            //--------------------

            fprintf(stderr,
                "%s: frame %d is evil. (error status = 0x%08x)\n",
                command, frame_idx, (unsigned int)frame_err_status);
            continue;
        }

        //--------------------
        // The frame quality flag is similar to the frame error
        // status.
        //--------------------

        uint16 frame_qual_flag;
        if (SDreaddata(frame_qual_flag_sds_id, start,
            NULL, edges, (VOIDP)&frame_qual_flag) == FAIL)
        {
            fprintf(stderr,
                "%s: error reading SD data for frame_qual_flag\n",
                command);
            exit(1);
        }

        if (frame_qual_flag != 0)
        {
            //--------------------
            // This frame it just not up to our high standards
            // of "quality". No, I don't know what that means.
            // Let's dump it anyway.
            //--------------------

            fprintf(stderr,
                "%s: frame %d is evil. (quality flag = %0x)\n", command,
                frame_idx, frame_qual_flag);
            continue;
        }

        float32 cell_lat[100];
        if (SDreaddata(cell_lat_sds_id, start, NULL, edges,
            (VOIDP)cell_lat) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for cell_lat\n",
                command);
            exit(1);
        }

        float32 cell_lon[100];
        if (SDreaddata(cell_lon_sds_id, start, NULL, edges,
            (VOIDP)cell_lon) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for cell_lon\n",
                command);
            exit(1);
        }

        for (int i = 0; i < 100; i++)
        {
            int lat_idx;
            lat_index.GetNearestIndexClipped(cell_lat[i], &lat_idx);
            int lon_idx;
            lon_index.GetNearestIndexClipped(cell_lon[i], &lon_idx);

            if (array[lon_idx][lat_idx] == 0)
                array[lon_idx][lat_idx] = 2;
            else if (array[lon_idx][lat_idx] == 1)
                array[lon_idx][lat_idx] = 3;
        }
    }

    //--------------------
    // We are almost done with this file. But first, we MUST
    // end our access to all of the SDSs.
    //--------------------

    if (SDendaccess(frame_err_status_sds_id) == FAIL ||
        SDendaccess(frame_qual_flag_sds_id) == FAIL ||
        SDendaccess(cell_lat_sds_id) == FAIL ||
        SDendaccess(cell_lon_sds_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending SD access\n", command);
        exit(1);
    }

    //--------------------
    // Finally, we can say goodbye to this file. Buh-bye!
    //--------------------

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending SD\n", command);
        exit(1);
    }

    //-----------------//
    // write the array //
    //-----------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }
    int x_size = LON_BINS;
    int y_size = LAT_BINS;
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(array, sizeof(float), LAT_BINS * LON_BINS, ofp) !=
        LAT_BINS * LON_BINS)
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            output_file);
        exit(1);
    }
    fclose(ofp);

    return (0);
}

//--------------------
// The following are three helper functions. Normally they would be
// put into a library, but if you leave them here you won't have to
// worry about linking.
//--------------------

//---------//
// no_path //
//---------//

const char*
no_path(
    const char*  string)
{
    const char* last_slash = strrchr(string, '/');
    if (! last_slash)
        return(string);
    return(last_slash + 1);
}

#define LINE_LENGTH  78

//-------//
// usage //
//-------//

void
usage(
    const char*  command,
    const char*  option_array[],
    const int    exit_value)
{
    fprintf(stderr, "usage: %s", command);
    int skip = 11;
    int position = 7 + strlen(command);
    for (int i = 0; option_array[i]; i++)
    {
        int length = 1 + strlen(option_array[i]);
        position += length;
        if (position > LINE_LENGTH)
        {
            fprintf(stderr, "\n%*s", skip, " ");
            position = skip + length;
        }
        fprintf(stderr, " %s", option_array[i]);
    }
    fprintf(stderr, "\n");
    exit(exit_value);
}

//------------//
// SDnametoid //
//------------//

int32
SDnametoid(
    int32  sd_id,
    char*  sds_name)
{
    int32 sds_index = SDnametoindex(sd_id, sds_name);
    if (sds_index == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD name (%s) to index\n",
            sds_name);
        exit(1);
    }
    int32 sds_id = SDselect(sd_id, sds_index);
    if (sds_id == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD index (%ld) to ID\n",
            sds_index);
        exit(1);
    }
    return (sds_id);
}
