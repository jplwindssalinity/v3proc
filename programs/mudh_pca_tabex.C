//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_tabex
//
// SYNOPSIS
//    mudh_pca_tabex [ -n min_samples ] <input_pcatab> <output_pcatab>
//
// DESCRIPTION
//    Fill in missing pcatab elements by expanding an averaging
//    window until the minimum number of samples are available.
//
// OPTIONS
//    [ -n min_samples ]   The minimum number of samples.
//
// OPERANDS
//    <input_pcatab>   Input pcatab.
//    <output_pcatab>  Output pcatab.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_tabex -n 100 pcatab.1 pcatab.1ex
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
#include <stdlib.h>
#include "Misc.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "n:"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -n min_samples ]", "<input_pcatab>",
    "<output_pcatab>", 0 };

static double norain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double sample_count_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

static double norain_tab_2[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab_2[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double sample_count_2[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

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

    unsigned long min_samples = 50;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'n':
            min_samples = (unsigned long)atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* input_pcatab_file = argv[optind++];
    const char* output_pcatab_file = argv[optind++];

    //------------//
    // open files //
    //------------//

    FILE* pcatab_ifp = fopen(input_pcatab_file, "r");
    if (pcatab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening pcatab file %s\n", command,
            input_pcatab_file);
        exit(1);
    }
    unsigned int size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;

    FILE* pcatab_ofp = fopen(output_pcatab_file, "w");
    if (pcatab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output pcatab file %s\n",
            command, output_pcatab_file);
        exit(1);
    }

    //------------------//
    // read pcatab file //
    //------------------//

    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
      if (fread(norain_tab_1, sizeof(double), size, pcatab_ifp) != size ||
          fread(rain_tab_1, sizeof(double), size, pcatab_ifp) != size ||
          fread(sample_count_1, sizeof(double), size, pcatab_ifp) != size)
      {
        if (swath_idx == 1 && feof(pcatab_ifp))
            break;
        else
        {
          fprintf(stderr, "%s: error reading pcatab file %s\n", command,
              input_pcatab_file);
          exit(1);
        }
      }

      //------------------------//
      // do the expansion thing //
      //------------------------//

      int max_delta = 5;

      for (int center_i = 0; center_i < NBD_DIM; center_i++)
      {
        for (int center_j = 0; center_j < SPD_DIM; center_j++)
        {
          for (int center_k = 0; center_k < DIR_DIM; center_k++)
          {
            for (int center_l = 0; center_l < MLE_DIM; center_l++)
            {
              // already have enough samples (and 2.0 flag not set)
              if (sample_count_1[center_i][center_j][center_k][center_l] >=
                  min_samples &&
                  norain_tab_1[center_i][center_j][center_k][center_l] < 1.5 &&
                  rain_tab_1[center_i][center_j][center_k][center_l] < 1.5)
              {
                norain_tab_2[center_i][center_j][center_k][center_l] =
                  norain_tab_1[center_i][center_j][center_k][center_l];
                rain_tab_2[center_i][center_j][center_k][center_l] =
                  rain_tab_1[center_i][center_j][center_k][center_l];
                sample_count_2[center_i][center_j][center_k][center_l] =
                  sample_count_1[center_i][center_j][center_k][center_l];
                continue;
              }

              // need to expand window to get samples
              for (int delta = 0; delta < max_delta; delta++)
              {
                int start_i = center_i - delta;
                int end_i = center_i + delta + 1;
                if (start_i < 0) start_i = 0;
                if (end_i > NBD_DIM) end_i = NBD_DIM;

                int start_j = center_j - delta;
                int end_j = center_j + delta + 1;
                if (start_j < 0) start_j = 0;
                if (end_j > SPD_DIM) end_j = SPD_DIM;

                int start_k = center_k - delta;
                int end_k = center_k + delta + 1;
                if (start_k < 0) start_k = 0;
                if (end_k > DIR_DIM) end_k = DIR_DIM;

                int start_l = center_l - delta;
                int end_l = center_l + delta + 1;
                if (start_l < 0) start_l = 0;
                if (end_l > MLE_DIM) end_l = MLE_DIM;

                unsigned long window_count = 0;
                double nr_weighted_sum = 0.0;
                double r_weighted_sum = 0.0;
                for (int i = start_i; i < end_i; i++)
                {
                  for (int j = start_j; j < end_j; j++)
                  {
                    for (int k = start_k; k < end_k; k++)
                    {
                      for (int l = start_l; l < end_l; l++)
                      {
                        if (norain_tab_1[i][j][k][l] > 1.5 ||
                            rain_tab_1[i][j][k][l] > 1.5)
                        {
                          continue;
                        }
                        nr_weighted_sum += sample_count_1[i][j][k][l] * 
                            norain_tab_1[i][j][k][l];
                        r_weighted_sum += sample_count_1[i][j][k][l] * 
                            rain_tab_1[i][j][k][l];
                        window_count +=
                            (unsigned long)sample_count_1[i][j][k][l];
                      }
                    }
                  }
                }
                if (window_count >= min_samples)
                {
                  norain_tab_2[center_i][center_j][center_k][center_l] =
                    nr_weighted_sum / (double)window_count;
                  rain_tab_2[center_i][center_j][center_k][center_l] =
                    r_weighted_sum / (double)window_count;
                  sample_count_2[center_i][center_j][center_k][center_l] =
                    (double)window_count;
//printf("%d %d %d %d = %d\n", center_i, center_j, center_k, center_l, delta);
                  break;
                }
              }
// printf("%d %d %d %d %g\n", center_i, center_j, center_k, center_l, classtab_2[center_i][center_j][center_k][center_l]);
            }
          }
        }
      }

      //----------------------//
      // write the new pcatab //
      //----------------------//

      if (fwrite(norain_tab_2, sizeof(double), size, pcatab_ofp) != size ||
          fwrite(rain_tab_2, sizeof(double), size, pcatab_ofp) != size ||
          fwrite(sample_count_2, sizeof(double), size, pcatab_ofp) != size)
      {
          fprintf(stderr, "%s: error writing pcatab file %s\n", command,
              output_pcatab_file);
          exit(1);
      }
    }

    fclose(pcatab_ifp);
    fclose(pcatab_ofp);

    return (0);
}
