//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_image
//
// SYNOPSIS
//    mudh_image [ -n ] [ -m thresh ] [ -r thresh ] <start_rev>
//        <end_rev> <output_file>
//
// DESCRIPTION
//    Reads in a set of MUDH files and generates an image file.
//
// OPTIONS
//    [ -n ]         Need NBD (basically, eliminate outer beam only)
//    [ -m thresh ]  Make the image for MUDH, using the specified
//                     threshold to classify rain.
//    [ -r thresh ]  Make the image for SSM/I integrated rain rate,
//                     using the specified threshold to classify rain.
//
// OPERANDS
//    <start_rev>    The starting rev number.
//    <end_rev>      The ending rev number.
//    <output_file>  The output file name.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_image 1500 1550 1500-1550.im
//      % mudh_image -r 2.0 1500 1550 1500-1550.ssmi.im
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
#include <stdlib.h>
#include "Misc.h"
#include "Index.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "nm:r:"

#define LON_BINS  720
#define LON_MIN   0.0
#define LON_MAX   360.0
#define LAT_BINS  360
#define LAT_MIN   -90.0
#define LAT_MAX   90.0

#define CHAR_IMAGE_HEADER   "imc "

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -n ]", "[ -m thresh ]", "[ -r thresh ]",
    "<start_rev>", "<end_rev>", "<output_file>", 0 };

static float           value_tab[AT_WIDTH][CT_WIDTH];
static unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

static unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
static unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
static unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
unsigned short spd_array[AT_WIDTH][CT_WIDTH];
unsigned short dir_array[AT_WIDTH][CT_WIDTH];
unsigned short mle_array[AT_WIDTH][CT_WIDTH];
unsigned short lon_array[AT_WIDTH][CT_WIDTH];
unsigned short lat_array[AT_WIDTH][CT_WIDTH];

// 0 = no info
// 1 = clear
// 2 = no-mans-land  (used for SSM/I files)
// 3 = rain
unsigned char image[LON_BINS][LAT_BINS];

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

    float mudh_thresh = 0.0;
    int opt_mudh_thresh = 0;
    float irr_thresh = 0.0;
    int opt_rain = 0;
    int opt_need_nbd = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'n':
            opt_need_nbd = 1;
            break;
        case 'm':
            mudh_thresh = atof(optarg);
            opt_mudh_thresh = 1;
            break;
        case 'r':
            irr_thresh = atof(optarg);
            opt_rain = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_file = argv[optind++];

    if (opt_rain == opt_mudh_thresh)
    {
        fprintf(stderr, "%s: must specify -r or -m\n", command);
        exit(1);
    }

    //--------------//
    // prelim stuff //
    //--------------//

    Index lon_index;
    lon_index.SpecifyWrappedCenters(LON_MIN, LON_MAX, LON_BINS);
    Index lat_index;
    lat_index.SpecifyEdges(LAT_MIN, LAT_MAX, LAT_BINS);
    unsigned int size = AT_WIDTH * CT_WIDTH;

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //------------------------------------//
        // read the MUDH file for lon and lat //
        //------------------------------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%d.mudh", rev);
        FILE* ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s (coninuing)\n",
                command, mudh_file);
            continue;
        }
        if (fread(nbd_array, sizeof(short), size, ifp) != size ||
            fread(spd_array, sizeof(short), size, ifp) != size ||
            fread(dir_array, sizeof(short), size, ifp) != size ||
            fread(mle_array, sizeof(short), size, ifp) != size ||
            fread(lon_array, sizeof(short), size, ifp) != size ||
            fread(lat_array, sizeof(short), size, ifp) != size)
        {
            fclose(ifp);
            fprintf(stderr, "%s: error reading MUDH file %s (continuing)\n",
                command, mudh_file);
            continue;
        }
        fclose(ifp);

        //----------------------------------//
        // read the rain determination file //
        //----------------------------------//

        char filename[1024];
        if (opt_rain)
        {
            sprintf(filename, "/export/svt11/hudd/ssmi/%d.irain", rev);
            ifp = fopen(filename, "r");
            if (ifp == NULL)
            {
                fprintf(stderr, "%s: error opening file %s (continuing)\n",
                    command, filename);
                continue;
            }
            if (fread(rain_rate, sizeof(char), size, ifp) != size ||
                fread(time_dif,  sizeof(char), size, ifp) != size ||
                fseek(ifp, size, SEEK_CUR) == -1 ||
                fread(integrated_rain_rate, sizeof(short), size, ifp) != size)
            {
                fprintf(stderr, "%s: error reading input rain file %s\n",
                    command, filename);
                exit(1);
            }
            fclose(ifp);
            for (int ati = 0; ati < AT_WIDTH; ati++)
            {
                for (int cti = 0; cti < CT_WIDTH; cti++)
                {
                    if (integrated_rain_rate[ati][cti] >= 1000)
                        continue;
                    float irr = integrated_rain_rate[ati][cti] * 0.1;

                    //--------------------------------//
                    // determine lon and lat indicies //
                    //--------------------------------//

                    if (lon_array[ati][cti] == MAX_SHORT ||
                        lat_array[ati][cti] == MAX_SHORT)
                    {
                        continue;
                    }

                    int lon_idx;
                    float lon = (float)lon_array[ati][cti] * 0.01;
                    lon_index.GetNearestWrappedIndex(lon, &lon_idx);

                    int lat_idx;
                    float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
                    lat_index.GetNearestIndex(lat, &lat_idx);

                    if (irr == 0)
                    {
                        // clear
                        if (image[lon_idx][lat_idx] < 1)
                            image[lon_idx][lat_idx] = 1;
                    }
                    else if (irr > irr_thresh)
                    {
                        // rain
                        image[lon_idx][lat_idx] = 3;
                    }
                    else
                    {
                        // no mans land
                        if (image[lon_idx][lat_idx] < 2)
                            image[lon_idx][lat_idx] = 2;
                    }
                }
            }
        }
        else if (opt_mudh_thresh)
        {
            sprintf(filename, "%d.pflag", rev);
            ifp = fopen(filename, "r");
            if (ifp == NULL)
            {
                fprintf(stderr, "%s: error opening file %s (continuing)\n",
                    command, filename);
                continue;
            }
            if (fread(value_tab, sizeof(float), size, ifp) != size ||
                fread(flag_tab,   sizeof(char), size, ifp) != size)
            {
                fprintf(stderr, "%s: error reading input flag file %s\n",
                    command, filename);
                exit(1);
            }
            fclose(ifp);
            for (int ati = 0; ati < AT_WIDTH; ati++)
            {
                for (int cti = 0; cti < CT_WIDTH; cti++)
                {
                    if (flag_tab[ati][cti] == 2)
                        continue;

                    // check the "must have NBD" case
                    if (opt_need_nbd && nbd_array[ati][cti] == MAX_SHORT)
                        continue;

                    //--------------------------------//
                    // determine lon and lat indicies //
                    //--------------------------------//

                    if (lon_array[ati][cti] == MAX_SHORT ||
                        lat_array[ati][cti] == MAX_SHORT)
                    {
                        continue;
                    }

                    int lon_idx;
                    float lon = (float)lon_array[ati][cti] * 0.01;
                    lon_index.GetNearestWrappedIndex(lon, &lon_idx);

                    int lat_idx;
                    float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
                    lat_index.GetNearestIndex(lat, &lat_idx);

                    if (value_tab[ati][cti] <= mudh_thresh)
                    {
                        // clear
                        if (image[lon_idx][lat_idx] < 1)
                            image[lon_idx][lat_idx] = 1;
                    }
                    else
                    {
                        // rain
                        image[lon_idx][lat_idx] = 3;
                    }
                }
            }
        }
    }

    //-----------------//
    // write out image //
    //-----------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }
    char* im_string = CHAR_IMAGE_HEADER;
    int lon_bins = LON_BINS;
    int lat_bins = LAT_BINS;
    fwrite(im_string, 4, 1, ofp);
    fwrite(&lon_bins, 4, 1, ofp);
    fwrite(&lat_bins, 4, 1, ofp);
    for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
    {
        fwrite(image[lon_idx], sizeof(unsigned char),
            lat_bins, ofp);
    }
    fclose(ofp);

    return (0);
}
