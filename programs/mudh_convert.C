//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//     mudh_convert
//
// SYNOPSIS
//     mudh_convert [ -d ] [ -f ] <input_classtab> <output_classtab>
//
// DESCRIPTION
//    Generate classification information.
//
// OPTIONS
//    [ -d ]  Double to float conversion.
//    [ -f ]  Float to double conversion.
//
// OPERANDS
//    <input_classtab>   Duh.
//    <output_classtab>  Duh again.
//
// EXAMPLES
//    An example of a command line is:
//      %  mudh_convert -d classtab.bws classtab.gs
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
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "df"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d ]", "[ -f ]", "<input_classtab>",
    "<output_classtab>", 0 };

static double d_classtab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static float  f_classtab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

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

    int opt_double_to_float = 0;
    int opt_float_to_double = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'd':
            opt_double_to_float = 1;
            break;
        case 'f':
            opt_float_to_double = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* input_file = argv[optind++];
    const char* output_file = argv[optind++];
    if (opt_double_to_float == opt_float_to_double)
    {
        fprintf(stderr, "%s: must specify -d OR -f\n", command);
        exit(1);
    }

    //--------------------//
    // read classtab file //
    //--------------------//

    FILE* ifp = fopen(input_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening classtab file %s\n", command,
            input_file);
        exit(1);
    }
    unsigned long size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;

    size_t retval = 0;
    if (opt_double_to_float)
    {
        retval = fread(d_classtab, sizeof(double), size, ifp);
    }
    else if (opt_float_to_double)
    {
        retval = fread(f_classtab, sizeof(float), size, ifp);
    }

    if (retval != size)
    {
        fprintf(stderr, "%s: error reading classtab file %s\n", command,
            input_file);
        exit(1);
    }
    fclose(ifp);

    //----------//
    // transfer //
    //----------//

    for (int i = 0; i < NBD_DIM; i++)
    {
        for (int j = 0; j < SPD_DIM; j++)
        {
            for (int k = 0; k < DIR_DIM; k++)
            {
                for (int l = 0; l < MLE_DIM; l++)
                {
                    if (opt_double_to_float)
                    {
                        f_classtab[i][j][k][l] = (float)d_classtab[i][j][k][l];
                    }
                    else
                    {
                        d_classtab[i][j][k][l] = (double)f_classtab[i][j][k][l];
                    }
                }
            }
        }
    }

    //-------------------//
    // write output file //
    //-------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening classtab file %s\n", command,
            output_file);
        exit(1);
    }

    retval = 0;
    if (opt_double_to_float)
    {
        retval = fwrite(f_classtab, sizeof(float), size, ofp);
    }
    else if (opt_float_to_double)
    {
        retval = fwrite(d_classtab, sizeof(double), size, ofp);
    }

    if (retval != size)
    {
        fprintf(stderr, "%s: error writing classtab file %s\n", command,
            output_file);
        exit(1);
    }
    fclose(ofp);

    return (0);
}
