//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    dtc_apbmc
//
// SYNOPSIS
//    dtc_apbmc <dtc_a> <dtc_b> <dtc_c> <dtc_out>
//
// DESCRIPTION
//    Reads the DTC A, B, and C and generates an output DTC which
//    is A+B-C (hence the name ap(lus)bm(inus)c).  The idea here
//    is to be able to apply theoretical deltas to corrected DTC.
//    So if you have an estimated set of constants at one point (1),
//    and theoretical constants at that point (1) plus another (2),
//    you can estimate the constants at the other point (2) using
//    dtc_apbmc est.1 theo.2 theo.1 est.2
//    which gives est.2 = est.1 + (theo.2 - theo.1)
//
// OPTIONS
//
// OPERANDS
//    The following operands are supported:
//      <dtc_a>    DTC A.
//      <dtc_b>    DTC B.
//      <dtc_c>    DTC C.
//      <dtc_out>  DTC out.
//
// EXAMPLES
//    An example of a command line is:
//      % dtc_apbmc dtc.1 theo.2 theo.1 dtc.2
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
#include <math.h>
#include <unistd.h>
#include "Misc.h"
#include "Array.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""
#define QUOTE      '"'

#define ORBIT_STEPS            256
#define NUMBER_OF_QSCAT_BEAMS  2

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<dtc_a>", "<dtc_b>", "<dtc_c>", "<dtc_out>",
    0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    extern int optind;
//  extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 4)
        usage(command, usage_array, 1);

    const char* dtc_a_file = argv[optind++];
    const char* dtc_b_file = argv[optind++];
    const char* dtc_c_file = argv[optind++];
    const char* dtc_out_file = argv[optind++];

    //--------------------//
    // allocate for terms //
    //--------------------//

    double** a_terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS,
        3);
    if (a_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }
    double** b_terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS,
        3);
    if (b_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }
    double** c_terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS,
        3);
    if (c_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }
    double** out_terms = (double **)make_array(sizeof(double), 2, ORBIT_STEPS,
        3);
    if (out_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }

    //-------------------//
    // read in DTC files //
    //-------------------//

    DopplerTracker dtc_a;
    if (dtc_a.ReadBinary(dtc_a_file) == NULL)
    {
        fprintf(stderr, "%s: error reading DTC A %s\n", command,
            dtc_a_file);
        exit(1);
    }

    DopplerTracker dtc_b;
    if (dtc_b.ReadBinary(dtc_b_file) == NULL)
    {
        fprintf(stderr, "%s: error reading DTC B %s\n", command,
            dtc_b_file);
        exit(1);
    }

    DopplerTracker dtc_c;
    if (dtc_c.ReadBinary(dtc_c_file) == NULL)
    {
        fprintf(stderr, "%s: error reading DTC C %s\n", command,
            dtc_c_file);
        exit(1);
    }

    //-----------//
    // get terms //
    //-----------//

    dtc_a.GetTerms(a_terms);
    dtc_b.GetTerms(b_terms);
    dtc_c.GetTerms(c_terms);

    //----------//
    // math 'em //
    //----------//

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            for (int coef_idx = 0; coef_idx < 3; coef_idx++)
            {
                double a_amp = *(*(a_terms + orbit_step) + 0);
                double a_phase = *(*(a_terms + orbit_step) + 1);
                double a_bias = *(*(a_terms + orbit_step) + 2);

                double b_amp = *(*(b_terms + orbit_step) + 0);
                double b_phase = *(*(b_terms + orbit_step) + 1);
                double b_bias = *(*(b_terms + orbit_step) + 2);

                double c_amp = *(*(c_terms + orbit_step) + 0);
                double c_phase = *(*(c_terms + orbit_step) + 1);
                double c_bias = *(*(c_terms + orbit_step) + 2);

                // a + b
                double d_amp = sqrt(a_amp*a_amp +
                    2.0*a_amp*b_amp*cos(a_phase+b_phase) + b_amp*b_amp);
                double d_bias = a_bias + b_bias;
                double y = a_amp*sin(a_phase) + b_amp*sin(b_phase);
                double x = a_amp*cos(a_phase) + b_amp*cos(b_phase);
                double d_phase = atan2(y, x);

                // c gets subtracted (leave the phase alone)
                c_amp = -c_amp;
                c_bias = -c_bias;

                // + (-c)
                double e_amp = sqrt(d_amp*d_amp +
                    2.0*d_amp*c_amp*cos(d_phase-c_phase) + c_amp*c_amp);
                double e_bias = d_bias + c_bias;
                y = d_amp*sin(d_phase) + c_amp*sin(c_phase);
                x = d_amp*cos(d_phase) + c_amp*cos(c_phase);
                double e_phase = atan2(y, x);

                *(*(out_terms + orbit_step) + 0) = e_amp;
                *(*(out_terms + orbit_step) + 1) = e_phase;
                *(*(out_terms + orbit_step) + 2) = e_bias;
            }
        }
    }

    //-------------------//
    // create output DTC //
    //-------------------//

    DopplerTracker dtc_out;
    dtc_out.Allocate(ORBIT_STEPS);
    dtc_out.SetTerms(out_terms);
    dtc_out.WriteBinary(dtc_out_file);

    return(0);
}
