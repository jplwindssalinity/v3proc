//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_eval
//
// SYNOPSIS
//    mudh_eval [ -n ] [ -s min_spd:max_spd ] [ -t t1:t2:t3 ]
//       [ -d mudh_dir ] <mudhtab> <minutes> <irr_thresh> <start_rev>
//       <end_rev> <output_base>
//
// DESCRIPTION
//    Generate an evaluation chart for the fixed integrated
//    rain rate threshold (plotted vs. flagged percentage).
//
// OPTIONS
//    [ -n ]  Only evalulate when NBD was available.
//    [ -s min_spd:max_spd ]  Use the ECMWF speed range specified.
//    [ -t t1:t2:t3 ]  Three thresholds to use for the mudh thresh.
//    [ -d mudh_dir ]    An alternate directory for MUDH files.
//
// OPERANDS
//    <mudhtab>       The mudhtab.  Used for estimating the % threshold.
//    <minutes>       The collocation time (in minutes) for SSM/I
//    <irr_thresh>    The integrated rain rate threshold defining "RAIN"
//    <start_rev>     Duh.
//    <end_rev>       Duh again.
//    <output_base>   The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_eval 30 2.0 1550 1850 1550-1850
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
#include <math.h>
#include "Misc.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "ns:t:d:"
#define QUOTE      '"'

#define DEFAULT_MUDH_DIR  "/export/svt11/hudd/allmudh"
#define REV_DIGITS        5

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -n ]", "[ -s min_spd:max_spd ]",
    "[ -t t1:t2:t3 ]", "[ -d mudh_dir ]", "<mudhtab>", "<minutes>",
    "<irr_thresh>", "<start_rev>", "<end_rev>", "<output_base>", 0 };

// MUDHtab
static double  norain_prob[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double  rain_prob[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double  sample_count[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

// flag
static float           value_tab[AT_WIDTH][CT_WIDTH];
static unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

// irain
static unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
static unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
static unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

// MUDH
static unsigned short  nbd_array[AT_WIDTH][CT_WIDTH];
static unsigned short  spd_array[AT_WIDTH][CT_WIDTH];
static unsigned short  dir_array[AT_WIDTH][CT_WIDTH];
static unsigned short  mle_array[AT_WIDTH][CT_WIDTH];
static unsigned short  lon_array[AT_WIDTH][CT_WIDTH];
static unsigned short  lat_array[AT_WIDTH][CT_WIDTH];

// ECMWF
static float           ecmwf_spd_array[AT_WIDTH][CT_WIDTH];

static unsigned long   total_wvc;
static unsigned long   classified_wvc;

// Index 1: SSM/I 0=irr_zero, 1=irr>fixed_thresh, 2=otherwise
// Index 2: mudh threshold index (threshold = index / 500.0)
// Index 3: MUDH  0=no_rain,  1=rain
static unsigned long   ssmi_mthresh_mudh[3][100][2];

// Index 1: Flag fraction index
// Index 2: SSM/I 0=irr_zero, 1=irr>fixed_thresh, 2=otherwise
// Index 3: IRR threshold index (threshold = index / 10.0)
// Index 4: MUDH  0=no_rain,  1=rain
static unsigned long   ssmi_ithresh_mudh[3][3][100][2];

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

    int opt_nbd = 0;
    int opt_spd = 0;
    float min_spd = 0.0;
    float max_spd = 0.0;
    int opt_thresh = 0;
    float t1 = 0.0;
    float t2 = 0.0;
    float t3 = 0.0;

    char* mudh_dir = DEFAULT_MUDH_DIR;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 's':
            if (sscanf(optarg, "%f:%f", &min_spd, &max_spd) != 2)
            {
                fprintf(stderr, "%s: error parsing speed range (%s)\n",
                    command, optarg);
                exit(1);
            }
            opt_spd = 1;
            break;
        case 't':
            if (sscanf(optarg, "%f:%f:%f", &t1, &t2, &t3) != 3)
            {
                fprintf(stderr, "%s: error parsing thresholds (%s)\n",
                    command, optarg);
                exit(1);
            }
            opt_thresh = 1;
            break;
        case 'n':
            opt_nbd = 1;
            break;
        case 'd':
            mudh_dir = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 5)
        usage(command, usage_array, 1);

    const char* mudhtab_file = argv[optind++];
    int minutes = atoi(argv[optind++]);
    float irr_thresh = atof(argv[optind++]);
    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //-----------------------//
    // read the mudhtab file //
    //-----------------------//

    FILE* mudhtab_ifp = fopen(mudhtab_file, "r");
    if (mudhtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    unsigned int md_size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;
    if (fread(norain_prob, sizeof(double), md_size, mudhtab_ifp) != md_size ||
        fread(rain_prob, sizeof(double), md_size, mudhtab_ifp) != md_size ||
        fread(sample_count, sizeof(double), md_size, mudhtab_ifp) != md_size)
    {
        fprintf(stderr, "%s: error reading mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    fclose(mudhtab_ifp);

    //-------------------------------------------------------//
    // calculate thresholds for 5.0, 7.5, and 10.0 % flagged //
    //-------------------------------------------------------//

    double min_thresh[3] = { 0.0, 0.0, 0.0 };
    double max_thresh[3] = { 1.0, 1.0, 1.0 };
    double target_frac[3] = { 0.05, 0.075, 0.1 };
    double try_thresh[3];
    unsigned long rain_count[3];

    do
    {
      for (int i = 0; i < 3; i++)
      {
        try_thresh[i] = (max_thresh[i] + min_thresh[i]) / 2.0;
        rain_count[i] = 0;
      }
      unsigned long total_count = 0;
      for (int inbd = 0; inbd < NBD_DIM; inbd++)
      {
        for (int ispd = 0; ispd < SPD_DIM; ispd++)
        {
          for (int idir = 0; idir < DIR_DIM; idir++)
          {
            for (int imle = 0; imle < MLE_DIM; imle++)
            {
              total_count +=
                (unsigned long)(sample_count[inbd][ispd][idir][imle] + 0.5);
              for (int i = 0; i < 3; i++)
              {
                if (rain_prob[inbd][ispd][idir][imle] > try_thresh[i])
                {
                  rain_count[i] +=
                    (unsigned long)(sample_count[inbd][ispd][idir][imle] + 0.5);
                }
              }
            }
          }
        }
      }
      double sum_dif = 0.0;
      for (int i = 0; i < 3; i++)
      {
        double flag_frac = (double)rain_count[i] / (double)total_count;
        if (flag_frac < target_frac[i])
        {
          max_thresh[i] = try_thresh[i];
        }
        else
        {
          min_thresh[i] = try_thresh[i];
        }
        sum_dif += fabs(flag_frac - target_frac[i]);
      }
      if (sum_dif < 0.001)
          break;
    } while (1);
printf("%g %g %g\n", try_thresh[0], try_thresh[1], try_thresh[2]);

    if (opt_thresh)
    {
        try_thresh[0] = t1;
        try_thresh[1] = t2;
        try_thresh[2] = t3;
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned int size = CT_WIDTH * AT_WIDTH;
    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read flag file //
        //----------------//

        char flag_file[1024];
        sprintf(flag_file, "%s/%0*d.pflag", mudh_dir, REV_DIGITS, rev);
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
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%0*d.irain", REV_DIGITS,
            rev);

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

        //----------------//
        // read mudh file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%s/%0*d.mudh", mudh_dir, REV_DIGITS, rev);
        ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s (continuing)\n",
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
            fprintf(stderr, "%s: error reading MUDH file %s\n", command,
                mudh_file);
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
                if (abs(co_time) > minutes)
                    integrated_rain_rate[ati][cti] = 2000;    // flag as bad
            }
        }

        //------------------------------//
        // read ECMWF file if necessary //
        //------------------------------//

        if (opt_spd)
        {
            char ecmwf_file[1024];
            sprintf(ecmwf_file,
                "/seapac/disk12/RAIN/ecmwf_coll_wind/E2B_%05d.dat", rev);
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
/*
        for (int i = 0; i < AT_WIDTH; i++)
            for (int j = 0; j < CT_WIDTH; j++)
                if (ecmwf_spd_array[i][j] != 0.0)
                    printf("%g %g %g\n", lon_array[i][j] * 0.01, lat_array[i][j] * 0.01 - 90.0, ecmwf_spd_array[i][j]);

        exit(1);
*/

        //-------------//
        // process rev //
        //-------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                // skip "unevaluatable" WVC, i.e. WVC without a retreived
                // wind vector or a collocated SSM/I measurement

                if (spd_array[ati][cti] == MAX_SHORT ||
                    integrated_rain_rate[ati][cti] >= 1000)
                {
                    continue;
                }

                // if NBD only flag is set, eliminate NBD N/A data
                if (opt_nbd && nbd_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

                // check wind speed range if desired
                if (opt_spd)
                {
                    if (ecmwf_spd_array[ati][cti] < min_spd ||
                        ecmwf_spd_array[ati][cti] > max_spd)
                    {
                        continue;
                    }
                }

                total_wvc++;

                //--------------------------------//
                // check if WVC can be classified //
                //--------------------------------//

                float mudh_value = value_tab[ati][cti];
                if (mudh_value < 0.0 && mudh_value > 1.0)
                    continue;

                classified_wvc++;

                //-------------------------//
                // for fixed irr threshold //
                //-------------------------//

                float irr = integrated_rain_rate[ati][cti] * 0.1;

                // determine ssmi class
                int ssmi_flag;
                if (irr == 0.0)
                    ssmi_flag = 0;    // rainfree
                else if (irr > irr_thresh)
                    ssmi_flag = 1;    // rain
                else
                    ssmi_flag = 2;    // no-mans-land

                for (int tidx = 0; tidx < 100; tidx++)
                {
                    // determine mudh class
                    float m_thresh = (float)tidx / 500.0;
                    int mudh_flag;
                    if (mudh_value > m_thresh)
                        mudh_flag = 1;
                    else
                        mudh_flag = 0;

                    ssmi_mthresh_mudh[ssmi_flag][tidx][mudh_flag]++;
                }

                //--------------------------------//
                // for the three fixed flag rates //
                //--------------------------------//

                for (int fidx = 0; fidx < 3; fidx++)
                {
                    // determine MUDH class
                    int mudh_flag;
                    float m_thresh = try_thresh[fidx];
                    if (mudh_value > m_thresh)
                        mudh_flag = 1;
                    else
                        mudh_flag = 0;

                    for (int tidx = 0; tidx < 100; tidx++)
                    {
                        // determine SSM/I class
                        float i_thresh = (float)tidx / 10.0;

                        if (irr == 0.0)
                            ssmi_flag = 0;    // rainfree
                        else if (irr > i_thresh)
                            ssmi_flag = 1;    // rain
                        else
                            ssmi_flag = 2;    // no-mans-land

                        ssmi_ithresh_mudh[fidx][ssmi_flag][tidx][mudh_flag]++;
                    }
                }
            }
        }
    }

    //----------------------------------------------//
    // generate the fixed integrated rain rate file //
    //----------------------------------------------//

    char filename[1024];
    sprintf(filename, "%s.firr.eval", output_base);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    double classified_percent = 100.0 *
        (double)classified_wvc / (double)total_wvc;
    fprintf(ofp, "@ title %cRain Means Integrated Rain > %g km*mm/hr%c\n",
        QUOTE, irr_thresh, QUOTE);
    fprintf(ofp, "@ subtitle %c%g percent classified%c\n", QUOTE,
        classified_percent, QUOTE);

    fprintf(ofp, "@ xaxis label %cPercent flagged as rain%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ legend on\n");
    fprintf(ofp, "@ legend string 0 %cMisclassification (unflagged rain)%c\n",
        QUOTE, QUOTE);
    fprintf(ofp, "@ legend string 1 %cFalse Alarm%c\n",
        QUOTE, QUOTE);
//    fprintf(ofp, "@ legend string 2 %cThreshold%c\n", QUOTE, QUOTE);

    for (int tidx = 0; tidx < 100; tidx++)
    {
        float m_thresh = (float)tidx / 500.0;
        double mudh_rain_count =
            (double)ssmi_mthresh_mudh[0][tidx][1] +
            (double)ssmi_mthresh_mudh[1][tidx][1] +
            (double)ssmi_mthresh_mudh[2][tidx][1];
        double mudh_rain_percent = 100.0 * mudh_rain_count /
            (double)classified_wvc;

        double ssmi_rain_mudh_clear_count =
            (double)ssmi_mthresh_mudh[1][tidx][0];
        double ssmi_rain_count =
            (double)ssmi_mthresh_mudh[1][tidx][0] +
            (double)ssmi_mthresh_mudh[1][tidx][1];
        double ssmi_rain_mudh_clear_percent = 100.0 *
            ssmi_rain_mudh_clear_count / ssmi_rain_count;

        double ssmi_clear_mudh_rain_count =
            (double)ssmi_mthresh_mudh[0][tidx][1];
        double ssmi_clear_count =
            (double)ssmi_mthresh_mudh[0][tidx][0] +
            (double)ssmi_mthresh_mudh[0][tidx][1];
        double ssmi_clear_mudh_rain_percent = 100.0 *
            ssmi_clear_mudh_rain_count / ssmi_clear_count;

        fprintf(ofp, "%g %g %g %g\n", mudh_rain_percent,
            ssmi_rain_mudh_clear_percent, ssmi_clear_mudh_rain_percent,
            m_thresh);
    }
    fclose(ofp);

    //----------------------------------//
    // generate the fixed fraction file //
    //----------------------------------//

    sprintf(filename, "%s.fper.eval", output_base);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    double percent_flagged_as_rain[3];
    for (int fidx = 0; fidx < 3; fidx++)
    {
        double mudh_rain_count =
            (double)ssmi_ithresh_mudh[fidx][0][0][1] +
            (double)ssmi_ithresh_mudh[fidx][1][0][1] +
            (double)ssmi_ithresh_mudh[fidx][2][0][1];
        percent_flagged_as_rain[fidx] = 100.0 * mudh_rain_count /
            (double)classified_wvc;
    }

/*
    fprintf(ofp, "@ title %cRain Means Integrated Rain > %g km*mm/hr%c\n",
        QUOTE, irr_thresh, QUOTE);
    fprintf(ofp, "@ title %cPercent flagged as rain = %.2f, %.2f, %.2f%c\n",
        QUOTE, percent_flagged_as_rain[0], percent_flagged_as_rain[1],
        percent_flagged_as_rain[2], QUOTE);
*/
    fprintf(ofp, "@ subtitle %c%g percent classified%c\n", QUOTE,
        classified_percent, QUOTE);

    fprintf(ofp, "@ xaxis label %cIntegrated Rain Rate (km*mm/hr)%c\n",
        QUOTE, QUOTE);
    fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ legend on\n");
    fprintf(ofp, "@ legend string 0 %cMisclassification (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[0], QUOTE);
    fprintf(ofp, "@ legend string 1 %cFalse Alarm (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[0],  QUOTE);
    fprintf(ofp, "@ legend string 2 %cMisclassification (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[1], QUOTE);
    fprintf(ofp, "@ legend string 3 %cFalse Alarm (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[1],  QUOTE);
    fprintf(ofp, "@ legend string 4 %cMisclassification (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[2], QUOTE);
    fprintf(ofp, "@ legend string 5 %cFalse Alarm (%.1f %% flagged)%c\n",
        QUOTE, percent_flagged_as_rain[2],  QUOTE);
/*
    fprintf(ofp, "@ legend string 1 %cFalse Alarm%c\n",
        QUOTE, QUOTE);
    fprintf(ofp, "@ legend string 2 %cThreshold%c\n", QUOTE, QUOTE);
*/

    for (int fidx = 0; fidx < 3; fidx++)
    {
    for (int tidx = 0; tidx < 100; tidx++)
    {
        float i_thresh = (float)tidx / 10.0;
/*
        double mudh_rain_count =
            (double)ssmi_ithresh_mudh[fidx][0][tidx][1] +
            (double)ssmi_ithresh_mudh[fidx][1][tidx][1] +
            (double)ssmi_ithresh_mudh[fidx][2][tidx][1];
        double mudh_rain_percent = 100.0 * mudh_rain_count /
            (double)classified_wvc;
*/

        double ssmi_rain_mudh_clear_count =
            (double)ssmi_ithresh_mudh[fidx][1][tidx][0];
        double ssmi_rain_count =
            (double)ssmi_ithresh_mudh[fidx][1][tidx][0] +
            (double)ssmi_ithresh_mudh[fidx][1][tidx][1];
        double ssmi_rain_mudh_clear_percent = 100.0 *
            ssmi_rain_mudh_clear_count / ssmi_rain_count;

        double ssmi_clear_mudh_rain_count =
            (double)ssmi_ithresh_mudh[fidx][0][tidx][1];
        double ssmi_clear_count =
            (double)ssmi_ithresh_mudh[fidx][0][tidx][0] +
            (double)ssmi_ithresh_mudh[fidx][0][tidx][1];
        double ssmi_clear_mudh_rain_percent = 100.0 *
            ssmi_clear_mudh_rain_count / ssmi_clear_count;

        fprintf(ofp, "%g %g %g\n", i_thresh, ssmi_rain_mudh_clear_percent,
            ssmi_clear_mudh_rain_percent);
    }
    fprintf(ofp, "&\n");
    }
    fclose(ofp);

    return (0);
}
