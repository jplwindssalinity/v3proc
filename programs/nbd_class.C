//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    nbd_class
//
// SYNOPSIS
//    nbd_class [ -c ] [ -s conclass_file ] [ -m minutes ]
//        [ -r rain_rate ] <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Makes some nbd and rain rate comparisons.
//
// OPTIONS
//    [ -c ]                Generate class files.
//    [ -s conclass_file ]  Read and use conclass file for classifying.
//    [ -m minutes ]        Time difference maximum.
//    [ -r rain_rate ]      The SSM/I rain rate to threshold.
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % nbd_class -m 15 1500 1600 15min
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

#define OPTSTRING    "cs:m:r:"
#define ATI_SIZE     1624
#define CTI_SIZE     76
#define NBD_SCALE    10.0
#define MIN_SAMPLES  20

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c ]", "[ -s conclass_file ]",
    "[ -m minutes ]", "[ -r rain_rate ]", "<start_rev>", "<end_rev>",
    "<output_base>", 0 };

// parameter (nbd,spd,dir,mle,prog); ssmi rain (0=no,1=yes); counts vs param
static unsigned long counts[5][2][256];
static unsigned long matrix_sum[2][2];
static unsigned long total_count = 0;
static unsigned short conclass[16][16][16][16];

unsigned long spd_avail_count = 0;
unsigned long with_nbd_count = 0;
unsigned long without_nbd_count = 0;

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
    int opt_class = 0;
    int opt_conclass = 0;
    const char* conclass_file = NULL;

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
            opt_class = 1;
            break;
        case 's':
            opt_conclass = 1;
            conclass_file = optarg;
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

    char filename[2048];
    sprintf(filename, "%s.histo", output_base);
    FILE* histo_ofp = fopen(filename, "w");
    if (histo_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    sprintf(filename, "%s.prob", output_base);
    FILE* prob_ofp = fopen(filename, "w");
    if (prob_ofp == NULL)
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

    FILE* class_with_nbd_ofp = NULL;
    FILE* class_without_nbd_ofp = NULL;
    if (opt_class)
    {
        sprintf(filename, "%s.class.withnbd", output_base);
        class_with_nbd_ofp = fopen(filename, "w");
        if (class_with_nbd_ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }

        sprintf(filename, "%s.class.withoutnbd", output_base);
        class_without_nbd_ofp = fopen(filename, "w");
        if (class_without_nbd_ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
    }

    //--------------------//
    // read conclass file //
    //--------------------//

    if (opt_conclass)
    {
        FILE* conclass_ifp = fopen(conclass_file, "r");
        if (conclass_ifp == NULL)
        {
            fprintf(stderr, "%s: error opening conclass file %s\n", command,
                conclass_file);
            exit(1);
        }
        fread(conclass, sizeof(short), 16 * 16 * 16 * 16, conclass_ifp);
        fclose(conclass_ifp);
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

        //-------------------------------//
        // generate classifying function //
        //-------------------------------//

        float prb_array[ATI_SIZE][CTI_SIZE];
        float max_prb_array = 0.0;
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                //-----------------------------------------//
                // accumulate for percentage class capable //
                //-----------------------------------------//

                if (spd_array[ati][cti] != 255)
                    spd_avail_count++;

                if (dir_array[ati][cti] != 255 &&
                    mle_array[ati][cti] != 255)
                {
                    if (nbd_array[ati][cti] != 255)
                        with_nbd_count++;
                    else
                        without_nbd_count++;
                }

                prb_array[ati][cti] = 0.0;

                if (rain_rate[ati][cti] >= 250 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }

                //----------//
                // conclass //
                //----------//

                double nbd = (double)nbd_array[ati][cti] / 20.0 - 6.0;
                double spd = (double)spd_array[ati][cti] / 5.0;
                double dir = (double)dir_array[ati][cti] / 2.0;
                double mle = (double)mle_array[ati][cti] / 8.0 - 30.0;

                int inbd = (int)((nbd +  4.0) * 14.0 / 10.0 + 0.5);
                int ispd = (int)((spd +  0.0) * 15.0 / 30.0 + 0.5);
                int idir = (int)((dir +  0.0) * 15.0 / 90.0 + 0.5);
                int imle = (int)((mle + 10.0) * 15.0 / 10.0 + 0.5);

                if (inbd < 0) inbd = 0;
                if (inbd > 14) inbd = 14;
                // hack in missing nbd element
                if (nbd_array[ati][cti] == 255) inbd = 15;

                if (ispd < 0) ispd = 0;
                if (ispd > 15) ispd = 15;
                if (idir < 0) idir = 0;
                if (idir > 15) idir = 15;
                if (imle < 0) imle = 0;
                if (imle > 15) imle = 15;

//                float total = (float)conclass[inbd][ispd][idir][imle] / 2.0;
//                float total = (float)conclass[inbd][ispd][idir][imle] / 25.0;
                int irain = conclass[inbd][ispd][idir][imle];
                irain /= 256;
                int iclear = conclass[inbd][ispd][idir][imle];
                iclear &= 0x00ff;

/*
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
                total *= 1000.0;
*/

                if (opt_class)
                {
                    float rr = (float)rain_rate[ati][cti] * 0.1;
                    float fclear = (float)iclear * 0.5;
                    float frain = (float)irain * 0.5;
                    if (inbd == 15)
                    {
                        fprintf(class_without_nbd_ofp,
                            "%g %g %g %g %g %g %g %d %d %d\n", nbd, spd, dir,
                            mle, fclear, frain, rr, rev, ati, cti);
                    }
                    else
                    {
                        fprintf(class_with_nbd_ofp,
                            "%g %g %g %g %g %g %g %d %d %d\n", nbd, spd, dir,
                            mle, fclear, frain, rr, rev, ati, cti);
                    }
                }
            }
        }

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
                int ssmi_flag = 0;
                if (rr > rain_threshold)
                    ssmi_flag = 1;

                int nbd_idx = nbd_array[ati][cti];
                counts[0][ssmi_flag][nbd_idx]++;

                int spd_idx = spd_array[ati][cti];
                counts[1][ssmi_flag][spd_idx]++;

                int dir_idx = dir_array[ati][cti];
                counts[2][ssmi_flag][dir_idx]++;

                int mle_idx = mle_array[ati][cti];
                counts[3][ssmi_flag][mle_idx]++;

                int prb_idx = (int)(255.0 * prb_array[ati][cti] /
                    max_prb_array + 0.5);
                counts[4][ssmi_flag][prb_idx]++;
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
    // write histograms //
    //------------------//

    const char* param_map[] = { "NBD", "Speed", "Direction", "MLE", "Prob" };
    const char* rr_map[] = { "Clear", "Rain" };
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

/*
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

    if (opt_class && spd_file != NULL)
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
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, flag);
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
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, rain_rate[ati][cti]);
            }
        }
    }
*/

    //-----------------//
    // write out stats //
    //-----------------//

    printf("  Total WVC: %8ld\n", spd_avail_count);
    printf("   With NBD: %8ld (%2.2f %%)\n", with_nbd_count,
        100.0 * (double)with_nbd_count / (double)spd_avail_count);
    printf("Without NBD: %8ld (%2.2f %%)\n", without_nbd_count,
        100.0 * (double)without_nbd_count / (double)spd_avail_count);

    //-------------//
    // close files //
    //-------------//

    fclose(histo_ofp);
    fclose(matrix_ofp);
    fclose(prob_ofp);
    if (opt_class)
    {
        fclose(class_with_nbd_ofp);
        fclose(class_without_nbd_ofp);
    }
/*
    fclose(vs_ofp);
*/

    return (0);
}
