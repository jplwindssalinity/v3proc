//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    done_files
//
// SYNOPSIS
//    done_files <done_log> <file...>
//
// DESCRIPTION
//    Adds the files to the end of the done log.
//
// OPTIONS
//    The following options are supported:
//      None
//
// OPERANDS
//    The following operands are supported:
//      <done_log>  The log containing done files.
//      <file...>   Files that are done with.
//
// EXAMPLES
//    An example of a command line is:
//      % done_files l1a.dop.done QS1A.1 QS1A.2
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
#include <string.h>
#include "Misc.h"
#include "List.h"
#include "List.C"
#include "new_files.h"

//-----------//
// CONSTANTS //
//-----------//

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

const char* usage_array[] = { "<done_log>", "<file...>", 0 };

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
    if (argc < 2)
        usage(command, usage_array, 1);

    int opt_idx = 1;
    const char* done_log = argv[opt_idx++];
    int file_start_idx = opt_idx;

    //-----------------------------//
    // open done log for appending //
    //-----------------------------//

    FILE* ofp = fopen(done_log, "a");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening done log %s\n", command, done_log);
        exit(1);
    }

    //---------------------------------//
    // append files to end of done log //
    //---------------------------------//

    for (int file_idx = file_start_idx; file_idx < argc; file_idx++)
    {
        fprintf(ofp, "%s\n", argv[file_idx]);
    }

    //--------------------//
    // close the done log //
    //--------------------//

    fclose(ofp);

    return (0);
}
