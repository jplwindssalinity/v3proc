//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_super_metrics
//
// SYNOPSIS
//    l2b_super_metrics [ -c config_file ] [ -o output_base ]
//        [ -m output_metric_file ] [ -s low_speed:high_speed ]
//        [ input_metric_files... ]
//
// DESCRIPTION
//    Reads in any input metric files, calculates wind retrieval
//    metrics for the l2b file (if specified) and generates
//    plottable output files (if specified) and/or a new
//    combined metric file.
//
// OPTIONS
//   [ -c config_file ]           Use this config_file to determine the
//                                  l2b file and truth file.
//   [ -o output_base ]           The base name to use for the plottable
//                                  output files. The default output base
//                                  is "metric".
//   [ -m output_metric_file ]    Generate an output metric file.
//   [ -m low_speed:high_speed ]  Only produce output for this range
//                                  of speeds.
//   [ input_metric_files... ]    Combine these metric files.
//
// OPERANDS
//    None.
//
// EXAMPLES
//    An example of a command line is:
//      % l2b_super_metrics -c qscat.cfg -o plot -m combo.met 1.met 2.met
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
#include <stdlib.h>
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSimDefs.h"
#include "Metrics.h"
#include "L2B.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class List<EarthPosition>;

//-----------//
// CONSTANTS //
//-----------//

#define DEFAULT_OUTPUT_BASE  "metrics"

#define OPTSTRING            "c:o:m:s:"

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

int opt_speed = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -o output_base ]",
    "[ -m output_metric_file ]", "[ -s low_speed:high_speed ]",
    "[ input_metric_files... ]", NULL };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //-----------//
    // variables //
    //-----------//

    char* config_file = NULL;
    char* output_metric_file = NULL;
    char* output_base = DEFAULT_OUTPUT_BASE;

    float low_speed, high_speed;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'c':
            config_file = optarg;
            break;
        case 'm':
            output_metric_file = optarg;
            break;
        case 'o':
            output_base = optarg;
            break;
        case 's':
            if (sscanf(optarg, "%f:%f", &low_speed, &high_speed) != 2)
            {
                fprintf(stderr, "%s: error determining speed range %s\n",
                    command, optarg);
                exit(1);
            }
            opt_speed = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    int start_idx = optind;
    int end_idx = argc;

    //---------------------------------//
    // check for appropriate arguments //
    //---------------------------------//

    if (config_file == NULL && end_idx == start_idx)
    {
        fprintf(stderr, "%s: insufficient input\n", command);
        usage(command, usage_array, 1);
    }

    //---------------------------------------//
    // read and parse the configuration file //
    //---------------------------------------//

    ConfigList config_list;
    char* l2b_file = NULL;
    char* truth_type = NULL;
    char* truth_file = NULL;
    if (config_file != NULL)
    {
        if (! config_list.Read(config_file))
        {
            fprintf(stderr, "%s: error reading config file %s\n",
                command, config_file);
            exit(1);
        }

        l2b_file = config_list.Get(L2B_FILE_KEYWORD);
        if (l2b_file == NULL)
        {
            fprintf(stderr, "%s: missing L2B file in config file %s\n",
                command, config_file);
            exit(1);
        }

        truth_type = config_list.Get(WINDFIELD_TYPE_KEYWORD);
        if (truth_type == NULL)
        {
            fprintf(stderr, "%s: must specify truth windfield type\n", command);
            exit(1);
        }

        truth_file = config_list.Get(WINDFIELD_FILE_KEYWORD);
        if (truth_file == NULL)
        {
            fprintf(stderr, "%s: must specify truth windfield file\n", command);
            exit(1);
        }
    }

    //--------------------------------------//
    // read in and combine the metric files //
    //--------------------------------------//

    Metrics metrics;
    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        char* metric_file = argv[file_idx];
        if (! metrics.Read(metric_file))
        {
            fprintf(stderr, "%s: error reading metrics file %s\n", command,
                metric_file);
            exit(1);
        }
    }

    //-----------------------//
    // read in level 2B file //
    //-----------------------//

    L2B l2b;
    WindSwath* swath = NULL;
    if (l2b_file != NULL)
    {
        if (! l2b.Read(l2b_file))
        {
            fprintf(stderr, "%s: error reading L2B file %s\n", command,
                l2b_file);
            exit(1);
        }
        swath = &(l2b.frame.swath);
    }

    //----------------------------//
    // read in "truth" wind field //
    //----------------------------//

    WindField truth;
    if (truth_file != NULL && truth_type != NULL)
    {
        if (! truth.ReadType(truth_file, truth_type))
        {
            fprintf(stderr,
                "%s: error reading truth wind field of type %s from file %s\n",
                command, truth_type, truth_file);
            exit(1);
        }
    }

    //-------------------//
    // configure metrics //
    //-------------------//

    if (opt_speed)
    {
        if (! metrics.SetWindSpeedRange(low_speed, high_speed))
        {
            fprintf(stderr,
                "%s: error setting metric wind speed range (%g - %g)\n",
                command, low_speed, high_speed);
            exit(1);
        }
    }

    //------------------//
    // generate metrics //
    //------------------//

    if (swath != NULL)
    {
        if (! metrics.Evaluate(swath, l2b.header.crossTrackResolution, &truth))
        {
            fprintf(stderr, "%s: error evaluating wind field\n", command);
            exit(1);
        }
    }

    //--------------------//
    // output metric file //
    //--------------------//

    if (output_metric_file != NULL)
    {
        if (! metrics.Write(output_metric_file))
        {
            fprintf(stderr, "%s: error writing metric file %s\n", command,
                output_metric_file);
            exit(1);
        }
    }

    //-------------------------//
    // output performance data //
    //-------------------------//

    if (output_base != NULL)
    {
        if (! metrics.WritePlotData(output_base))
        {
            fprintf(stderr, "%s: error writing plot data files\n", command);
            exit(1);
        }
    }

    return (0);
}
