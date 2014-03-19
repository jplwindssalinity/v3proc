//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    patch_dtc
//
// SYNOPSIS
//    patch_dtc [ -io ] <dtc_land> <dtc_ocean> <dtc_hybrid>
//
// DESCRIPTION
//    Reads the two dtc files and patches them together.
//    Ya needs to do this for each beam.
//
// OPTIONS
//    [ -i ]  Use inner beam south pole patch.
//    [ -o ]  Use outer beam south pole patch.
//
// OPERANDS
//    The following operands are supported:
//      <dtc_land>    The input DTC file for land.
//      <dtc_ocean>   The input DTC file for ocean only.
//      <dtc_hybrid>  Output patched DTC file.
//
// EXAMPLES
//    An example of a command line is:
//      % patch_dtc dtc.land.1 dtc.ocean.1 dtc.hybrid.1
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
#include <unistd.h>
#include <math.h>
#include "Misc.h"
#include "Array.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "io"
#define QUOTE      '"'

#define ORBIT_STEPS            256
#define NUMBER_OF_QSCAT_BEAMS  2

#define SOUTH_POLE_FADE_ORBIT_STEPS  2

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

const char* usage_array[] = { "[ -io ]", "<dtc_land>", "<dtc_ocean>",
    "<dtc_hybrid>", 0 };

// beam, min/max
int south_pole_range[2][2] =
{
    { 172, 210 },
    { 170, 218 }
};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    int beam_idx = -1;

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
        case 'i':
            beam_idx = 0;
            break;
        case 'o':
            beam_idx = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 3)
        usage(command, usage_array, 1);

    if (beam_idx == -1)
    {
        fprintf(stderr, "%s: need -i or -o\n", command);
        exit(1);
    }

    const char* dtc_land_file = argv[optind++];
    const char* dtc_ocean_file = argv[optind++];
    const char* dtc_hybrid_file = argv[optind++];

    //--------------------//
    // allocate for terms //
    //--------------------//

    double** land_terms = (double **)make_array(sizeof(double), 2,
        ORBIT_STEPS, 3);
    if (land_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }
    double** ocean_terms = (double **)make_array(sizeof(double), 2,
        ORBIT_STEPS, 3);
    if (ocean_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }
    double** hybrid_terms = (double **)make_array(sizeof(double), 2,
        ORBIT_STEPS, 3);
    if (hybrid_terms == NULL)
    {
        fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
        exit(1);
    }

    //-------------------//
    // read in DTC files //
    //-------------------//

    DopplerTracker dtc_land;
    if (dtc_land.ReadBinary(dtc_land_file) == NULL)
    {
        fprintf(stderr, "%s: error reading land DTC %s\n", command,
            dtc_land_file);
        exit(1);
    }

    DopplerTracker dtc_ocean;
    if (dtc_ocean.ReadBinary(dtc_ocean_file) == NULL)
    {
        fprintf(stderr, "%s: error reading ocean DTC %s\n", command,
            dtc_ocean_file);
        exit(1);
    }

    //-----------//
    // get terms //
    //-----------//

    dtc_land.GetTerms(land_terms);
    dtc_ocean.GetTerms(ocean_terms);

    //---------//
    // combine //
    //---------//

    double ocean_factor, land_factor;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (orbit_step >= south_pole_range[beam_idx][0] &&
            orbit_step <= south_pole_range[beam_idx][1])
        {
            int dist1 = orbit_step - south_pole_range[beam_idx][0];
            int dist2 = south_pole_range[beam_idx][1] - orbit_step;
            int min_dist = MIN(dist1, dist2);
            if (min_dist <= SOUTH_POLE_FADE_ORBIT_STEPS)
            {
                land_factor = (double)min_dist /
                    (double)SOUTH_POLE_FADE_ORBIT_STEPS;
                ocean_factor = 1.0 - land_factor;
            }
            else
            {
                ocean_factor = 0.0;
                land_factor = 1.0;
            }
        }
        else
        {
            ocean_factor = 1.0;
            land_factor = 0.0;
        }

        for (int coef_idx = 0; coef_idx < 3; coef_idx++)
        {
            *(*(hybrid_terms + orbit_step) + coef_idx) =
                ocean_factor * *(*(ocean_terms + orbit_step) + coef_idx) +
                land_factor * *(*(land_terms + orbit_step) + coef_idx);
        }
    }

    //-------------------//
    // create output DTC //
    //-------------------//

    DopplerTracker dtc_hybrid;
    dtc_hybrid.Allocate(ORBIT_STEPS);
    dtc_hybrid.SetTerms(hybrid_terms);
    dtc_hybrid.WriteBinary(dtc_hybrid_file);

    return(0);
}
