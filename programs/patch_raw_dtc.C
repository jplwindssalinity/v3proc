//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    patch_dtc
//
// SYNOPSIS
//    patch_dtc <raw_dtc_land> <raw_dtc_ocean> <raw_dtc_hybrid>
//
// DESCRIPTION
//    Reads the two dtc files and patches them together.
//
// OPTIONS
//
// OPERANDS
//    The following operands are supported:
//      <raw_dtc_land>    The input raw DTC file for land.
//      <raw_dtc_ocean>   The input raw DTC file for ocean only.
//      <raw_dtc_hybrid>  Output patched raw DTC file.
//
// EXAMPLES
//    An example of a command line is:
//      % patch_dtc dtc.land.raw dtc.ocean.raw dtc.hybrid.raw
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
#include <math.h>
#include "Misc.h"
#include "Array.h"
#include "Tracking.h"
#include "Tracking.C"

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

const char* usage_array[] = { "<raw_dtc_land>", "<raw_dtc_ocean>",
    "<raw_dtc_hybrid>", 0 };

double**  g_land_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_land_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
double**  g_ocean_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_ocean_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
double**  g_hybrid_terms[NUMBER_OF_QSCAT_BEAMS];
char      g_hybrid_good[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];

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

    if (argc != optind + 3)
        usage(command, usage_array, 1);

    const char* raw_dtc_land = argv[optind++];
    const char* raw_dtc_ocean = argv[optind++];
    const char* raw_dtc_hybrid = argv[optind++];

    //--------------------//
    // allocate for terms //
    //--------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        g_land_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        if (g_land_terms[beam_idx] == NULL)
        {
            fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
            exit(1);
        }
        g_ocean_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        if (g_ocean_terms[beam_idx] == NULL)
        {
            fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
            exit(1);
        }
        g_hybrid_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        if (g_hybrid_terms[beam_idx] == NULL)
        {
            fprintf(stderr, "%s: error allocating DTC term arrays\n", command);
            exit(1);
        }
    }

    //-----------------------//
    // read in raw DTC files //
    //-----------------------//

    FILE* land_term_fp = fopen(raw_dtc_land, "r");
    if (land_term_fp == NULL)
    {
        fprintf(stderr, "%s: error opening raw land term file %s\n", command,
            raw_dtc_land);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            fread((void *)g_land_terms[beam_idx][orbit_step], sizeof(double),
                3, land_term_fp);
            fread((void *)&(g_land_good[beam_idx][orbit_step]), sizeof(char),
                1, land_term_fp);
        }
    }
    fclose(land_term_fp);

    FILE* ocean_term_fp = fopen(raw_dtc_ocean, "r");
    if (ocean_term_fp == NULL)
    {
        fprintf(stderr, "%s: error opening raw ocean term file %s\n", command,
            raw_dtc_ocean);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            fread((void *)g_ocean_terms[beam_idx][orbit_step], sizeof(double),
                3, ocean_term_fp);
            fread((void *)&(g_ocean_good[beam_idx][orbit_step]), sizeof(char),
                1, ocean_term_fp);
        }
    }
    fclose(ocean_term_fp);

    //---------//
    // combine //
    //---------//

    double** source_terms;
    double** dest_terms;
    char* source_good;
    char* dest_good;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            if (orbit_step >= 169 && orbit_step <= 215)
            {
                source_terms = g_land_terms[beam_idx];
                source_good = g_land_good[beam_idx];
            }
            else
            {
                source_terms = g_ocean_terms[beam_idx];
                source_good = g_ocean_good[beam_idx];
            }
            dest_terms = g_hybrid_terms[beam_idx];
            dest_good = g_hybrid_good[beam_idx];

            for (int coef_idx = 0; coef_idx < 3; coef_idx++)
            {
                *(*(dest_terms + orbit_step) + coef_idx) =
                    *(*(source_terms + orbit_step) + coef_idx);
                *(dest_good + orbit_step) = *(source_good + orbit_step);
            }
        }
    }

    //---------------------------------------------------------//
    // write out raw terms and good array for later processing //
    //---------------------------------------------------------//

    FILE* term_fp = fopen(raw_dtc_hybrid, "w");
    if (term_fp == NULL)
    {
        fprintf(stderr, "%s: error opening raw hybrid term file %s\n", command,
            raw_dtc_hybrid);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
        {
            fwrite((void *)g_hybrid_terms[beam_idx][orbit_step],
                sizeof(double), 3, term_fp);
            fwrite((void *)&(g_hybrid_good[beam_idx][orbit_step]),
                sizeof(char), 1, term_fp);
        }
    }
    fclose(term_fp);

    return(0);
}
