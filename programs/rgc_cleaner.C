//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    rgc_cleaner
//
// SYNOPSIS
//    rgc_cleaner <config_file> <output_base>
//
// DESCRIPTION
//    Cleans up the RGC by eliminating the amplitude and phase terms
//    and by making the ideal delay be centered in the quantized window.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  The config_file needed listing all
//                           input parameters, input files, and output
//                           files.
//
//      <output_base>      The output basename for the RGC files.
//
// EXAMPLES
//    An example of a command line is:
//      % rgc_cleaner sws1b.cfg cleanrgc
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
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "QscatConfigDefs.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;

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

const char* usage_array[] = { "<config_file>", "<output_base>", 0};

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

    int arg_idx = 1;
    const char* config_file = argv[arg_idx++];
    const char* output_base = argv[arg_idx++];

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading config file %s\n", command,
            config_file);
        exit(1);
    }

    //-----------------//
    // read in the RGC //
    //-----------------//

    char keyword[1024];
    char number[8];
    RangeTracker rt[2];
    for (int i = 0; i < 2; i++) {
        sprintf(number, "%d", i + 1);
        substitute_string(BEAM_x_RGC_FILE_KEYWORD, "x", number, keyword);
        char* rt_filename = config_list.Get(keyword);
        if (! rt[i].ReadBinary(rt_filename)) {
            fprintf(stderr, "%s: error reading RGC %s\n", command,
                rt_filename);
            exit(1);
        }
    }

    //------------------------------------//
    // determine the effective gate width //
    //------------------------------------//

    float tx_pulse_width;
    config_list.GetFloat(TX_PULSE_WIDTH_KEYWORD, &tx_pulse_width);

    float rx_gate_width[2];
    for (int i = 0; i < 2; i++) {
        sprintf(number, "%d", i + 1);
        substitute_string(BEAM_x_RX_GATE_WIDTH_KEYWORD, "x", number, keyword);
        config_list.GetFloat(keyword, &(rx_gate_width[i]));
    }

    float egw[2];
    for (int i = 0; i < 2; i++) {
        egw[i] = rx_gate_width[i] - tx_pulse_width;
    }

    //-------------------------------------//
    // clear the amplitude and phase terms //
    //-------------------------------------//
    // and center in the quantization window

    for (int i = 0; i < 2; i++) {
        rt[i].ClearAmpPhase();
        rt[i].QuantizeCenter(egw[i]);
    }

    //----------------//
    // write them out //
    //----------------//

    for (int i = 0; i < 2; i++) {
        char filename[1024];
        sprintf(filename, "%s.%d", output_base, i + 1);
        if (! rt[i].WriteBinary(filename)) {
            fprintf(stderr, "%s: error writing RGC file %s\n", command,
                filename);
            exit(1);
        }
    }

    return (0);
}
