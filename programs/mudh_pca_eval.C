//==============================================================//
// Copyright (C) 1999-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_eval
//
// SYNOPSIS
//    mudh_pca_eval [ -r irr_threshold ] [ -s min_spd:max_spd ]
//        [ -f flag_dir ] [ -c time ] <start_rev> <end_rev>
//        <output_base>
//
// DESCRIPTION
//    Generate an evaluation chart of misclassification and
//    false alarm versus fraction flagged.
//
// OPTIONS
//    [ -r irr_threshold ]    Used to define rain.  Defaults to 2.0 km*mm/hr
//    [ -s min_spd:max_spd ]  Use the ECMWF speed range specified.
//    [ -f flag_dir ]         The directory containing the flag files.
//    [ -c time ]             The collocation time.  Defaults to 30 minutes.
//
// OPERANDS
//    <start_rev>     Duh.
//    <end_rev>       Duh again.
//    <output_base>   The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_eval 2500 3000 2500-3000
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
// AUTHORS
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
#include <math.h>
#include "Misc.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "r:s:f:c:"
#define QUOTE      '"'

#define DEFAULT_FLAG_DIR          "/export/svt11/hudd/pca"
#define DEFAULT_RAIN_DIR          "/export/svt11/hudd/ssmi"
#define DEFAULT_ECMWF_DIR         "/seapac/disk12/RAIN/ecmwf_coll_wind"

#define DEFAULT_IRR_THRESHOLD     2.0
#define DEFAULT_COLLOCATION_TIME  30
#define REV_DIGITS                5
#define PROB_BINS                 100
#define PROB_STEP                 0.05

// MUDH classes reduced to three choices
enum { MUDH_CLEAR = 0, MUDH_RAIN, MUDH_UNKNOWN, MUDH_3CLASS_COUNT };

// SSM/I classes reduced to three choices
enum { SSMI_CLEAR = 0, SSMI_MIST, SSMI_RAIN, SSMI_CLASS_COUNT };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r irr_threshold ]",
    "[ -s min_spd:max_spd ]", "[ -f flag_dir ]", "[ -c time ]",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// flag
static float           value_tab[AT_WIDTH][CT_WIDTH];
static unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

// irain
static unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
static unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
static unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

// ECMWF
static float           ecmwf_spd_array[AT_WIDTH][CT_WIDTH];

// Index 1: Swath index 0(both beams), 1(outer beam only), 2(unknown)
// Index 2: SSM/I class 0(irr=0), 1(0<irr<=irr_threshold), 2(irr>irr_threshold)
// Index 3: MUDH PCA class 0(clear), 1(rain), 2(unknown)
// Index 4: probability threshold index
static unsigned long counts[3][SSMI_CLASS_COUNT][MUDH_3CLASS_COUNT][PROB_BINS];

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

    int opt_spd = 0;
    float min_spd = 0.0;
    float max_spd = 0.0;

    float irr_threshold = DEFAULT_IRR_THRESHOLD;
    int collocation_time = DEFAULT_COLLOCATION_TIME;
    char* flag_dir = DEFAULT_FLAG_DIR;
    char* rain_dir = DEFAULT_RAIN_DIR;
    char* ecmwf_dir = DEFAULT_ECMWF_DIR;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            irr_threshold = atof(optarg);
            break;
        case 's':
            if (sscanf(optarg, "%f:%f", &min_spd, &max_spd) != 2)
            {
                fprintf(stderr, "%s: error parsing speed range (%s)\n",
                    command, optarg);
                exit(1);
            }
            opt_spd = 1;
            break;
        case 'f':
            flag_dir = optarg;
            break;
        case 'c':
            collocation_time = atoi(optarg);
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

    //--------------------//
    // process rev by rev //
    //--------------------//

    int rev_count = 0;
    unsigned int size = CT_WIDTH * AT_WIDTH;
    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read flag file //
        //----------------//

        char flag_file[1024];
        sprintf(flag_file, "%s/%0*d.flag", flag_dir, REV_DIGITS, rev);
        FILE* ifp = fopen(flag_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr,
                "%s: error opening input flag file %s (continuing)\n",
                command, flag_file);
            continue;
        }
        if (fread(value_tab, sizeof(float), size, ifp) != size ||
            fread(flag_tab,   sizeof(char), size, ifp) != size)
        {
            fprintf(stderr,
                "%s: error reading input flag file %s\n", command, flag_file);
            exit(1);
        }
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "%s/%0*d.irain", rain_dir, REV_DIGITS, rev);
        ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s (continuing)\n",
                command, rain_file);
            continue;
        }
        if (fread(rain_rate, sizeof(char), size, ifp) != size ||
            fread(time_dif,  sizeof(char), size, ifp) != size ||
            fseek(ifp, size, SEEK_CUR) == -1 ||
            fread(integrated_rain_rate, sizeof(short), size, ifp) != size)
        {
            fprintf(stderr, "%s: error reading input rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fclose(ifp);

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (abs(co_time) > collocation_time)
                    integrated_rain_rate[ati][cti] = 2000;    // flag as bad
            }
        }

        //------------------------------//
        // read ECMWF file if necessary //
        //------------------------------//

        if (opt_spd)
        {
            char ecmwf_file[1024];
            sprintf(ecmwf_file, "%s/E2B_%05d.dat", ecmwf_dir, rev);
            ifp = fopen(ecmwf_file, "r");
            if (ifp == NULL)
            {
                fprintf(stderr,
                    "%s: error opening ECMWF file %s (continuing)\n",
                    command, ecmwf_file);
                continue;
            }

            unsigned int seek_size =
                4 + 21 * AT_WIDTH + 4 +              // time
                4 + CT_WIDTH * AT_WIDTH * 4 + 4 +    // lat
                4 + CT_WIDTH * AT_WIDTH * 4 + 4 +    // lon
                4;                                   // speed (record length)
            unsigned int size = CT_WIDTH * AT_WIDTH;

            if (fseek(ifp, seek_size, SEEK_CUR) == -1 ||
                fread(ecmwf_spd_array, sizeof(float), size, ifp) != size)
            {
                fprintf(stderr, "%s: error reading ECMWF file %s\n", command,
                    ecmwf_file);
                exit(1);
            }
            fclose(ifp);
        }

        //-------------//
        // process rev //
        //-------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                //------------------//
                // check wind speed //
                //------------------//

                if (opt_spd)
                {
                    if (ecmwf_spd_array[ati][cti] < min_spd ||
                        ecmwf_spd_array[ati][cti] > max_spd)
                    {
                        continue;
                    }
                }

                //-------------------------//
                // check ssmi availability //
                //-------------------------//

                if (integrated_rain_rate[ati][cti] >= 1000)
                    continue;

                //----------------------//
                // determine ssmi class //
                //----------------------//

                float irr = integrated_rain_rate[ati][cti] * 0.1;
                int ssmi_class;
                if (irr == 0.0)
                    ssmi_class = SSMI_CLEAR;
                else if (irr > irr_threshold)
                    ssmi_class = SSMI_RAIN;
                else
                    ssmi_class = SSMI_MIST;

                //-------------------------------------//
                // step through probability thresholds //
                //-------------------------------------//

                for (int prob_idx = 0; prob_idx <= PROB_BINS; prob_idx++)
                {
                    float prob_thresh = (float)prob_idx * PROB_STEP;

                    //---------------//
                    // classify MUDH //
                    //---------------//

                    float prob_rain = value_tab[ati][cti];
                    int swath_idx = 2;    // unknown
                    int mudh_class = MUDH_UNKNOWN;
                    switch (flag_tab[ati][cti])
                    {
                    case BOTH_CLEAR:
                    case BOTH_RAIN:
                        swath_idx = 0;    // both beams
                        if (prob_rain <= prob_thresh)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case BOTH_UNKNOWN:
                        swath_idx = 0;    // both beams
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case OUTER_CLEAR:
                    case OUTER_RAIN:
                        swath_idx = 1;    // outer beam
                        if (prob_rain <= prob_thresh)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case OUTER_UNKNOWN:
                        swath_idx = 1;    // outer beam
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case UNKNOWN:
                        swath_idx = 2;    // unknown beam
                        // it will stay unknown
                        mudh_class = UNKNOWN;
                        break;
                    default:
                        fprintf(stderr, "%s: unknown classification\n",
                            command);
                        exit(1);
                        break;
                    }

                    counts[swath_idx][ssmi_class][mudh_class][prob_idx]++;
                }
            }
        }
        rev_count++;
    }

    //-------------//
    // output file //
    //-------------//

    const char* swath_ext[] = { "both", "outer" };
    const char* swath_string[] = { "Both beams", "Outer beam only" };
    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
        //------------------------------//
        // determine classified percent //
        //------------------------------//

        unsigned long total_count = 0;
        unsigned long classified_count = 0;
        for (int ssmi_class = 0; ssmi_class < SSMI_CLASS_COUNT; ssmi_class++)
        {
            // it doesn't matter what probility bin we use, so why not zero
            total_count += counts[swath_idx][ssmi_class][MUDH_CLEAR][0];
            total_count += counts[swath_idx][ssmi_class][MUDH_RAIN][0];
            total_count += counts[swath_idx][ssmi_class][MUDH_UNKNOWN][0];

            classified_count += counts[swath_idx][ssmi_class][MUDH_CLEAR][0];
            classified_count += counts[swath_idx][ssmi_class][MUDH_RAIN][0];
        }
        double classified_percent = 100.0 * (double)classified_count /
            (double)total_count;

        //-----------//
        // open file //
        //-----------//

        char filename[1024];
        sprintf(filename, "%s.%s", output_base, swath_ext[swath_idx]);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }

        fprintf(ofp, "@ title %c%s, %d revs (%d - %d)%c\n", QUOTE,
            swath_string[swath_idx], rev_count, start_rev, end_rev, QUOTE);
        fprintf(ofp, "@ subtitle %c%g percent classified%c\n", QUOTE,
             classified_percent, QUOTE);

        fprintf(ofp, "@ xaxis label %cPercent flagged as rain%c\n", QUOTE,
            QUOTE);
        fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ legend on\n");
//        fprintf(ofp, "@ legend string 0 %cUnflagged Rain (misclass)%c\n",
//            QUOTE, QUOTE);
//        fprintf(ofp, "@ legend string 1 %cFalse Alarm%c\n", QUOTE, QUOTE);

        for (int prob_idx = 0; prob_idx <= PROB_BINS; prob_idx++)
        {
//            float prob_thresh = (float)prob_idx * PROB_STEP;

            double mudh_r_count =
                (double)counts[swath_idx][SSMI_CLEAR][MUDH_RAIN][prob_idx] +
                (double)counts[swath_idx][SSMI_MIST][MUDH_RAIN][prob_idx] +
                (double)counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            double mudh_r_ssmi_r_count =
                (double)counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            double mudh_r_ssmi_mr_count =
                (double)counts[swath_idx][SSMI_MIST][MUDH_RAIN][prob_idx] +
                (double)counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            double mudh_c_count =
                (double)counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx] +
                (double)counts[swath_idx][SSMI_MIST][MUDH_CLEAR][prob_idx] +
                (double)counts[swath_idx][SSMI_RAIN][MUDH_CLEAR][prob_idx];

            double mudh_c_ssmi_c_count =
                (double)counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx];

            double mudh_c_ssmi_cm_count =
                (double)counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx] +
                (double)counts[swath_idx][SSMI_MIST][MUDH_CLEAR][prob_idx];

            if (mudh_r_count + mudh_c_count != classified_count)
            {
                fprintf(stderr, "%s: error in here!\n", command);
            }

            double mudh_r_percent = 100.0 * mudh_r_count / classified_count;
            double mudh_r_ssmi_r_percent =
                100.0 * mudh_r_ssmi_r_count / mudh_r_count;
            double mudh_r_ssmi_mr_percent =
                100.0 * mudh_r_ssmi_mr_count / mudh_r_count;
            double mudh_c_ssmi_c_percent =
                100.0 * mudh_c_ssmi_c_count / mudh_c_count;
            double mudh_c_ssmi_cm_percent =
                100.0 * mudh_c_ssmi_cm_count / mudh_c_count;

            fprintf(ofp, "%g %g %g %g %g\n", mudh_r_percent,
                mudh_r_ssmi_r_percent, mudh_r_ssmi_mr_percent,
                mudh_c_ssmi_c_percent, mudh_c_ssmi_cm_percent);
        }
        fclose(ofp);
    }

    return (0);
}
