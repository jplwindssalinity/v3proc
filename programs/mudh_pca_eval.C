//==============================================================//
// Copyright (C) 1999-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_eval
//
// SYNOPSIS
//    mudh_pca_eval [ -e ecmwf_dir ] [ -t inner:outer ]
//        [ -r irr_threshold ] [ -s min_spd:max_spd ] [ -f flag_dir ]
//        [ -c time ] <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Generate an evaluation chart of misclassification and
//    false alarm versus fraction flagged.  Or, if inner and outer
//    thresholds are specified, plot versus integrated rain rate.
//
// OPTIONS
//    [ -e ecmwf_dir ]        Directory to get ECMWF files from.
//    [ -t inner:outer ]      Fixed thresholds.  Plot vs. IRR
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

#define OPTSTRING  "e:r:s:f:c:t:"
#define QUOTE      '"'

#define DEFAULT_FLAG_DIR          "/export/svt11/hudd/pca"
#define DEFAULT_RAIN_DIR          "/export/svt11/hudd/ssmi"
#define DEFAULT_ECMWF_DIR         "/seapac/disk11/ECMWF"

#define DEFAULT_IRR_THRESHOLD     2.0
#define DEFAULT_COLLOCATION_TIME  30
#define REV_DIGITS                5

// for the vs. rain rate histogram
#define IRR_BINS                  100
#define IRR_STEP                  0.3

// for the vs. probability histogram
#define PROB_BINS                 100
#define PROB_OFFSET               -2
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

const char* usage_array[] = { "[ -t inner:outer ]", "[ -r irr_threshold ]",
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

// Index 1: Swath index 0(inner swath), 1(outer swath), 2(unknown)
// Index 2: SSM/I class 0(irr=0), 1(0<irr<=irr_threshold), 2(irr>irr_threshold)
// Index 3: MUDH PCA class 0(clear), 1(rain), 2(unknown)
// Index 4: 0=probability, 1=irr
// Index 5: probability or rain index
static unsigned long counts[3][SSMI_CLASS_COUNT][MUDH_3CLASS_COUNT][PROB_BINS];

static unsigned long irr_counts[3][MUDH_3CLASS_COUNT][IRR_BINS];

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

    int opt_vs_irr = 0;
    float inner_threshold = 0.0;
    float outer_threshold = 0.0;

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
        case 'e':
            ecmwf_dir = optarg;
            break;
        case 't':
            if (sscanf(optarg, "%f:%f", &inner_threshold, &outer_threshold) !=
                2)
            {
                fprintf(stderr, "%s: error parsing thresholds (%s)\n",
                    command, optarg);
                exit(1);
            }
            opt_vs_irr = 1;
            break;
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

    unsigned long valid_wvc_count[2];
    valid_wvc_count[0] = 0;    // inner swath
    valid_wvc_count[1] = 0;    // outer swath
    unsigned long classified_wvc_count[2];
    classified_wvc_count[0] = 0;
    classified_wvc_count[1] = 0;

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

                //-----------------------//
                // count classifications //
                //-----------------------//

                switch (flag_tab[ati][cti])
                {
                case INNER_CLEAR:
                case INNER_RAIN:
                    classified_wvc_count[0]++;
                    valid_wvc_count[0]++;
                    break;
                case INNER_UNKNOWN:
                    valid_wvc_count[0]++;
                    break;
                case OUTER_CLEAR:
                case OUTER_RAIN:
                    classified_wvc_count[1]++;
                    valid_wvc_count[1]++;
                    break;
                case OUTER_UNKNOWN:
                    valid_wvc_count[1]++;
                    break;
                case NO_WIND:
                    break;
                case UNKNOWN:
                    break;
                }

                //--------------------------//
                // for the fixed thresholds //
                //--------------------------//

                if (opt_vs_irr)
                {
                    int rain_idx = (int)(irr / IRR_STEP + 0.5);
                    if (rain_idx > IRR_BINS - 1) rain_idx = IRR_BINS - 1;

                    //---------------//
                    // classify MUDH //
                    //---------------//

                    float prob_rain = value_tab[ati][cti];
                    int swath_idx = 2;    // unknown
                    int mudh_class = MUDH_UNKNOWN;
                    switch (flag_tab[ati][cti])
                    {
                    case INNER_CLEAR:
                    case INNER_RAIN:
                        swath_idx = 0;    // inner swath
                        if (prob_rain <= inner_threshold)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case INNER_UNKNOWN:
                        swath_idx = 0;    // inner swath
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case OUTER_CLEAR:
                    case OUTER_RAIN:
                        swath_idx = 1;    // outer swath
                        if (prob_rain <= outer_threshold)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case OUTER_UNKNOWN:
                        swath_idx = 1;    // outer swath
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case NO_WIND:
                    case UNKNOWN:
                        swath_idx = 2;    // unknown beam
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    default:
                        fprintf(stderr, "%s: unknown classification\n",
                            command);
                        exit(1);
                        break;
                    }
                    irr_counts[swath_idx][mudh_class][rain_idx]++;
                }

                //-------------------------------------//
                // step through probability thresholds //
                //-------------------------------------//

                for (int prob_idx = 0; prob_idx < PROB_BINS; prob_idx++)
                {
                    float prob_thresh = pow(10.0,
                        (float)prob_idx * PROB_STEP + PROB_OFFSET);

                    //---------------//
                    // classify MUDH //
                    //---------------//

                    float prob_rain = value_tab[ati][cti];
                    int swath_idx = 2;    // unknown
                    int mudh_class = MUDH_UNKNOWN;
                    switch (flag_tab[ati][cti])
                    {
                    case INNER_CLEAR:
                    case INNER_RAIN:
                        swath_idx = 0;    // inner swath
                        if (prob_rain <= prob_thresh)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case INNER_UNKNOWN:
                        swath_idx = 0;    // inner swath
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case OUTER_CLEAR:
                    case OUTER_RAIN:
                        swath_idx = 1;    // outer swath
                        if (prob_rain <= prob_thresh)
                            mudh_class = MUDH_CLEAR;
                        else
                            mudh_class = MUDH_RAIN;
                        break;
                    case OUTER_UNKNOWN:
                        swath_idx = 1;    // outer swath
                        // it will stay unknown
                        mudh_class = MUDH_UNKNOWN;
                        break;
                    case NO_WIND:
                    case UNKNOWN:
                        swath_idx = 2;    // unknown beam
                        mudh_class = MUDH_UNKNOWN;
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

    //--------------//
    // output files //
    //--------------//

    const char* swath_ext[] = { "inner", "outer" };
    const char* swath_string[] = { "Inner swath", "Outer swath" };
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

        double classified_percent = 100.0 *
            (double)classified_wvc_count[swath_idx] /
            (double)valid_wvc_count[swath_idx];

        // it doesn't matter what probility bin we use, so why not zero
        unsigned long ssmi_r_count =
            counts[swath_idx][SSMI_RAIN][MUDH_CLEAR][0] +
            counts[swath_idx][SSMI_RAIN][MUDH_RAIN][0] +
            counts[swath_idx][SSMI_RAIN][MUDH_UNKNOWN][0];

        unsigned long ssmi_c_count =
            counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][0] +
            counts[swath_idx][SSMI_CLEAR][MUDH_RAIN][0] +
            counts[swath_idx][SSMI_CLEAR][MUDH_UNKNOWN][0];

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

        char subtitle[1024];
        subtitle[0] = '\0';
        if (opt_spd)
        {
            sprintf(subtitle, "%.1f to %.1f m/s (ECMWF), ", min_spd, max_spd);
        }

        fprintf(ofp, "@ title %c%s, %d revs (%d - %d)%c\n", QUOTE,
            swath_string[swath_idx], rev_count, start_rev, end_rev, QUOTE);
        fprintf(ofp, "@ subtitle %c%s%g percent classified%c\n", QUOTE,
             subtitle, classified_percent, QUOTE);

        fprintf(ofp, "@ xaxis label %cPercent flagged as rain%c\n", QUOTE,
            QUOTE);
        fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ legend on\n");
        int legend_idx = 0;
/*
        fprintf(ofp, "@ legend string %d %cSSM/I R, MUDH R    %c\n",
            legend_idx++, QUOTE, QUOTE);
        fprintf(ofp, "@ legend string %d %cSSM/I RM, MUDH R    %c\n",
            legend_idx++, QUOTE, QUOTE);
        fprintf(ofp, "@ legend string %d %cSSM/I C, MUDH C    %c\n",
            legend_idx++, QUOTE, QUOTE);
        fprintf(ofp, "@ legend string %d %cSSM/I CM, MUDH C    %c\n",
            legend_idx++, QUOTE, QUOTE);
*/
        fprintf(ofp, "@ legend string %d %cMUDH C, SSM/I R    %c\n",
            legend_idx++, QUOTE, QUOTE);
        fprintf(ofp, "@ legend string %d %cMUDH R, SSM/I C    %c\n",
            legend_idx++, QUOTE, QUOTE);

        for (int prob_idx = 1; prob_idx < PROB_BINS; prob_idx++)
        {
            float prob_thresh = pow(10.0,
                (float)prob_idx * PROB_STEP + PROB_OFFSET);

            unsigned long mudh_r_count =
                counts[swath_idx][SSMI_CLEAR][MUDH_RAIN][prob_idx] +
                counts[swath_idx][SSMI_MIST][MUDH_RAIN][prob_idx] +
                counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            unsigned long ssmi_c_mudh_r_count =
                counts[swath_idx][SSMI_CLEAR][MUDH_RAIN][prob_idx];

            unsigned long ssmi_mr_mudh_r_count =
                counts[swath_idx][SSMI_MIST][MUDH_RAIN][prob_idx] +
                counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            unsigned long ssmi_r_mudh_r_count =
                counts[swath_idx][SSMI_RAIN][MUDH_RAIN][prob_idx];

            unsigned long mudh_c_count =
                counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx] +
                counts[swath_idx][SSMI_MIST][MUDH_CLEAR][prob_idx] +
                counts[swath_idx][SSMI_RAIN][MUDH_CLEAR][prob_idx];

            unsigned long ssmi_c_mudh_c_count =
                counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx];

            unsigned long ssmi_cm_mudh_c_count =
                counts[swath_idx][SSMI_CLEAR][MUDH_CLEAR][prob_idx] +
                counts[swath_idx][SSMI_MIST][MUDH_CLEAR][prob_idx];

            unsigned long ssmi_r_mudh_c_count =
                counts[swath_idx][SSMI_RAIN][MUDH_CLEAR][prob_idx];

            if (mudh_r_count + mudh_c_count != classified_count)
            {
                fprintf(stderr, "%s: Not adding up right!\n", command);
                fprintf(stderr, "   MUDH rain = %ld\n", mudh_r_count);
                fprintf(stderr, "  MUDH clear = %ld\n", mudh_c_count);
                fprintf(stderr, "  MUDH total = %ld\n", classified_count);
                fprintf(stderr, "%d %d\n", swath_idx, prob_idx);
            }

            double mudh_r_percent = 0.0;
            if (classified_count > 0)
                mudh_r_percent = 100.0 * mudh_r_count / classified_count;
            double ssmi_r_mudh_r_percent_m = 0.0;
            double ssmi_mr_mudh_r_percent_m = 0.0;
            if (mudh_r_count > 0)
            {
                ssmi_r_mudh_r_percent_m = 100.0 * ssmi_r_mudh_r_count /
                    mudh_r_count;
                ssmi_mr_mudh_r_percent_m = 100.0 * ssmi_mr_mudh_r_count /
                    mudh_r_count;
            }
            double ssmi_c_mudh_c_percent_m = 0.0;
            double ssmi_cm_mudh_c_percent_m = 0.0;
            if (mudh_c_count > 0)
            {
                ssmi_c_mudh_c_percent_m = 100.0 * ssmi_c_mudh_c_count /
                    mudh_c_count;
                ssmi_cm_mudh_c_percent_m = 100.0 * ssmi_cm_mudh_c_count /
                    mudh_c_count;
            }

            double ssmi_r_mudh_c_percent_s = 0.0;
            if (ssmi_r_count > 0)
            {
                ssmi_r_mudh_c_percent_s = 100.0 * ssmi_r_mudh_c_count /
                    ssmi_r_count;
            }

            double ssmi_c_mudh_r_percent_s = 0.0;
            if (ssmi_c_count > 0)
            {
                ssmi_c_mudh_r_percent_s = 100.0 * ssmi_c_mudh_r_count /
                    ssmi_c_count;
            }

            if (mudh_r_percent > 50.0)
                continue;    // don't bother
/*
            fprintf(ofp, "%g %g %g %g %g %g %g\n", mudh_r_percent,
                ssmi_r_mudh_r_percent_m, ssmi_mr_mudh_r_percent_m,
                ssmi_c_mudh_c_percent_m, ssmi_cm_mudh_c_percent_m,
                ssmi_r_mudh_c_percent_s, ssmi_c_mudh_r_percent_s);
*/
            fprintf(ofp, "%g %g %g %g\n", mudh_r_percent,
                ssmi_r_mudh_c_percent_s, ssmi_c_mudh_r_percent_s,
                prob_thresh);
        }
        fclose(ofp);
    }

    //--------------//
    // vs rain rate //
    //--------------//

    if (opt_vs_irr)
    {
        for (int swath_idx = 0; swath_idx < 2; swath_idx++)
        {
            //------------------------------//
            // determine classified percent //
            //------------------------------//

            unsigned long total_count = 0;
            unsigned long classified_count = 0;
            for (int rain_idx = 0; rain_idx < IRR_BINS; rain_idx++)
            {
                total_count +=
                    irr_counts[swath_idx][MUDH_CLEAR][rain_idx];
                total_count +=
                    irr_counts[swath_idx][MUDH_RAIN][rain_idx];
                total_count +=
                    irr_counts[swath_idx][MUDH_UNKNOWN][rain_idx];

                classified_count +=
                    irr_counts[swath_idx][MUDH_CLEAR][rain_idx];
                classified_count +=
                    irr_counts[swath_idx][MUDH_RAIN][rain_idx];
            }

            if (total_count != valid_wvc_count[swath_idx] ||
                classified_count != classified_wvc_count[swath_idx])
            {
                fprintf(stderr, "%s: something funny here.\n", command);
                exit(0);
            }

            double classified_percent = 100.0 *
                (double)classified_wvc_count[swath_idx] /
                (double)valid_wvc_count[swath_idx];

            //-----------//
            // open file //
            //-----------//

            char filename[1024];
            sprintf(filename, "%s.irr.%s", output_base, swath_ext[swath_idx]);
            FILE* ofp = fopen(filename, "w");
            if (ofp == NULL)
            {
                fprintf(stderr, "%s: error opening output file %s\n", command,
                    filename);
                exit(1);
            }

            char subtitle[1024];
            subtitle[0] = '\0';
            if (opt_spd)
            {
                sprintf(subtitle, "%.1f to %.1f m/s (ECMWF), ", min_spd,
                    max_spd);
            }

            fprintf(ofp, "@ title %c%s, %d revs (%d - %d)%c\n", QUOTE,
                swath_string[swath_idx], rev_count, start_rev, end_rev, QUOTE);
            fprintf(ofp, "@ subtitle %c%s%g percent classified%c\n", QUOTE,
                 subtitle, classified_percent, QUOTE);

            fprintf(ofp, "@ xaxis label %cIntegrated Rain Rate (km*mm/hr)%c\n",
                QUOTE, QUOTE);
            fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
            fprintf(ofp, "@ legend on\n");
            fprintf(ofp, "@ legend string 0 %cMUDH C    %c\n",
                QUOTE, QUOTE);
            fprintf(ofp, "@ legend string 1 %cMUDH R    %c\n",
                QUOTE, QUOTE);

            for (int rain_idx = 0; rain_idx < IRR_BINS; rain_idx++)
            {
                unsigned long mudh_r_count =
                    irr_counts[swath_idx][MUDH_RAIN][rain_idx];

                unsigned long mudh_c_count =
                    irr_counts[swath_idx][MUDH_CLEAR][rain_idx];

                unsigned long mudh_count =
                    irr_counts[swath_idx][MUDH_CLEAR][rain_idx] +
                    irr_counts[swath_idx][MUDH_RAIN][rain_idx] +
                    irr_counts[swath_idx][MUDH_UNKNOWN][rain_idx];

                if (mudh_count == 0)
                    continue;

                double mudh_r_percent = 100.0 * mudh_r_count / mudh_count;
                double mudh_c_percent = 100.0 * mudh_c_count / mudh_count;

                float irr = (float)rain_idx * IRR_STEP;
                fprintf(ofp, "%g %g %g\n", (float)irr,
                    mudh_c_percent, mudh_r_percent);
            }
            fclose(ofp);
        }
    }

    return (0);
}
