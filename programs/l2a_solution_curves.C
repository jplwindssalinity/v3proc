//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_solution_curves
//
// SYNOPSIS
//    l2a_solution_curves <config_file> <hdf_l2a_file> <cti> <ati>
//        <output_file>
//
// DESCRIPTION
//    Generates solution curves for the specified wind vector cell.
//
// OPTIONS
//
// OPERANDS
//    <config_file>   The config file (for the GMF).
//    <hdf_l2a_file>  The input HDF L2A file.
//    <cti>           The cross track index of the desired WVC.
//    <ati>           The along track index of the desired WVC.
//    <output_file>   The output file.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_solution_curves $cfg l2a.203 1043 61 1043.61.sc
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
#include <unistd.h>
#include "Misc.h"
#include "ConfigList.h"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "ConfigSim.h"
#include "L2AH.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<StringPair>;
template class BufferedList<OrbitState>;
template class List<AngleInterval>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING        ""

#define UNNORMALIZE_MLE_FLAG  0    // leave the mle alone

#define QUOTE  '"'

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<config_file>", "<hdf_l2a_file>", "<cti>",
    "<ati>", "<output_file>", 0 };

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

    if (argc < optind + 5)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    int target_cti = atoi(argv[optind++]);
    int target_ati = atoi(argv[optind++]);
    const char* output_file = argv[optind++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, config_file);
        exit(1);
    }

    //-------------------------------------//
    // read the geophysical model function //
    //-------------------------------------//

    GMF gmf;
    char* gmf_filename = config_list.Get(GMF_FILE_KEYWORD);
    if (gmf_filename == NULL)
    {
        fprintf(stderr, "%s: error determining GMF file\n", command);
        exit(1);
    }

    if (! gmf.ReadOldStyle(gmf_filename))
    {
        fprintf(stderr, "%s: error reading GMF file %s\n", command,
            gmf_filename);
        exit(1);
    }

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //------------------//
    // set the l2a file //
    //------------------//

    L2AH l2ah;
    if (! l2ah.OpenForReading(l2a_hdf_file))
    {
        fprintf(stderr, "%s: error opening L2A HDF file %s\n", command,
            l2a_hdf_file);
        exit(1);
    }

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    MeasList* meas_list = l2ah.GetWVC(target_cti, target_ati,
        L2AH::OCEAN_ONLY);
    if (meas_list == NULL)
    {
        fprintf(stderr, "%s: error getting WVC %d, %d\n", command, target_cti,
            target_ati);
        exit(1);
    }

    if (meas_list->NodeCount() == 0)
    {
        fprintf(stderr, "%s: WVC %d, %d has no measurements\n", command,
            target_cti, target_ati);
        exit(1);
    }

    fprintf(ofp, "@ subtitle %cWVC %d, %d%c\n", QUOTE, target_cti, target_ati,
        QUOTE);

    //-----------------------//
    // write solution curves //
    //-----------------------//

    gmf.WriteSolutionCurves(ofp, meas_list, &kp);

    fclose(ofp);

    return (0);
}
