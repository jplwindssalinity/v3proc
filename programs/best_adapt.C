//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    best_adapt
//
// SYNOPSIS
//    best_adapt [ -h ] <cfg_file> <prob_file> <l2b_input_file>
//        <vctr_base> <l2b_output_file>
//
// DESCRIPTION
//    This filter attempts to set the best vector, ONE WIND VECTOR
//    CELL AT A TIME! Yes, this means it should be really slow.
//    It also uses (and updates) a probability file. If you don't want
//    to lose the contents of the probability file, make a backup
//    *BEFORE* running this program -- it will stomp.
//
// OPTIONS
//    The following options are supported:
//      [ -h ]  Input and output files are HDF
//
// OPERANDS
//    The following operands are supported:
//      <cfg_file>         The configuration file
//      <prob_file>        The probability count file.
//      <l2b_input_file>   The input Level 2B wind field
//      <vctr_base>        The output vctr file basename
//      <l2b_output_file>  The output Level 2B wind field
//
// EXAMPLES
//    An example of a command line is:
//    % best_filter run.cfg adapt.prob l2b.dat l2b.vctr l2b.out
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

#define WORST_PROB           -9e9
#define HDF_NUM_AMBIGUITIES   4

#define FIRST_MIN_VALUE  0.0
#define FIRST_MAX_VALUE  1.0
#define FIRST_INDICIES   100

#define NEIGHBOR_MIN_VALUE  0
#define NEIGHBOR_MAX_VALUE  25
#define NEIGHBOR_INDICIES   25

#define DIF_RATIO_MIN_VALUE  0.0
#define DIF_RATIO_MAX_VALUE  1.0
#define DIF_RATIO_INDICIES   10

#define BALANCE_MIN_VALUE  0.0
#define BALANCE_MAX_VALUE  1.0
#define BALANCE_INDICIES   10

#define SPEED_MIN_VALUE  0.0
#define SPEED_MAX_VALUE  30.0
#define SPEED_INDICIES   30

#define WINDOW_SIZE  5
#define MIN_SAMPLES  4

#define MEDIAN_FILTER_WINDOW_SIZE  7

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

enum ChoiceE { NO_CHOICE, FIRST, FILTER };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -h ]", "<cfg_file>", "<prob_file>",
    "<l2b_input_file>", "<vctr_base>", "<l2b_output_file>", 0 };

WindVectorPlus*  original_selected[CT_WIDTH][AT_WIDTH];
char             rain_contaminated[CT_WIDTH][AT_WIDTH];

float            first_obj_prob[CT_WIDTH][AT_WIDTH];
unsigned char    neighbor_count[CT_WIDTH][AT_WIDTH];
float            dif_ratio[CT_WIDTH][AT_WIDTH];
float            balance[CT_WIDTH][AT_WIDTH];
float            speed[CT_WIDTH][AT_WIDTH];

unsigned int  first_count_array[FIRST_INDICIES];
unsigned int  first_good_array[FIRST_INDICIES];

unsigned int filter_count_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][BALANCE_INDICIES][SPEED_INDICIES];
unsigned int filter_good_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][BALANCE_INDICIES][SPEED_INDICIES];

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

    if (argc < optind + 5)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* prob_file = argv[optind++];
    const char* l2b_input_file = argv[optind++];
    const char* vctr_base = argv[optind++];
    const char* l2b_output_file = argv[optind++];

    //-------------------//
    // read in prob file //
    //-------------------//

    FILE* ifp = fopen(prob_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            prob_file);
        fprintf(stderr, "    File will be created.\n");
    }
    else
    {
        fread(first_count_array, sizeof(unsigned int), FIRST_INDICIES, ifp);
        fread(first_good_array, sizeof(unsigned int), FIRST_INDICIES, ifp);
        fread(filter_count_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
          ifp);
        fread(filter_good_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
          ifp);
        fclose(ifp);
    }

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

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_input_file);

    // Overwrite with new name
    l2b.SetOutputFilename(l2b_output_file);

    //------------------------------------//
    // create and configure the converter //
    //------------------------------------//

    L2AToL2B l2a_to_l2b;
    if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

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
            fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command, l2b_input_file);
            exit(1);
        }
    }

/*
    if (! l2b.OpenForWriting())
    {
        fprintf(stderr, "%s: error opening L2B file %s for writing\n",
            command, l2b_output_file);
        exit(1);
    }
*/

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
        //--------------------//
        // Read Nudge Vectors //
        //--------------------//

        if (! l2b.ReadNudgeVectorsFromHdfL2B(l2b_input_file))
        {
            fprintf(stderr,
                "%s: error reading nudge vectors from HDF L2B file %s\n",
                command, l2b_input_file);
            exit(1);
        }

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
        }
    }

    //----------------------------------------//
    // create arrays for median filter passes //
    //----------------------------------------//

    WindVectorPlus*** filter_selection =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2, CT_WIDTH,
        AT_WIDTH);

    char** change = (char**)make_array(sizeof(char), 2, CT_WIDTH, AT_WIDTH);
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            filter_selection[cti][ati] = NULL;
            change[cti][ati] = 0;
        }
    }

    //-----------------------------------//
    // initialize some index calculators //
    //-----------------------------------//

    Index first_index, neighbor_index, dif_ratio_index, balance_index,
        speed_index;

    first_index.SpecifyCenters(FIRST_MIN_VALUE, FIRST_MAX_VALUE,
        FIRST_INDICIES);
    neighbor_index.SpecifyCenters(NEIGHBOR_MIN_VALUE, NEIGHBOR_MAX_VALUE,
        NEIGHBOR_INDICIES);
    dif_ratio_index.SpecifyCenters(DIF_RATIO_MIN_VALUE, DIF_RATIO_MAX_VALUE,
        DIF_RATIO_INDICIES);
    balance_index.SpecifyCenters(BALANCE_MIN_VALUE, BALANCE_MAX_VALUE,
        BALANCE_INDICIES);
    speed_index.SpecifyCenters(SPEED_MIN_VALUE, SPEED_MAX_VALUE,
        SPEED_INDICIES);

    //----------------//
    // loop till done //
    //----------------//

    int half_window = WINDOW_SIZE / 2;
    char filename[1024];
    int loop_idx = 0;
    int first_count = 0;
    int filter_count = 0;
    do
    {
        //----------------------------------------------------------------//
        // find the WVC with the highest probability of being set "right" //
        //----------------------------------------------------------------//

        float best_prob = WORST_PROB;
        ChoiceE best_choice = NO_CHOICE;
        int best_first_idx = 0;
        int best_neighbor_idx = 0;
        int best_dif_ratio_idx = 0;
        int best_balance_idx = 0;
        int best_speed_idx = 0;
        int best_ati = -1;
        int best_cti = -1;
        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                WVC* wvc = swath->GetWVC(cti, ati);
                if (wvc == NULL)
                    continue;

                // ignore rain contaminated WVC
                if (rain_contaminated[cti][ati])
                    continue;

                //----------------------------------------//
                // calculate the first ranked probability //
                //----------------------------------------//

                float first_prob;
                int tmp_idx[2];
                float first_coef[2];
                first_index.GetLinearCoefsClipped(first_obj_prob[cti][ati],
                    tmp_idx, first_coef);
                if (first_count_array[tmp_idx[0]] >= MIN_SAMPLES &&
                    first_count_array[tmp_idx[1]] >= MIN_SAMPLES)
                {
                    first_prob = first_coef[0] *
                        (float)first_good_array[tmp_idx[0]] /
                        (float)first_count_array[tmp_idx[0]] +
                        first_coef[1] *
                        (float)first_good_array[tmp_idx[1]] /
                        (float)first_count_array[tmp_idx[1]];
                }
                else
                {
                    first_prob = first_obj_prob[cti][ati];
                }

                if (first_prob > best_prob)
                {
                    best_prob = first_prob;
                    first_index.GetNearestIndex(first_obj_prob[cti][ati],
                        &best_first_idx);
                    best_choice = FIRST;
                    best_ati = ati;
                    best_cti = cti;
                }

                //----------------------------------//
                // calculate the filter probability //
                //----------------------------------//

                float filter_prob;
           
                int neighbor_idx, dif_ratio_idx, balance_idx, speed_idx;
                neighbor_index.GetNearestIndex(neighbor_count[cti][ati],
                        &neighbor_idx);
                dif_ratio_index.GetNearestIndex(dif_ratio[cti][ati],
                        &dif_ratio_idx);
                balance_index.GetNearestIndex(balance[cti][ati],
                        &balance_idx);
                speed_index.GetNearestIndex(speed[cti][ati],
                        &speed_idx);

                if (filter_count_array[neighbor_idx][dif_ratio_idx][balance_idx][speed_idx] >= MIN_SAMPLES)
                {
                    filter_prob =
(float)filter_good_array[neighbor_idx][dif_ratio_idx][balance_idx][speed_idx] /
(float)filter_count_array[neighbor_idx][dif_ratio_idx][balance_idx][speed_idx];
                }
                else
                {
                    // assign a probability randomly
                    filter_prob = drand48();
                    filter_prob *= drand48();  // to skew it lower
                    // don't do something stupid
                    if (neighbor_count[cti][ati] < 2)
                        filter_prob *= drand48();    // make it lower
                }

                if (filter_prob > best_prob &&
                    wvc->selected != filter_selection[cti][ati])
                {
                    best_prob = filter_prob;
                    best_neighbor_idx = neighbor_idx;
                    best_dif_ratio_idx = dif_ratio_idx;
                    best_balance_idx = balance_idx;
                    best_speed_idx = speed_idx;
                    best_choice = FILTER;
                    best_ati = ati;
                    best_cti = cti;
                }
            }
        }
        if (best_ati == -1 && best_cti == -1)
        {
            printf("No more bests\n");
            break;
        }

        //-------//
        // do it //
        //-------//

        WVC* wvc = swath->GetWVC(best_cti, best_ati);
        if (wvc == NULL)
        {
            fprintf(stderr, "%s: missing WVC\n", command);
            continue;
        }
        switch (best_choice)
        {
        case FIRST:
            wvc->selected = wvc->ambiguities.GetByIndex(0);

            if (wvc->selected == original_selected[best_cti][best_ati])
                first_good_array[best_first_idx]++;
            first_count_array[best_first_idx]++;

            // never let this WVC get initialized by first rank again
            first_obj_prob[best_cti][best_ati] = WORST_PROB;
            change[best_cti][best_ati] = 1;
            first_count++;
            break;

        case FILTER:
            if (filter_selection[best_cti][best_ati] == NULL)
            {
                fprintf(stderr, "%s: no filter selection\n", command);
                exit(1);
            }
            wvc->selected = filter_selection[best_cti][best_ati];

            if (wvc->selected == original_selected[best_cti][best_ati])
            {
  filter_good_array[best_neighbor_idx][best_dif_ratio_idx][best_balance_idx][best_speed_idx]++;
            }
 filter_count_array[best_neighbor_idx][best_dif_ratio_idx][best_balance_idx][best_speed_idx]++;

            // don't let this WVC get initialized by first rank
            first_obj_prob[best_cti][best_ati] = WORST_PROB;  // done with it
            change[best_cti][best_ati] = 1;
            filter_count++;
            break;

        default:
            fprintf(stderr, "%s: some kind of weird error\n", command);
            exit(1);
            break;
        }

        //---------------------------------//
        // evaluate stuff that has changed //
        //---------------------------------//

        int big_cti_min = best_cti - half_window;
        int big_cti_max = best_cti + half_window + 1;
        if (big_cti_min < 0)
            big_cti_min = 0;
        if (big_cti_max > CT_WIDTH)
            big_cti_max = CT_WIDTH;

        int big_ati_min = best_ati - half_window;
        int big_ati_max = best_ati + half_window + 1;
        if (big_ati_min < 0)
            big_ati_min = 0;
        if (big_ati_max > AT_WIDTH)
            big_ati_max = AT_WIDTH;

        for (int ati = big_ati_min; ati < big_ati_max; ati++)
        {
            int ati_min = ati - half_window;
            int ati_max = ati + half_window + 1;
            if (ati_min < 0)
                ati_min = 0;
            if (ati_max > AT_WIDTH)
                ati_max = AT_WIDTH;
            for (int cti = big_cti_min; cti < big_cti_max; cti++)
            {
                int cti_min = cti - half_window;
                int cti_max = cti + half_window + 1;
                if (cti_min < 0)
                    cti_min = 0;
                if (cti_max > CT_WIDTH)
                    cti_max = CT_WIDTH;

                //-----------------------------------//
                // this is the WVC we are evaluating //
                //-----------------------------------//

                WVC* wvc = swath->GetWVC(cti, ati);
                if (! wvc)
                    continue;

                //-------------------------------------//
                // quick get some of the values needed //
                //-------------------------------------//

                int selected_count = 0;
                int available_count = 0;
                float dcti_sum = 0.0;
                float dati_sum = 0.0;
                for (int i = cti_min; i < cti_max; i++)
                {
                    for (int j = ati_min; j < ati_max; j++)
                    {
                        if (i == cti && j == ati)
                            continue;

                        WVC* other_wvc = swath->GetWVC(i, j);
                        if (! other_wvc)
                            continue;

                        available_count++;    // other wvc exists

                        WindVectorPlus* other_wvp = other_wvc->selected;
                        if (! other_wvp)
                            continue;

                        selected_count++;   // wvc has a valid selection

                        // for balance calculation
                        dcti_sum += (i - cti);
                        dati_sum += (j - ati);
                    }
                }
                // balance calculation
                float balance_dist = 0.0;
                if (selected_count> 0)
                {
                    float avg_dcti = dcti_sum / (float)selected_count;
                    float avg_dati = dati_sum / (float)selected_count;
                    balance_dist = sqrt(avg_dcti*avg_dcti + avg_dati*avg_dati);
                }

                //-------------------------//
                // then, do the evaluation //
                //-------------------------//

                float min_vector_dif_sum = (float)HUGE_VAL;
                float min_vector_dif_avg = (float)HUGE_VAL;
                float second_vector_dif_sum = (float)HUGE_VAL;
                WindVectorPlus* new_selected = NULL;

                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    float vector_dif_sum = 0.0;
                    float x1 = wvp->spd * cos(wvp->dir);
                    float y1 = wvp->spd * sin(wvp->dir);

                    for (int i = cti_min; i < cti_max; i++)
                    {
                        for (int j = ati_min; j < ati_max; j++)
                        {
                            if (i == cti && j == ati)
                                continue;      // don't check central vector

                            WVC* other_wvc = swath->GetWVC(i, j);
                            if (! other_wvc)
                                continue;

                            WindVectorPlus* other_wvp = other_wvc->selected;
                            if (! other_wvp)
                                continue;

                            float x2 = other_wvp->spd * cos(other_wvp->dir);
                            float y2 = other_wvp->spd * sin(other_wvp->dir);

                            float dx = x2 - x1;
                            float dy = y2 - y1;
                            vector_dif_sum += sqrt(dx*dx + dy*dy);
                        }
                    }
                    if (vector_dif_sum < min_vector_dif_sum)
                    {
                        second_vector_dif_sum = min_vector_dif_sum;
                        min_vector_dif_sum = vector_dif_sum;
                        min_vector_dif_avg = vector_dif_sum /
                            (float)selected_count;
                        new_selected = wvp;
                    }
                    else if (vector_dif_sum < second_vector_dif_sum)
                    {
                        second_vector_dif_sum = vector_dif_sum;
                    }
                }   // done with ambiguities

                // how must does the best beat the second best?
                float best_to_second_ratio = 1.0;
                if (second_vector_dif_sum > 0.0 &&
                    second_vector_dif_sum != (float)HUGE_VAL)
                {
                    best_to_second_ratio = min_vector_dif_sum /
                        second_vector_dif_sum;
                }
                float dif_to_speed_ratio = 1.0;
                if (new_selected != NULL && selected_count > 0 &&
                    new_selected->spd > 0.0)
                {
                    float avg_vector_dif = min_vector_dif_avg /
                        (float)selected_count;
                    dif_to_speed_ratio = avg_vector_dif /
                        new_selected->spd;
                }

                //----------------//
                // store the info //
                //----------------//

                neighbor_count[cti][ati] = selected_count;
                dif_ratio[cti][ati] = best_to_second_ratio;
                balance[cti][ati] = balance_dist;
                speed[cti][ati] = new_selected->spd;
                filter_selection[cti][ati] = new_selected;
// printf("%d %g %g\n", selected_count, best_to_second_ratio, balance_dist);
            }
        }
        if (loop_idx % 1000 == 0 && loop_idx != 0)
        {
            int total_count = first_count + filter_count;
            float first_percent = 100.0 * (float)first_count /
                (float)total_count;
            float filter_percent = 100.0 * (float)filter_count /
                (float)total_count;
            printf("%d  1st:%.2f  Fil:%.2f\n", loop_idx, first_percent,
                filter_percent);
            sprintf(filename, "%s.%06d", vctr_base, loop_idx);
            l2b.WriteVctr(filename, 0);

            //-----------------//
            // write prob file //
            //-----------------//

            FILE* ofp = fopen(prob_file, "w");
            if (ofp == NULL)
            {
                fprintf(stderr, "%s: error opening prob file %s\n", command,
                    prob_file);
                exit(1);
            }
            else
            {
                fwrite(first_count_array, sizeof(unsigned int), FIRST_INDICIES,
                    ofp);
                fwrite(first_good_array, sizeof(unsigned int), FIRST_INDICIES,
                    ofp);
                fwrite(filter_count_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
                    ofp);
                fwrite(filter_good_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
                    ofp);
                fclose(ofp);
            }

// compare to original
unsigned long comp_total_count = 0;
unsigned long match_count = 0;
for (int ati = 0; ati < AT_WIDTH; ati++)
{
  for (int cti = 0; cti < CT_WIDTH; cti++)
  {
    WVC* wvc = swath->GetWVC(cti, ati);
    if (wvc == NULL)
        continue;
    if (wvc->selected == NULL)
        continue;
    if (wvc->selected == original_selected[cti][ati])
        match_count++;
    comp_total_count++;
  }
}
printf("Match = %.2f %%\n", 100.0 * (float)match_count /
  (float)comp_total_count);

        }
        loop_idx++;
    } while (1);

    sprintf(filename, "%s.done", vctr_base);
    l2b.WriteVctr(filename, 0);

    free_array(filter_selection, 2, CT_WIDTH, AT_WIDTH);
    free_array(change, 2, CT_WIDTH, AT_WIDTH);

    //-----------------//
    // write prob file //
    //-----------------//

    FILE* ofp = fopen(prob_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            prob_file);
        exit(1);
    }
    else
    {
        fwrite(first_count_array, sizeof(unsigned int), FIRST_INDICIES,
            ofp);
        fwrite(first_good_array, sizeof(unsigned int), FIRST_INDICIES,
            ofp);
        fwrite(filter_count_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
          ofp);
        fwrite(filter_good_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*BALANCE_INDICIES*SPEED_INDICIES,
          ofp);
        fclose(ofp);
    }

    //---------------------//
    // final median filter //
    //---------------------//

    swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);

/*
    //----------------------//
    // write out vctr files //
    //----------------------//

    int max_rank = swath->GetMaxAmbiguityCount();
    for (int i = 0; i <= max_rank; i++)
    {
        sprintf(filename, "%s.ftm.%d", vctr_base, i);
        if (! l2b.WriteVctr(filename, i))
        {
            fprintf(stderr, "%s: error writing vctr file %s\n", command,
                filename);
            exit(1);
        }
    }

    //--------------------------------------//
    // count the number of selected vectors //
    //--------------------------------------//

    int valid_wvc = 0;
    int selected_wvc = 0;
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;
            valid_wvc++;

            if (wvc->selected == NULL)
                continue;
            selected_wvc++;
        }
    }

    printf("%d out of %d (%.2f %%)\n", selected_wvc, valid_wvc, 100.0 *
        (float)selected_wvc / (float)valid_wvc);
*/

    //--------//
    // output //
    //--------//

    if (! opt_hdf)
    {
        if (! l2b.WriteHeader() || ! l2b.WriteDataRec())
        {
            fprintf(stderr, "%s: error writing L2B file\n", command);
            exit(1);
        }
    }

    //---------//
    // HDF I/O //
    //---------//

    if (opt_hdf)
    {
        //-------------------------------------------//
        // First Update Arrays with selected vectors //
        //-------------------------------------------//

        for (int i = 0; i < atibins; i++)
        {
            for (int j = 0; j < ctibins; j++)
            {
                WVC* wvc = l2b.frame.swath.swath[j][i];
                if (! wvc)
                {
                    num_ambigs[i][j] = 0;
                    continue;
                }
                else if (! (wvc->selected))
                {
                    num_ambigs[i][j] = 0;
                    continue;
                }
                else
                {
                    WindVectorPlus* wvp = wvc->ambiguities.GetHead();
                    for (int k = 0; k < num_ambigs[i][j]; k++)
                    {
                        if (wvc->selected == wvp)
                            sel_idx[i][j] = k + 1;
                        wvp = wvc->ambiguities.GetNext();
                    }
/*
                    int k = num_ambigs[i][j] - 1;
                    spd[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->spd;
                    dir[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->dir;
*/
                }
            }
        }

        //-----------------//
        // Update HDF file //
        //-----------------//

        if (! l2b.frame.swath.UpdateHdf(l2b_output_file, spd, dir, num_ambigs,
            sel_idx))
        {
            fprintf(stderr, "%s: Unable to update hdf file %s\n", command,
                l2b_output_file);
            exit(1);
        }
    }

    l2b.Close();
    free_array((void*)spd, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)dir, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)num_ambigs, 2, atibins, ctibins);
    free_array((void*)sel_idx, 2, atibins, ctibins);

    return (0);
}
