//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_flag_rates
//
// SYNOPSIS
//    mudh_flag_rates [ -p spd:spd ] [ -s ssmi_dir ] [ -c time ]
//        [ -m mudh_dir ] <flag_dir> <start_rev> <end_rev>
//        <output_base>
//
// DESCRIPTION
//    Generates plots of rainflag rates on the earth for both
//    SSM/I and QSCAT.
//
// OPTIONS
//    [ -p spd:spd ]   Restrict the output to a speed range.
//    [ -m mudh_dir ]  The location of the MUDH files.
//    [ -s ssmi_dir ]  The location of the SSM/I irr files.
//    [ -c time ]      Collocation time for IRR.
//
// OPERANDS
//    <flag_dir>     Duh.
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  Triple duh.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_flag_rates -c 30 . 1500 1600 1500-1600
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
#include <ieeefp.h>
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

#include "mudh.h"

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

#define OPTSTRING         "c:m:p:s:"
#define BIG_DIM           100
#define QUOTE             '"'
#define REV_DIGITS        5

#define DEFAULT_MUDH_DIR  "/export/svt11/hudd/allmudh"
#define DEFAULT_IRR_DIR   "/export/svt11/hudd/ssmi"

#define LON_BINS  720
#define LON_MIN   0.0
#define LON_MAX   360.0
#define LAT_BINS  361
#define LAT_MIN   -90.0
#define LAT_MAX   90.0

#define IRR_THRESH  2.0    // must be GREATER then (but not equal to)
#define MIN_POINTS  50    // need this many points to estimate a %

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_spd = 0;
int opt_collocate = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c min ]", "[ -m mudh_dir ]",
    "[ -p spd:spd ]", "[ -s ssmi_dir ]", "<flag_dir>", "<start_rev>",
    "<end_rev>", "<output_base>", 0 };

unsigned long total_qscat_count[LON_BINS][LAT_BINS];
unsigned long qscat_rain_count[LON_BINS][LAT_BINS];

unsigned long qscat_inner_total_count[LON_BINS][LAT_BINS];
unsigned long qscat_inner_rain_count[LON_BINS][LAT_BINS];
unsigned long qscat_outer_total_count[LON_BINS][LAT_BINS];
unsigned long qscat_outer_rain_count[LON_BINS][LAT_BINS];

unsigned long total_ssmi_count[LON_BINS][LAT_BINS];
unsigned long ssmi_rain_count[LON_BINS][LAT_BINS];

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

    int collocation_time = 0;

    char* mudh_dir = DEFAULT_MUDH_DIR;
    char* irr_dir = DEFAULT_IRR_DIR;

    float min_spd = 0.0;
    float max_spd = 0.0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'm':
            mudh_dir = optarg;
            break;
        case 'p':
            if (sscanf(optarg, " %f:%f", &min_spd, &max_spd) != 2)
            {
                fprintf(stderr, "%s: error parsing speed range %s\n", command,
                    optarg);
                exit(1);
            }
            opt_spd = 1;
            break;
        case 'c':
            collocation_time = atoi(optarg);
            opt_collocate = 1;
            break;
        case 's':
            irr_dir = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* flag_dir = argv[optind++];
    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
    unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
    unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

    float           index_tab[AT_WIDTH][CT_WIDTH];
    unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

    unsigned short  nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short  spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short  dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short  mle_array[AT_WIDTH][CT_WIDTH];
    unsigned short  lon_array[AT_WIDTH][CT_WIDTH];
    unsigned short  lat_array[AT_WIDTH][CT_WIDTH];

    Index lon_index;
    lon_index.SpecifyWrappedCenters(LON_MIN, LON_MAX, LON_BINS);
    Index lat_index;
    lat_index.SpecifyEdges(LAT_MIN, LAT_MAX, LAT_BINS);

    unsigned int array_size = CT_WIDTH * AT_WIDTH;

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "%s/%0*d.irain", irr_dir, REV_DIGITS, rev);
        FILE* ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s (continuing)\n",
                command, rain_file);
            continue;
        }
        if (fread(rain_rate, sizeof(char), array_size, ifp) != array_size ||
            fread(time_dif, sizeof(char), array_size, ifp) != array_size ||
            fseek(ifp, array_size, SEEK_CUR) != 0 ||
            fread(integrated_rain_rate, sizeof(short), array_size, ifp) !=
            array_size)
        {
            fprintf(stderr, "%s: error reading rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fclose(ifp);

        //----------------//
        // read MUDH file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%s/%0*d.mudh", mudh_dir, REV_DIGITS, rev);
        ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(nbd_array, sizeof(short), array_size, ifp);
        fread(spd_array, sizeof(short), array_size, ifp);
        fread(dir_array, sizeof(short), array_size, ifp);
        fread(mle_array, sizeof(short), array_size, ifp);
        fread(lon_array, sizeof(short), array_size, ifp);
        fread(lat_array, sizeof(short), array_size, ifp);
        fclose(ifp);

        //----------------//
        // read flag file //
        //----------------//

        char flag_file[1024];
        sprintf(flag_file, "%s/%0*d.flag", flag_dir, REV_DIGITS, rev);
        ifp = fopen(flag_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening flag file %s\n", command,
                flag_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ifp);
        fread(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (opt_collocate && abs(co_time) > collocation_time)
                    integrated_rain_rate[ati][cti] = 2000;
            }
        }

        //------------------//
        // accumulate array //
        //------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                //-------------------------------------//
                // if collocating, SSM/I must be valid //
                //-------------------------------------//

                if (opt_collocate && integrated_rain_rate[ati][cti] >= 1000)
                    continue;
                double irr = (double)integrated_rain_rate[ati][cti] * 0.1;

                //-----------------------------------//
                // is this a valid wind vector cell? //
                //-----------------------------------//

                if (spd_array[ati][cti] == MAX_SHORT ||
                    mle_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

                if (opt_spd)
                {
                    double spd = (double)spd_array[ati][cti] * 0.01;
                    if (spd < min_spd || spd > max_spd)
                        continue;
                }

                int lon_idx;
                float lon = (float)lon_array[ati][cti] * 0.01;
                lon_index.GetNearestIndexWrapped(lon, &lon_idx);

                int lat_idx;
                float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
                lat_index.GetNearestIndexClipped(lat, &lat_idx);

                switch (flag_tab[ati][cti])
                {
                case 0: // inner swath, no rain
                    qscat_inner_total_count[lon_idx][lat_idx]++;
                    break;
                case 3: // outer swath, no rain
                    qscat_outer_total_count[lon_idx][lat_idx]++;
                    break;
                case 1: // inner swath, rain
                    qscat_inner_total_count[lon_idx][lat_idx]++;
                    qscat_inner_rain_count[lon_idx][lat_idx]++;
                    qscat_rain_count[lon_idx][lat_idx]++;
                    break;
                case 4: // outer swath, rain
                    qscat_outer_total_count[lon_idx][lat_idx]++;
                    qscat_outer_rain_count[lon_idx][lat_idx]++;
                    qscat_rain_count[lon_idx][lat_idx]++;
                    break;
                case 2: // inner swath, unclassifiable
                case 5: // outer swath, unclassifiable
                case 6: // no wind
                case 7: // ???
                    continue;
                }
                total_qscat_count[lon_idx][lat_idx]++;

                //----------------------------//
                // accumulate valid ssmi data //
                //----------------------------//

                if (integrated_rain_rate[ati][cti] >= 1000)
                    continue;

                if (irr > IRR_THRESH)
                    ssmi_rain_count[lon_idx][lat_idx]++;
                total_ssmi_count[lon_idx][lat_idx]++;
            }
        }
    }

    //-------------//
    // write table //
    //-------------//

    float qscat_in_array[LON_BINS][LAT_BINS];
    float qscat_out_array[LON_BINS][LAT_BINS];
    float qscat_array[LON_BINS][LAT_BINS];
    float ssmi_array[LON_BINS][LAT_BINS];
    float dif_array[LON_BINS][LAT_BINS];
    for (int lon_idx = 0; lon_idx < LON_BINS; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < LAT_BINS; lat_idx++)
        {
            qscat_in_array[lon_idx][lat_idx] = 0.0;
            qscat_out_array[lon_idx][lat_idx] = 0.0;
            qscat_array[lon_idx][lat_idx] = 0.0;
            ssmi_array[lon_idx][lat_idx] = 0.0;
            if (total_qscat_count[lon_idx][lat_idx] > MIN_POINTS)
            {
                qscat_array[lon_idx][lat_idx] =
                    (float)qscat_rain_count[lon_idx][lat_idx] /
                    (float)total_qscat_count[lon_idx][lat_idx];
            }
            if (total_ssmi_count[lon_idx][lat_idx] > MIN_POINTS)
            {
                ssmi_array[lon_idx][lat_idx] =
                    (float)ssmi_rain_count[lon_idx][lat_idx] /
                    (float)total_ssmi_count[lon_idx][lat_idx];
            }
            if (total_qscat_count[lon_idx][lat_idx] > MIN_POINTS &&
                total_ssmi_count[lon_idx][lat_idx] > MIN_POINTS)
            {
                dif_array[lon_idx][lat_idx] = qscat_array[lon_idx][lat_idx] -
                    ssmi_array[lon_idx][lat_idx];
            }
            if (qscat_inner_total_count[lon_idx][lat_idx] > MIN_POINTS)
            {
                qscat_in_array[lon_idx][lat_idx] =
                    (float)qscat_inner_rain_count[lon_idx][lat_idx] /
                    (float)qscat_inner_total_count[lon_idx][lat_idx];
            }
            if (qscat_outer_total_count[lon_idx][lat_idx] > MIN_POINTS)
            {
                qscat_out_array[lon_idx][lat_idx] =
                    (float)qscat_outer_rain_count[lon_idx][lat_idx] /
                    (float)qscat_outer_total_count[lon_idx][lat_idx];
            }
        }
    }

    int lon_width = LON_BINS;
    int lat_width = LAT_BINS;

    char filename[1024];
    sprintf(filename, "%s.ssmi", output_base);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&lon_width, sizeof(int), 1, ofp);
    fwrite(&lat_width, sizeof(int), 1, ofp);
    fwrite(ssmi_array, sizeof(float), LON_BINS * LAT_BINS, ofp);
    fclose(ofp);

    sprintf(filename, "%s.qscat", output_base);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&lon_width, sizeof(int), 1, ofp);
    fwrite(&lat_width, sizeof(int), 1, ofp);
    fwrite(qscat_array, sizeof(float), LON_BINS * LAT_BINS, ofp);
    fclose(ofp);

    sprintf(filename, "%s.dif", output_base);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&lon_width, sizeof(int), 1, ofp);
    fwrite(&lat_width, sizeof(int), 1, ofp);
    fwrite(dif_array, sizeof(float), LON_BINS * LAT_BINS, ofp);
    fclose(ofp);

    sprintf(filename, "%s.in.qscat", output_base);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&lon_width, sizeof(int), 1, ofp);
    fwrite(&lat_width, sizeof(int), 1, ofp);
    fwrite(qscat_in_array, sizeof(float), LON_BINS * LAT_BINS, ofp);
    fclose(ofp);

    sprintf(filename, "%s.out.qscat", output_base);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&lon_width, sizeof(int), 1, ofp);
    fwrite(&lat_width, sizeof(int), 1, ofp);
    fwrite(qscat_out_array, sizeof(float), LON_BINS * LAT_BINS, ofp);
    fclose(ofp);

    return (0);
}
