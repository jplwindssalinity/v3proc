//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_table
//
// SYNOPSIS
//    mudh_table [ -h ] [ -m minutes ] [ -r rain_rate ] <start_rev>
//        <end_rev> <output_base>
//
// DESCRIPTION
//    Generates a mudh table for classification.
//
// OPTIONS
//    [ -h ]            Make a sample histogram.
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
//      % mudh_table -m 15 1500 1600 rv1500-1600.15min
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

#define OPTSTRING    "hm:r:"
#define AT_WIDTH     1624
#define CT_WIDTH     76
#define MIN_SAMPLES  100

#define MAX_SHORT  65535

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_hist = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -h ]", "[ -m minutes ]", "[ -r rain_rate ]",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// last index: SSM/I class (0=all, 1=rainfree, 2=rain)
static unsigned long counts[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM][3];

static double norain_tab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double all_count[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

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

    int minutes = 60;              // default one hour
    float rain_threshold = 1.0;    // default one mm/hr

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'h':
            opt_hist = 1;
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

    //--------------//
    // simple calcs //
    //--------------//

    float nbd_spread = NBD_MAX - NBD_MIN;
    int max_inbd = NBD_DIM - 2;    // save room for missing NBD index

    float spd_spread = SPD_MAX - SPD_MIN;
    int max_ispd = SPD_DIM - 1;

    float dir_spread = DIR_MAX - DIR_MIN;
    int max_idir = DIR_DIM - 1;

    float mle_spread = MLE_MAX - MLE_MIN;
    int max_imle = MLE_DIM - 1;

    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.mudhtab", output_base);
    FILE* mudhtab_ofp = fopen(filename, "w");
    if (mudhtab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //---------------//
        // read nbd file //
        //---------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%d.mudh", rev);
        FILE* ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(nbd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(spd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(dir_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(mle_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%d.rain", rev);
        unsigned char rain_rate[AT_WIDTH][CT_WIDTH];
        unsigned char time_dif[AT_WIDTH][CT_WIDTH];
        ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fread(rain_rate, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fread(time_dif, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (abs(co_time) > minutes)
                    rain_rate[ati][cti] = 255;
            }
        }

        //----------------//
        // generate table //
        //----------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                // need the following: SSM/I rain rate, speed, direction, MLE
                if (rain_rate[ati][cti] >= 250 ||
                    spd_array[ati][cti] == MAX_SHORT ||
                    dir_array[ati][cti] == MAX_SHORT ||
                    mle_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

                // convert back to real values
                double nbd = (double)nbd_array[ati][cti] * 0.001 - 10.0;
                double spd = (double)spd_array[ati][cti] * 0.01;
                double dir = (double)dir_array[ati][cti] * 0.01;
                double mle = (double)mle_array[ati][cti] * 0.001 - 30.0;

                // convert to tighter indicies
                int inbd = (int)((nbd - NBD_MIN) * (float)max_inbd /
                    nbd_spread + 0.5);
                int ispd = (int)((spd - SPD_MIN) * (float)max_ispd /
                    spd_spread + 0.5);
                int idir = (int)((dir - DIR_MIN) * (float)max_idir /
                    dir_spread + 0.5);
                int imle = (int)((mle - MLE_MIN) * (float)max_imle /
                    mle_spread + 0.5);

                // nbd is scaled to fewer values so we can...
                if (inbd < 0) inbd = 0;
                if (inbd > max_inbd) inbd = max_inbd;

                // ...hack in a "missing nbd" index
                if (nbd_array[ati][cti] == MAX_SHORT) inbd = max_inbd + 1;

                if (ispd < 0) ispd = 0;
                if (ispd > max_ispd) ispd = max_ispd;
                if (idir < 0) idir = 0;
                if (idir > max_idir) idir = max_idir;
                if (imle < 0) imle = 0;
                if (imle > max_imle) imle = max_imle;

                float rr = (float)rain_rate[ati][cti] * 0.1;
                if (rr == 0.0)
                    counts[inbd][ispd][idir][imle][1]++;    // rainfree
                if (rr > rain_threshold)
                    counts[inbd][ispd][idir][imle][2]++;    // rain
                counts[inbd][ispd][idir][imle][0]++;        // all
            }
        }
    }

    //-------------//
    // write table //
    //-------------//

    for (int i = 0; i < NBD_DIM; i++)
    {
        for (int j = 0; j < SPD_DIM; j++)
        {
            for (int k = 0; k < DIR_DIM; k++)
            {
                for (int l = 0; l < MLE_DIM; l++)
                {
                    norain_tab[i][j][k][l] = 0.0;
                    rain_tab[i][j][k][l] = 0.0;
                    all_count[i][j][k][l] = (double)counts[i][j][k][l][0];
                    if (counts[i][j][k][l][0] > MIN_SAMPLES)
                    {
                        double norain_prob = (double)counts[i][j][k][l][1] /
                            (double)counts[i][j][k][l][0];
                        double rain_prob = (double)counts[i][j][k][l][2] /
                            (double)counts[i][j][k][l][0];

                        norain_tab[i][j][k][l] = norain_prob;
                        rain_tab[i][j][k][l] = rain_prob;
                    }
                    else
                    {
                        // mark as uncalculatable (2.0)
                        norain_tab[i][j][k][l] = 2.0;
                        rain_tab[i][j][k][l] = 2.0;
                    }
                }
            }
        }
    }
    fwrite(norain_tab, sizeof(double),
        NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM, mudhtab_ofp);
    fwrite(rain_tab, sizeof(double),
        NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM, mudhtab_ofp);
    fwrite(all_count, sizeof(double),
        NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM, mudhtab_ofp);

    //-------------//
    // close files //
    //-------------//

    fclose(mudhtab_ofp);

    //-----------------------//
    // make a histogram file //
    //-----------------------//

    if (opt_hist)
    {
        sprintf(filename, "%s.hist", output_base);
        FILE* hist_ofp = fopen(filename, "w");
        if (hist_ofp == NULL)
        {
            fprintf(stderr, "%s: error opening histogram file %s\n", command,
                filename);
            exit(1);
        }

        // find max count
        unsigned long max_count = 0;
        unsigned long total_count = 0;
        for (int i = 0; i < NBD_DIM; i++)
        {
            for (int j = 0; j < SPD_DIM; j++)
            {
                for (int k = 0; k < DIR_DIM; k++)
                {
                    for (int l = 0; l < MLE_DIM; l++)
                    {
                        total_count += counts[i][j][k][l][0];
                        if (counts[i][j][k][l][0] > max_count)
                            max_count = counts[i][j][k][l][0];
                    }
                }
            }
        }

        unsigned long cell_PDF_sum = 0;
        unsigned long data_sum = 0;
        unsigned long data_PDF_sum = 0;
        for (unsigned long target = 0; target <= max_count; target++)
        {
            unsigned long target_count = 0;
            for (int i = 0; i < NBD_DIM; i++)
            {
                for (int j = 0; j < SPD_DIM; j++)
                {
                    for (int k = 0; k < DIR_DIM; k++)
                    {
                        for (int l = 0; l < MLE_DIM; l++)
                        {
                            if (counts[i][j][k][l][0] == target)
                            {
                                target_count++;
                            }
                        }
                    }
                }
            }
            cell_PDF_sum += target_count;
            data_sum = (target_count * target);
            data_PDF_sum += data_sum;
            double cell_PDF = (double)cell_PDF_sum /
                (double)(NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM);
            double data_PDF = (double)data_PDF_sum / (double)(total_count);
            fprintf(hist_ofp, "%ld %g %g\n", target, cell_PDF, data_PDF);
        }
        fclose(hist_ofp);
    }

    return (0);
}
