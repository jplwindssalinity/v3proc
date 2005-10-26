//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    obprob
//
// SYNOPSIS
//    obprob <config_file> <hdf_l2a_file> <dirprob_file>
//
// DESCRIPTION
//    "Retrieves" the wind by calculating the probability density
//    as a function of direction. The speed of the peak direction
//    slice and the integrated probability (over speed) for that
//    direction are calculated and stored.
//
// OPTIONS
//
// OPERANDS
//    <config_file>   The config file (for the GMF).
//    <hdf_l2a_file>  The input HDF L2A file.
//    <dirprob_file>  The output flag file.
//
// EXAMPLES
//    An example of a command line is:
//      % obprob $cfg l2a.203 203.dirprob
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
#include "ObProb.h"

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
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING        ""

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<config_file>", "<hdf_l2a_file>",
    "<dirprob_file>", 0 };

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

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    const char* dirprob_file = argv[optind++];

    //-------------------------//
    // read in the config file //
    //-------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading config file %s\n", command,
            config_file);
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

    //--------------------//
    // open the L2AH file //
    //--------------------//

    L2AH l2ah;
    if (! l2ah.OpenForReading(l2a_hdf_file))
    {
        fprintf(stderr, "%s: error opening L2A HDF file %s\n", command,
            l2a_hdf_file);
        exit(1);
    }

    //-------------------//
    // open dirprob file //
    //-------------------//

    FILE* ofp = fopen(dirprob_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening dirprob file %s\n", command,
            dirprob_file);
        exit(1);
    }

    //--------------//
    // for each WVC //
    //--------------//

    ObProb op;
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* ml = l2ah.GetWVC(cti, ati, L2AH::OCEAN_ONLY);
            if (ml == NULL)
                continue;
            if (ml->NodeCount() < 4)
                continue;

            //-----------------------------//
            // determine the probabilities //
            //-----------------------------//

            if (! op.RetrieveProbabilities(&gmf, ml, &kp))
            {
                fprintf(stderr, "%s: error retrieving probabilities\n",
                    command);
                exit(1);
            }

            //--------------------//
            // free the meas list //
            //--------------------//

            ml->FreeContents();

            //----------------------//
            // write the obprob out //
            //----------------------//

            op.cti = cti;
            op.ati = ati;
            if (! op.Write(ofp))
            {
                fprintf(stderr, "%s: error writing obprob to file %s\n",
                    command, dirprob_file);
                exit(1);
            }
        }
    }

    fclose(ofp);

    return (0);
}
