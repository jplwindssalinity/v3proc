//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_gstab
//
// SYNOPSIS
//    mudh_gstab [ -r ] [ -n min_samples ] <input_mudhtab> <output_gstab>
//
// DESCRIPTION
//    Reformat mudhtab to the floating point array needed by the GS.
//    Also, fills in -3 flags for data with less than the specified
//    number of samples.
//
// OPTIONS
//    [ -r ]               Require NBD values.
//    [ -n min_samples ]   The minimum number of samples.
//
// OPERANDS
//    <input_mudhtab>   Input mudhtab.
//    <output_gstab>    Output gstab.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_gstab -n 20 input.mudhtabex table.gstab
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

#define OPTSTRING    "rn:"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_require_nbd = 0;
int opt_min_samples = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r ]", "[ -n min_samples ]",
    "<input_mudhtab>", "<output_gstab>", 0 };

static double norain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double sample_count_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

static float gstab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

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

    unsigned long min_samples = 0;

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            opt_require_nbd = 1;
            break;
        case 'n':
            min_samples = (unsigned long)atoi(optarg);
            opt_min_samples = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* input_mudhtab_file = argv[optind++];
    const char* output_gstab_file = argv[optind++];

    //-------------------//
    // read mudhtab file //
    //-------------------//

    FILE* mudhtab_ifp = fopen(input_mudhtab_file, "r");
    if (mudhtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mudhtab file %s\n", command,
            input_mudhtab_file);
        exit(1);
    }
    unsigned int size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;
    if (fread(norain_tab_1, sizeof(double), size, mudhtab_ifp) != size ||
        fread(rain_tab_1, sizeof(double), size, mudhtab_ifp) != size ||
        fread(sample_count_1, sizeof(double), size, mudhtab_ifp) != size)
    {
        fprintf(stderr, "%s: error reading mudhtab file %s\n", command,
            input_mudhtab_file);
        exit(1);
    }
    fclose(mudhtab_ifp);

    //-------------------//
    // transfer to gstab //
    //-------------------//

    for (int inbd = 0; inbd < NBD_DIM; inbd++)
    {
      for (int ispd = 0; ispd < SPD_DIM; ispd++)
      {
        for (int idir = 0; idir < DIR_DIM; idir++)
        {
          for (int imle = 0; imle < MLE_DIM; imle++)
          {
            if (opt_min_samples &&
                sample_count_1[inbd][ispd][idir][imle] < min_samples)
            {
              gstab[inbd][ispd][idir][imle] = -3.0;
            }
            else if (opt_require_nbd && inbd == NBD_DIM - 1)
            {
              gstab[inbd][ispd][idir][imle] = -3.0;
            }
            else
            {
              gstab[inbd][ispd][idir][imle] =
                (float)rain_tab_1[inbd][ispd][idir][imle];
            }
          }
        }
      }
    }

    //-----------------//
    // write the gstab //
    //-----------------//

    FILE* gstab_ofp = fopen(output_gstab_file, "w");
    if (gstab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output gstab file %s\n",
            command, output_gstab_file);
        exit(1);
    }
    if (fwrite(gstab, sizeof(float), size, gstab_ofp) != size)
    {
        fprintf(stderr, "%s: error writing gstab file %s\n", command,
            output_gstab_file);
        exit(1);
    }
    fclose(gstab_ofp);

    return (0);
}
