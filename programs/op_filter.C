//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    op_filter
//
// SYNOPSIS
//    op_filter <distprob_file> <obprob_file> <gamma> <output_base>
//
// DESCRIPTION
//    Performs ambiguity removal using probabilities.
//
// OPTIONS
//
// OPERANDS
//    <distprob_file>   A file containing delta speed and direction probs.
//    <obprob_file>     The input probability file.
//    <gamma>           0.0=treat errors as uncorrelated. 1.0=correlate
//    <output_base>     The usual.
//
// EXAMPLES
//    An example of a command line is:
//      % op_filter $cfg distprob.dat 203.obprob 203
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
#define WINDOW_SIZE  5

#define START_ATI  200

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
    "<gamma>", "<output_base>", 0 };

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

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* distprob_file = argv[optind++];
    const char* obprob_file = argv[optind++];
    float gamma = atof(argv[optind++]);
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

    //-----------------------//
    // open the output files //
    //-----------------------//

    int gamma_int = (int)(gamma * 100.0 + 0.5);

    char filename[2048];
    sprintf(filename, "%s.%03d.flw", output_base, gamma_int);
    FILE* add_ofp = fopen_or_exit(filename, "w", command,
        "flower output file", 1);

    sprintf(filename, "%s.mle", output_base);
    FILE* mle_ofp = fopen_or_exit(filename, "w", command,
        "flower mle output file", 1);

    sprintf(filename, "%s.%03d.wnd", output_base, gamma_int);
    FILE* wnd_ofp = fopen_or_exit(filename, "w", command, "wind output file",
        1);

    sprintf(filename, "%s.%03d.prb", output_base, gamma_int);
    FILE* prb_ofp = fopen_or_exit(filename, "w", command,
        "probability output file", 1);

    //--------//
    // filter //
    //--------//

    ObProbArray opa2;
    for (int loop_idx = 0; loop_idx < MAX_LOOPS; loop_idx++)
    {
        for (int ati = START_ATI; ati < AT_WIDTH; ati++)
        {
            int cti1, cti2, cti_step;
            if (ati % 2 == 0)
            {
                cti1 = 0;
                cti2 = CT_WIDTH;
                cti_step = 1;
            }
            else
            {
                cti1 = CT_WIDTH - 1;
                cti2 = -1;
                cti_step = -1;
            }
            for (int cti = cti1; cti != cti2; cti += cti_step)
            {
                if (loop_idx == 0)
                {
                    ObProb* ns = opa.GetObProb(cti, ati);
                    if (ns == NULL)
                        continue;

                    ns->WriteFlower(mle_ofp, 3.0);
                    fprintf(mle_ofp, "%d %d\n", cti, ati);

                    ns = opa.LocalProb(&dp, WINDOW_SIZE, cti, ati, gamma);
                    if (ns == NULL)
                        continue;

                    ns->WriteBestVector(wnd_ofp);
                    ns->WriteBestProb(prb_ofp);

                    ns->WriteFlower(add_ofp, 0.0, 0.75);
                    fprintf(add_ofp, "%d %d\n", cti, ati);
                    delete ns;

//                    opa2.array[cti][ati] = ns;
                }           
            }
        }
    }

    fclose(mle_ofp);
    fclose(add_ofp);
    fclose(wnd_ofp);
    fclose(prb_ofp);

    //------------------------//
    // generate a flower plot //
    //------------------------//

//    opa2.WriteFlower("flower.out", 100, 200);

    return (0);
}
