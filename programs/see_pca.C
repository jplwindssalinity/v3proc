//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    see_pca
//
// SYNOPSIS
//    see_pca <pca_file>
//
// DESCRIPTION
//    Reports on the PCA file.
//
// OPTIONS
//    None.
//
// OPERANDS
//    <pca_file>     The PCA file.  If short, assumes both beam case.
//
// EXAMPLES
//    An example of a command line is:
//      % see_pca pca.021100
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
#include "Misc.h"
#include "mudh.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING         ""

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<pca_file>", 0 };

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

    if (argc < optind + 1)
        usage(command, usage_array, 1);

    const char* pca_file = argv[optind++];

    //---------------//
    // read PCA file //
    //---------------//

    int opt_outer_swath = 0;

    // first index 0=both beams, 1=outer only
    float pca_weights[2][PC_COUNT][PARAM_COUNT];
    float pca_mean[2][PARAM_COUNT];
    float pca_std[2][PARAM_COUNT];
    float pca_min[2][PC_COUNT];
    float pca_max[2][PC_COUNT];

    FILE* ifp = fopen(pca_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening PCA file %s\n", command,
            pca_file);
        exit(1);
    }
    unsigned int w_size = PC_COUNT * PARAM_COUNT;
    if (fread(pca_weights[0], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        fprintf(stderr, "%s: error reading first half of PCA file %s\n",
            command, pca_file);
        exit(1);
    }
    if (fread(pca_weights[1], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        // only complain if it is NOT EOF
        if (! feof(ifp))
        {
            fprintf(stderr, "%s: error reading second half of PCA file %s\n",
                command, pca_file);
            exit(1);
        }
    }
    else
    {
        opt_outer_swath = 1;
    }
    fclose(ifp);

    //----------------//
    // print out info //
    //----------------//

    char* swath_string[] = { "Both Beam Case", "Outer Beam Only Case" };
    char* param_string[] = { "NBD", "Spd", "Dir", "MLE", "ENOF", "Qual", "TbH",
        "TbV", "TbHs", "TbVs", "Tran" };

    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
        if (swath_idx == 1 && ! opt_outer_swath)
            break;
        printf("%s\n", swath_string[swath_idx]);
        for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
        {
            printf("  Principle Component %d\n", pc_idx + 1);
            for (int param_idx = 0; param_idx < PARAM_COUNT; param_idx++)
            {
                printf("    %5s : %g (u=%g, s=%g, m=%g, M=%g)\n",
                    param_string[param_idx],
                    pca_weights[swath_idx][pc_idx][param_idx],
                    pca_mean[swath_idx][param_idx],
                    pca_std[swath_idx][param_idx],
                    pca_min[swath_idx][param_idx],
                    pca_max[swath_idx][param_idx]);
            }
        }
    }

    return (0);
}
