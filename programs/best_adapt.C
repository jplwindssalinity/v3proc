//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    best_adapt
//
// SYNOPSIS
//    best_adapt [ -h ] [ -acijr ] [ -m fraction ] [ -d # ] <cfg_file>
//        <prob_file> <l2b_input_file> <vctr_base> <l2b_output_file>
//        [ eval_file ]
//
// DESCRIPTION
//    This filter attempts to set the best vector, ONE WIND VECTOR
//    CELL AT A TIME! Yes, this means it should be really slow.
//    It also uses (and updates) probability files. The first
//    ranked probability is no longer updated! -- Use first_prob to
//    create a starter file for best_adapt.
//
// OPTIONS
//    The following options are supported:
//      [ -h ]  Input and output files are HDF
//      [ -a ]  Adapt. Update the probability table on the fly.
//      [ -c ]  Correct. Fix bad selections before proceeding.
//      [ -i ]  Interpolate. Interpolate filter probabilities. Slow.
//      [ -j ]  Jack. Jack up probabilities based on N.
//      [ -r ]  Random. Use random probabilities for low sample events.
//      [ -m fraction ]  Auto(M)atic. Correct while the good fraction
//        is below fraction.  Don't correct otherwise, plus lower
//        probabilities -std.
//      [ -d # ]  Dump. Spit out info every # loops.
//
// OPERANDS
//    The following operands are supported:
//      <cfg_file>         The configuration file
//      <prob_file>        The input probability count file.
//      <l2b_input_file>   The input Level 2B wind field
//      <vctr_base>        The output vctr file basename
//      <l2b_output_file>  The output Level 2B wind field
//      [ eval_file ]      The output file for evaluation information.
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

#define OPTSTRING  "hacijrd:m:"

#define WORST_PROB           -9e9
#define HDF_NUM_AMBIGUITIES   4

#define WINDOW_SIZE  5
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

int write_eval(FILE* ofp, int bn_idx, int bdr_idx, int bs_idx,
    int bc_idx, int bp_idx, int good);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -h ]", "[ -acijr ]", "[ -m fraction ]",
    "[ -d # ]", "<cfg_file>", "<prob_file>", "<l2b_input_file>",
    "<vctr_base>", "<l2b_output_file>", "<eval_file>", 0 };

WindVectorPlus*  original_selected[CT_WIDTH][AT_WIDTH];
char             rain_contaminated[CT_WIDTH][AT_WIDTH];

char             has_neighbors[CT_WIDTH][AT_WIDTH];
float            first_obj_prob[CT_WIDTH][AT_WIDTH];
float            first_actual_prob[CT_WIDTH][AT_WIDTH];
float            first_speed[CT_WIDTH][AT_WIDTH];

unsigned char  filter_neighbor_idx[CT_WIDTH][AT_WIDTH];
unsigned char  filter_dif_ratio_idx[CT_WIDTH][AT_WIDTH];
unsigned char  filter_speed_idx[CT_WIDTH][AT_WIDTH];
unsigned char  filter_cti_idx[CT_WIDTH];
unsigned char  filter_prob_idx[CT_WIDTH][AT_WIDTH];

unsigned short  first_count_array[FIRST_INDICIES][CTI_INDICIES][SPEED_INDICIES];
unsigned short  first_good_array[FIRST_INDICIES][CTI_INDICIES][SPEED_INDICIES];

unsigned short  filter_count_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];
unsigned short  filter_good_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];

int opt_hdf = 0;
int opt_adapt = 0;
int opt_correct = 0;
int opt_interpolate = 0;
int opt_jack = 0;
int opt_random = 0;
int opt_dump = 0;
int dump_loops = 0;
int opt_matic = 0;
float matic_fraction = 0.0;

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
        case 'a':
            opt_adapt = 1;
            break;
        case 'c':
            opt_correct = 1;
            break;
        case 'i':
            opt_interpolate = 1;
            break;
        case 'j':
            opt_jack = 1;
            break;
        case 'r':
            opt_random = 1;
            break;
        case 'm':
            opt_matic = 1;
            matic_fraction = atof(optarg);
            break;
        case 'd':
            opt_dump = 1;
            dump_loops = atoi(optarg);
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

    const char* eval_file = NULL;
    if (optind < argc)
        eval_file = argv[optind++];

    if (opt_matic)
    {
        printf("  Matic mode.\n");
        printf("  (Disabling correction/jacking/random)\n");
        opt_correct = 0;
        opt_jack = 0;
        opt_random = 0;
        printf("  (Enabling adapt)\n");
        opt_adapt = 1;
    }

    //-------------------//
    // read in prob file //
    //-------------------//

    size_t first_nitems = FIRST_INDICIES * CTI_INDICIES * SPEED_INDICIES;
    size_t filter_nitems = NEIGHBOR_INDICIES * DIF_RATIO_INDICIES *
        SPEED_INDICIES * CTI_INDICIES * PROB_INDICIES;

    FILE* ifp = fopen(prob_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            prob_file);
        fprintf(stderr, "    File will be created.\n");
    }
    else
    {
        if (fread(first_count_array, sizeof(unsigned short), first_nitems,
              ifp) != first_nitems ||
            fread(first_good_array, sizeof(unsigned short), first_nitems,
              ifp) != first_nitems)
        {
            fprintf(stderr, "%s: error reading probability file %s\n",
                command, prob_file);
            exit(1);
        }

        if (fread(filter_count_array, sizeof(unsigned short), filter_nitems,
              ifp) != filter_nitems ||
            fread(filter_good_array, sizeof(unsigned short), filter_nitems,
              ifp) != filter_nitems)
        {
            fprintf(stderr, "%s: only first ranked probabilities available\n",
                command);
            fprintf(stderr, "    continuing...\n");
        }
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

    //-----------------------------------//
    // initialize some index calculators //
    //-----------------------------------//

    Index first_index, neighbor_index, dif_ratio_index, speed_index,
        cti_index, prob_index;

    first_index.SpecifyCenters(FIRST_MIN_VALUE, FIRST_MAX_VALUE,
        FIRST_INDICIES);
    neighbor_index.SpecifyCenters(NEIGHBOR_MIN_VALUE, NEIGHBOR_MAX_VALUE,
        NEIGHBOR_INDICIES);
    dif_ratio_index.SpecifyCenters(DIF_RATIO_MIN_VALUE, DIF_RATIO_MAX_VALUE,
        DIF_RATIO_INDICIES);
    speed_index.SpecifyCenters(SPEED_MIN_VALUE, SPEED_MAX_VALUE,
        SPEED_INDICIES);
    cti_index.SpecifyCenters(CTI_MIN_VALUE, CTI_MAX_VALUE, CTI_INDICIES);
    prob_index.SpecifyCenters(PROB_MIN_VALUE, PROB_MAX_VALUE, PROB_INDICIES);

    //----------------------------------//
    // precalculate filter cti indicies //
    //----------------------------------//

    for (int cti = 0; cti < CT_WIDTH; cti++)
    {
        int cti_idx;
        cti_index.GetNearestIndexClipped(cti, &cti_idx);
        filter_cti_idx[cti] = (unsigned char)cti_idx;
    }

    //--------------------//
    // open the eval file //
    //--------------------//

    FILE* eval_ofp = NULL;
    if (eval_file)
    {
        eval_ofp = fopen(eval_file, "w");
        if (eval_ofp == NULL)
        {
            fprintf(stderr, "%s: error opening eval file %s\n", command,
                eval_file);
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
            first_speed[cti][ati] = wvp1->spd;

            //------------------------------------//
            // calculate the actual probabilities //
            //------------------------------------//

            int ff_idx[2];
            float ff_coef[2];
            first_index.GetLinearCoefsClipped(first_obj_prob[cti][ati],
                ff_idx, ff_coef);

            int fc_idx[2];
            float fc_coef[2];
            cti_index.GetLinearCoefsClipped(cti, fc_idx, fc_coef);

            int fs_idx[2];
            float fs_coef[2];
            speed_index.GetLinearCoefsClipped(first_speed[cti][ati], fs_idx,
                fs_coef);

            double good_sum = 0.0;
            double total_sum = 0.0;
            for (int i = 0; i < 2; i++)
            {
                int ff_i = ff_idx[i];
                float ff_c = ff_coef[i];
                for (int j = 0; j < 2; j++)
                {
                    int fc_i = fc_idx[j];
                    float fc_c = fc_coef[j];
                    for (int k = 0; k < 2; k++)
                    {
                        int fs_i = fs_idx[k];
                        float fs_c = fs_coef[k];

                        double factor = ff_c * fc_c * fs_c;
                        good_sum += factor *
                            first_good_array[ff_i][fc_i][fs_i];
                        total_sum += factor *
                            first_count_array[ff_i][fc_i][fs_i];
                    }
                }
            }
            if (total_sum == 0.0)
            {
                // no samples
                first_actual_prob[cti][ati] = 0.25;
            }
            else
            {
                // samples
                first_actual_prob[cti][ati] = (float)(good_sum / total_sum);
                first_actual_prob[cti][ati] -= sqrt(0.25 / (double)total_sum);
            }
        }
    }

    //----------------------------------------//
    // create arrays for median filter passes //
    //----------------------------------------//

    WindVectorPlus*** filter_selection =
        (WindVectorPlus***)make_array(sizeof(WindVectorPlus*), 2, CT_WIDTH,
        AT_WIDTH);

/*
    char** change = (char**)make_array(sizeof(char), 2, CT_WIDTH, AT_WIDTH);
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            filter_selection[cti][ati] = NULL;
            change[cti][ati] = 0;
        }
    }
*/

    //----------------//
    // loop till done //
    //----------------//

    int half_window = WINDOW_SIZE / 2;
    char filename[1024];
    int loop_idx = 0;
    int first_count = 0;
    int filter_count = 0;
    int did_good = 0;
    int did_corrected = 0;
    int did = 0;
    int corrected_enabled_count = 0;
    float lowest_first = 2.0;
    float highest_first = 0.0;
    float lowest_filter = 2.0;
    float highest_filter = 0.0;
    do
    {
      //----------------------------------------------------------------//
      // find the WVC with the highest probability of being set "right" //
      //----------------------------------------------------------------//

      float best_prob = WORST_PROB;
      ChoiceE best_choice = NO_CHOICE;
      int best_first_idx = 0;
      int bn_idx = 0;
      int bdr_idx = 0;
      int bs_idx = 0;
      int bc_idx = 0;
      int bp_idx = 0;
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

          float first_prob = first_actual_prob[cti][ati];

          //---------------------------------//
          // see if the first ranked is best //
          //---------------------------------//
          // only use if to initialize

          if (wvc->selected == NULL && first_prob > best_prob)
          {
            best_prob = first_prob;
            first_index.GetNearestIndexClipped(first_obj_prob[cti][ati],
              &best_first_idx);
            best_choice = FIRST;
            best_ati = ati;
            best_cti = cti;
          }

          //----------------------------------//
          // calculate the filter probability //
          //----------------------------------//

          float filter_prob = 0.0;    // default for no neighbors
          if (has_neighbors[cti][ati])
          {
            int neighbor_idx = filter_neighbor_idx[cti][ati];
            int dif_ratio_idx = filter_dif_ratio_idx[cti][ati];
            int speed_idx = filter_speed_idx[cti][ati];
            int cti_idx = filter_cti_idx[cti];
            int prob_idx = filter_prob_idx[cti][ati];

            double good_sum = (double)filter_good_array[neighbor_idx][dif_ratio_idx][speed_idx][cti_idx][prob_idx];
            double total_sum = (double)filter_count_array[neighbor_idx][dif_ratio_idx][speed_idx][cti_idx][prob_idx];

/*
            if (opt_interpolate)
            {
              int n_idx[2];
              float n_coef[2];
              neighbor_index.GetLinearCoefsClipped(neighbor_count[cti][ati],
                n_idx, n_coef);

              int dr_idx[2];
              float dr_coef[2];
              dif_ratio_index.GetLinearCoefsClipped(dif_ratio[cti][ati],
                dr_idx, dr_coef);

              int s_idx[2];
              float s_coef[2];
              speed_index.GetLinearCoefsClipped(speed[cti][ati], s_idx,
                s_coef);

              int c_idx[2];
              float c_coef[2];
              cti_index.GetLinearCoefsClipped(cti, c_idx, c_coef);

              int p_idx[2];
              float p_coef[2];
              prob_index.GetLinearCoefsClipped(prob[cti][ati], p_idx, p_coef);

              for (int i = 0; i < 2; i++)
              {
                int n_i = n_idx[i];
                float n_c = n_coef[i];
                for (int j = 0; j < 2; j++)
                {
                  int dr_i = dr_idx[j];
                  float dr_c = dr_coef[j];
                  for (int k = 0; k < 2; k++)
                  {
                    int s_i = s_idx[k];
                    float s_c = s_coef[k];
                    for (int l = 0; l < 2; l++)
                    {
                      int c_i = c_idx[l];
                      float c_c = c_coef[l];
                      for (int m = 0; m < 2; m++)
                      {
                        int p_i = p_idx[m];
                        float p_c = p_coef[m];
                        double factor = n_c * dr_c * s_c * c_c * p_c;
                        good_sum += factor *
                          (double)filter_good_array[n_i][dr_i][s_i][c_i][p_i];
                        total_sum += factor *
                          (double)filter_count_array[n_i][dr_i][s_i][c_i][p_i];
                      }
                    }
                  }
                }
              }
            }
*/

            //-------------------------------//
            // adjust the filter probability //
            //-------------------------------//

            filter_prob = (float)(good_sum / total_sum);
            if (opt_matic)
            {
              float good_fraction = 1.0;

              if (did > 0)
                good_fraction = (float)did_good / (float)did;

              if (good_fraction > matic_fraction)
              {
                // doing well, don't correct
                if (opt_correct)
                {
                  printf("  Correcting: disabled       ");
                  fflush(stdout);
                  printf("\r");
                  opt_correct = 0;
                  opt_jack = 0;
                  opt_random = 0;
                }
              }
              else
              {
                // doing poorly, correct
                if (! opt_correct)
                {
                  corrected_enabled_count++;
                  printf("  Correcting: enabled (%d) ",
                    corrected_enabled_count);
                  fflush(stdout);
                  printf("\r");
                  opt_correct = 1;
                  opt_jack = 1;
                  opt_random = 1;
                }
              }
            }

            if (opt_jack)
            {
              if (total_sum == 0.0)
              {
                // no samples
                if (opt_random)
                  filter_prob = drand48();
                else
                  filter_prob = 0.0;
              }
              else
              {
                // samples
                filter_prob += sqrt(0.25 / (double)filter_count_array[neighbor_idx][dif_ratio_idx][speed_idx][cti_idx][prob_idx]);
              }
            }
            else
            {
              filter_prob -= sqrt(0.25 / (double)filter_count_array[neighbor_idx][dif_ratio_idx][speed_idx][cti_idx][prob_idx]);
            }

            //---------------------------//
            // see if the filter is best //
            //---------------------------//
            // 1) filter is best
            // 2) filter is better than the first ranked
            // 3) filter does something useful
            // 4) if correcting, filter is pure propagation

            if (filter_prob > best_prob &&
              filter_prob > first_prob &&
              wvc->selected != filter_selection[cti][ati] &&
              (! opt_correct || wvc->selected == NULL))
            {
              best_prob = filter_prob;
              bn_idx = neighbor_idx;
              bdr_idx = dif_ratio_idx;
              bs_idx = speed_idx;
              bc_idx = cti_idx;
              bp_idx = prob_idx;
              best_choice = FILTER;
              best_ati = ati;
              best_cti = cti;
            }
          }
        }
      }
      if (best_ati == -1 && best_cti == -1)
      {
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
          if (best_prob > highest_first)
            highest_first = best_prob;
          if (best_prob < lowest_first)
            lowest_first = best_prob;

          wvc->selected = wvc->ambiguities.GetByIndex(0);

//          change[best_cti][best_ati] = 1;
          first_count++;
          break;

        case FILTER:
          if (best_prob > highest_filter)
            highest_filter = best_prob;
          if (best_prob < lowest_filter)
            lowest_filter = best_prob;

          if (filter_selection[best_cti][best_ati] == NULL)
          {
            fprintf(stderr, "%s: no filter selection\n", command);
            exit(1);
          }
          wvc->selected = filter_selection[best_cti][best_ati];

          if (opt_adapt)
          {
            if (filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] >= MAX_SHORT)
            {
              printf("chopping %d %d\n",
                filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx],
                filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx]);
              double new_count =
                (double)filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] * 15.0 / 16.0;
              double new_good =
                (double)filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] * 15.0 / 16.0;
              filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] =
                (unsigned short)(new_count + 0.5);
              filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] =
                (unsigned short)(new_good + 0.5);
              if (filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] >= filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx])
              {
                filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx] =
                  filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx];
              }
            }

            if (wvc->selected == original_selected[best_cti][best_ati])
            {
              filter_good_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx]++;
            }
            filter_count_array[bn_idx][bdr_idx][bs_idx][bc_idx][bp_idx]++;
          }
          if (eval_ofp)
          {
            if (wvc->selected == original_selected[best_cti][best_ati])
            {
              write_eval(eval_ofp, bn_idx, bdr_idx, bs_idx, bc_idx, bp_idx, 1);
            }
            else
            {
              write_eval(eval_ofp, bn_idx, bdr_idx, bs_idx, bc_idx, bp_idx, 0);
            }
          }
//          change[best_cti][best_ati] = 1;
          filter_count++;
          break;

        default:
          fprintf(stderr, "%s: some kind of weird error\n", command);
          exit(1);
          break;
      }

      //--------------------//
      // correct if desired //
      //--------------------//

      if (opt_correct &&
        wvc->selected != original_selected[best_cti][best_ati])
      {
        did_corrected++;
        wvc->selected = original_selected[best_cti][best_ati];
      }

      //----------//
      // evaluate //
      //----------//
      // note: if correction is on, you always do good

      did++;
      if (wvc->selected == original_selected[best_cti][best_ati])
        did_good++;

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
            }
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
              min_vector_dif_avg = vector_dif_sum / (float)selected_count;
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

          int neighbor_idx;
          neighbor_index.GetNearestIndexClipped(selected_count,
            &neighbor_idx);
          filter_neighbor_idx[cti][ati] = (unsigned char)neighbor_idx;

          int dif_ratio_idx;
          dif_ratio_index.GetNearestIndexClipped(best_to_second_ratio,
            &dif_ratio_idx);
          filter_dif_ratio_idx[cti][ati] = (unsigned char)dif_ratio_idx;

          int speed_idx;
          speed_index.GetNearestIndexClipped(new_selected->spd,
            &speed_idx);
          filter_speed_idx[cti][ati] = (unsigned char)speed_idx;

          int prob_idx;
          prob_index.GetNearestIndexClipped(new_selected->obj,
            &prob_idx);
          filter_prob_idx[cti][ati] = (unsigned char)prob_idx;

          filter_selection[cti][ati] = new_selected;
          has_neighbors[cti][ati] = 1;
        }
      }
      if (opt_dump && loop_idx % dump_loops == 0 && loop_idx != 0)
      {
        int total_count = first_count + filter_count;
        float first_percent = 100.0 * (float)first_count /
          (float)total_count;
        float filter_percent = 100.0 * (float)filter_count /
          (float)total_count;
        printf("%d  1st:%.2f%%  Fil:%.2f%%\n", loop_idx, first_percent,
          filter_percent);
        first_count = 0;
        filter_count = 0;

        printf("  Corrected percent : %.2f%%\n",
          100.0 * (float)did_corrected / (float)did);

        printf("  1st: %g to %g   Fil: %g to %g\n", lowest_first,
          highest_first, lowest_filter, highest_filter);
        lowest_first = 2.0;
        highest_first = 0.0;
        lowest_filter = 2.0;
        highest_filter = 0.0;

        sprintf(filename, "%s.%06d", vctr_base, loop_idx);
        l2b.WriteVctr(filename, 0);

        //-----------------//
        // write prob file //
        //-----------------//

        if (opt_adapt)
        {
          FILE* ofp = fopen(prob_file, "w");
          if (ofp == NULL)
          {
            fprintf(stderr, "%s: error opening prob file %s\n", command,
              prob_file);
            exit(1);
          }
          else
          {
            fwrite(first_count_array, sizeof(unsigned short),
              first_nitems, ofp);
            fwrite(first_good_array, sizeof(unsigned short),
              first_nitems, ofp);
            fwrite(filter_count_array, sizeof(unsigned short),
              filter_nitems, ofp);
            fwrite(filter_good_array, sizeof(unsigned short),
            filter_nitems, ofp);
            fclose(ofp);
          }
        }

        if (! opt_correct || opt_matic)
        {
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
          printf("  Match = %.2f %%\n", 100.0 * (float)match_count /
            (float)comp_total_count);
        }
      }
      loop_idx++;
    } while (1);

    //----------------//
    // spit some info //
    //----------------//

    printf("  Corrected percent : %.2f%%\n", 100.0 * (float)did_corrected /
        (float)did);

    //-------------------------//
    // write final vector file //
    //-------------------------//

    sprintf(filename, "%s.done", vctr_base);
    l2b.WriteVctr(filename, 0);

    free_array(filter_selection, 2, CT_WIDTH, AT_WIDTH);
//    free_array(change, 2, CT_WIDTH, AT_WIDTH);

    //-----------------//
    // write prob file //
    //-----------------//

    if (opt_adapt)
    {
        FILE* ofp = fopen(prob_file, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening prob file %s\n", command,
                prob_file);
            exit(1);
        }
        else
        {
            fwrite(first_count_array, sizeof(unsigned short), first_nitems,
                ofp);
            fwrite(first_good_array, sizeof(unsigned short), first_nitems,
                ofp);
            fwrite(filter_count_array, sizeof(unsigned short), filter_nitems,
              ofp);
            fwrite(filter_good_array, sizeof(unsigned short), filter_nitems,
              ofp);
            fclose(ofp);
        }
    }

    //---------------------//
    // final median filter //
    //---------------------//

    swath->MedianFilter(MEDIAN_FILTER_WINDOW_SIZE, 200, 0, 0, 0);

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

    if (eval_ofp)
        fclose(eval_ofp);
    l2b.Close();
    free_array((void*)spd, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)dir, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)num_ambigs, 2, atibins, ctibins);
    free_array((void*)sel_idx, 2, atibins, ctibins);

    return (0);
}

int
write_eval(
    FILE*  ofp,
    int    bn_idx,
    int    bdr_idx,
    int    bs_idx,
    int    bc_idx,
    int    bp_idx,
    int    good)
{
    unsigned char array[6];
    array[0] = (unsigned char)bn_idx;
    array[1] = (unsigned char)bdr_idx;
    array[2] = (unsigned char)bs_idx;
    array[3] = (unsigned char)bc_idx;
    array[4] = (unsigned char)bp_idx;
    array[5] = (unsigned char)good;
    if (fwrite(array, sizeof(unsigned char), 6, ofp) != 6)
        return(0);
    return(1);
}
