//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    big_filter
//
// SYNOPSIS
//    big_filter <sim_cfg_file> <l2b_input_file> <min_prob> <vctr_base>
//        <l2b_output_file> [ hdf_source_flag ] [hdf_target_flag ]
//
// DESCRIPTION
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <l2b_file>     The input Level 2B wind field
//      [ vctr_base ]  The output vctr file basename
//
// EXAMPLES
//    An example of a command line is:
//    % big_filter l2b.dat l2b.vctr
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
#include "BufferedList.h"
#include "Tracking.h"
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

#define DIF_DIM           37
#define RSPD_DIM          8
#define MAX_SPD           20.0
#define RANK_DIM          4

//--------------------------//
// local probability filter //
//--------------------------//

#define LOCAL_PROBABILITY_WINDOW_SIZE  7
#define MIN_COUNT                      40
#define MIN_CTI_FOR_LOCAL_PROB         12
#define MAX_CTI_FOR_LOCAL_PROB         68

#define MEDIAN_FILTER_WINDOW_SIZE  5
#define MAX_RETRIEVED_ANGLE_DIF  30.0

#define CT_WIDTH  78

#define HDF_NUM_AMBIGUITIES   4

#define SOLO_VECTOR_PROB    0.5

//---------//
// nudging //
//---------//

#define MAX_NUDGE_ANGLE_DIF      20.0
#define NUDGE_MATCH_ANGLE        30.0    // nudge only when this close

#define MIN_CTI_FOR_NUDGE   0
#define MAX_CTI_FOR_NUDGE   0
/*
#define MIN_CTI_FOR_NUDGE   12
#define MAX_CTI_FOR_NUDGE   68
*/

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

const char* usage_array[] = { "<sim_cfg_file>", "<l2b_input_file>",
    "<min_prob>", "<vctr_base>", "<l2b_output_file>", "[ hdf_source_flag ]",
    "[ hdf_target_flag ]", 0 };

extern float g_speed_stopper;
extern int g_number_needed;
extern float g_too_different;
extern float g_second_choice_fraction;
extern float g_error_of_best;

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
    if (argc < 6 || argc > 8)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* l2b_input_file = argv[clidx++];
    float min_prob = atof(argv[clidx++]);
    const char* vctr_base = argv[clidx++];
    const char* l2b_output_file = argv[clidx++];

    int hdf_source_flag = 0;
    int hdf_target_flag = 0;
    if (argc == 8)
    {
        hdf_source_flag = atoi(argv[clidx++]);
        hdf_target_flag = atoi(argv[clidx++]);
    }

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

    if (hdf_source_flag)
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

    if (hdf_target_flag)
    {
        //--------------------//
        // Read Nudge Vectors //
        //--------------------//

        if (! hdf_source_flag)
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

    int half_window = LOCAL_PROBABILITY_WINDOW_SIZE / 2;
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        int ati_min = ati - half_window;
        int ati_max = ati + half_window + 1;
        if (ati_min < 0)
            ati_min = 0;
        if (ati_max > AT_WIDTH)
            ati_max = AT_WIDTH;

        for (int cti = MIN_CTI_FOR_LOCAL_PROB; cti < MAX_CTI_FOR_LOCAL_PROB;
            cti++)
        {
            int cti_min = cti - half_window;
            int cti_max = cti + half_window + 1;
            if (cti_min < MIN_CTI_FOR_LOCAL_PROB)
                cti_min = MIN_CTI_FOR_LOCAL_PROB;
            if (cti_max > MAX_CTI_FOR_LOCAL_PROB)
                cti_max = MAX_CTI_FOR_LOCAL_PROB;

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
            if (best_avg_prob > min_prob)
            {
                wvc->selected = best_vector;
                continue;
            }

            //-----------------------//
            // check for streamlines //
            //-----------------------//

            WindVectorPlus* wvp_1 = wvc->ambiguities.GetHead();
            WindVectorPlus* wvp_2 = wvc->ambiguities.GetNext();
            if (wvp_1 == NULL || wvp_2 == NULL)
                continue;

            float ang_dif = ANGDIF(wvp_1->dir, wvp_2->dir) * rtd;
            if (ang_dif < 170.0)
                continue;    // not a streamline, don't try to nudge

            //--------------------//
            // do the nudge check //
            //--------------------//

            if (best_vector == NULL)
                continue;

            float best_dir = best_vector->dir;
            float nudge_dir = wvc->nudgeWV->dir;

            int good = 1;
            for (int i = ati_min; i < ati_max; i++)
            {
                for (int j = cti_min; j < cti_max; j++)
                {
                    WVC* other_wvc = swath->GetWVC(j, i);
                    if (other_wvc == NULL)
                        continue;

                    WindVectorPlus* nearest =
                        other_wvc->GetNearestToDirection(best_dir);

                    if (nearest == NULL)
                        continue;

                    float other_dir = nearest->dir;
                    float dif = ANGDIF(best_dir, other_dir) * rtd;
                    if (dif > MAX_RETRIEVED_ANGLE_DIF)
                    {
                        good = 0;
                        break;
                    }

                    float other_nudge_dir = other_wvc->nudgeWV->dir;
                    float nudge_dif = ANGDIF(nudge_dir, other_nudge_dir) * rtd;
                    if (nudge_dif > MAX_NUDGE_ANGLE_DIF)
                    {
                        good = 0;
                        break;
                    }
                }
            }
            if (good == 1)
            {
                WindVectorPlus* nudge_choice =
                    wvc->GetNearestToDirection(wvc->nudgeWV->dir, 2);

                float angdif = ANGDIF(wvc->nudgeWV->dir, nudge_choice->dir) *
                    rtd;
                if (angdif < NUDGE_MATCH_ANGLE &&
                    cti >= MIN_CTI_FOR_NUDGE && cti <= MAX_CTI_FOR_NUDGE)
                {
                    wvc->selected = nudge_choice;
                }
            }
        }
    }

    char filename[1024];
    sprintf(filename, "%s.biginit", vctr_base);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command,
            filename);
        exit(1);
    }

    //------------------------//
    // Just Ambiguity Removal //
    //------------------------//

/*
    float speed[] =  {    5.0,    5.0,  5.0,  5.0,    5.0,    5.0, 5.0, 5.0,
         3.0,  3.0, 3.0,  3.0,  0.0,  0.0, 0.0,  0.0, 0.0 };
    float second[] = {   0.25,   0.25, 0.25, 0.25,    0.3,    0.3, 0.3, 0.3,
        0.25, 0.25, 0.3,  0.3, 0.25, 0.25, 0.3,  0.3, 1.0 };
    int number[] =   {     10,      8,   10,    8,     10,      8,  10,   8,
          10,    8,  10,    8,   10,    8,  10,    8,   1 };
    int border[] =   { EDGE, EDGE,    0,    0, EDGE, EDGE,   0,   0,
           0,    0,   0,    0,    0,    0,   0,    0,   0 };
*/
/*
    float speed[] =  {    5.0,    5.0,  5.0,  5.0,    5.0,    5.0, 5.0, 5.0,
         3.0,  3.0, 3.0,  3.0,  2.0,  2.0, 0.0,  0.0 };
    float second[] = {   0.25,   0.25, 0.25, 0.25,    0.3,    0.3, 0.3, 0.3,
        0.25, 0.25, 0.3,  0.3, 0.25, 0.25, 0.3,  0.3 };
    int number[] =   {      8,      4,    8,    4,      8,      4,   8,   4,
           8,    4,   8,    4,    8,    4,   8,    4 };
    int border[] =   { EDGE, EDGE,    0,    0, EDGE, EDGE,   0,   0,
           0,    0,   0,    0,    0,    0,   0,    0 };
*/

    float speed[] =  {  5.0,  5.0,  3.0,  3.0 };
    float second[] = { 0.25, 0.25, 0.25, 0.25 };
    float bad[] =    {  0.1,  0.1,  0.1,  0.1 };
    int number[] =   {   10,   10,   10,   10 };
    int border[] =   { EDGE,    0, EDGE,    0 };

for (int pass = 0; pass < 4; pass++)
{
printf("Pass %d\n", pass);
    g_speed_stopper = speed[pass];
    g_second_choice_fraction = second[pass];
    g_number_needed = number[pass];
    g_error_of_best = bad[pass];
    
    swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 150, border[pass], 0, 0);
    sprintf(filename, "%s.pass.%02d", vctr_base, pass);
    l2b.WriteVctr(filename, 0);
}

//------------------//
// fill in any gaps //
//------------------//

/*
for (int ati = 0; ati < AT_WIDTH; ati++)
{
    for (int cti = 0; cti < CT_WIDTH; cti++)
    {
        WVC* wvc = swath->GetWVC(cti, ati);
        if (wvc == NULL)
            continue;
        if (wvc->selected)
            continue;
        wvc->selected = wvc->ambiguities.GetHead();
    }
}
swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 50, 0, 0, 0);
*/

    if (! hdf_target_flag)
    {
        if (! l2b.WriteHeader() || ! l2b.WriteDataRec())
        {
            fprintf(stderr, "%s: error writing L2B file\n", command);
            exit(1);
        }
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

    //---------//
    // HDF I/O //
    //---------//

    if (hdf_target_flag)
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
