//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    see_probs
//
// SYNOPSIS
//    see_probs [ -r ] [ -p # ] <prob_file>
//
// DESCRIPTION
//    This program generates slices through the probability table.
//    It also reports the total number of samples and allows the
//    user to put a cap on the total number of samples per bin.
//    A bunch of output files are generated.
//
// OPTIONS
//    The following options are supported:
//      [ -r ]    Show raw probabilities (don't sample subtract)
//      [ -p # ]  Show # best picks
//
// OPERANDS
//    The following operands are supported:
//      <prob_file>        The probability count file.
//
// EXAMPLES
//    An example of a command line is:
//    % see_probs adapt.prob
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
#include <string.h>
#include "Misc.h"
#include "Index.h"
#include "mudh.h"
#include "best.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "rp:"

#define QUOTE      '"'

#define MIN_SAMPLES      100
#define HISTO_MAX        200
#define NUMBER_OF_PICKS  20

#define HISTO_SAMPLES    100

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

const char* usage_array[] = { "[ -r ]", "[ -p # ]", "<prob_file>", 0 };

unsigned short  first_count_array[FIRST_PROB_INDICIES][FIRST_REL_DIR_INDICIES][CTI_INDICIES][SPEED_INDICIES];
unsigned short  first_good_array[FIRST_PROB_INDICIES][FIRST_REL_DIR_INDICIES][CTI_INDICIES][SPEED_INDICIES];

unsigned short  filter_count_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];
unsigned short  filter_good_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];

int opt_raw = 0;
int opt_picks = NUMBER_OF_PICKS;

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    char raw_string[2] = "";

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            opt_raw = 1;
            strcpy(raw_string, ".r");
            break;
        case 'p':
            opt_picks = atoi(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 1)
        usage(command, usage_array, 1);

    const char* prob_file = argv[optind++];

    //-------------------//
    // read in prob file //
    //-------------------//

    FILE* ifp = fopen(prob_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            prob_file);
        exit(1);
    }
    else
    {
        fread(first_count_array, sizeof(unsigned short),
            FIRST_PROB_INDICIES*FIRST_REL_DIR_INDICIES*CTI_INDICIES*
            SPEED_INDICIES, ifp);
        fread(first_good_array, sizeof(unsigned short),
            FIRST_PROB_INDICIES*FIRST_REL_DIR_INDICIES*CTI_INDICIES*
            SPEED_INDICIES, ifp);
        fread(filter_count_array, sizeof(unsigned short),
            NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
            PROB_INDICIES, ifp);
        fread(filter_good_array, sizeof(unsigned short),
            NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
            PROB_INDICIES, ifp);
        fclose(ifp);
    }

    //-----------------------------------//
    // initialize some index calculators //
    //-----------------------------------//

    Index first_index, rel_dir_index, neighbor_index, dif_ratio_index,
        speed_index, cti_index, prob_index;

    first_index.SpecifyCenters(FIRST_PROB_MIN_VALUE, FIRST_PROB_MAX_VALUE,
        FIRST_PROB_INDICIES);
    rel_dir_index.SpecifyCenters(FIRST_REL_DIR_MIN_VALUE,
        FIRST_REL_DIR_MAX_VALUE, FIRST_REL_DIR_INDICIES);
    neighbor_index.SpecifyCenters(NEIGHBOR_MIN_VALUE, NEIGHBOR_MAX_VALUE,
        NEIGHBOR_INDICIES);
    dif_ratio_index.SpecifyCenters(DIF_RATIO_MIN_VALUE, DIF_RATIO_MAX_VALUE,
        DIF_RATIO_INDICIES);
    speed_index.SpecifyCenters(SPEED_MIN_VALUE, SPEED_MAX_VALUE,
        SPEED_INDICIES);
    cti_index.SpecifyCenters(CTI_MIN_VALUE, CTI_MAX_VALUE, CTI_INDICIES);
    prob_index.SpecifyCenters(PROB_MIN_VALUE, PROB_MAX_VALUE, PROB_INDICIES);

    //---------------//
    // count samples //
    //---------------//

    unsigned long sample_count = 0;
    unsigned long most_samples = 0;
    for (int i = 0; i < FIRST_PROB_INDICIES; i++)
    {
      for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
      {
        for (int k = 0; k < CTI_INDICIES; k++)
        {
            for (int l = 0; l < SPEED_INDICIES; l++)
            {
                sample_count += first_count_array[i][j][k][l];
                if (first_count_array[i][j][k][l] > most_samples)
                    most_samples = first_count_array[i][j][k][l];
            }
        }
      }
    }
    printf("First Ranked\n");
    printf("  Total : %ld\n", sample_count);
    printf("   Most : %ld\n", most_samples);

    char filename[1024];
    sprintf(filename, "%s%s.samp", prob_file, raw_string);
	FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    unsigned long sample_array[HISTO_MAX];
    for (int i = 0; i < HISTO_MAX; i++)
        sample_array[i] = 0;
    sample_count = 0;
    most_samples = 0;
    for (int i = 0; i < NEIGHBOR_INDICIES; i++)
    {
      for (int j = 0; j < DIF_RATIO_INDICIES; j++)
      {
        for (int k = 0; k < SPEED_INDICIES; k++)
        {
          for (int l = 0; l < CTI_INDICIES; l++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              sample_count += filter_count_array[i][j][k][l][m];
              if (filter_count_array[i][j][k][l][m] > most_samples)
                most_samples = filter_count_array[i][j][k][l][m];
              int idx = filter_count_array[i][j][k][l][m];
              if (idx >= HISTO_MAX)
                idx = HISTO_MAX - 1;
              sample_array[idx]++;
            }
          }
        }
      }
    }
    for (int i = 0; i < HISTO_MAX; i++)
    {
        fprintf(ofp, "%d %ld\n", i, sample_array[i]);
    }
    fclose(ofp);

    printf("\n");
    printf("Filter\n");
    printf("  Total : %ld\n", sample_count);
    printf("   Most : %ld\n", most_samples);

    //------------//
    // first rank //
    //------------//

    sprintf(filename, "%s%s.1p", prob_file, raw_string);
	ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / MLE Probability", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "MLE Probability", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int i = 0; i < FIRST_PROB_INDICIES; i++)
    {
        unsigned long total_count = 0;
        unsigned long good_count = 0;
        for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
        {
          for (int k = 0; k < CTI_INDICIES; k++)
          {
            for (int l = 0; l < SPEED_INDICIES; l++)
            {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
            }
          }
        }
        float fprob;
        first_index.IndexToValue(i, &fprob);
        if (total_count < MIN_SAMPLES)
            continue;
        float prob = (float)good_count / (float)total_count;
        fprintf(ofp, "%g %g %ld\n", fprob, prob, total_count);
    }
    fclose(ofp);

    //--------------------//
    // relative direction //
    //--------------------//

    sprintf(filename, "%s%s.1rel", prob_file, raw_string);
	ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / Cross-swath Relative Direction", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "Cross-swath Relative Direction (deg)", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
    {
        unsigned long total_count = 0;
        unsigned long good_count = 0;
        for (int i = 0; i < FIRST_PROB_INDICIES; i++)
        {
          for (int k = 0; k < CTI_INDICIES; k++)
          {
            for (int l = 0; l < SPEED_INDICIES; l++)
            {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
            }
          }
        }
        float rel_dir;
        rel_dir_index.IndexToValue(j, &rel_dir);
        if (total_count < MIN_SAMPLES)
            continue;
        double prob = (double)good_count / (double)total_count;
        fprintf(ofp, "%g %g %ld\n", rel_dir, prob, total_count);
    }
    fclose(ofp);

    //-------------------------//
    // relative direction cuts //
    //-------------------------//

    sprintf(filename, "%s%s.1rels", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / Cross-swath Relative Direction (by MLE probability)",
        QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "Cross-swath Relative Direction (deg)", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    fprintf(ofp, "@ legend on\n");
    int leg_idx = 0;

    for (int i = 0; i < FIRST_PROB_INDICIES; i += 5)
    {
        float first_prob;
        first_index.IndexToValue(i, &first_prob);
        fprintf(ofp, "@ legend string %d %c%g%c\n", leg_idx++, QUOTE,
            first_prob, QUOTE);
        for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
        {
            float rel_dir;
            rel_dir_index.IndexToValue(j, &rel_dir);
            unsigned long total_count = 0;
            unsigned long good_count = 0;
            for (int k = 0; k < CTI_INDICIES; k++)
            {
              for (int l = 0; l < SPEED_INDICIES; l++)
              {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
              }
            }
            if (total_count < MIN_SAMPLES)
                continue;
            double prob = (double)good_count / (double)total_count;
            fprintf(ofp, "%g %g %ld\n", rel_dir, prob, total_count);
        }
        fprintf(ofp, "&\n");
    }
    fclose(ofp);

    //-------------//
    // cross track //
    //-------------//

    sprintf(filename, "%s%s.1cti", prob_file, raw_string);
	ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / Cross Track Index", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, "Cross Track Index", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int cti = 0; cti < 2 * CTI_INDICIES; cti++)
    {
        int k = cti;
        if (k > CTI_FOLD_MAX)
            k = CTI_FOLDER - k;
        unsigned long total_count = 0;
        unsigned long good_count = 0;
        for (int i = 0; i < FIRST_PROB_INDICIES; i++)
        {
          for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
          {
            for (int l = 0; l < SPEED_INDICIES; l++)
            {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
            }
          }
        }
/*
        float cti;
        cti_index.IndexToValue(use_cti, &cti);
*/
        if (total_count < MIN_SAMPLES)
            continue;
        double prob = (double)good_count / (double)total_count;
        fprintf(ofp, "%d %g %ld\n", cti, prob, total_count);
    }
    fclose(ofp);

    //--------------------//
    // first ranked speed //
    //--------------------//

    sprintf(filename, "%s%s.1spd", prob_file, raw_string);
	ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / First Ranked Speed", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "First Ranked Speed (m/s)", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int l = 0; l < SPEED_INDICIES; l++)
    {
        unsigned long total_count = 0;
        unsigned long good_count = 0;
        for (int i = 0; i < FIRST_PROB_INDICIES; i++)
        {
          for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
          {
            for (int k = 0; k < CTI_INDICIES; k++)
            {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
            }
          }
        }
        float speed;
        speed_index.IndexToValue(l, &speed);
        if (total_count < MIN_SAMPLES)
            continue;
        double prob = (double)good_count / (double)total_count;
        fprintf(ofp, "%g %g %ld\n", speed, prob, total_count);
    }
    fclose(ofp);

    //------------------//
    // first speed cuts //
    //------------------//

    sprintf(filename, "%s%s.1spds", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "First Ranked / MLE Probability (by speed)", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "MLE Probability", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    fprintf(ofp, "@ legend on\n");
    for (int l = 0; l < SPEED_INDICIES; l++)
    {
        float speed;
        speed_index.IndexToValue(l, &speed);
        fprintf(ofp, "@ legend string %d %c%g m/s%c\n", l, QUOTE, speed,
            QUOTE);
        for (int i = 0; i < FIRST_PROB_INDICIES; i++)
        {
            float first;
            first_index.IndexToValue(i, &first);
            unsigned long total_count = 0;
            unsigned long good_count = 0;
            for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
            {
              for (int k = 0; k < CTI_INDICIES; k++)
              {
                total_count += first_count_array[i][j][k][l];
                good_count += first_good_array[i][j][k][l];
              }
            }
            if (total_count < MIN_SAMPLES)
                continue;
            double prob = (double)good_count / (double)total_count;
            fprintf(ofp, "%g %g %ld\n", first, prob, total_count);
        }
        fprintf(ofp, "&\n");
    }
    fclose(ofp);

    //-----------//
    // neighbors //
    //-----------//

    sprintf(filename, "%s%s.n", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "Filter / Neighbors", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "Neighbors", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int i = 0; i < NEIGHBOR_INDICIES; i++)
    {
      unsigned long total_count = 0;
      unsigned long good_count = 0;
      for (int j = 0; j < DIF_RATIO_INDICIES; j++)
      {
        for (int k = 0; k < SPEED_INDICIES; k++)
        {
          for (int l = 0; l < CTI_INDICIES; l++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              total_count += filter_count_array[i][j][k][l][m];
              good_count += filter_good_array[i][j][k][l][m];
            }
          }
        }
      }
      if (total_count < MIN_SAMPLES)
          continue;
      float neighbors;
      neighbor_index.IndexToValue(i, &neighbors);
      double prob = (double)good_count / (double)total_count;
      fprintf(ofp, "%g %g %ld\n", neighbors, prob, total_count);
    }
    fclose(ofp);

    //-----------//
    // dif ratio //
    //-----------//

    sprintf(filename, "%s%s.dr", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "Filter / Difference Ratio", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "Difference Ratio", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int j = 0; j < DIF_RATIO_INDICIES; j++)
    {
      unsigned long total_count = 0;
      unsigned long good_count = 0;
      for (int i = 0; i < NEIGHBOR_INDICIES; i++)
      {
        for (int k = 0; k < SPEED_INDICIES; k++)
        {
          for (int l = 0; l < CTI_INDICIES; l++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              total_count += filter_count_array[i][j][k][l][m];
              good_count += filter_good_array[i][j][k][l][m];
            }
          }
        }
      }
      if (total_count < MIN_SAMPLES)
          continue;
      float dif_ratio;
      dif_ratio_index.IndexToValue(j, &dif_ratio);
      double prob = (double)good_count / (double)total_count;
      fprintf(ofp, "%g %g %ld\n", dif_ratio, prob, total_count);
    }
    fclose(ofp);

    //-------//
    // speed //
    //-------//

    sprintf(filename, "%s%s.spd", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "Filter / Speed", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "First Ranked Speed (m/s)", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int k = 0; k < SPEED_INDICIES; k++)
    {
      unsigned long total_count = 0;
      unsigned long good_count = 0;
      for (int i = 0; i < NEIGHBOR_INDICIES; i++)
      {
        for (int j = 0; j < DIF_RATIO_INDICIES; j++)
        {
          for (int l = 0; l < CTI_INDICIES; l++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              total_count += filter_count_array[i][j][k][l][m];
              good_count += filter_good_array[i][j][k][l][m];
            }
          }
        }
      }
      if (total_count < MIN_SAMPLES)
          continue;
      float speed;
      speed_index.IndexToValue(k, &speed);
      double prob = (double)good_count / (double)total_count;
      fprintf(ofp, "%g %g %ld\n", speed, prob, total_count);
    }
    fclose(ofp);

    //-----//
    // cti //
    //-----//

    sprintf(filename, "%s%s.cti", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "Filter / Cross Track Index", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "Cross Track Index", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int cti = 0; cti < CTI_INDICIES * 2; cti++)
    {
      int l = cti;
      if (l > CTI_FOLD_MAX)
        l = CTI_FOLDER - l;
      unsigned long total_count = 0;
      unsigned long good_count = 0;
      for (int i = 0; i < NEIGHBOR_INDICIES; i++)
      {
        for (int j = 0; j < DIF_RATIO_INDICIES; j++)
        {
          for (int k = 0; k < SPEED_INDICIES; k++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              total_count += filter_count_array[i][j][k][l][m];
              good_count += filter_good_array[i][j][k][l][m];
            }
          }
        }
      }
      if (total_count < MIN_SAMPLES)
          continue;
/*
      float cti;
      cti_index.IndexToValue(use_cti, &cti);
*/
      double prob = (double)good_count / (double)total_count;
      fprintf(ofp, "%d %g %ld\n", cti, prob, total_count);
    }
    fclose(ofp);

    //------//
    // prob //
    //------//

    sprintf(filename, "%s%s.prob", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(ofp, "@ title %c%s%c\n", QUOTE,
        "Filter / MLE Probability", QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE,
        "MLE Probability", QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, "Probability Selected",
        QUOTE);
    for (int m = 0; m < PROB_INDICIES; m++)
    {
      unsigned long total_count = 0;
      unsigned long good_count = 0;
      for (int i = 0; i < NEIGHBOR_INDICIES; i++)
      {
        for (int j = 0; j < DIF_RATIO_INDICIES; j++)
        {
          for (int k = 0; k < SPEED_INDICIES; k++)
          {
            for (int l = 0; l < CTI_INDICIES; l++)
            {
              total_count += filter_count_array[i][j][k][l][m];
              good_count += filter_good_array[i][j][k][l][m];
            }
          }
        }
      }
      if (total_count < MIN_SAMPLES)
          continue;
      float sprob;
      prob_index.IndexToValue(m, &sprob);
      double prob = (double)good_count / (double)total_count;
      fprintf(ofp, "%g %g %ld\n", sprob, prob, total_count);
    }
    fclose(ofp);

    //------------------------//
    // probability histograms //
    //------------------------//

    unsigned long prob_histo[HISTO_SAMPLES+1];
    for (int i = 0; i < HISTO_SAMPLES+1; i++)
        prob_histo[i] = 0;

    sprintf(filename, "%s%s.histo", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    unsigned long total_count = 0;
    for (int i = 0; i < FIRST_PROB_INDICIES; i++)
    {
      for (int j = 0; j < FIRST_REL_DIR_INDICIES; j++)
      {
        for (int k = 0; k < CTI_INDICIES; k++)
        {
          for (int l = 0; l < SPEED_INDICIES; l++)
          {
            double prob = (double)first_good_array[i][j][k][l] /
              (double)first_count_array[i][j][k][l];
            if (! opt_raw)
            {
              prob -= sqrt(0.25 / (double)first_count_array[i][j][k][l]);
            }
            int prob_idx = (int)(prob * HISTO_SAMPLES + 0.5);
            if (prob_idx < 0 || prob_idx > HISTO_SAMPLES)
              continue;
            prob_histo[prob_idx] += first_count_array[i][j][k][l];
            total_count += first_count_array[i][j][k][l];
          }
        }
      }
    }
    unsigned long cum_count = 0;
    for (int i = 0; i < HISTO_SAMPLES+1; i++)
    {
      double prob = (double)i / (double)HISTO_SAMPLES;
      cum_count += prob_histo[i];
      fprintf(ofp, "%g %g %g\n", prob, (double)prob_histo[i] /
        (double)total_count, (double)cum_count / (double)total_count);
    }

    fprintf(ofp, "&\n");

    for (int i = 0; i < HISTO_SAMPLES+1; i++)
        prob_histo[i] = 0;

    total_count = 0;
    for (int i = 0; i < NEIGHBOR_INDICIES; i++)
    {
      for (int j = 0; j < DIF_RATIO_INDICIES; j++)
      {
        for (int k = 0; k < SPEED_INDICIES; k++)
        {
          for (int l = 0; l < CTI_INDICIES; l++)
          {
            for (int m = 0; m < PROB_INDICIES; m++)
            {
              double prob =
                (double)filter_good_array[i][j][k][l][m] /
                (double)filter_count_array[i][j][k][l][m];
              if (! opt_raw)
              {
                prob -= sqrt(0.25 /
                  (double)filter_count_array[i][j][k][l][m]);
              }
              int prob_idx = (int)(prob * HISTO_SAMPLES + 0.5);
              if (prob_idx < 0 || prob_idx > HISTO_SAMPLES)
                continue;
              prob_histo[prob_idx] += first_count_array[i][j][k][l];
              total_count += first_count_array[i][j][k][l];
            }
          }
        }
      }
    }
    cum_count = 0;
    for (int i = 0; i < HISTO_SAMPLES+1; i++)
    {
      cum_count += prob_histo[i];
      double prob = (double)i / (double)HISTO_SAMPLES;
      fprintf(ofp, "%g %g %g\n", prob, (double)prob_histo[i] /
        (double)total_count, (double)cum_count / (double)total_count);
    }
    fclose(ofp);

    //-----------//
    // top picks //
    //-----------//

    sprintf(filename, "%s%s.pick", prob_file, raw_string);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    for (int pick_idx = 0; pick_idx < opt_picks; pick_idx++)
    {
      double max_filter_prob = 0.0;
      int max_i = 0;
      int max_j = 0;
      int max_k = 0;
      int max_l = 0;
      int max_m = 0;
      for (int i = 0; i < NEIGHBOR_INDICIES; i++)
      {
        for (int j = 0; j < DIF_RATIO_INDICIES; j++)
        {
          for (int k = 0; k < SPEED_INDICIES; k++)
          {
            for (int l = 0; l < CTI_INDICIES; l++)
            {
              for (int m = 0; m < PROB_INDICIES; m++)
              {
                if (filter_count_array[i][j][k][l][m] < 1)
                  continue;
                double filter_prob =
                    (double)filter_good_array[i][j][k][l][m] /
                    (double)filter_count_array[i][j][k][l][m];
                if (! opt_raw)
                {
                    filter_prob -= sqrt(0.25 /
                        (double)filter_count_array[i][j][k][l][m]);
                }
                if (filter_prob > max_filter_prob)
                {
                    max_filter_prob = filter_prob;
                    max_i = i;
                    max_j = j;
                    max_k = k;
                    max_l = l;
                    max_m = m;
                }
              }
            }
          }
        }
      }
      float neighbors;
      neighbor_index.IndexToValue(max_i, &neighbors);
      float dif_ratio;
      dif_ratio_index.IndexToValue(max_j, &dif_ratio);
      float speed;
      speed_index.IndexToValue(max_k, &speed);
      int use_cti = max_l;
      if (use_cti > CTI_FOLD_MAX)
        use_cti = CTI_FOLDER - use_cti;
      float cti;
      cti_index.IndexToValue(use_cti, &cti);
      float prob;
      prob_index.IndexToValue(max_m, &prob);
      fprintf(ofp, "Pick %d\n", pick_idx + 1);
      fprintf(ofp, "  Neighbors %g\n", neighbors);
      fprintf(ofp, "  Dif Ratio %g\n", dif_ratio);
      fprintf(ofp, "      Speed %g\n", speed);
      fprintf(ofp, "        CTI %g\n", cti);
      fprintf(ofp, "       Prob %g\n", prob);
      fprintf(ofp, "  %d / %d -> %g\n",
          filter_good_array[max_i][max_j][max_k][max_l][max_m],
          filter_count_array[max_i][max_j][max_k][max_l][max_m],
          max_filter_prob);
      fprintf(ofp, "\n");
      filter_good_array[max_i][max_j][max_k][max_l][max_m] = 0;
    }
    fclose(ofp);

    return (0);
}
