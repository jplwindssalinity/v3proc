//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    wr_nbd_to_apts
//
// SYNOPSIS
//    wr_nbd_to_apts <hdf_l2b_file> <nbd_file>
//        <apts_output_file>
//
// DESCRIPTION
//    Uses the latitudes and longitudes from the nbd file to
//    produce an apts file.
//
// OPTIONS
//    None.
//
// OPERANDS
//    <hdf_l2b_file>      The input HDF L2B file.
//    <nbd_file>          The input NBD file.
//    <apts_output_file>  The output apts file.
//
// EXAMPLES
//    An example of a command line is:
//      % wr_nbd_to_apts $cfg l2b.203 203.nbd 203.apts
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
#include "L2AHdf.h"
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "SeaPac.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING        ""
#define MAX_S0_PER_ROW   3240
#define MAX_CROSS_TRACK  76
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004
#define ANTENNA_BEAM     0x0004
#define MIN_RAIN_DN      0
#define NBD_SCALE        16.0

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<hdf_l2b_file>",
    "<nbd_file>", "<apts_output_file>", 0 };

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

    const char* l2b_hdf_file = argv[optind++];
    const char* nbd_file = argv[optind++];
    const char* apts_output_file = argv[optind++];

    //---------------//
    // read nbd file //
    //---------------//

    char nbd_array[1624][76];
    FILE* ifp = fopen(nbd_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening NBD file %s\n", command,
            nbd_file);
        exit(1);
    }
    fread(nbd_array, sizeof(char), 76 * 1624, ifp);
    fclose(ifp);

    //-----------------------//
    // read in level 2B file //
    //-----------------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_hdf_file);
    if (! l2b.ReadHDF())
    {
        fprintf(stderr, "%s: error reading L2B file %s\n", command,
            l2b_hdf_file);
        exit(1);
    }
    WindSwath* swath = &(l2b.frame.swath);

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(apts_output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            apts_output_file);
        exit(1);
    }
    fprintf(ofp, "apts\n");

    //---------------//
    // for each cell //
    //---------------//

    for (int ati = 0; ati < 1624; ati++)
    {
        for (int cti = 0; cti < 72; cti++)
        {
            if (nbd_array[ati][cti] == -128)
                continue;
 
            WVC* l2b_wvc = swath->GetWVC(cti, ati);
            if (l2b_wvc == NULL)
                continue;

            fprintf(ofp, "%g %g %g\n", l2b_wvc->lonLat.longitude * rtd,
                l2b_wvc->lonLat.latitude * rtd,
                (float)nbd_array[ati][cti] / NBD_SCALE);
        }
    }

    //-------------//
    // close files //
    //-------------//

    fclose(ofp);

    return (0);
}
