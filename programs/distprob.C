//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    distprob
//
// SYNOPSIS
//    distprob <cfg_file> <output_file> <hdf_l2b_file...>
//
// DESCRIPTION
//    This program calculates probabilities of speed and direction
//    changes as a function of speed and distance.
//
// OPTIONS
//    The following options are supported:
//
// OPERANDS
//    The following operands are supported:
//      <cfg_file>         The simulation configuration file.
//      <output_file>      The starting point for output file names.
//      <hdf_l2b_file...>  The list of input Level 2B files.
//
// EXAMPLES
//    An example of a command line is:
//    % distprob sim.cfg distprob.dat /seapac/disk2/L2B/data/SW*
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
#include <math.h>
#include "ConfigList.h"
#include "L2AToL2B.h"
#include "ConfigSim.h"
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "PointList.h"
#include "ObProb.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Point>;
template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
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

const char* usage_array[] = { "<cfg_file>", "<output_file>",
    "<hdf_l2b_file...>", 0 };

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

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* output_file = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;

    DistProb dp;

    //---------------------------//
    // determine the window size //
    //---------------------------//

    int window_size = 2 * (int)(MAX_DISTANCE / WVC_RESOLUTION) + 1;

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, config_file);
        exit(1);
    }

    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        const char* hdf_l2b_file = argv[file_idx];
printf("%s\n", hdf_l2b_file);

        //-------------------//
        // read the l2b file //
        //-------------------//

        L2B l2b;
        if (! l2b.ReadPureHdf(hdf_l2b_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                hdf_l2b_file);
            exit(1);
        }
        WindSwath* swath = &(l2b.frame.swath);

        //----------------------------//
        // estimate the probabilities //
        //----------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                WVC* wvc = swath->GetGoodWVC(cti, ati);
                if (wvc == NULL)
                    continue;

                WindVectorPlus* wvc_nudge = wvc->nudgeWV;

                //------------------------------------//
                // construct a list of neighbor WVC's //
                //------------------------------------//

                PointList neighbor_list;
                neighbor_list.SetRange(0, CT_WIDTH, 0, AT_WIDTH);
                neighbor_list.AddWindow(cti, ati, window_size);

                while (! neighbor_list.IsEmpty())
                {
                    //-------------//
                    // get the WVC //
                    //-------------//

                    neighbor_list.GotoHead();
                    Point* neighbor_point = neighbor_list.RemoveCurrent();
                    int other_cti = neighbor_point->cti;
                    int other_ati = neighbor_point->ati;
                    delete neighbor_point;

                    WVC* other_wvc = swath->GetGoodWVC(other_cti, other_ati);
                    if (other_wvc == NULL)
                        continue;

                    if (other_wvc == wvc)
                        continue;

                    WindVectorPlus* other_wvc_nudge = other_wvc->nudgeWV;

                    //--------------------------//
                    // calculate the parameters //
                    //--------------------------//

                    int dati = other_ati - ati;
                    int dcti = other_cti - cti;
                    float distance = (float)(WVC_RESOLUTION *
                        sqrt((double)(dati * dati + dcti * dcti)));
                    if (distance > MAX_DISTANCE)
                        continue;

                    float speed = wvc_nudge->spd;

                    float dspeed = other_wvc_nudge->spd - wvc_nudge->spd;

                    float ddirection = ANGDIF(other_wvc_nudge->dir,
                        wvc_nudge->dir);

                    //-----------------------//
                    // calculate the indices //
                    //-----------------------//

                    int dist_idx = dp.DistanceToIndex(distance);
                    int speed_idx = dp.SpeedToIndex(speed);
                    int dspeed_idx = dp.DeltaSpeedToIndex(dspeed);
                    int ddir_idx = dp.DeltaDirectionToIndex(ddirection);

                    //------------//
                    // accumulate //
                    //------------//

                    dp.count[dist_idx][speed_idx][dspeed_idx][ddir_idx]++;
                }
            }
        }
        if (! dp.Write(output_file))
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                output_file);
            exit(0);
        }
    }

    //--------------------//
    // write output files //
    //--------------------//

    if (! dp.Write(output_file))
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(0);
    }
    return(0);
}
