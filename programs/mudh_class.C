//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_class
//
// SYNOPSIS
//    mudh_class [ -c ] [ -m minutes ] [ -r rain_rate ] <mudhtab>
//        <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Generate classification information.
//
// OPTIONS
//    [ -c ]               Generate class files.
//    [ -m minutes ]       Time difference maximum.
//    [ -r rain_rate ]     The SSM/I rain rate to threshold.
//
// OPERANDS
//    <mudhtab>      Use this mudhtab.
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_class -c -m 60 -r 1.0 mudhtab.1 1500 1600 run5
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

#define OPTSTRING    "ch:s:m:r:"
#define ATI_SIZE     1624
#define CTI_SIZE     76

#define REV_DOY_M    7.029760E-2
#define REV_DOY_B    1.703545E+2

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
    "<mudhtab>", "<start_rev>", "<end_rev>", "<output_base>", 0 };

// Index 1: Parameter: nbd, spd, dir, mle
// Index 2: Source: 0=data, 1=table average
// Index 3: Probability: 0=rainfree, 1=rain, 2=no-mans-land
// Index 4: NBD availability: 0=no nbd, 1=with nbd
// Index 5: parameter index
static unsigned long counts[4][2][3][2][256];

// just skip the data source
double prob_sum[4][3][2][256];

static double norain_tab[16][16][16][16];
static double rain_tab[16][16][16][16];

unsigned long classifiable_count = 0;
unsigned long with_nbd_count = 0;
unsigned long without_nbd_count = 0;
unsigned long missing_prob_count = 0;

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

    int minutes = 60;              // one hours
    float rain_threshold = 1.0;    // one mm/hr
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
        case 'c':
            opt_class = 1;
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

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* mudhtab_file = argv[optind++];
    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //-------------------//
    // read mudhtab file //
    //-------------------//

    FILE* mudhtab_ifp = fopen(mudhtab_file, "r");
    if (mudhtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    unsigned int size = 16 * 16 * 16 * 16;
    if (fread(norain_tab, sizeof(double), size, mudhtab_ifp) != size ||
        fread(rain_tab, sizeof(double), size, mudhtab_ifp) != size)
    {
        fprintf(stderr, "%s: error reading mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    fclose(mudhtab_ifp);

    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];

    sprintf(filename, "%s.prob", output_base);
    FILE* prob_ofp = fopen(filename, "w");
    if (prob_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    FILE* class_with_nbd_ofp = NULL;
    FILE* class_without_nbd_ofp = NULL;
    FILE* param_ofp[4];
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

    const char* param_map[] = { "NBD", "Spd", "Dir", "MLE" };
    for (int i = 0; i < 4; i++)
    {
        sprintf(filename, "%s.%s", output_base, param_map[i]);
        param_ofp[i] = fopen(filename, "w");
        if (param_ofp[i] == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char  nbd_array[ATI_SIZE][CTI_SIZE];
    unsigned char  spd_array[ATI_SIZE][CTI_SIZE];
    unsigned char  dir_array[ATI_SIZE][CTI_SIZE];
    unsigned char  mle_array[ATI_SIZE][CTI_SIZE];
    unsigned short lon_array[ATI_SIZE][CTI_SIZE];
    unsigned short lat_array[ATI_SIZE][CTI_SIZE];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read mudh file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%d.mudh", rev);
        FILE* ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            exit(1);
        }
        unsigned long size = CTI_SIZE * ATI_SIZE;
        if (fread(nbd_array,  sizeof(char), size, ifp) != size ||
            fread(spd_array,  sizeof(char), size, ifp) != size ||
            fread(dir_array,  sizeof(char), size, ifp) != size ||
            fread(mle_array,  sizeof(char), size, ifp) != size ||
            fread(lon_array, sizeof(short), size, ifp) != size ||
            fread(lat_array, sizeof(short), size, ifp) != size)
        {
            fclose(ifp);
            fprintf(stderr, "%s: error reading MUDH file %s (continuing)\n",
                command, mudh_file);
            continue;
        }
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%d.irain", rev);

        unsigned char rain_rate[ATI_SIZE][CTI_SIZE];
        unsigned char time_dif[ATI_SIZE][CTI_SIZE];
        unsigned short integrated_rain_rate[ATI_SIZE][CTI_SIZE];

        ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fread(rain_rate, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fread(time_dif, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fseek(ifp, CTI_SIZE * ATI_SIZE, SEEK_CUR);    // skip ins.
        fread(integrated_rain_rate, sizeof(short), CTI_SIZE * ATI_SIZE, ifp);
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
                    integrated_rain_rate[ati][cti] = 2000;    // flag as bad
            }
        }

        //-------------//
        // process rev //
        //-------------//

        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                //----------------------------//
                // accumulate some count info //
                //----------------------------//
                // even if you can't evaulte it

                if (spd_array[ati][cti] != 255 &&
                    dir_array[ati][cti] != 255 &&
                    mle_array[ati][cti] != 255)
                {
                    classifiable_count++;

                    if (nbd_array[ati][cti] != 255)
                        with_nbd_count++;
                    else
                        without_nbd_count++;
                }

                //-------------------------------------//
                // if you can't evaluate it, forget it //
                //-------------------------------------//

                if (integrated_rain_rate[ati][cti] >= 1000 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255 ||
                    mle_array[ati][cti] == 255)
                {
                    continue;
                }

                //--------------------------//
                // determine table indicies //
                //--------------------------//

                // convert back to real values
                double nbd = (double)nbd_array[ati][cti] / 20.0 - 6.0;
                double spd = (double)spd_array[ati][cti] / 5.0;
                double dir = (double)dir_array[ati][cti] / 2.0;
                double mle = (double)mle_array[ati][cti] / 8.0 - 30.0;

                // convert to tighter indicies
                // nbd -4 to 6
                // spd 0 to 30
                // dir 0 to 90
                // mle -10 to 0
                int inbd = (int)((nbd +  4.0) * 14.0 / 10.0 + 0.5);
                int ispd = (int)((spd +  0.0) * 15.0 / 30.0 + 0.5);
                int idir = (int)((dir +  0.0) * 15.0 / 90.0 + 0.5);
                int imle = (int)((mle + 10.0) * 15.0 / 10.0 + 0.5);

                // nbd is scaled to 15 values so we can...
                if (inbd < 0) inbd = 0;
                if (inbd > 14) inbd = 14;

                // ...hack in a "missing nbd" index
                if (nbd_array[ati][cti] == 255) inbd = 15;

                if (ispd < 0) ispd = 0;
                if (ispd > 15) ispd = 15;
                if (idir < 0) idir = 0;
                if (idir > 15) idir = 15;
                if (imle < 0) imle = 0;
                if (imle > 15) imle = 15;

                //----------------------//
                // determine ssmi class //
                //----------------------//

                float irr = integrated_rain_rate[ati][cti] * 0.1;
                int ssmi_flag;
                if (irr == 0.0)
                    ssmi_flag = 0;    // rainfree
                else if (irr > rain_threshold)
                    ssmi_flag = 1;    // rain
                else
                    ssmi_flag = 2;    // no-mans-land

                //------------------------------------------//
                // look up probabilities from the mudhtable //
                //------------------------------------------//

                double norain_prob = norain_tab[inbd][ispd][idir][imle];
                double rain_prob = rain_tab[inbd][ispd][idir][imle];

                //----------------------------------------------------//
                // if there is insufficient probability info, skip it //
                //----------------------------------------------------//

                if (norain_prob > 1.5 || rain_prob > 1.5)
                {
                    missing_prob_count++;
                    continue;
                }

                //----------------------//
                // write to class files //
                //----------------------//

                if (opt_class)
                {
//                    float lon = (float)lon_array[ati][cti] * 0.01;
//                    float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
                    if (inbd == 15)
                    {
                        fprintf(class_without_nbd_ofp,
                            "%g %g %g %g %g %g %g %d %d %d\n", nbd, spd,
                            dir, mle, norain_prob, rain_prob, irr, rev, ati,
                            cti);
                    }
                    else
                    {
                        fprintf(class_with_nbd_ofp,
                            "%g %g %g %g %g %g %g %d %d %d\n", nbd, spd,
                            dir, mle, norain_prob, rain_prob, irr, rev, ati,
                            cti);
                    }
                }

                //----------------------//
                // accumulate from data //
                //----------------------//

                int nbd_idx = nbd_array[ati][cti];
                int spd_idx = spd_array[ati][cti];
                int dir_idx = dir_array[ati][cti];
                int mle_idx = mle_array[ati][cti];

                int nbd_avail = 1;
                if (nbd_idx == 255)
                    nbd_avail = 0;

                counts[0][0][ssmi_flag][nbd_avail][nbd_idx]++;
                counts[1][0][ssmi_flag][nbd_avail][spd_idx]++;
                counts[2][0][ssmi_flag][nbd_avail][dir_idx]++;
                counts[3][0][ssmi_flag][nbd_avail][mle_idx]++;

                //-----------------------//
                // accumulate from table //
                //-----------------------//

                counts[0][1][0][nbd_avail][nbd_idx]++;
                counts[0][1][1][nbd_avail][nbd_idx]++;
                prob_sum[0][0][nbd_avail][nbd_idx] += norain_prob;
                prob_sum[0][1][nbd_avail][nbd_idx] += rain_prob;

                counts[1][1][0][nbd_avail][spd_idx]++;
                counts[1][1][1][nbd_avail][spd_idx]++;
                prob_sum[1][0][nbd_avail][spd_idx] += norain_prob;
                prob_sum[1][1][nbd_avail][spd_idx] += rain_prob;

                counts[2][1][0][nbd_avail][dir_idx]++;
                counts[2][1][1][nbd_avail][dir_idx]++;
                prob_sum[2][0][nbd_avail][dir_idx] += norain_prob;
                prob_sum[2][1][nbd_avail][dir_idx] += rain_prob;

                counts[3][1][0][nbd_avail][mle_idx]++;
                counts[3][1][1][nbd_avail][mle_idx]++;
                prob_sum[3][0][nbd_avail][mle_idx] += norain_prob;
                prob_sum[3][1][nbd_avail][mle_idx] += rain_prob;
            }
        }
    }

    //---------------------//
    // write probabilities //
    //---------------------//

/*
    const char* param_map[] = { "NBD", "Speed", "Direction", "MLE" };
    const char* rr_map[] = { "Rainfree", "Rain" };
*/
    float param_b[] = { 6.0, 0.0, 0.0, 30.0 };
    float param_m[] = { 20.0, 5.0, 2.0, 8.0 };

    // from data
    for (int param_idx = 0; param_idx < 4; param_idx++)
    {
        for (int nbd_avail = 0; nbd_avail < 2; nbd_avail++)
        {
            if (param_idx == 0 && nbd_avail == 0)
                continue;
            for (int idx = 0; idx < 255; idx++)
            {
                // determine total number of points
                unsigned long cell_total = 0;
                for (int cond_idx = 0; cond_idx < 3; cond_idx++)
                {
                    cell_total +=
                        counts[param_idx][0][cond_idx][nbd_avail][idx];
                }
                if (cell_total == 0)
                    continue;

                float value = (float)idx / param_m[param_idx] -
                    param_b[param_idx];
                double rainfree_prob =
                    (double)counts[param_idx][0][0][nbd_avail][idx] /
                    (double)cell_total;
                double rain_prob =
                    (double)counts[param_idx][0][1][nbd_avail][idx] /
                    (double)cell_total;

                fprintf(param_ofp[param_idx], "%g %g %g\n", value,
                    rainfree_prob, rain_prob);
            }
            fprintf(param_ofp[param_idx], "&\n");
        }
    }

    // average from table
    for (int param_idx = 0; param_idx < 4; param_idx++)
    {
        for (int nbd_avail = 0; nbd_avail < 2; nbd_avail++)
        {
            if (param_idx == 0 && nbd_avail == 0)
                continue;
            for (int idx = 0; idx < 255; idx++)
            {
                if (counts[param_idx][1][0][nbd_avail][idx] == 0 ||
                    counts[param_idx][1][1][nbd_avail][idx] == 0)
                {
                    continue;
                }

                float value = (float)idx / param_m[param_idx] -
                    param_b[param_idx];
                double rainfree_prob =
                    prob_sum[param_idx][0][nbd_avail][idx] /
                    (double)counts[param_idx][1][0][nbd_avail][idx];
                double rain_prob =
                    prob_sum[param_idx][1][nbd_avail][idx] /
                    (double)counts[param_idx][1][1][nbd_avail][idx];

                fprintf(param_ofp[param_idx], "%g %g %g\n", value,
                    rainfree_prob, rain_prob);
            }
            fprintf(param_ofp[param_idx], "&\n");
        }
    }

    //-----------------//
    // write out stats //
    //-----------------//

    printf("  Total WVC: %8ld\n", classifiable_count);
    printf("   With NBD: %8ld (%2.2f %%)\n", with_nbd_count,
        100.0 * (double)with_nbd_count / (double)classifiable_count);
    printf("Without NBD: %8ld (%2.2f %%)\n", without_nbd_count,
        100.0 * (double)without_nbd_count / (double)classifiable_count);
    printf("Missing P's: %8ld (%2.2f %%)\n", missing_prob_count,
        100.0 * (double)missing_prob_count / (double)classifiable_count);

    //-------------//
    // close files //
    //-------------//

    fclose(prob_ofp);
    if (opt_class)
    {
        fclose(class_with_nbd_ofp);
        fclose(class_without_nbd_ofp);
    }

    return (0);
}
