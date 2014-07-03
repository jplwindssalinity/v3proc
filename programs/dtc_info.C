//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    dtc_info
//
// SYNOPSIS
//    dtc_info [ -c ref_dtc_file ] <dtc_file> <info_base>
//
// DESCRIPTION
//    Reads in a set of Doppler Tracking Constants and produces some
//    information files.
//
// OPTIONS
//    The following options are supported:
//      [ -c ref_dtc_file ]  Compare the results to this file.
//
// OPERANDS
//    The following operands are supported:
//        <dtc_file>   The DTC input file.
//        <info_base>  The basename for output information files.
//
// EXAMPLES
//    Examples of command lines are:
//      % dtc_info -c beam1.old.dtc beam1.dtc beam1_info
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

int  freq_scan(const char* filename, const char* dtc_file,
         DopplerTracker* doppler_tracker);
int  coef_step(const char* filename, const char* dtc_file,
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

const char* usage_array[] = { "[ -c ref_dtc_file ]", "<dtc_file>",
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

    char* ref_dtc_file = NULL;

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
            ref_dtc_file = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* dtc_file = argv[optind++];
    const char* info_base = argv[optind++];

    //--------------//
    // read the DTC //
    //--------------//

    DopplerTracker doppler_tracker;
    if (! doppler_tracker.ReadBinary(dtc_file))
    {
        fprintf(stderr, "Error reading binary DTC file %s\n", dtc_file);
        exit(1);
    }

    DopplerTracker ref_doppler_tracker;
    if (ref_dtc_file != NULL)
    {
        if (! ref_doppler_tracker.ReadBinary(ref_dtc_file))
        {
            fprintf(stderr, "Error reading binary DTC file %s\n",
                ref_dtc_file);
            exit(1);
        }
    }

    //--------------------------//
    // generate some basic info //
    //--------------------------//

    char filename[1024];
    sprintf(filename, "%s.ascii", info_base);
    doppler_tracker.WriteAscii(filename);

    sprintf(filename, "%s.freq", info_base);
    freq_scan(filename, dtc_file, &doppler_tracker);

    sprintf(filename, "%s.coef", info_base);
    coef_step(filename, dtc_file, &doppler_tracker);

    if (ref_dtc_file != NULL)
    {
        sprintf(filename, "%s.dfreq", info_base);
        dfreq_scan(filename, dtc_file, &doppler_tracker, ref_dtc_file,
            &ref_doppler_tracker);
    }

    return(0);
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

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;
            short dop_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, azimuth_step,
                0, 0.0, &dop_dn);
            float freq = TX_FREQUENCY_CMD_RESOLUTION * dop_dn;
            fprintf(ofp, "%g %g\n", x_value, freq);
        }
    }

    fclose(ofp);
    return(1);
}

//-----------//
// coef_step //
//-----------//

int
coef_step(
    const char*      filename,
    const char*      dtc_file,
    DopplerTracker*  doppler_tracker)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    double** terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS, 3);
    doppler_tracker->GetTerms(terms);

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double amplitude = *(*(terms + orbit_step) + 0);
        double phase = *(*(terms + orbit_step) + 1) * rtd;
        double bias = *(*(terms + orbit_step) + 2);

        fprintf(ofp, "%d %g %g %g\n", orbit_step, amplitude, phase, bias);
    }

    fclose(ofp);

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

    for (unsigned short orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (unsigned short azimuth_step = 0; azimuth_step < 32768;
            azimuth_step += 91)
        {
            float x_value = (float)orbit_step + (float)azimuth_step / 32768.0;

            short dop_dn;
            doppler_tracker->GetCommandedDoppler(orbit_step, azimuth_step,
                0, 0.0, &dop_dn);
            float freq = TX_FREQUENCY_CMD_RESOLUTION * dop_dn;

            short ref_dop_dn;
            ref_doppler_tracker->GetCommandedDoppler(orbit_step,
                azimuth_step, 0, 0.0, &ref_dop_dn);
            float ref_freq = TX_FREQUENCY_CMD_RESOLUTION * ref_dop_dn;

            fprintf(ofp, "%g %g\n", x_value, freq - ref_freq);
        }
    }

    fclose(ofp);
    return(1);
}
