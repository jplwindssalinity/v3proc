//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    land_map_convert
//
// SYNOPSIS
//    land_map_convert <original_land_map> <output_land_map>
//
// DESCRIPTION
//    Reads the land map and produces a simple land map.
//    0 = ocean only
//    1 = non-ocean or near land
//
// OPERANDS
//    The following operands are supported:
//      <original_land_map>  Input original land map.
//      <output_land_map>    Output simple land map.
//
// EXAMPLES
//    An example of a command line is:
//      % land_map_convert landmap.dat outmap.dat
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
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1BHdf.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"


using std::list;
using std::map;

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;


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

int    writemap(const char* filename, char** map, int lon_samples,
           int lat_samples, float lon_res, float lat_res);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<original_cfg_file>", "<output_land_map>", 0 };
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
    const char* original_cfg_file = argv[clidx++];
    const char* output_land_map_file = argv[clidx++];
    ConfigList cfg;
    if(!cfg.Read(original_cfg_file)){
      fprintf(stderr,"Error reading cfg file %s\n",original_cfg_file);
      exit(1);
    }
    //------------------//
    // read in land map //
    //------------------//

    LandMap original_land_map;
    if (!ConfigLandMap(&original_land_map, &cfg))
    {
        fprintf(stderr, "%s: error configuring land map\n", command);
        exit(1);
    }

    //--------------------------//
    // generate output land map //
    //--------------------------//
    // elements are accessed by truncation

    int coarse_lon_samples = (int)(two_pi / (COURSE_GRID_LON_DEG * dtr) + 0.5);
    int coarse_lat_samples = (int)(pi / (COURSE_GRID_LAT_DEG * dtr) + 0.5);
    float coarse_lon_res = two_pi / coarse_lon_samples;
    float coarse_lat_res = pi / coarse_lat_samples;

    SimpleLandMap smap_1;
    SimpleLandMap smap_2;

    smap_1.Allocate(coarse_lon_samples, coarse_lat_samples);
    char** map_1 = smap_1.GetMap();

    smap_2.Allocate(coarse_lon_samples, coarse_lat_samples);
    char** map_2 = smap_2.GetMap();

    for (int lon_idx = 0; lon_idx < coarse_lon_samples; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < coarse_lat_samples; lat_idx++)
        {
            *(*(map_1 + lon_idx) + lat_idx) = 0;
            *(*(map_2 + lon_idx) + lat_idx) = 0;
        }
    }

    //------------------//
    // fill in land map //
    //------------------//

    int fine_lon_samples = (int)(two_pi / (FINE_GRID_LON_DEG * dtr) + 0.5) + 1;
    int fine_lat_samples = (int)(pi / (FINE_GRID_LAT_DEG * dtr) + 0.5) + 1;
    float fine_lon_res = two_pi / fine_lon_samples;
    float fine_lat_res = pi / fine_lat_samples;

    for (int lon_idx = 0; lon_idx < fine_lon_samples; lon_idx++)
    {
        float lon = ((float)lon_idx + 0.5) * fine_lon_res;
        int coarse_lon_idx = (int)(lon / coarse_lon_res);
        coarse_lon_idx = (coarse_lon_idx + coarse_lon_samples) %
            coarse_lon_samples;
        for (int lat_idx = 0; lat_idx < fine_lat_samples; lat_idx++)
        {
            float lat = ((float)lat_idx + 0.5) * fine_lat_res - pi_2;
            int coarse_lat_idx = (int)((lat + pi_2) / coarse_lat_res);
            if (coarse_lat_idx < 0)
                coarse_lat_idx = 0;
            if (coarse_lat_idx >= coarse_lat_samples)
                coarse_lat_idx = coarse_lat_samples - 1;
            if (original_land_map.IsLand(lon, lat))
            {
                *(*(map_1 + coarse_lon_idx) + coarse_lat_idx) = 1;
            }
        }
    }

    writemap("map.1", map_1, coarse_lon_samples, coarse_lat_samples,
        coarse_lon_res, coarse_lat_res);

    smap_1.Write(output_land_map_file);

    return(0);
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
