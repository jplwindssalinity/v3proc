//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    op_filter
//
// SYNOPSIS
//    op_filter <change_file> <obprob_file> <output_base>
//
// DESCRIPTION
//    Performs ambiguity removal using probabilities.
//
// OPTIONS
//
// OPERANDS
//    <change_file>   A file containing delta speed and direction probs.
//    <obprob_file>   The input probability file.
//    <output_base>   The usual.
//
// EXAMPLES
//    An example of a command line is:
//      % op_filter $cfg change.dat 203.obprob 203
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

// centered on every 1 m/s
#define MIN_SPEED  0.0
#define MAX_SPEED  30.0
#define IDX_SPEED  31

// centered on every 25 km
#define MIN_DIST  0.0
#define MAX_DIST  1000.0
#define IDX_DIST  41

#define MIN_DSPEED  -10.0
#define MAX_DSPEED  10.0
#define IDX_DSPEED  21

#define MIN_DDIR  0.0
#define MAX_DDIR  180.0
#define IDX_DDIR  37

#define MAX_LOOPS    1
#define WINDOW_SIZE  7

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<distprob_file>", "<obprob_file>",
    "<output_base>", 0 };

unsigned long count[IDX_SPEED][IDX_DIST][IDX_DSPEED][IDX_DDIR];

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

    const char* distprob_file = argv[optind++];
    const char* obprob_file = argv[optind++];
    const char* output_base = argv[optind++];

    //------------------------//
    // read the distprob file //
    //------------------------//

    DistProb dp;
    if (! dp.Read(distprob_file))
    {
        fprintf(stderr, "%s: error opening distprob file %s\n", command,
            distprob_file);
        exit(1);
    }

    //------------------//
    // read obprob file //
    //------------------//

    ObProbArray opa;
    if (! opa.Read(obprob_file))
    {
        fprintf(stderr, "%s: error reading obprob file %s\n", command,
            obprob_file);
        exit(1);
    }

    //----------------------//
    // open the output file //
    //----------------------//

    FILE* ofp = fopen_or_exit(output_base, "w", command, "flower output file",
        1);

    //--------//
    // filter //
    //--------//

    ObProbArray opa2;
    for (int loop_idx = 0; loop_idx < MAX_LOOPS; loop_idx++)
    {
        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                if (loop_idx == 0)
                {
                    ObProb* ns = opa.GetObProb(cti, ati);
                    ns->WriteFlower(ofp);

/*
                    ns = opa.LocalProb(&dp, WINDOW_SIZE, cti, ati);
                    opa2.array[cti][ati] = ns;
*/
                }           
            }
        }
    }

    fclose(ofp);

    //------------------------//
    // generate a flower plot //
    //------------------------//

//    opa2.WriteFlower("flower.out", 100, 200);

    return (0);
}
