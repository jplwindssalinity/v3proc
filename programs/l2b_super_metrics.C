//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_super_metrics
//
// SYNOPSIS
//    l2b_super_metrics [ -o output_base ] [ -m output_metric_file ]
//        [ -s low_speed:high_speed ] [ -D max_direction_diff ]
//        [ config_file... ] [ metric_file... ]
//
// DESCRIPTION
//    Reads in any input metric files, calculates wind retrieval
//    metrics for the l2b file (if specified) and generates
//    plottable output files (if specified) and/or a new
//    combined metric file.
//
// OPTIONS
//   [ -o output_base ]           The base name to use for the plottable
//                                  output files. The default output base
//                                  is "metric".
//   [ -m output_metric_file ]    Generate an output metric file.
//   [ -s low_speed:high_speed ]  Only produce output for this range
//                                  of speeds.
//   [ -D max_direction_diff ]    Retricts data to those within the specified
//                                  number of degrees.
//   [ config_file... ]           Evaluate based on these configs.
//   [ metric_file... ]           Combine these metric files.
//
// OPERANDS
//    None.
//
// EXAMPLES
//    An example of a command line is:
//      % l2b_super_metrics -o plot -m combo.met 1.met 2.met 3/3.cfg 4/4.cfg
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

#define SPEED_RESOLUTION  0.5
#define SPEED_BINS        100

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "o:m:s:D:"

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
int opt_dir = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -o output_base ]",
    "[ -m output_metric_file ]", "[ -s low_speed:high_speed ]",
    "[ -D max_direction_diff ]", "[ config_file... ]", "[ metric_file... ]",
    NULL };

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

    char* output_metric_file = NULL;
    char* output_base = NULL;

    float low_speed, high_speed;
    float max_direction_error = 0.0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
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
        case 'D':
            max_direction_error = atof(optarg) * dtr;
            opt_dir = 1;
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

    if (end_idx == start_idx)
    {
        usage(command, usage_array, 1);
    }
    if (output_base == NULL && output_metric_file == NULL)
    {
        fprintf(stderr, "%s: where do you want the output to go?\n", command);
        exit(1);
    }

    //-------------------------------------//
    // read in and combine the input files //
    //-------------------------------------//

    ConfigList config_list;
    Metrics total_metrics;
    Metrics metrics;

    //-------------------//
    // configure metrics //
    //-------------------//

    if (opt_speed)
    {
        if (! metrics.SetWindSpeedRange(low_speed, high_speed))
        {
            fprintf(stderr,
                "%s: error setting wind speed range (%g - %g)\n",
                command, low_speed, high_speed);
            exit(1);
        }
    }

    if (opt_dir)
    {
        metrics.SetMaxDirectionError(max_direction_error);
    }

    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        //-----------------------------------//
        // determine what kind of file it is //
        //-----------------------------------//

        char* input_file = argv[file_idx];
        if (metrics.Read(input_file))
        {
            //---------------------------------//
            // it is a metric file, accumulate //
            //---------------------------------//

            total_metrics += metrics;
        }
        else if (config_list.Read(input_file))
        {
            //--------------------------------------//
            // it is a configuration file, evaluate //
            //--------------------------------------//

            char* l2b_file = config_list.Get(L2B_FILE_KEYWORD);
            if (l2b_file == NULL)
            {
                fprintf(stderr, "%s: missing L2B file in config file %s\n",
                    command, input_file);
                exit(1);
            }

            char* truth_type = config_list.Get(TRUTH_WIND_TYPE_KEYWORD);
            if (truth_type == NULL)
            {
                fprintf(stderr, "%s: must specify truth windfield type\n",
                    command);
                exit(1);
            }

            char* truth_file = config_list.Get(TRUTH_WIND_FILE_KEYWORD);
            if (truth_file == NULL)
            {
                fprintf(stderr, "%s: must specify truth windfield file\n",
                    command);
                exit(1);
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

            WindField truth_windfield;
            L2B truth_l2b;
            LonLatWind* truth = NULL;
            if (truth_file != NULL && truth_type != NULL)
            {
                if (strcasecmp(truth_type, L2B_TYPE) == 0) {

                    //-----------//
                    // L2B truth //
                    //-----------//

                    if (! truth_l2b.Read(truth_file)) {
                        fprintf(stderr, "%s: error reading truth L2B file %s\n",
                            command, truth_file);
                        exit(1);
                    }
                    truth = &(truth_l2b.frame.swath);
                } else {

                    //------------------//
                    // Windfield truth? //
                    //------------------//

                    if (! truth_windfield.ReadType(truth_file, truth_type)) {
                        fprintf(stderr,
                            "%s: error reading %s wind field from file %s\n",
                            command, truth_type, truth_file);
                        exit(1);
                    }

                    //-------------------//
                    // scale wind speeds //
                    //-------------------//

                    config_list.MemorizeLogFlag();
                    config_list.DoNothingForMissingKeywords();
                    float scale;
                    if (config_list.GetFloat(TRUTH_WIND_SPEED_MULTIPLIER_KEYWORD,
                        &scale))
                    {
                        truth_windfield.ScaleSpeed(scale);
                        fprintf(stderr, "Warning: scaling all wind speeds by %g\n",
                            scale);
                    }
                    config_list.RestoreLogFlag();

                    truth = &truth_windfield;
                }
            }

            //------------------//
            // generate metrics //
            //------------------//

            if (swath != NULL)
            {
                if (! metrics.Evaluate(swath, l2b.header.crossTrackResolution,
                    SPEED_BINS, SPEED_RESOLUTION, truth))
                {
                    fprintf(stderr, "%s: error evaluating wind field\n",
                        command);
                    exit(1);
                }
                total_metrics += metrics;
            }

            //------------------------------//
            // clear the configuration list //
            //------------------------------//

            config_list.FreeContents();
        }
    }

    //--------------------//
    // output metric file //
    //--------------------//

    if (output_metric_file != NULL)
    {
        if (! total_metrics.Write(output_metric_file))
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
        if (! total_metrics.WritePlotData(output_base))
        {
            fprintf(stderr, "%s: error writing plot data files\n", command);
            exit(1);
        }
    }

    return (0);
}
