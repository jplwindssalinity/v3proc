//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    rain_apts
//
// SYNOPSIS
//    rain_apts [ -i ] [ -m minutes ] <rain_file> <mudh_file>
//        <output_base>
//
// DESCRIPTION
//    Generates apts files of rain rate and integrated rain rate.
//
// OPTIONS
//    [ -i ]          Generate integrated rain rate files.
//    [ -m minutes ]  Collocated within minutes.
//
// OPERANDS
//    <rain_file>    The input rain file.
//    <mudh_file>    The matching MUDH file (for lon and lat)
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % rain_apts -i 1208.rain 1208.mud 1208
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
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "Interpolate.h"
#include "SeaPac.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "im:"
#define ATI_SIZE     1624
#define CTI_SIZE     76

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -i ]", "[ -m minutes ]", "<rain_file>",
    "<mudh_file>", "<output_base>", 0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    int opt_irr = 0;
    int minutes = 60;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'i':
            opt_irr = 1;
            break;
        case 'm':
            minutes = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* rain_file = argv[optind++];
    const char* mudh_file = argv[optind++];
    const char* output_base = argv[optind++];

    //----------------//
    // read mudh file //
    //----------------//

    unsigned char  nbd_array[ATI_SIZE][CTI_SIZE];
    unsigned char  spd_array[ATI_SIZE][CTI_SIZE];
    unsigned char  dir_array[ATI_SIZE][CTI_SIZE];
    unsigned char  mle_array[ATI_SIZE][CTI_SIZE];
    unsigned short lon_array[ATI_SIZE][CTI_SIZE];
    unsigned short lat_array[ATI_SIZE][CTI_SIZE];

    FILE* mudh_ifp = fopen(mudh_file, "r");
    if (mudh_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    unsigned long size = CTI_SIZE * ATI_SIZE;
    if (fread(nbd_array,  sizeof(char), size, mudh_ifp) != size ||
        fread(spd_array,  sizeof(char), size, mudh_ifp) != size ||
        fread(dir_array,  sizeof(char), size, mudh_ifp) != size ||
        fread(mle_array,  sizeof(char), size, mudh_ifp) != size ||
        fread(lon_array, sizeof(short), size, mudh_ifp) != size ||
        fread(lat_array, sizeof(short), size, mudh_ifp) != size)
    {
        fclose(mudh_ifp);
        fprintf(stderr, "%s: error reading MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    fclose(mudh_ifp);

    //----------------//
    // read rain file //
    //----------------//

    unsigned char rain_rate[ATI_SIZE][CTI_SIZE];
    unsigned char time_dif[ATI_SIZE][CTI_SIZE];
    unsigned char which_ssmi[ATI_SIZE][CTI_SIZE];
    unsigned short integrated_rain_rate[ATI_SIZE][CTI_SIZE];

    FILE* rain_ifp = fopen(rain_file, "r");
    if (rain_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening rain file %s\n", command,
            rain_file);
        exit(1);
    }
    fread(rain_rate, sizeof(char), CTI_SIZE * ATI_SIZE, rain_ifp);
    fread(time_dif, sizeof(char), CTI_SIZE * ATI_SIZE, rain_ifp);
    fread(which_ssmi, sizeof(char), CTI_SIZE * ATI_SIZE, rain_ifp);
    fread(integrated_rain_rate, sizeof(short), CTI_SIZE * ATI_SIZE, rain_ifp);
    fclose(rain_ifp);

    //---------------------------------------//
    // eliminate rain data out of time range //
    //---------------------------------------//

    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            int co_time = time_dif[ati][cti] * 2 - 180;
            if (abs(co_time) > minutes)
            {
                // flag as bad
                rain_rate[ati][cti] = 255;
                integrated_rain_rate[ati][cti] = 2000;
            }
        }
    }

    //-------------------------//
    // generate rain rate file //
    //-------------------------//

    char filename[2048];

    sprintf(filename, "%s.rr.apts", output_base);
    FILE* rr_ofp = fopen(filename, "w");
    if (rr_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening rain rate output file %s\n",
            command, filename);
        exit(1);
    }
    fprintf(rr_ofp, "apts\n");
    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            if (rain_rate[ati][cti] >= 250)
                continue;
            float lon = (float)lon_array[ati][cti] * 0.01;
            float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
            float rr = rain_rate[ati][cti] * 0.1;
            fprintf(rr_ofp, "%g %g %g\n", lon, lat, rr);
        }
    }
    fclose(rr_ofp);

    //------------------------------------//
    // generate integrated rain rate file //
    //------------------------------------//

    sprintf(filename, "%s.irr.apts", output_base);
    FILE* irr_ofp = fopen(filename, "w");
    if (irr_ofp == NULL)
    {
        fprintf(stderr,
            "%s: error opening integrated rain rate output file %s\n",
            command, filename);
        exit(1);
    }
    fprintf(irr_ofp, "apts\n");
    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            if (integrated_rain_rate[ati][cti] >= 1000)
                continue;
            float lon = (float)lon_array[ati][cti] * 0.01;
            float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
            float irr = integrated_rain_rate[ati][cti] * 0.1;
            fprintf(irr_ofp, "%g %g %g\n", lon, lat, irr);
        }
    }
    fclose(irr_ofp);

    return (0);
}
