//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    first_probs
//
// SYNOPSIS
//    first_probs [ -h ] <cfg_file> <first_prob_file>
//        <l2b_input_file...>
//
// DESCRIPTION
//    This program reads in a bunch of l2b files and estimates the
//    probability of the first ranked vector being "right" as a
//    funciton of objective function probability and first ranked
//    wind speed.
//
// OPTIONS
//    The following options are supported:
//      [ -h ]  Input and output files are HDF
//
// OPERANDS
//    The following operands are supported:
//      <cfg_file>           The configuration file
//      <first_prob_file>    The output probability count file.
//      <l2b_input_file...>  The input Level 2B wind field
//
// EXAMPLES
//    An example of a command line is:
//    % first_probs run.cfg adapt.prob l2b.1.dat l2b.2.dat l2b.3.dat
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
#include <stdlib.h>
#include <math.h>
#include "ConfigList.h"
#include "L2AToL2B.h"
#include "ConfigSim.h"
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "Index.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "mudh.h"
#include "best.h"

//-----------//
// TEMPLATES //
//-----------//

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

#define OPTSTRING  "h"

#define HDF_NUM_AMBIGUITIES   4

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

const char* usage_array[] = { "[ -h ]", "<cfg_file>", "<first_prob_file>",
    "<l2b_input_file...>", 0 };

WindVectorPlus*  original_selected[CT_WIDTH][AT_WIDTH];
char             rain_contaminated[CT_WIDTH][AT_WIDTH];

float            first_obj_prob[CT_WIDTH][AT_WIDTH];
float            first_obj_speed[CT_WIDTH][AT_WIDTH];

unsigned short  first_count_array[FIRST_INDICIES][SPEED_INDICIES];
unsigned short  first_good_array[FIRST_INDICIES][SPEED_INDICIES];

int opt_hdf = 0;

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
        case 'h':
            opt_hdf = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* first_prob_file = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading config file %s\n",
            command, config_file);
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

    //-----------------------------------//
    // initialize some index calculators //
    //-----------------------------------//

    Index first_index, speed_index;
    first_index.SpecifyCenters(FIRST_MIN_VALUE, FIRST_MAX_VALUE,
        FIRST_INDICIES);
    speed_index.SpecifyCenters(SPEED_MIN_VALUE, SPEED_MAX_VALUE,
        SPEED_INDICIES);

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    for (int idx = start_idx; idx < end_idx; idx++)
    {
        const char* l2b_input_file = argv[idx];
        printf("%s\n", l2b_input_file);
        L2B l2b;
        l2b.SetInputFilename(l2b_input_file);

        //-------------------//
        // read the l2b file //
        //-------------------//

        if (opt_hdf)
        {
            if( l2b.ReadHDF() == 0)
            {
                fprintf(stderr, "%s: error opening HDF L2B file %s\n", command,
                    l2b_input_file);
                exit(1);
            }
        }
        else
        {
            if (! l2b.OpenForReading())
            {
                fprintf(stderr, "%s: error opening L2B file %s\n", command,
                    l2b_input_file);
                exit(1);
            }
            if (! l2b.ReadHeader())
            {
                fprintf(stderr, "%s: error reading L2B header from file %s\n",
                    command, l2b_input_file);
                exit(1);
            }
            if (! l2b.ReadDataRec())
            {
                fprintf(stderr,
                    "%s: error reading L2B data record from file %s\n",
                    command, l2b_input_file);
                exit(1);
            }
        }

        //---------//
        // HDF I/O //
        //---------//

        int ctibins = 0;
        int atibins = 0;

        float** spd = NULL;
        float** dir = NULL;
        int** num_ambigs = NULL;
        int** sel_idx = NULL;

        if (opt_hdf)
        {
            //---------------//
            // Create Arrays //
            //---------------//

            ctibins = l2b.frame.swath.GetCrossTrackBins();
            atibins = l2b.frame.swath.GetAlongTrackBins();
            spd = (float**)make_array(sizeof(float), 2, atibins,
                ctibins * HDF_NUM_AMBIGUITIES);
            dir = (float**)make_array(sizeof(float), 2, atibins,
                ctibins * HDF_NUM_AMBIGUITIES);

            num_ambigs = (int**) make_array(sizeof(int), 2, atibins, ctibins);
            sel_idx = (int**) make_array(sizeof(int), 2, atibins, ctibins);

            if (! l2b.GetArraysForUpdatingDirthHdf(spd, dir, num_ambigs))
            {
                fprintf(stderr,
                    "%s: Failure to create array for updating hdf file\n",
                    command);
                exit(1);
            }
        }

        //----------------------------//
        // clear all selected vectors //
        //----------------------------//

        WindSwath* swath = &(l2b.frame.swath);
        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                WVC* wvc = swath->GetWVC(cti, ati);
                if (wvc == NULL)
                    continue;
                original_selected[cti][ati] = wvc->selected;
                wvc->selected = NULL;    // initialize
            }
        }

        //-------------------------------------------//
        // replace mle values with mle probabilities //
        //-------------------------------------------//
        // set the rain contaminated array
        // set the first ranked probability array

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                WVC* wvc = swath->GetWVC(cti, ati);
                if (wvc == NULL)
                    continue;

                WindVectorPlus* wvp1 = wvc->ambiguities.GetHead();
                if (wvp1 == NULL)
                    continue;

                if (wvc->rainFlagBits & RAIN_FLAG_UNUSABLE ||
                    wvc->rainFlagBits & RAIN_FLAG_RAIN)
                {
                    rain_contaminated[cti][ati] = 1;
                }

                float wvc_prob_sum = 0.0;
                float scale = wvp1->obj;
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    wvp->obj = exp((wvp->obj - scale)/2.0);
                    wvc_prob_sum += wvp->obj;
                }

                if (wvc_prob_sum == 0.0)
                    continue;

                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    wvp->obj /= wvc_prob_sum;
                }
                first_obj_prob[cti][ati] = wvp1->obj;
                first_obj_speed[cti][ati] = wvp1->spd;

                //---------------//
                // learn quickly //
                //---------------//

                if (first_obj_prob[cti][ati] > FIRST_MIN_VALUE &&
                    ! rain_contaminated[cti][ati])
                {
                    int fidx,sidx;
                    first_index.GetNearestIndexClipped(first_obj_prob[cti][ati],
                       &fidx);
                    speed_index.GetNearestIndexClipped(
                       first_obj_speed[cti][ati], &sidx);

                    // scale if necessary
                    if (first_count_array[fidx][sidx] >= MAX_SHORT)
                    {
printf("chopping %d %d\n", first_good_array[fidx][sidx],
    first_count_array[fidx][sidx]);
                        double new_count =
                          (double)first_count_array[fidx][sidx] * 15.0 / 16.0;
                        double new_good =
                          (double)first_good_array[fidx][sidx] * 15.0 / 16.0;
                        first_count_array[fidx][sidx] =
                          (unsigned short)(new_count + 0.5);
                        first_good_array[fidx][sidx] =
                          (unsigned short)(new_good + 0.5);
                        if (first_good_array[fidx][sidx] >=
                            first_count_array[fidx][sidx])
                        {
                          first_good_array[fidx][sidx] =
                            first_count_array[fidx][sidx];
                        }
                    }
                    if (wvp1 == original_selected[cti][ati])
                        first_good_array[fidx][sidx]++;
                    first_count_array[fidx][sidx]++;
                }
            }
        }
        free_array((void *)spd, 2, atibins, ctibins * HDF_NUM_AMBIGUITIES);
        free_array((void *)dir, 2, atibins, ctibins * HDF_NUM_AMBIGUITIES);
        free_array((void *)num_ambigs, 2, atibins, ctibins);
        free_array((void *)sel_idx, 2, atibins, ctibins);

        l2b.Close();
    }

    //-----------------//
    // write prob file //
    //-----------------//

    FILE* ofp = fopen(first_prob_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            first_prob_file);
        exit(1);
    }
    else
    {
        fwrite(first_count_array, sizeof(unsigned short),
            FIRST_INDICIES*SPEED_INDICIES, ofp);
        fwrite(first_good_array, sizeof(unsigned short),
            FIRST_INDICIES*SPEED_INDICIES, ofp);
        fclose(ofp);
    }

    exit(0);
}
