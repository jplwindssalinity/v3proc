//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    log_cfg
//
// SYNOPSIS
//    log_cfg <config_file> [ output_file ]
//
// DESCRIPTION
//    Reads in a configuration file (with stomping) and writes it
//    out for use as a record of the input data. Included files
//    will be followed.
//
// OPTIONS
//    The following options are supported:
//      [ output_file ]  Write to this file instead of stdout.
//
// OPERANDS
//    The following operand is supported:
//      <config_file>  The configuration file.
//
// EXAMPLES
//    An example of a command line is:
//      % log_cfg sws1b.cfg sws1b.clog
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
#include "ConfigList.h"
#include "List.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;

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

const char* usage_array[] = { "<config_file>", "[ output_file ]", 0};

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
    if (argc < 2 || argc > 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* output_file = NULL;
    if (argc == 3)
        output_file = argv[clidx++];

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //--------------------//
    // write out log file //
    //--------------------//

    if (output_file)
    {
        if (! config_list.Write(output_file))
        {
            fprintf(stderr, "%s: error writing output log file %s\n",
                command, output_file);
            exit(1);
        }
    }
    else
    {
        if (! config_list.Write(stdout))
        {
            fprintf(stderr, "%s: error writing output log file\n", command);
            exit(1);
        }
    }

    return (0);
}
