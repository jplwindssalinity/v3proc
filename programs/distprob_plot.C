//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    distprob_plot
//
// SYNOPSIS
//    distprob <distprob_file> <output_base>
//
// DESCRIPTION
//    This program generates plottable files of projected probabilities.
//
// OPTIONS
//    The following options are supported:
//
// OPERANDS
//    The following operands are supported:
//      <distprob_file>  The distprob file.
//      <output_base>    The output base.
//
// EXAMPLES
//    An example of a command line is:
//    % distprob_plot distprob.dat distprob
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
#include "Misc.h"
#include "ObProb.h"
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
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class List<MeasSpot>;
template class List<EarthPosition>;
template class List<long>;
template class List<OffsetList>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

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

const char* usage_array[] = { "<distprob_file>", "<output_base>", 0 };

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

    const char* distprob_file = argv[optind++];
    const char* output_base = argv[optind++];

    //------------------------//
    // read the distprob file //
    //------------------------//

    DistProb dp;
    if (! dp.Read(distprob_file))
    {
        fprintf(stderr, "%s: error reading distprob file %s\n", command,
            distprob_file);
        exit(0);
    }

    //---------------------------//
    // generate the output files //
    //---------------------------//

    char filename[1024];
    for (int distance_idx = 0; distance_idx < DISTANCE_BINS; distance_idx++)
    {
        float distance = dp.IndexToDistance(distance_idx);
        for (int speed_idx = 0; speed_idx < SPEED_BINS; speed_idx++)
        {
            float speed = dp.IndexToSpeed(speed_idx);
            sprintf(filename, "%s.%02d.%02d.dspd", output_base, distance_idx,
                speed_idx);
            FILE* ofp = fopen(filename, "w");
            if (ofp == NULL)
            {
                fprintf(stderr, "%s: error opening output file %s\n", command,
                    filename);
                exit(1);
            }
            fprintf(ofp, "# distance = %g\n", distance);
            fprintf(ofp, "# speed = %g\n", speed);

            for (int dspeed_idx = 0; dspeed_idx < DSPEED_BINS; dspeed_idx++)
            {
                unsigned long sum = 0;
                for (int ddirection_idx = 0; ddirection_idx < DDIRECTION_BINS;
                     ddirection_idx++)
                {
                sum +=
                 dp.count[distance_idx][speed_idx][dspeed_idx][ddirection_idx];
                }
                fprintf(ofp, "%g %g\n", dp.IndexToDeltaSpeed(dspeed_idx),
                    (double)sum / dp.sum);
            }
            fclose(ofp);
        }
    }

    return(0);
}
