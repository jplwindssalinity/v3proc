//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_starter
//
// SYNOPSIS
//    hdf_starter [ -o output_file ] <L1A_file...>
//
// DESCRIPTION
//    This is a sample program which shows you how to read from
//    an HDF file. More importantly, it is a starter program which
//    you can modify to create your own program. I have tried to make
//    the code as HDF-ish as possible. In other words, you will
//    be calling *actual* HDF routines and learn a bit about HDF as
//    you go. Don't panic! It really isn't all that complicated.
//    This program will extract all 12 slices for the loopback
//    cals, and create an average slice profile. Not very useful,
//    but a decent enough example.
//
// OPTIONS
//    The following options are supported:
//      [ -o output_file ]  The output file. If not specified, the
//                            output will be written to standard output.
//
// OPERANDS
//    The following operands are supported:
//      <L1A_file...>  A list of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_starter -o average_cal_profile.dat /seapac/disk1/L1A/data/QS_S1A*
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
// AUTHORS
//    James N. Huddleston <mailto:James.N.Huddleston@jpl.nasa.gov>
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
#include <string.h>
#include "Misc.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "o:"

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

const char* usage_array[] = { "[ -o output_file ]", "<L1A_file...>", 0 };

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
    extern char* optarg;
    extern int optind;

    // by default, we will write to standard output
    FILE* ofp = stdout;
    char* output_file = NULL;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'o':
            output_file = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc <= optind)
        usage(command, usage_array, 1);

    int start_file_idx = optind;
    int end_file_idx = argc;

    //----------------------//
    // open the output file //
    //----------------------//

    if (output_file != NULL)
    {
        // only bother trying to open the file if the user specified one.
        ofp = fopen(output_file, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening file %s\n", command,
                output_file);
            exit(1);
        }
    }

    return (0);
}
