//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    tc_info
//
// SYNOPSIS
//    tc_info [ -c ref_tc_file ] <tc_file> <info_base>
//
// DESCRIPTION
//    Reads in a set of tracking constants and produces some
//    information files.
//
// OPTIONS
//    The following options are supported:
//      [ -c ref_tc_file ]  Compare the results to this file.
//
// OPERANDS
//    The following operands are supported:
//        <tc_file>    The tracking constants input file.
//        <info_base>  The basename for output information files.
//
// EXAMPLES
//    Examples of command lines are:
//      % tc_info -c dtc.1 dtc.new.1 dtc.info.1
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include <unistd.h>
#include "Misc.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Qscat.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "c:"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int  delay_scan(const char* filename, const char* rgc_file,
         RangeTracker* range_tracker);
int  ddelay_scan(const char* filename, const char* rgc_file,
         RangeTracker* range_tracker, const char* ref_rgc_file,
         RangeTracker* ref_range_tracker);

int  freq_scan(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);
int  dtc_coef_step(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);
int  dfreq_scan(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker, const char* ref_dtc_file,
         DopplerTracker* ref_doppler_tracker);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c ref_tc_file ]", "<tc_file>",
    "<info_base>", 0 };

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

    enum TcType { UNKNOWN, RANGE, DOPPLER };
    char* ref_tc_file = NULL;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'c':
            ref_tc_file = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* tc_file = argv[optind++];
    const char* info_base = argv[optind++];

    //-------------------------//
    // determine the file type //
    //-------------------------//

    struct stat buf;
    if (stat(tc_file, &buf) != 0)
    {
        fprintf(stderr, "%s: error stating file %s\n", command, tc_file);
        exit(1);
    }
    TcType type = UNKNOWN;
    switch (buf.st_size)
    {
    case 796:
        type = RANGE;
        break;
    case 1564:
        type = DOPPLER;
        break;
    default:
        fprintf(stderr, "%s: error determining tracking constant type\n",
            command);
        fprintf(stderr, "    File size = %ld\n", buf.st_size);
        exit(1);
        break;
    }

    //--------------------//
    // read the constants //
    //--------------------//

    RangeTracker range_tracker, ref_range_tracker;
    DopplerTracker doppler_tracker, ref_doppler_tracker;
    char filename[1024];
    switch (type)
    {
    case RANGE:
        if (! range_tracker.ReadBinary(tc_file))
        {
            fprintf(stderr, "Error reading RGC file %s\n", tc_file);
            exit(1);
        }
        if (ref_tc_file != NULL)
        {
            if (! ref_range_tracker.ReadBinary(ref_tc_file))
            {
                fprintf(stderr, "Error reading RGC reference file %s\n",
                    ref_tc_file);
                exit(1);
            }
        }

        sprintf(filename, "%s.ascii", info_base);
        range_tracker.WriteAscii(filename);

        sprintf(filename, "%s.delay", info_base);
        delay_scan(filename, tc_file, &range_tracker);

        if (ref_tc_file != NULL)
        {
            sprintf(filename, "%s.ddelay", info_base);
            ddelay_scan(filename, tc_file, &range_tracker, ref_tc_file,
                &ref_range_tracker);
        }
        break;
    case DOPPLER:
        if (! doppler_tracker.ReadBinary(tc_file))
        {
            fprintf(stderr, "Error reading DTC file %s\n", tc_file);
            exit(1);
        }
        if (ref_tc_file != NULL)
        {
            if (! ref_doppler_tracker.ReadBinary(ref_tc_file))
            {
                fprintf(stderr, "Error reading DTC reference file %s\n",
                    ref_tc_file);
                exit(1);
            }
        }

        sprintf(filename, "%s.ascii", info_base);
        doppler_tracker.WriteAscii(filename);

        sprintf(filename, "%s.freq", info_base);
        freq_scan(filename, tc_file, &doppler_tracker);

        dtc_coef_step(info_base, tc_file, &doppler_tracker);

        if (ref_tc_file != NULL)
        {
            sprintf(filename, "%s.dfreq", info_base);
            dfreq_scan(filename, tc_file, &doppler_tracker, ref_tc_file,
                &ref_doppler_tracker);
        }
        break;
    default:
        fprintf(stderr, "%s: unknown constants type\n", command);
        exit(1);
    }

    return(0);
}

//------------//
// delay_scan //
//------------//

#define QUOTE  '"'

int
delay_scan(
    const char*    filename,
    const char*    rgc_file,
    RangeTracker*  range_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %c%s%c\n", QUOTE, "RGC Delay", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "Orbit Step", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Time (ms)", QUOTE);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;
            unsigned char delay_dn;
            float dummy;
            range_tracker->GetRxGateDelay(orbit_step, azimuth_step, 0, 0,
                &delay_dn, &dummy);
            float table_delay = RX_GATE_DELAY_CMD_RESOLUTION *
                range_tracker->rxRangeMem;
            float delay = RX_GATE_DELAY_CMD_RESOLUTION * delay_dn;
            fprintf(ofp, "%g %g %g\n", x_value, delay * 1000.0,
                table_delay * 1000.0);
        }
    }

    fclose(ofp);
    return(1);
}

//-------------//
// ddelay_scan //
//-------------//

int
ddelay_scan(
    const char*    filename,
    const char*    rgc_file,
    RangeTracker*  range_tracker,
    const char*    ref_rgc_file,
    RangeTracker*  ref_range_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %c%s%c\n", QUOTE, "RGC Delta Delay", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "Orbit Step", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Time (ms)", QUOTE);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;

            unsigned char delay_dn;
            float dummy;
            range_tracker->GetRxGateDelay(orbit_step, azimuth_step, 0, 0,
                &delay_dn, &dummy);
            float table_delay = RX_GATE_DELAY_CMD_RESOLUTION *
                range_tracker->rxRangeMem;
            float delay = RX_GATE_DELAY_CMD_RESOLUTION * delay_dn;

            unsigned char ref_delay_dn;
            ref_range_tracker->GetRxGateDelay(orbit_step, azimuth_step, 0, 0,
                &ref_delay_dn, &dummy);
            float ref_table_delay = RX_GATE_DELAY_CMD_RESOLUTION *
                ref_range_tracker->rxRangeMem;
            float ref_delay = RX_GATE_DELAY_CMD_RESOLUTION * ref_delay_dn;

            fprintf(ofp, "%g %g %g\n", x_value, (delay - ref_delay) * 1000.0,
                (table_delay - ref_table_delay) * 1000.0);
        }
    }

    fclose(ofp);
    return(1);
}

//-----------//
// freq_scan //
//-----------//

int
freq_scan(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %c%s%c\n", QUOTE, "DTC Frequency", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "Orbit Step", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Frequency (Hz)", QUOTE);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;
            short dop_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, azimuth_step,
                0, 0.0, &dop_dn);
            // convert table freq to commanded freq
            float cmd_freq = -doppler_tracker->tableFrequency;
            float freq = TX_FREQUENCY_CMD_RESOLUTION * dop_dn;
            fprintf(ofp, "%g %g %g\n", x_value, freq, cmd_freq);
        }
    }

    fclose(ofp);
    return(1);
}

//---------------//
// dtc_coef_step //
//---------------//

int
dtc_coef_step(
    const char*      basename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    static char* term_string[3] = { "amp", "phase", "bias" };
    static char* title_string[3] = { "Amplitude", "Phase", "Bias" };
    static char* unit_string[3] = { "Frequency (Hz)", "Angle (radians)",
        "Frequency (Hz)" };

    double** terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS, 3);
    doppler_tracker->GetTerms(terms);

    for (int term_idx = 0; term_idx < 3; term_idx++)
    {
        char filename[1024];
        sprintf(filename, "%s.%s", basename, term_string[term_idx]);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
            return(0);

        fprintf(ofp, "@ subtitle %c%s%c\n", QUOTE, title_string[term_idx],
            QUOTE);
        fprintf(ofp, "@ xaxis label %cOrbit Step%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, unit_string[term_idx],
            QUOTE);

        for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS;
            orbit_step++)
        {
            fprintf(ofp, "%d %g\n", orbit_step,
                *(*(terms + orbit_step) + term_idx));
        }

        fclose(ofp);
    }
    free_array(terms, 2, ORBIT_STEPS, 3);

    return(1);
}

//------------//
// dfreq_scan //
//------------//

int
dfreq_scan(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker,
    const char*      ref_dtc_file,
    DopplerTracker*  ref_doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %c%s%c\n", QUOTE, "DTC Frequency Difference", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "Orbit Step", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Frequency (Hz)", QUOTE);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;

            short dop_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, azimuth_step,
                0, 0.0, &dop_dn);
            float cmd_freq = -doppler_tracker->tableFrequency;
            float freq = TX_FREQUENCY_CMD_RESOLUTION * dop_dn;

            short ref_dop_dn;
            ref_doppler_tracker->GetCommandedDoppler(orbit_step,
                azimuth_step, 0, 0.0, &ref_dop_dn);
            float ref_cmd_freq = -ref_doppler_tracker->tableFrequency;
            float ref_freq = TX_FREQUENCY_CMD_RESOLUTION * ref_dop_dn;

            fprintf(ofp, "%g %g %g\n", x_value, freq - ref_freq,
                cmd_freq - ref_cmd_freq);
        }
    }

    fclose(ofp);
    return(1);
}
