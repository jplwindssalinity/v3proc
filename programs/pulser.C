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

    //------------------------------//
    // create and configure pulsers //
    //------------------------------//

    PulserCluster pulser_cluster;
    int number_of_beams;
    config_list.GetInt("NUMBER_OF_BEAMS", &number_of_beams);
    for (int pulser_id = 1; pulser_id <= number_of_beams; pulser_id++)
    {
        Pulser* new_pulser = new Pulser();
        if (new_pulser == NULL)
        {
            fprintf(stderr, "%s: error creating pulser for beam %d\n",
                command, pulser_id);
            exit(1);
        }
        if (! new_pulser->Config(pulser_id, &config_list))
        {
            fprintf(stderr, "%s: error configuring pulser for beam %d\n",
                command, pulser_id);
            exit(1);
        }
        if (! pulser_cluster.Append(new_pulser))
        {
            fprintf(stderr,
                "%s: error adding pulser to pulser cluster for beam %d\n",
                command, pulser_id);
            exit(1);
        }
    }

    //----------------------------------//
    // set the nadir angle and altitude //
    //----------------------------------//

    double nadir_look_angle;
    config_list.GetDouble("NADIR_LOOK_ANGLE", &nadir_look_angle);
    Pulser::SetNadirLookAngle(nadir_look_angle);

    double altitude;
    config_list.GetDouble("ALTITUDE", &altitude);
printf(" call %g\n", altitude);
    pulser_cluster.SetAltitude(altitude);

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
    pulser_cluster.WritePulseTiming(filename);

    return(0);
}

/*
//---------------//
// config_pulser //
//---------------//

BeamControl*
config_beam_control(
    int          pulser_id,
    ConfigList*  config_list)
{
    // convert number to a string
    char number[8];
    sprintf(number, "%d", pulser_id);

    char keyword[1024];

    //---------------------------//
    // get info from config list //
    //---------------------------//

    substitute_string("BEAM_x_PRI", "x", number, keyword);
    double pri;
    if (! config_list->GetDouble(keyword, &pri))
        return(NULL);

    substitute_string("BEAM_x_PRI_STEP", "x", number, keyword);
    double pri_step;
    if (! config_list->GetDouble(keyword, &pri_step))
        return(NULL);

    substitute_string("BEAM_x_PULSE_WIDTH", "x", number, keyword);
    double pulse_width;
    if (! config_list->GetDouble(keyword, &pulse_width))
        return(NULL);

    substitute_string("BEAM_x_PULSE_WIDTH_STEP", "x", number, keyword);
    double pulse_width_step;
    if (! config_list->GetDouble(keyword, &pulse_width_step))
        return(NULL);

    substitute_string("BEAM_x_OFFSET", "x", number, keyword);
    double offset;
    if (! config_list->GetDouble(keyword, &offset))
        return(NULL);

    substitute_string("BEAM_x_OFFSET_STEP", "x", number, keyword);
    double offset_step;
    if (! config_list->GetDouble(keyword, &offset_step))
        return(NULL);

    substitute_string("BEAM_x_LOOK_ANGLE", "x", number, keyword);
    double look_angle;
    if (! config_list->GetDouble(keyword, &look_angle))
        return(NULL);

    substitute_string("BEAM_x_BEAM_WIDTH", "x", number, keyword);
    double beam_width;
    if (! config_list->GetDouble(keyword, &beam_width))
        return(NULL);

    substitute_string("BEAM_x_IN_FLIGHT", "x", number, keyword);
    double in_flight;
    if (! config_list->GetDouble(keyword, &in_flight))
        return(NULL);

    substitute_string("BEAM_x_ANGLE_BUFFER", "x", number, keyword);
    double angle_buffer;
    if (! config_list->GetDouble(keyword, &angle_buffer))
        return(NULL);

    substitute_string("BEAM_x_TIME_BUFFER", "x", number, keyword);
    double time_buffer;
    if (! config_list->GetDouble(keyword, &time_buffer))
        return(NULL);

    //-----------------------------//
    // create the new beam control //
    //-----------------------------//

    BeamControl* new_beam_control = new BeamControl();
    if (new_beam_control == NULL)
        return(NULL);

    new_beam_control->SetBeamNumber(pulser_id);
    new_beam_control->SetPri(pri);
    new_beam_control->SetPriStep(pri_step);
    new_beam_control->SetPulseWidth(pulse_width);
    new_beam_control->SetPulseWidthStep(pulse_width_step);
    new_beam_control->SetOffset(offset);
    new_beam_control->SetOffsetStep(offset_step);
    new_beam_control->SetLookAngle(look_angle);
    new_beam_control->SetBeamWidth(beam_width);
    new_beam_control->SetInFlight(in_flight);
    new_beam_control->SetAngleBuffer(angle_buffer);
    new_beam_control->SetTimeBuffer(time_buffer);
    return(new_beam_control);
}
*/
