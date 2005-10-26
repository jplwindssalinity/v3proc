//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    best_filter
//
// SYNOPSIS
//    best_filter [ -h ] <cfg_file> <l2b_input_file> <vctr_base>
//        <l2b_output_file>
//
// DESCRIPTION
//    This filter attempts to set the best vector, ONE WIND VECTOR
//    CELL AT A TIME! Yes, this means it should be really slow.
//
// OPTIONS
//    The following options are supported:
//      [ -h ]  Input and output files are HDF
//
// OPERANDS
//    The following operands are supported:
//      <cfg_file>         The configuration file
//      <l2b_input_file>   The input Level 2B wind field
//      <vctr_base>        The output vctr file basename
//      <l2b_output_file>  The output Level 2B wind field
//
// EXAMPLES
//    An example of a command line is:
//    % best_filter run.cfg l2b.dat l2b.vctr l2b.out
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
#include <unistd.h>
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

#define WORST_SCORE           -9e9
#define HDF_NUM_AMBIGUITIES   4

#define FILTER_FACTOR      13.0

#define FIRST_BEST_VALUE   1.0
#define FIRST_BEST_SCORE   2.0
#define FIRST_BREAK_VALUE  0.8   // gives a score of 1.0
#define FIRST_WORST_VALUE  0.0   // gives a score of 0.0

#define NEIGHBOR_BEST_VALUE   25
#define NEIGHBOR_BEST_SCORE   7.0
#define NEIGHBOR_BREAK_VALUE  15    // gives a score of 1.0
#define NEIGHBOR_WORST_VALUE  0     // gives a score of 0.0

#define DIF_RATIO_BEST_VALUE   0.0
#define DIF_RATIO_BEST_SCORE   2.0
#define DIF_RATIO_BREAK_VALUE  0.5   // gives a score of 1.0
#define DIF_RATIO_WORST_VALUE  1.0   // gives a score of 0.0

/*
#define LOCAL_PROBABILITY_WINDOW_SIZE  7
#define MIN_COUNT                      40
#define MIN_CTI_INNER_SWATH            12
#define MAX_CTI_INNER_SWATH            68
#define TOP_PROB                       0.9

#define MEDIAN_FILTER_WINDOW_SIZE      5

#define CT_WIDTH  76


#define EDGE  13
*/

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

const char* usage_array[] = { "[ -h ]", "<cfg_file>", "<l2b_input_file>",
    "<vctr_base>", "<l2b_output_file>", 0 };

WindVectorPlus*  original_selected[CT_WIDTH][AT_WIDTH];
char             rain_contaminated[CT_WIDTH][AT_WIDTH];

float            first_score[CT_WIDTH][AT_WIDTH];
unsigned char    neighbor_count[CT_WIDTH][AT_WIDTH];
float            dif_ratio[CT_WIDTH][AT_WIDTH];
float            filter_score[CT_WIDTH][AT_WIDTH];

unsigned int  first_count_array[100];    // probability percent
unsigned int  first_match_array[100];
unsigned int  filter_count_array[49][100];    // neighbor count, dif ratio
unsigned int  filter_match_array[49][100];

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

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* l2b_input_file = argv[optind++];
    const char* vctr_base = argv[optind++];
    const char* l2b_output_file = argv[optind++];

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

    if (! l2b.OpenForWriting())
    {
        fprintf(stderr, "%s: error opening L2B file %s for writing\n",
            command, l2b_output_file);
        exit(1);
    }

    //---------//
    // HDF I/O //
    //---------//

    int ctibins = 0;
    int atibins = 0;

    float** spd = NULL;
    float** dir = NULL;
    int** num_ambigs = NULL;

    if (opt_hdf)
    {
        //--------------------//
        // Read Nudge Vectors //
        //--------------------//

        if (! opt_hdf)
        {
            if (! l2b.ReadNudgeVectorsFromHdfL2B(l2b_input_file))
            {
                fprintf(stderr,
                    "%s: error reading nudge vectors from HDF L2B file %s\n",
                    command, l2b_input_file);
                exit(1);
            }
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
//            first_score[cti][ati] = wvp1->obj;

// convert to score
float m, b;
if (wvp1->obj > FIRST_BREAK_VALUE)
{
  m = ((float)FIRST_BEST_SCORE - 1.0) /
    ((float)FIRST_BEST_VALUE -
    (float)FIRST_BREAK_VALUE);
  b = 1.0 - m * (float)FIRST_BREAK_VALUE;
}
else
{
  m = (1.0 - 0.0) / ((float)FIRST_BREAK_VALUE -
    (float)FIRST_WORST_VALUE);
  b = 1.0 - m * (float)FIRST_BREAK_VALUE;
}
first_score[cti][ati] = m * (float)wvp1->obj + b;
        }
    }

/*
    //--------------------------------------------------//
    // calculate the average probability for the window //
    //--------------------------------------------------//
    // INNER SWATH ONLY!

    int half_window = LOCAL_PROBABILITY_WINDOW_SIZE / 2;
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        int ati_min = ati - half_window;
        int ati_max = ati + half_window + 1;
        if (ati_min < 0)
            ati_min = 0;
        if (ati_max > AT_WIDTH)
            ati_max = AT_WIDTH;

        for (int cti = MIN_CTI_INNER_SWATH; cti < MAX_CTI_INNER_SWATH; cti++)
        {
            int cti_min = cti - half_window;
            int cti_max = cti + half_window + 1;
            if (cti_min < MIN_CTI_INNER_SWATH)
                cti_min = MIN_CTI_INNER_SWATH;
            if (cti_max > MAX_CTI_INNER_SWATH)
                cti_max = MAX_CTI_INNER_SWATH;

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            float best_avg_prob = 0.0;
            WindVectorPlus* best_vector = NULL;

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                float target_dir = wvp->dir;
                int wvc_count = 0;
                float prob_sum = 0.0;
                for (int i = ati_min; i < ati_max; i++)
                {
                    for (int j = cti_min; j < cti_max; j++)
                    {
                        WVC* other_wvc = swath->GetWVC(j, i);
                        if (other_wvc == NULL)
                            continue;

                        //------------------------//
                        // only use rainfree data //
                        //------------------------//

                        int unusable = other_wvc->rainFlagBits &
                            RAIN_FLAG_UNUSABLE;
                        if (unusable)
                            continue;
                        int rain = other_wvc->rainFlagBits &
                            RAIN_FLAG_RAIN;
                        if (rain)
                            continue;

                        //----------------------------//
                        // find the nearest ambiguity //
                        //----------------------------//

                        WindVectorPlus* nearest =
                            other_wvc->GetNearestToDirection(target_dir);

                        //------------------------------//
                        // get the probability indicies //
                        //------------------------------//

                        WindVectorPlus* other_wvp_1 =
                            other_wvc->ambiguities.GetHead();
                        if (other_wvp_1 == NULL)
                            continue;

                        float near_prob = 0.0;

                        for (WindVectorPlus* other_wvp =
                            other_wvc->ambiguities.GetHead(); other_wvp;
                            other_wvp = other_wvc->ambiguities.GetNext())
                        {
                            if (other_wvp == nearest)
                                near_prob = other_wvp->obj;
                        }

                        //--------------------------------------//
                        // accumulate the nearest's probability //
                        //--------------------------------------//

                        prob_sum += near_prob;
                        wvc_count++;
                    }
                }
                float avg_prob = prob_sum / (float)wvc_count;
                if (avg_prob > best_avg_prob && wvc_count > MIN_COUNT)
                {
                    best_avg_prob = avg_prob;
                    best_vector = wvp;
                }
            }
            local_prob[cti][ati] = best_avg_prob;
            local_best[cti][ati] = best_vector;
        }
    }
*/

    //----------------------------------------//
    // create arrays for median filter passes //
    //----------------------------------------//

/*
    WindVectorPlus*** new_selected_array =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2, CT_WIDTH,
        AT_WIDTH);
*/

    WindVectorPlus*** filter_selection =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2, CT_WIDTH,
        AT_WIDTH);

    char** change = (char**)make_array(sizeof(char), 2, CT_WIDTH, AT_WIDTH);
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            filter_selection[cti][ati] = NULL;
//            new_selected_array[cti][ati] = NULL;
            change[cti][ati] = 0;
        }
    }

    //----------------//
    // loop till done //
    //----------------//

FILE* score_ofp = NULL;

    int half_window = 2;
    char filename[1024];
    int loop_idx = 0;
    int first_count = 0;
    int filter_count = 0;
    float best_first_score = WORST_SCORE;
    float best_filter_score = WORST_SCORE;
    do
    {
        //---------------//
        // find the best //
        //---------------//

        float best_score = WORST_SCORE;
        ChoiceE best_choice = NO_CHOICE;
        int best_ati = -1;
        int best_cti = -1;
        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                if (rain_contaminated[cti][ati])
                    continue;
                if (first_score[cti][ati] > best_first_score)
                    best_first_score = first_score[cti][ati];
                if (first_score[cti][ati] > best_score)
                {
                    best_score = first_score[cti][ati];
                    best_choice = FIRST;
                    best_ati = ati;
                    best_cti = cti;
                }
                if (filter_score[cti][ati] > best_filter_score)
                    best_filter_score = filter_score[cti][ati];
                if (filter_score[cti][ati] > best_score)
                {
                    best_score = filter_score[cti][ati];
                    best_filter_score = filter_score[cti][ati];
                    best_choice = FILTER;
                    best_ati = ati;
                    best_cti = cti;
                }
            }
        }
        if (best_ati == -1 && best_cti == -1)
            break;

        //-------//
        // do it //
        //-------//

        WVC* wvc = swath->GetWVC(best_cti, best_ati);
        if (wvc == NULL)
        {
            fprintf(stderr, "%s: missing WVC\n", command);
            break;
        }
        int score_idx, neighbor_idx, dif_ratio_idx;
        switch (best_choice)
        {
        case FIRST:
            wvc->selected = wvc->ambiguities.GetByIndex(0);

            score_idx = (int)(100.0 * first_score[best_cti][best_ati] + 0.5);
            if (score_idx < 0) score_idx = 0;
            if (score_idx >= 100) score_idx = 99;
            if (wvc->selected == original_selected[best_cti][best_ati])
                first_match_array[score_idx]++;
            first_count_array[score_idx]++;

            first_score[best_cti][best_ati] = WORST_SCORE;    // done with it
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

            neighbor_idx = neighbor_count[best_cti][best_ati];
            dif_ratio_idx = (int)(dif_ratio[best_cti][best_ati] * 100.0 + 0.5);
            if (dif_ratio_idx < 0) dif_ratio_idx = 0;
            if (dif_ratio_idx >= 100) dif_ratio_idx = 99;
            if (wvc->selected == original_selected[best_cti][best_ati])
                filter_match_array[neighbor_idx][dif_ratio_idx]++;
            filter_count_array[neighbor_idx][dif_ratio_idx]++;

            first_score[best_cti][best_ati] = WORST_SCORE;    // done with it
            filter_score[best_cti][best_ati] = WORST_SCORE;    // done with it
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

                float min_vector_dif_sum = (float)HUGE_VAL;
                float min_vector_dif_avg = (float)HUGE_VAL;
                float second_vector_dif_sum = (float)HUGE_VAL;
                int selected_count = 0;
                WindVectorPlus* new_selected = NULL;

                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    float vector_dif_sum = 0.0;
                    float x1 = wvp->spd * cos(wvp->dir);
                    float y1 = wvp->spd * sin(wvp->dir);

                    selected_count = 0;
                    int available_count = 0;
                    for (int i = cti_min; i < cti_max; i++)
                    {
                        for (int j = ati_min; j < ati_max; j++)
                        {
                            if (i == cti && j == ati)
                                continue;      // don't check central vector

                            WVC* other_wvc = swath->GetWVC(i, j);
                            if (! other_wvc)
                                continue;

                            available_count++;    // other wvc exists

                            WindVectorPlus* other_wvp = other_wvc->selected;
                            if (! other_wvp)
                                continue;

                            selected_count++;   // wvc has a valid selection

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

                //--------------------------//
                // calculate the new scores //
                //--------------------------//

                neighbor_count[cti][ati] = selected_count;
                dif_ratio[cti][ati] = best_to_second_ratio;
                float m, b;

                // neighbor
                if (selected_count > NEIGHBOR_BREAK_VALUE)
                {
                    m = ((float)NEIGHBOR_BEST_SCORE - 1.0) /
                        ((float)NEIGHBOR_BEST_VALUE -
                        (float)NEIGHBOR_BREAK_VALUE);
                    b = 1.0 - m * (float)NEIGHBOR_BREAK_VALUE;
                }
                else
                {
                    m = (1.0 - 0.0) / ((float)NEIGHBOR_BREAK_VALUE -
                        (float)NEIGHBOR_WORST_VALUE);
                    b = 1.0 - m * (float)NEIGHBOR_BREAK_VALUE;
                }
                float neighbor_score = m * (float)selected_count + b;

                // dif ratio
                if (best_to_second_ratio > DIF_RATIO_BREAK_VALUE)
                {
                    m = ((float)DIF_RATIO_BEST_SCORE - 1.0) /
                        ((float)DIF_RATIO_BEST_VALUE -
                        (float)DIF_RATIO_BREAK_VALUE);
                    b = 1.0 - m * (float)DIF_RATIO_BREAK_VALUE;
                }
                else
                {
                    m = (1.0 - 0.0) / ((float)DIF_RATIO_BREAK_VALUE -
                        (float)DIF_RATIO_WORST_VALUE);
                    b = 1.0 - m * (float)DIF_RATIO_BREAK_VALUE;
                }
                float dif_ratio_score = m * (float)best_to_second_ratio + b;

                float new_score = FILTER_FACTOR * neighbor_score *
                    dif_ratio_score;

                // only change the filter score if it means something new
                if (filter_selection[cti][ati] != new_selected)
                {
                    filter_score[cti][ati] = new_score;
if (score_ofp)
  fprintf(score_ofp, "%g %g %g\n", neighbor_score, dif_ratio_score,
    filter_score[cti][ati]);
                }
                filter_selection[cti][ati] = new_selected;
            }
        }
        if (loop_idx % 500 == 0 && loop_idx != 0)
        {
            int total_count = first_count + filter_count;
            float first_percent = 100.0 * (float)first_count /
                (float)total_count;
            float filter_percent = 100.0 * (float)filter_count /
                (float)total_count;
            sprintf(filename, "%s.%06d", vctr_base, loop_idx);
            l2b.WriteVctr(filename, 0);
            printf("1st:%.1f  Fil:%.1f, Best 1st:%g  Best Fil:%g\n",
                first_percent, filter_percent, best_first_score,
                best_filter_score);
            best_first_score = WORST_SCORE;
            best_filter_score = WORST_SCORE;

sprintf(filename, "%s.%06d.scr", vctr_base, loop_idx);
if (score_ofp)
  fclose(score_ofp);
score_ofp = fopen(filename, "w");

FILE* first_fp = fopen("first.dat", "w");
for (int i = 0; i < 100; i++)
{
  if (first_count_array[i] > 1)
  fprintf(first_fp, "%g %g\n", (float)i / 100.0,
    (float)first_match_array[i] / (float)first_count_array[i]);
}
fclose(first_fp);
FILE* filter_fp = fopen("filter.dat", "w");
for (int i = 0; i < 49; i++)
{
  for (int j = 0; j < 100; j++)
  {
    if (filter_count_array[i][j] > 1)
    fprintf(filter_fp, "%d %g %g\n", i, (float)j / 100.0,
      (float)filter_match_array[i][j] / (float)filter_count_array[i][j]);
  }
}
fclose(filter_fp);

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
                    int k = num_ambigs[i][j] - 1;
                    spd[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->spd;
                    dir[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->dir;
                }
            }
        }

        //-----------------//
        // Update HDF file //
        //-----------------//

        if (! l2b.frame.swath.UpdateHdf(l2b_output_file, spd, dir, num_ambigs,
            num_ambigs))
        {
            fprintf(stderr, "%s: Unable to update hdf file\n", command);
            exit(1);
        }
    }

    l2b.Close();
    free_array((void*)spd, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)dir, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)num_ambigs, 2, atibins, ctibins);

    return (0);
}
