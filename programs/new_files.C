//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    new_files
//
// SYNOPSIS
//    new_files <locator_file> <type> [ done_log ]
//
// DESCRIPTION
//    Prints out a list of new files of the specified type.
//    If the done log is not specified, new_files prints out all
//    files.
//
// OPTIONS
//    The following options are supported:
//      [ done_log ]  Log of files user is done with.
//
// OPERANDS
//    The following operands are supported:
//      <locator_file>  File containing the following information:
//                        type  mature_age          (for each type)
//                        type  directory  pattern  (repeated)
//      <type>          The target type.
//
// EXAMPLES
//    An example of a command line is:
//      % new_files locator.dat l1a l1a.dop.done
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

#define OPTSTRING  "e"

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

const char* usage_array[] = { "[ -e ]", "<locator_file>", "<type>",
    "[ done_log ]", 0 };

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

    FILE* err_fp = NULL;  // don't report non-fatal errors, just keep going

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'e':
            err_fp = stderr;    // report errors
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2 && argc != optind + 3)
        usage(command, usage_array, 1);

    const char* locator_file = argv[optind++];
    const char* type = argv[optind++];
    const char* done_log = NULL;
    if (argc == optind + 1)
        done_log = argv[optind++];

    //----------------------------//
    // read locator list for type //
    //----------------------------//

    PatternList pattern_list;
    int retval = pattern_list.Read(locator_file, type, stderr);
    switch (retval)
    {
    case 0:    // OK
        break;
    case 1:    // error
        fprintf(stderr, "%s: error reading pattern list %s\n", command,
            locator_file);
        exit(1);
        break;
    case 2:    // no type match
        fprintf(stderr, "%s: unknown type %s\n", command, type);
        exit(1);
        break;
    default:    // huh?
        fprintf(stderr, "%s: unknown return code (%d).  Yikes!\n", command,
            retval);
        exit(1);
        break;
    }

    //---------------//
    // read done log //
    //---------------//

    FileList done_log_list;
    if (done_log != NULL)
    {
        if (! done_log_list.Read(done_log, stderr))
        {
            fprintf(stderr, "%s: error reading done log %s\n", command,
                done_log);
            exit(1);
        }
    }

    //--------------------------//
    // create list of new files //
    //--------------------------//

    FileList new_file_list;
    if (! pattern_list.NewStableFiles(&new_file_list, &done_log_list,
        err_fp) && err_fp != NULL)
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
