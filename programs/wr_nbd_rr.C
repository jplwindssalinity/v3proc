//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    wr_nbd_rr
//
// SYNOPSIS
//    wr_nbd_rr [ -m minutes ] [ -t nbd_min_thresh:nbd_max_thresh ]
//        [ -r rain_thresh ] [ -s spd_file ] [ -c ] <nbd_file>
//        <rain_file> <output_base>
//
// DESCRIPTION
//    Makes some nbd and rain rate comparisons.
//
// OPTIONS
//    [ -m minutes ]                        Time difference maximum.
//    [ -t nbd_min_thresh:nbd_max_thresh ]  The NDB thresholds.
//    [ -r rain_thresh ]                    The rain rate threshold.
//    [ -s spd_file ]                       Use the speed file for the histo.
//    [ -c ]                                Make classified files.
//
// OPERANDS
//    <nbd_file>     The input NBD file.
//    <rain_file>    The Freilich SSM/I file.
//    <output_base>  The output apts file.
//
// EXAMPLES
//    An example of a command line is:
//      % wr_nbd_rr 203.nbd 203.rain 203
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
#include <unistd.h>
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

#define OPTSTRING  "m:t:r:s:c"
#define ATI_SIZE   1624
#define CTI_SIZE   76
#define NBD_SCALE  10.0

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m minutes ]",
    "[ -t nbd_min_thresh:nbd_max_thresh ]", "[ -r rain_thresh ]",
    "[ -s spd_file ]", "[ -c ]", "<nbd_file>", "<rain_file>",
    "<output_base>", 0 };

static unsigned long counts[256][256][256];

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

    int minutes = 180;    // three hours
    float nbd_min_threshold = -3.0;
    float nbd_max_threshold = 3.0;
    float rain_threshold = 0.0;
    char* spd_file = NULL;
    int opt_class = 0;

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
            minutes = atoi(optarg);
            break;
        case 't':
            if (sscanf(optarg, "%f:%f", &nbd_min_threshold,
                &nbd_max_threshold) != 2)
            {
                fprintf(stderr, "%s: error interpreting thresholds %s\n",
                    command, optarg);
                usage(command, usage_array, 1);
            }
            break;
        case 'r':
            if (sscanf(optarg, "%f", &rain_threshold) != 1)
            {
                fprintf(stderr, "%s: error interpreting threshold %s\n",
                    command, optarg);
                usage(command, usage_array, 1);
            }
            break;
        case 's':
            spd_file = optarg;
            break;
        case 'c':
            opt_class = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* nbd_file = argv[optind++];
    const char* rain_file = argv[optind++];
    const char* output_base = argv[optind++];

    //---------------//
    // read nbd file //
    //---------------//

    char nbd_array[ATI_SIZE][CTI_SIZE];
    FILE* ifp = fopen(nbd_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening NBD file %s\n", command,
            nbd_file);
        exit(1);
    }
    fread(nbd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
    fclose(ifp);

    //----------------//
    // read rain file //
    //----------------//

    unsigned char rain_rate[ATI_SIZE][CTI_SIZE];
    unsigned char time_dif[ATI_SIZE][CTI_SIZE];
    ifp = fopen(rain_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening rain file %s\n", command,
            rain_file);
        exit(1);
    }
    fread(rain_rate, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
    fread(time_dif, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
    fclose(ifp);

    //---------------//
    // read spd file //
    //---------------//

    unsigned char spd_array[ATI_SIZE][CTI_SIZE];
    if (spd_file != NULL)
    {
        ifp = fopen(spd_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening speed file %s\n", command,
                spd_file);
            exit(1);
        }
        fread(spd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);
    }

    //---------------------------------------//
    // eliminate rain data out of time range //
    //---------------------------------------//

    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            int co_time = time_dif[ati][cti] * 2 - 180;
            if (abs(co_time) > minutes)
                rain_rate[ati][cti] = 255;
        }
    }

    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.histo", output_base);
    FILE* histo_ofp = fopen(filename, "w");
    if (histo_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    sprintf(filename, "%s.matrix", output_base);
    FILE* matrix_ofp = fopen(filename, "w");
    if (matrix_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    sprintf(filename, "%s.vs", output_base);
    FILE* vs_ofp = fopen(filename, "w");
    if (vs_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    FILE* class_ofp = NULL;
    if (opt_class && spd_file != NULL)
    {
        sprintf(filename, "%s.class", output_base);
        class_ofp = fopen(filename, "w");
        if (class_ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
    }

    //----------------------//
    // accumulate histogram //
    //----------------------//

    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            // check data
            if (rain_rate[ati][cti] > 250 ||
                nbd_array[ati][cti] == -128)
            {
                continue;
            }
            int rr_idx = rain_rate[ati][cti];
            int nbd_idx = (int)nbd_array[ati][cti] + 128;
            int spd_idx = 0;
            if (spd_file != NULL)
                spd_idx = spd_array[ati][cti];
            counts[spd_idx][rr_idx][nbd_idx]++;
        }
    }

    //------------------//
    // write histograms //
    //------------------//

    int min_spd_idx = 0;
    int max_spd_idx = 0;
    if (spd_file != NULL)
    {
        max_spd_idx = 255;
    }
    for (int spd_idx = min_spd_idx; spd_idx <= max_spd_idx; spd_idx++)
    {
        for (int rr_idx = 0; rr_idx < 256; rr_idx++)
        {
            for (int nbd_idx = 0; nbd_idx < 256; nbd_idx++)
            {
                if (counts[spd_idx][rr_idx][nbd_idx] == 0)
                    continue;
                fprintf(histo_ofp, "%d %d %d %lu\n", spd_idx, rr_idx, nbd_idx,
                    counts[spd_idx][rr_idx][nbd_idx]);
            }
        }
    }

    //------------------//
    // the count matrix //
    //------------------//

    unsigned long matrix[256][2][2];
    for (int spd_idx = 0; spd_idx < 256; spd_idx++)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                matrix[spd_idx][i][j] = 0;
            }
        }
    }
    int ssmi_flag, nbd_flag;
    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            if (rain_rate[ati][cti] > 250)
                continue;    // no ssmi data

            // threshold
            float rr = rain_rate[ati][cti] * 0.1;
            if (rr > rain_threshold)
                ssmi_flag = 1;
            else
                ssmi_flag = 0;

            if (nbd_array[ati][cti] == -128)
                continue;

            float nbd = (float)nbd_array[ati][cti] / NBD_SCALE;
            if (nbd < nbd_min_threshold || nbd > nbd_max_threshold)
                nbd_flag = 1;
            else
                nbd_flag = 0;

            int spd_idx = 0;
            if (spd_file != NULL)
                spd_idx = spd_array[ati][cti];

            // accumulate
            matrix[spd_idx][ssmi_flag][nbd_flag]++;
        }
    }

    //------------------//
    // write out matrix //
    //------------------//

    for (int spd_idx = 0; spd_idx < 256; spd_idx++)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (matrix[spd_idx][i][j] > 0)
                {
                    fprintf(matrix_ofp, "%d %d %d %lu\n", spd_idx, i, j,
                        matrix[spd_idx][i][j]);
                }
            }
        }
    }

    //-----------------//
    // the versus file //
    //-----------------//

    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            if (rain_rate[ati][cti] > 250 ||
                nbd_array[ati][cti] == -128)
            {
                continue;
            }
            float rr = rain_rate[ati][cti] * 0.1;
            float nbd = nbd_array[ati][cti] * 0.1;
            fprintf(vs_ofp, "%g %g\n", rr, nbd);
        }
    }

    //-----------------//
    // classifier file //
    //-----------------//

    if (opt_class && spd_file != NULL)
    {
/*
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128)
                {
                    continue;
                }
                float rr = rain_rate[ati][cti] * 0.1;
                float nbd = nbd_array[ati][cti] * 0.1;
                float spd = spd_array[ati][cti] * 0.2;
                int flag = 0;
                if (rr > rain_threshold)
                    flag = 1;
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, flag);
            }
        }
*/
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128)
                {
                    continue;
                }
                float rr = rain_rate[ati][cti] * 0.1;
                float nbd = nbd_array[ati][cti] * 0.1;
                float spd = spd_array[ati][cti] * 0.2;
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, rain_rate[ati][cti]);
            }
        }
    }

    //-------------//
    // close files //
    //-------------//

    fclose(histo_ofp);
    fclose(matrix_ofp);
    fclose(vs_ofp);
    fclose(class_ofp);

    return (0);
}
