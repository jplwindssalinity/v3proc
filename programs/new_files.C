//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    new_files
//
// SYNOPSIS
//    new_files <pattern_file> <type> <done_log>
//
// DESCRIPTION
//    Prints out a list of new files of the specified type.
//
// OPTIONS
//    The following options are supported:
//      None
//
// OPERANDS
//    The following operands are supported:
//      <pattern_file>  File containing the following information:
//                        type  mature_age          (for each type)
//                        type  directory  pattern  (repeated)
//      <type>          The target type.
//      <done_log>      Log of files user is done with.
//
// EXAMPLES
//    An example of a command line is:
//      % new_files pattern.dat l1a l1a.dop.done
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

const char* usage_array[] = { "<pattern_file>", "<type>", "<done_log>", 0 };

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
    if (argc != 3)
        usage(command, usage_array, 1);

    int opt_idx = 1;
    const char* pattern_file = argv[opt_idx++];
    const char* type = argv[opt_idx++];
    const char* done_log = argv[opt_idx++];

    //----------------------------//
    // read pattern list for type //
    //----------------------------//

    PatternList pattern_list;
    if (! pattern_list.Read(pattern_file, type))
    {
        fprintf(stderr, "%s: error reading pattern list %s\n", command,
            pattern_file);
        exit(1);
    }

    //---------------//
    // read done log //
    //---------------//

    FileList done_log_list;
    if (! done_log_list.Read(done_log))
    {
        fprintf(stderr, "%s: error reading done log %s\n", command,
            done_log);
        exit(1);
    }

    //--------------------------//
    // create list of new files //
    //--------------------------//

    FileList new_file_list;
    if (! pattern_list.NewStableFiles(&new_file_list, &done_log_list))
    {
        fprintf(stderr, "%s: error creating list of files\n", command);
        exit(1);
    }

    //------------------//
    // report new files //
    //------------------//

    new_file_list.Print();

    return (0);
}
