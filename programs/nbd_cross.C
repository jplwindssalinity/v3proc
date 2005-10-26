//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    nbd_cross
//
// SYNOPSIS
//    nbd_cross [ -c ] [ -m minutes ] [ -r rain_rate ] <start_rev>
//        <end_rev> <output_base>
//
// DESCRIPTION
//    Makes some nbd and rain rate comparisons.
//
// OPTIONS
//    [ -c ]            Generate cross files.
//    [ -m minutes ]    Time difference maximum.
//    [ -r rain_rate ]  The SSM/I rain rate to threshold.
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % nbd_cross -m 15 1500 1600 15min
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

#define OPTSTRING    "cm:r:"
#define ATI_SIZE     1624
#define CTI_SIZE     76
#define NBD_SCALE    10.0
#define MIN_SAMPLES  20
#define QUOTE        '"'

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c ]", "[ -m minutes ]", "[ -r rain_rate ]",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// parameter (nbd,spd,dir,mle,prog); ssmi rain (0=no,1=yes); counts vs param
static unsigned long counts[5][2][256];
static unsigned long matrix_sum[2][2];
static unsigned long total_count = 0;

//static unsigned long matrix[256][2][2];

// coefficients for rain probability
// nbd, spd, dir, mle
float coefs[4][8] = {
{ 0.004101, 0.009178, 0.02454, 0.01253, -0.0007627, -0.001155,
    -1.824e-05, 3.002e-05 },
{ -0.01487, 0.02759, -0.0133, 0.002597, -0.0002396, 1.168e-05,
    -2.899e-07, 2.867e-09 },
{ 0.01036, -0.001523, 0.0002357, -1.633e-05, 5.561e-07, -9.823e-09,
    8.618e-11, -2.938e-13 },
{ 0.01049, 0.02512, 0.06439, 0.01727, -7.656e-05, -0.0005374,
    -6.75e-05, -2.565e-06 }
};

// if the parameter is outside this range, set probablitity to 0.0
float range[4][2] = {
{ -3.0, 5.0 },
{ 0.0, 28.0 },
{ 0.0, 90.0 },
{ -9.0, 0.0 }
};

const char* param_map[] = { "NBD", "Speed", "Direction", "MLE", "Prob" };
const char* rr_map[] = { "Clear", "Rain" };

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
    float rain_threshold = 0.0;
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'c':
            break;
        case 'm':
            minutes = atoi(optarg);
            break;
        case 'r':
            rain_threshold = atof(optarg);
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
    const char* output_base = argv[optind++];

    //-------------------//
    // open output files //
    //-------------------//

    FILE* cross_ofp[4][4];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            char filename[2048];
            sprintf(filename, "%s.%dx%d.cross", output_base, i, j);
            cross_ofp[i][j] = fopen(filename, "w");
            if (cross_ofp[i][j] == NULL)
            {
                fprintf(stderr, "%s: error opening output file %s\n", command,
                    filename);
                exit(1);
            }
           fprintf(cross_ofp[i][j], "@ xaxis label %c%s%c\n", QUOTE,
               param_map[i], QUOTE);
           fprintf(cross_ofp[i][j], "@ yaxis label %c%s%c\n", QUOTE,
               param_map[j], QUOTE);
        }
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char nbd_array[ATI_SIZE][CTI_SIZE];
    unsigned char spd_array[ATI_SIZE][CTI_SIZE];
    unsigned char dir_array[ATI_SIZE][CTI_SIZE];
    unsigned char mle_array[ATI_SIZE][CTI_SIZE];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //---------------//
        // read nbd file //
        //---------------//

        char nbd_file[1024];
        sprintf(nbd_file, "%d.nbd", rev);
        FILE* ifp = fopen(nbd_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening NBD file %s\n", command,
                nbd_file);
            exit(1);
        }
        fread(nbd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fread(spd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fread(dir_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fread(mle_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%d.rain", rev);
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

/*
        //-------------------------------//
        // generate classifying function //
        //-------------------------------//

        float prb_array[ATI_SIZE][CTI_SIZE];
        float max_prb_array = 0.0;
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                prb_array[ati][cti] = 0.0;

                if (rain_rate[ati][cti] >= 250 ||
                    nbd_array[ati][cti] == 255 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }

                float param_value[4];
                param_value[0] = (float)nbd_array[ati][cti] / 20.0 - 6.0;
                param_value[1] = (float)spd_array[ati][cti] / 5.0;
                param_value[2] = (float)dir_array[ati][cti] / 2.0;
                param_value[3] = (float)mle_array[ati][cti] / 8.0 - 30.0;
                float total = 1.0;
                for (int param_idx = 0; param_idx < 4; param_idx++)
                {
                    float value = 0.0;
                    if (param_value[param_idx] > range[param_idx][0] &&
                        param_value[param_idx] < range[param_idx][1])
                    {
                        value = polyval(param_value[param_idx],
                            coefs[param_idx], 7);
                        if (value < 0.0) value = 0.0;
                    }
                    total *= value;
                }
                prb_array[ati][cti] = total;
                if (total > max_prb_array)
                    max_prb_array = total;

                if (opt_cross)
                {
                    float rr = (float)rain_rate[ati][cti] * 0.1;
                    fprintf(cross_ofp, "%g %g %g %g %g %g %d %d %d\n",
                        param_value[0], param_value[1], param_value[2],
                        param_value[3], 1000.0 * total, rr, rev, ati, cti);
                }
            }
        }
*/

        //-----------------------//
        // accumulate histograms //
        //-----------------------//

        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == 255 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }
                float rr = rain_rate[ati][cti] * 0.1;
                int ssmi_flag;
                if (rr == 0.0)
                    ssmi_flag = 0;
                else if (rr > rain_threshold)
                    ssmi_flag = 1;
                else
                    continue;

                float param_value[4];
                param_value[0] = (float)nbd_array[ati][cti] / 20.0 - 6.0;
                param_value[1] = (float)spd_array[ati][cti] / 5.0;
                param_value[2] = (float)dir_array[ati][cti] / 2.0;
                param_value[3] = (float)mle_array[ati][cti] / 8.0 - 30.0;

                // write to cross file
                for (int i = 0; i < 4; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        fprintf(cross_ofp[i][j], "%g %g %d\n", param_value[i],
                            param_value[j], ssmi_flag);
                    }
                }
            }
        }

        //-------------------//
        // accumulate matrix //
        //-------------------//

/*
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == 255 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }

                // threshold
                float rr = rain_rate[ati][cti] * 0.1;
                int ssmi_flag = 0;
                if (rr > rain_threshold)
                    ssmi_flag = 1;

                // xxx
                if (prb_array[ati][cti] > 0.0020)
                    nbd_flag = 1;
                else
                    nbd_flag = 0;

                int spd_idx = spd_array[ati][cti];

                // accumulate
                matrix_sum[ssmi_flag][nbd_flag]++;
                matrix[spd_idx][ssmi_flag][nbd_flag]++;
                total_count++;
            }
        }
*/
    }

    //------------------//
    // write crosses //
    //------------------//

/*
    float param_b[] = { 6.0, 0.0, 0.0, 30.0, 0.0 };
    float param_m[] = { 20.0, 5.0, 2.0, 8.0, 255.0 };
    for (int param_idx = 0; param_idx < 5; param_idx++)
    {
        for (int rr_idx = 0; rr_idx < 2; rr_idx++)
        {
            fprintf(histo_ofp, "# %s, %s\n", param_map[param_idx],
                rr_map[rr_idx]);
            for (int idx = 0; idx < 256; idx++)
            {
                if (counts[param_idx][rr_idx][idx] == 0)
                    continue;
                float value = (float)idx / param_m[param_idx] -
                    param_b[param_idx];
                fprintf(histo_ofp, "%g %lu\n", value,
                    counts[param_idx][rr_idx][idx]);
            }
            fprintf(histo_ofp, "&\n");
        }
        // ratio of rain to total
        for (int idx = 0; idx < 256; idx++)
        {
            if (counts[param_idx][0][idx] + counts[param_idx][1][idx] <
                MIN_SAMPLES)
            {
                continue;
            }
            float value = (float)idx / param_m[param_idx] -
                param_b[param_idx];
            fprintf(prob_ofp, "%g %g\n", value,
                (double)counts[param_idx][1][idx] /
                ((double)counts[param_idx][0][idx] +
                counts[param_idx][1][idx]));
        }
        fprintf(prob_ofp, "&\n");
    }

    //-----------------------//
    // write out matrix sums //
    //-----------------------//

    char* i_map[] = { "SSM/I Clear", "SSM/I Rain" };
    char* j_map[] = { "NBD Clear", "NBD Rain" };
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            fprintf(matrix_ofp, "# %s, %s = %g %%\n", i_map[i], j_map[j],
                100.0 * (double)matrix_sum[i][j] / (double)total_count);
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
*/

/*
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

    if (opt_cross && spd_file != NULL)
    {
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
                fprintf(cross_ofp, "%g %g %d\n", spd, nbd, flag);
            }
        }
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
                fprintf(cross_ofp, "%g %g %d\n", spd, nbd, rain_rate[ati][cti]);
            }
        }
    }
*/

    //-------------//
    // close files //
    //-------------//

//    fclose(histo_ofp);
//    fclose(matrix_ofp);
//    fclose(prob_ofp);
/*
    fclose(vs_ofp);
*/

    return (0);
}
