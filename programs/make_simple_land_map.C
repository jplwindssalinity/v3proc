//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    make_simple_land_map
//
// SYNOPSIS
//    make_simple_land_map <original_land_map> <simple_land_map>
//
// DESCRIPTION
//    Reads the land map and produces a simple land map.
//    0 = ocean
//    1 = land
//    2 = coast or near coast
//
// OPERANDS
//    The following operands are supported:
//      <original_land_map>  Input original land map.
//      <output_land_map>    Output simple land map.
//
// EXAMPLES
//    An example of a command line is:
//      % make_simple_land_map landmap.dat outmap.dat
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

#define FINE_GRID_LON_KM     5.0
#define FINE_GRID_LAT_KM     5.0
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
int  writemap(const char* filename, char** map, int lon_samples,
         int lat_samples, float lon_res, float lat_res);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<original_land_map>", "<output_land_map>", 0 };
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
    const char* original_land_map_file = argv[clidx++];
    const char* output_land_map_file = argv[clidx++];

    //------------------//
    // read in land map //
    //------------------//

    LandMap original_land_map;
    if (! original_land_map.Initialize((char *)original_land_map_file, 1))
    {
        fprintf(stderr, "%s: error reading land map file %s\n", command,
            original_land_map_file);
        exit(1);
    }

    //------------------------//
    // create output land map //
    //------------------------//
    // elements are accessed by truncation

    int lon_samples = (int)(360.0 / COURSE_GRID_LON_DEG + 0.5);
    int lat_samples = (int)(180.0 / COURSE_GRID_LON_DEG + 0.5);
    SimpleLandMap output_land_map;
    output_land_map.Allocate(lon_samples, lat_samples);
    output_land_map.Fill(3);    // initialize as unknown

    char** output_map = output_land_map.GetMap();

    float lon_res = two_pi / lon_samples;
    float lat_res = pi / lat_samples;

    //-------------------------//
    // transfer and expand map //
    //-------------------------//

    for (int lon_idx = 0; lon_idx < lon_samples; lon_idx++)
    {
        // lon is at center of bin
        float lon = ((float)lon_idx + 0.5) * lon_res;
        lon = lon_fix(lon);
        for (int lat_idx = 0; lat_idx < lat_samples; lat_idx++)
        {
            // lat is at center of bin
            float lat = ((float)lat_idx + 0.5) * lat_res - pi_2;
            lat = lat_fix(lat);

            //-------------------------------------//
            // determine range of lon/lat to check //
            //-------------------------------------//

            float dlon, fine_grid_lon_deg;
            if (fabs(lat * rtd) > 85.0)
            {
                fine_grid_lon_deg = FINE_GRID_LON_KM /
                    (radius * cos(85.0 * dtr));
                dlon = pi;
            }
            else
            {
                fine_grid_lon_deg = FINE_GRID_LON_KM / (radius * cos(lat));
                dlon = KM_RANGE / (radius * cos(lat));
            }

            float dlat = KM_RANGE / radius;
            float fine_grid_lat_deg = FINE_GRID_LAT_KM / radius;

            float start_try_lon = lon - dlon;
            float end_try_lon = lon + dlon;
            while (start_try_lon < 0.0)
                start_try_lon += two_pi;
            while (end_try_lon < start_try_lon)
                end_try_lon += two_pi;

            float start_try_lat = lat - dlat;
            float end_try_lat = lat + dlat;
            if (start_try_lat < -pi_over_two)
            {
                start_try_lat = -pi_over_two;
                end_try_lon = start_try_lon + two_pi;
            }
            if (end_try_lat > pi_over_two)
            {
                end_try_lat = pi_over_two;
                end_try_lon = start_try_lon + two_pi;
            }

            //-----------------------//
            // initialize output map //
            //-----------------------//

            if (original_land_map.IsLand(lon, lat))
                *(*(output_map + lon_idx) + lat_idx) = 1;
            else
                *(*(output_map + lon_idx) + lat_idx) = 0;

            //-----------------------------//
            // check over reasonable range //
            //-----------------------------//

            for (float try_lon = start_try_lon; try_lon <= end_try_lon;
                try_lon += fine_grid_lon_deg)
            {
                float use_try_lon = lon_fix(try_lon);
                for (float try_lat = start_try_lat; try_lat <= end_try_lat;
                    try_lat += fine_grid_lat_deg)
                {
                    float use_try_lat = lat_fix(try_lat);
                    int output_flag = *(*(output_map + lon_idx) +
                        lat_idx);
                    if (output_flag == 2)    // mixed
                    {
                        goto done_with_cell;    // nothing can change this
                    }

                    float dist = lonlatdist(use_try_lon, use_try_lat, lon,
                        lat);
                    if (dist > KM_RANGE)
                        continue;

                    int is_land = original_land_map.IsLand(use_try_lon,
                        use_try_lat);
                    if ((output_flag == 1 && ! is_land) ||
                        (output_flag == 0 && is_land) )
                    {
                        // mixed
                        *(*(output_map + lon_idx) + lat_idx) = 2;
                        goto done_with_cell;
                    }
                }
            }
            done_with_cell:
                ;
        }
    }
    output_land_map.Write(output_land_map_file);
    writemap("simplemap.exp", output_map, lon_samples, lat_samples,
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

    for (int target_val = 1; target_val < 4; target_val++)
    {
      for (int i = 0; i < lon_samples; i++)
      {
          float lon = ((float)i + 0.5) * lon_res;
          for (int j = 0; j < lat_samples; j++)
          {
              float lat = ((float)j + 0.5) * lat_res - pi_2;
              if (*(*(map + i) + j) == target_val)
                  fprintf(ofp, "%g %g\n", lon, lat);
          }
      }
      fprintf(ofp, "&\n");
    }

    fclose(ofp);
    return(1);
}
