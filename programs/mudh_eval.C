//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_eval
//
// SYNOPSIS
//    mudh_eval [ -m minutes ] [ -r irr_thresh ] <start_rev> <end_rev>
//        <output_file>
//
// DESCRIPTION
//    Generate an evaluation chart.
//
// OPTIONS
//    [ -m minutes ]     Time difference maximum.
//    [ -r irr_thresh ]  The SSM/I integrated rain rate defining "RAIN".
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_file>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_eval -m 30 -r 2.0 1550 1850 1550-1850
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
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "m:r:"
#define QUOTE      '"'

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m minutes ]", "[ -r irr_thresh ]",
    "<start_rev>", "<end_rev>", "<output_file>", 0 };

static float           value_tab[AT_WIDTH][CT_WIDTH];
static unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

static unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
static unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
static unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

static unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
static unsigned short spd_array[AT_WIDTH][CT_WIDTH];
static unsigned short dir_array[AT_WIDTH][CT_WIDTH];
static unsigned short mle_array[AT_WIDTH][CT_WIDTH];
static unsigned short lon_array[AT_WIDTH][CT_WIDTH];
static unsigned short lat_array[AT_WIDTH][CT_WIDTH];

static unsigned long   total_wvc;
static unsigned long   classified_wvc;
// Index 1: SSM/I 0=irr_zero, 1=irr>thresh, 2=otherwise
// Index 2: threshold index (threshold = index / 1000.0)
// Index 3: MUDH  0=no_rain,  1=rain
static unsigned long   ssmi_mudh_rain_wvc[3][100][2];
static unsigned long   mudh_unclass_wvc[3];   // ssmi as above

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

    int minutes = 30;              // 30 minutes
    float irr_thresh = 2.0;    // 2 km*mm/hr

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
        case 'r':
            irr_thresh = atof(optarg);
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
        sprintf(flag_file, "%d.pflag", rev);
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
        sprintf(rain_file, "/export/svt11/hudd/ssmi/%d.irain", rev);

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
        sprintf(mudh_file, "%d.mudh", rev);
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
                total_wvc++;

                // determine ssmi class
                float irr = integrated_rain_rate[ati][cti] * 0.1;
                int ssmi_flag;
                if (irr == 0.0)
                    ssmi_flag = 0;    // rainfree
                else if (irr > irr_thresh)
                    ssmi_flag = 1;    // rain
                else
                    ssmi_flag = 2;    // no-mans-land

                // check if classifiable
                float mudh_value = value_tab[ati][cti];
                if (mudh_value < 0.0 || mudh_value > 1.0)
                {
                    mudh_unclass_wvc[ssmi_flag]++;
                    continue;
                }

                classified_wvc++;
                for (int tidx = 0; tidx < 100; tidx++)
                {
                    float t_thresh = (float)tidx / 1000.0;

                    // determine mudh class
                    int mudh_flag;
                    if (mudh_value > t_thresh)
                        mudh_flag = 1;
                    else
                        mudh_flag = 0;

                    ssmi_mudh_rain_wvc[ssmi_flag][tidx][mudh_flag]++;
                }
            }
        }
    }

    //----------------------//
    // generate output file //
    //----------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    double classified_percent = 100.0 *
        (double)classified_wvc / (double)total_wvc;
    fprintf(ofp, "@ subtitle %c%g percent classified%c\n", QUOTE,
        classified_percent, QUOTE);

    fprintf(ofp, "@ xaxis label %cThreshold%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ yaxis label %cPercent%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ legend on\n");
    fprintf(ofp, "@ legend string 0 %cRain Flagged%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ legend string 1 %cRain Marked Good%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ legend string 2 %cClear Marked Bad%c\n", QUOTE, QUOTE);

    for (int tidx = 0; tidx < 100; tidx++)
    {
        float t_thresh = (float)tidx / 1000.0;
        double mudh_rain_count =
            (double)ssmi_mudh_rain_wvc[0][tidx][1] +
            (double)ssmi_mudh_rain_wvc[1][tidx][1] +
            (double)ssmi_mudh_rain_wvc[2][tidx][1];
        double mudh_rain_percent = 100.0 * mudh_rain_count /
            (double)classified_wvc;

        double ssmi_rain_mudh_clear_count =
            (double)ssmi_mudh_rain_wvc[1][tidx][0];
        double ssmi_rain_count =
            (double)ssmi_mudh_rain_wvc[1][tidx][0] +
            (double)ssmi_mudh_rain_wvc[1][tidx][1];
        double ssmi_rain_mudh_clear_percent = 100.0 *
            ssmi_rain_mudh_clear_count / ssmi_rain_count;

        double ssmi_clear_mudh_rain_count =
            (double)ssmi_mudh_rain_wvc[0][tidx][1];
        double ssmi_clear_count =
            (double)ssmi_mudh_rain_wvc[0][tidx][0] +
            (double)ssmi_mudh_rain_wvc[0][tidx][1];
        double ssmi_clear_mudh_rain_percent = 100.0 *
            ssmi_clear_mudh_rain_count / ssmi_clear_count;

        fprintf(ofp, "%g %g %g %g\n", t_thresh, mudh_rain_percent,
            ssmi_rain_mudh_clear_percent, ssmi_clear_mudh_rain_percent);
    }

    return (0);
}
