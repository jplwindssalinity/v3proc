//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    pulser
//
// SYNOPSIS
//    pulser <config_file> <output_base>
//
// DESCRIPTION
//    Calculates optimum pulse timing parameters for fixed look angle
//    scatterometers. It also generates a reference chart for plotting
//    in xmgr.
//
// OPTIONS
//    The following options are supported:
//      None.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  The timing configuration file.
//      <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % pulse_timing timing.cfg timing
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
#include <string.h>
#include "Misc.h"
#include "ConfigList.h"
#include "Pulser.h"
#include "Constants.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Pulse>;
template class List<Pulser>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

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

const char* usage_array[] = { "<config_file>", "<output_base>", 0 };

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

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* output_base = argv[optind++];

    //------------------//
    // read config file //
    //------------------//

    ConfigList config_list;
    config_list.Read(config_file);

    //----------------------//
    // create and configure //
    //----------------------//

    PulserCluster pulser_cluster;
    if (! pulser_cluster.Config(&config_list))
    {
        fprintf(stderr, "%s: error configuring pulser cluster\n", command);
        exit(1);
    }

    //----------//
    // optimize //
    //----------//

    double max_duty_factor = pulser_cluster.Optimize();
    if (max_duty_factor == 0.0)
    {
        fprintf(stderr, "%s: unable to optimize\n", command);
        exit(0);
    }

    //-------------------------------//
    // generate a sample timing plot //
    //-------------------------------//

    char filename[1024];
    sprintf(filename, "%s.tdat", output_base);
    pulser_cluster.Recall();
    pulser_cluster.SetRtts(0);
    pulser_cluster.GenerateAllPulses();
    pulser_cluster.WritePulseTiming(filename);

    return(0);
}
