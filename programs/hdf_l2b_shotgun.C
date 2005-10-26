//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_l2b_shotgun
//
// SYNOPSIS
//    hdf_l2b_shotgun <ins_config_file> <hdf_l2b_file...>
//
// DESCRIPTION
//    Uses the shotgun approach of wiping out ambiguity removal
//    results near cyclones and letting the median filter do
//    its thing.  Output files replace the input files, so copy
//    them first.
//
// OPTIONS
//    None.
//
// OPERANDS
//    <ins_config_file>  QSCAT instrument configuration file.
//    <hdf_l2b_file...>  HDF L2B files.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_l2b_shotgun qscat.cfg QS_S2B*
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
#include <unistd.h>
#include "Misc.h"
#include "L2AToL2B.h"
#include "ConfigSim.h"
#include "ConfigList.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<Meas>;
template class List<AngleInterval>;
template class List<OffsetList>;
template class List<long>;
template class List<MeasSpot>;
template class List<EarthPosition>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;

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

const char* usage_array[] = { "<ins_config_file>", "<hdf_l2b_file...>", 0 };

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

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

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

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    int file_start_idx = optind;
    int file_end_idx = argc;

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
            config_file);
        exit(1);
    }

    //------------------------------------//
    // create and configure the converter //
    //------------------------------------//

    L2AToL2B l2a_to_l2b;
    if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

    //-------------------------//
    // loop over each L2B file //
    //-------------------------//

    for (int file_idx = file_start_idx; file_idx < file_end_idx; file_idx++)
    {
        //-----------------------//
        // read in level 2B file //
        //-----------------------//

        char* l2b_file = argv[file_idx];
        L2B l2b;
        l2b.SetInputFilename(l2b_file);
        if (! l2b.ReadHDF())
        {
            fprintf(stderr, "%s: error reading L2B file %s\n", command,
                l2b_file);
            exit(1);
        }

        WindSwath* swath = &(l2b.frame.swath);

        //-------------//
        // shoot it up //
        //-------------//

        swath->Shotgun(7, 21);
        swath->HideSpeed(12.0, 100.0);

        //---------------//
        // median filter //
        //---------------//

        swath->MedianFilter(l2a_to_l2b.medianFilterWindowSize,
            l2a_to_l2b.medianFilterMaxPasses, 0,
            l2a_to_l2b.useAmbiguityWeights, 0);

        swath->HideSpeed(100.0, 100.0);
        swath->MedianFilter(l2a_to_l2b.medianFilterWindowSize,
            l2a_to_l2b.medianFilterMaxPasses, 0,
            l2a_to_l2b.useAmbiguityWeights, 0);

        //--------------//
        // write it out //
        //--------------//

        swath->WriteVctr("shot.0", 0);
    }

    return (0);
}
