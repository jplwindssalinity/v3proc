//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    land_map_expand
//
// SYNOPSIS
//    land_map_expand <input_land_map> <output_land_map>
//
// DESCRIPTION
//    Reads the land map and produces a simple land map.
//    0 = ocean only
//    1 = non-ocean or near land
//
// OPERANDS
//    The following operands are supported:
//      <input_land_map>  Input simple land map.
//      <output_land_map>    Output simple land map.
//
// EXAMPLES
//    An example of a command line is:
//      % land_map_expand landmap.dat outmap.dat
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
// AUTHOR
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "Misc.h"
#include "LandMap.h"
#include "Array.h"
#include "Constants.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define FINE_GRID_LON_DEG    0.01
#define FINE_GRID_LAT_DEG    0.01
#define COURSE_GRID_LON_DEG  0.5
#define COURSE_GRID_LAT_DEG  0.5
#define KM_RANGE             100.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

float  lonlatdist(float lon1, float lat1, float lon2, float lat2);
int    writemap(const char* filename, char** map, int lon_samples,
           int lat_samples, float lon_res, float lat_res);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<input_land_map>", "<output_land_map>", 0 };
const float radius = 6378.0;
const float pi_2 = M_PI / 2.0;

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
    if (argc != 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* input_land_map_file = argv[clidx++];
    const char* output_land_map_file = argv[clidx++];

    //------------------//
    // read in land map //
    //------------------//

    SimpleLandMap input_land_map;
    if (! input_land_map.Read((char *)input_land_map_file))
    {
        fprintf(stderr, "%s: error reading land map file %s\n", command,
            input_land_map_file);
        exit(1);
    }

    //--------------------------//
    // generate output land map //
    //--------------------------//
    // elements are accessed by truncation

    int lon_samples = input_land_map.GetLonSamples();
    int lat_samples = input_land_map.GetLatSamples();
    SimpleLandMap output_land_map;
    output_land_map.Allocate(lon_samples, lat_samples);
    output_land_map.Zero();

    char** input_map = input_land_map.GetMap();
    char** output_map = output_land_map.GetMap();

    float lon_res = two_pi / lon_samples;
    float lat_res = pi / lat_samples;

    //---------------------------------------//
    // expand land out by specified distance //
    //---------------------------------------//

    for (int lon_idx = 0; lon_idx < lon_samples; lon_idx++)
    {
        // lon is at center of bin
        float lon = ((float)lon_idx + 0.5) * lon_res;
        for (int lat_idx = 0; lat_idx < lat_samples; lat_idx++)
        {
            // lat is at center of bin
            float lat = ((float)lat_idx + 0.5) * lat_res - pi_2;

            // only spread land
            if (*(*(input_map + lon_idx) + lat_idx) != 1)
                continue;

            int dlon_idx;
            if (fabs(lat * rtd) > 85.0)
            {
                dlon_idx = lon_samples;
            }
            else
            {
                float dlon = KM_RANGE / (radius * cos(lat));
                dlon_idx = (int)(dlon / lon_res) + 1;
            }
            float dlat = KM_RANGE / radius;
            int dlat_idx = (int)(dlat / lat_res) + 1;

            //------------------------------------------------//
            // extend distance to cover diagonal quantization //
            //------------------------------------------------//

            int diag_lat_idx;
            if (lat_idx < lat_samples / 2)
            {
                // southern hemi. - add to get closer to equator
                diag_lat_idx = lat_idx + 1;
            }
            else
            {
                // northern hemi. - subtract to get closer to equator
                diag_lat_idx = lat_idx - 1;
            }
            int diag_lon_idx = (lon_idx + 1) % lon_samples;
            float add_dist = lonlatdist(lon, lat,
                ((float)diag_lon_idx + 0.5) * lon_res,
                ((float)diag_lat_idx + 0.5) * lat_res - pi_2);

            int start_try_lon_idx = lon_idx - dlon_idx;
            int end_try_lon_idx = lon_idx + dlon_idx + 1;
            int start_try_lat_idx = lat_idx - dlat_idx;
            int end_try_lat_idx = lat_idx + dlat_idx + 1;
            if (start_try_lat_idx < 0)
                start_try_lat_idx = 0;
            if (end_try_lat_idx >= lat_samples)
                end_try_lat_idx = lat_samples - 1;

            //-----------------------------//
            // check over reasonable range //
            //-----------------------------//

            for (int try_lon_idx = start_try_lon_idx;
                try_lon_idx < end_try_lon_idx; try_lon_idx++)
            {
                int use_lon_idx = (try_lon_idx + lon_samples) %
                    lon_samples;
                float try_lon = ((float)use_lon_idx + 0.5) * lon_res;
                for (int try_lat_idx = start_try_lat_idx;
                    try_lat_idx < end_try_lat_idx; try_lat_idx++)
                {
                    int use_lat_idx = try_lat_idx;
                    float try_lat = ((float)use_lat_idx + 0.5) *
                        lat_res - pi_2;
                    float dist = lonlatdist(try_lon, try_lat, lon, lat);
                    if (dist < KM_RANGE + add_dist)
                    {
                        *(*(output_map + use_lon_idx) + use_lat_idx) = 1;
                    }
                }
            }
        }
    }
    output_land_map.Write(output_land_map_file);
    writemap("map.exp", output_map, lon_samples, lat_samples,
        lon_res, lat_res);

    return(0);
}

//------------//
// lonlatdist //
//------------//
// approximate great circle distance

float
lonlatdist(
    float  lon1,
    float  lat1,
    float  lon2,
    float  lat2)
{
    float c = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1);
    float a = atan(sqrt(1.0 - c * c) / c) + M_PI * (c - fabs(c)) / (2.0 * c);
    return(a * radius);
}

//----------//
// writemap //
//----------//

int
writemap(
    const char*  filename,
    char**       map,
    int          lon_samples,
    int          lat_samples,
    float        lon_res,
    float        lat_res)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    for (int i = 0; i < lon_samples; i++)
    {
        float lon = ((float)i + 0.5) * lon_res;
        for (int j = 0; j < lat_samples; j++)
        {
            float lat = ((float)j + 0.5) * lat_res - pi_2;
            if (*(*(map + i) + j))
                fprintf(ofp, "%g %g\n", lon, lat);
        }
    }

    fclose(ofp);
    return(1);
}
