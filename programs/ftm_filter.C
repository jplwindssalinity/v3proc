//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    ftm_filter
//
// SYNOPSIS
//    ftm_filter [ -fhnp ] [ -b bottom_prob ] <sim_cfg_file>
//        <l2b_input_file> <vctr_base> <l2b_output_file>
//
// DESCRIPTION
//
// OPTIONS
//    The following options are supported:
//      [ -f ]  Fill in unsafe areas with wild propagation.
//      [ -h ]  Input and output files are HDF
//      [ -n ]  Nudge.
//      [ -p ]  Propagate into low wind speeds.  Same criteria.
//      [ -b bottom_prob ]  Minimum probability (usu .7 or .2)
//
// OPERANDS
//    The following operands are supported:
//      <l2b_file>     The input Level 2B wind field
//      [ vctr_base ]  The output vctr file basename
//
// EXAMPLES
//    An example of a command line is:
//    % ftm_filter l2b.dat l2b.vctr
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

#define OPTSTRING  "fhnpb:"

//--------------------------//
// local probability filter //
//--------------------------//

#define LOCAL_PROBABILITY_WINDOW_SIZE  7
#define MIN_COUNT                      40
#define MIN_CTI_INNER_SWATH            12
#define MAX_CTI_INNER_SWATH            68
#define TOP_PROB                       0.9

#define MEDIAN_FILTER_WINDOW_SIZE      5

#define CT_WIDTH  78

#define HDF_NUM_AMBIGUITIES   4

#define EDGE  13

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

const char* usage_array[] = { "[ -fhnp ]", "[ -b bottom_prob ]",
    "<sim_cfg_file>", "<l2b_input_file>", "<vctr_base>",
    "<l2b_output_file>", 0 };

extern float g_speed_stopper;
extern int g_number_needed;
extern float g_too_different;
extern float g_error_ratio_of_best;
extern float g_error_of_best;
extern int   g_rain_bit_flag_on;

float            local_prob[AT_WIDTH][CT_WIDTH];
WindVectorPlus*  local_best[AT_WIDTH][CT_WIDTH];

int opt_fill = 0;
int opt_hdf = 0;
int opt_nudge = 0;
int opt_prop = 0;
float g_bottom_prob = 0.7;

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
        case 'b':
            g_bottom_prob = atof(optarg);
            break;
        case 'f':
            opt_fill = 1;
            break;
        case 'h':
            opt_hdf = 1;
            break;
        case 'n':
            opt_nudge = 1;
            break;
        case 'p':
            opt_prop = 1;
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
        fprintf(stderr, "%s: error reading sim config file %s\n",
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
            wvc->selected = NULL;    // initialize
        }
    }

    //-------------------------------------------//
    // replace mle values with mle probabilities //
    //-------------------------------------------//

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
        }
    }

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
            local_prob[ati][cti] = best_avg_prob;
            local_best[ati][cti] = best_vector;
        }
    }

    //----------------//
    // loop till done //
    //----------------//

    float speed[] =      {  5.0,  3.0,  5.0,  3.0 };
    float best_ratio[] = { 0.25, 0.25, 0.25, 0.25 };
    float best_error[] = { 0.05, 0.05, 0.05, 0.05 };
    int number[] =       {   10,   10,   10,   10 };
    int border[] =       { EDGE, EDGE,    0,    0 };

    char filename[1024];
    int init_idx = 0;
    for (float best_thresh = TOP_PROB; best_thresh > g_bottom_prob;
        best_thresh -= 0.1)
    {
        //------------//
        // initialize //
        //------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = MIN_CTI_INNER_SWATH; cti < MAX_CTI_INNER_SWATH;
                cti++)
            {
                WVC* wvc = swath->GetWVC(cti, ati);
                if (wvc == NULL)
                    continue;

                if (wvc->selected)
                    continue;

                //------------------------//
                // only use rainfree data //
                //------------------------//

                int unusable = wvc->rainFlagBits &
                    RAIN_FLAG_UNUSABLE;
                if (unusable)
                    continue;
                int rain = wvc->rainFlagBits & 
                    RAIN_FLAG_RAIN;
                if (rain)
                    continue;

                if (local_prob[ati][cti] > best_thresh)
                {
                    wvc->selected = local_best[ati][cti];
                }
            }
        }

        sprintf(filename, "%s.i%02d.p0", vctr_base, init_idx);
        l2b.WriteVctr(filename, 0);

        for (int pass_idx = 0; pass_idx < 2; pass_idx++)
        {
g_rain_bit_flag_on = 1;
            printf("Init %02d, Pass %d\n", init_idx, pass_idx + 1);
            g_speed_stopper = speed[pass_idx];
            g_error_ratio_of_best = best_ratio[pass_idx];
            g_number_needed = number[pass_idx];
            g_error_of_best = best_error[pass_idx];
    
            swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200,
                border[pass_idx], 0, 0);
            sprintf(filename, "%s.i%02d.p%d", vctr_base, init_idx,
                pass_idx + 1);
            l2b.WriteVctr(filename, 0);
        }
        init_idx++;
    }

    //------------------//
    // nudge initialize //
    //------------------//

    if (opt_nudge)
    {
        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                WVC* wvc = swath->GetWVC(cti, ati);
                if (wvc == NULL)
                    continue;

                if (wvc->selected)
                    continue;

                WindVectorPlus* wvp_1 = wvc->ambiguities.GetHead();
                WindVectorPlus* wvp_2 = wvc->ambiguities.GetNext();

                if (wvp_1 == NULL || wvp_2 == NULL)
                    continue;

                float ang_dif = ANGDIF(wvp_1->dir, wvp_2->dir) * rtd;
                if (ang_dif < 170.0)
                    continue;    // not a streamline, don't try to nudge

                float nudge_dir = wvc->nudgeWV->dir;
                WindVectorPlus* nearest =
                    wvc->GetNearestToDirection(nudge_dir, 2);
                wvc->selected = nearest;
            }
        }

        //--------------------------//
        // nudge propagation passes //
        //--------------------------//

        for (int pass_idx = 0; pass_idx < 4; pass_idx++)
        {
            printf("Nudge, Pass %d\n", pass_idx + 1);
            g_speed_stopper = speed[pass_idx];
            g_error_ratio_of_best = best_ratio[pass_idx];
            g_number_needed = number[pass_idx];
            g_error_of_best = best_error[pass_idx];

            swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200,
                border[pass_idx], 0, 0);
            sprintf(filename, "%s.n.p%d", vctr_base, pass_idx + 1);
            l2b.WriteVctr(filename, 0);
        }
    }

    //-------------//
    // fill passes //
    //-------------//

    if (opt_prop)
    {
        g_speed_stopper = 2.0;
        g_error_ratio_of_best = 0.25;
        g_number_needed = 10;
        g_error_of_best = 0.1;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.p1", vctr_base);
        l2b.WriteVctr(filename, 0);

        g_speed_stopper = 1.0;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.p2", vctr_base);
        l2b.WriteVctr(filename, 0);

        g_speed_stopper = 0.0;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.p3", vctr_base);
        l2b.WriteVctr(filename, 0);
    }

    if (opt_fill)
    {
        g_speed_stopper = 2.0;
        g_error_ratio_of_best = 1.0;
        g_number_needed = 1;
        g_error_of_best = 1.0;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.f1", vctr_base);
        l2b.WriteVctr(filename, 0);

        g_speed_stopper = 1.0;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.f2", vctr_base);
        l2b.WriteVctr(filename, 0);

        g_speed_stopper = 0.0;

        swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);
        sprintf(filename, "%s.f3", vctr_base);
        l2b.WriteVctr(filename, 0);
    }

    //----------------------//
    // write out vctr files //
    //----------------------//

    int max_rank = swath->GetMaxAmbiguityCount();
    for (int i = 0; i <= max_rank; i++)
    {
        sprintf(filename, "%s.%d", vctr_base, i);
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
