//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    see_probs
//
// SYNOPSIS
//    see_probs <prob_file>
//
// DESCRIPTION
//    This program generates slices through the probability table.
//    It also reports the total number of samples and allows the
//    user to put a cap on the total number of samples per bin.
//    A bunch of output files are generated.
//
// OPTIONS
//    The following options are supported:
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

#define OPTSTRING  "h"

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

const char* usage_array[] = { "<prob_file>", 0 };

float          first_obj_prob[CT_WIDTH][AT_WIDTH];
unsigned char  neighbor_count[CT_WIDTH][AT_WIDTH];
float          dif_ratio[CT_WIDTH][AT_WIDTH];
float          speed[CT_WIDTH][AT_WIDTH];

unsigned int  first_count_array[FIRST_INDICIES];
unsigned int  first_good_array[FIRST_INDICIES];

unsigned int filter_count_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];
unsigned int filter_good_array[NEIGHBOR_INDICIES][DIF_RATIO_INDICIES][SPEED_INDICIES][CTI_INDICIES][PROB_INDICIES];

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
        fread(first_count_array, sizeof(unsigned int),
            FIRST_INDICIES, ifp);
        fread(first_good_array, sizeof(unsigned int),
            FIRST_INDICIES, ifp);
        fread(filter_count_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
          PROB_INDICIES,
          ifp);
        fread(filter_good_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
          PROB_INDICIES,
          ifp);
        fclose(ifp);
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

    //---------------//
    // count samples //
    //---------------//

    unsigned long sample_count = 0;
    unsigned long most_samples = 0;
    for (int i = 0; i < FIRST_INDICIES; i++)
    {
        sample_count += first_count_array[i];
        if (first_count_array[i] > most_samples)
            most_samples = first_count_array[i];
    }
    printf("First Ranked\n");
    printf("  Total : %ld\n", sample_count);
    printf("   Most : %ld\n", most_samples);

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
            }
          }
        }
      }
    }
    printf("\n");
    printf("Filter\n");
    printf("  Total : %ld\n", sample_count);
    printf("   Most : %ld\n", most_samples);

    //------------//
    // first rank //
    //------------//

    char filename[1024];
    sprintf(filename, "%s.1st", prob_file);
	FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    for (int i = 0; i < FIRST_INDICIES; i++)
    {
        if (first_count_array[i] == 0)
            continue;
        float first_prob;
        first_index.IndexToValue(i, &first_prob);
        float prob = (float)first_good_array[i] /
            (float)first_count_array[i];
        fprintf(ofp, "%g %g %d\n", first_prob, prob,
            first_count_array[i]);
    }
    fclose(ofp);

    //-----------//
    // neighbors //
    //-----------//

    sprintf(filename, "%s.n", prob_file);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
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
      if (total_count == 0)
          continue;
      float neighbors;
      neighbor_index.IndexToValue(i, &neighbors);
      float prob = (float)good_count / (float)total_count;
      fprintf(ofp, "%g %g %ld\n", neighbors, prob, total_count);
    }
    fclose(ofp);

    //-----------//
    // dif ratio //
    //-----------//

    sprintf(filename, "%s.dr", prob_file);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
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
      if (total_count == 0)
          continue;
      float dif_ratio;
      dif_ratio_index.IndexToValue(j, &dif_ratio);
      float prob = (float)good_count / (float)total_count;
      fprintf(ofp, "%g %g %ld\n", dif_ratio, prob, total_count);
    }
    fclose(ofp);

    //-------//
    // speed //
    //-------//

    sprintf(filename, "%s.spd", prob_file);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
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
      if (total_count == 0)
          continue;
      float speed;
      speed_index.IndexToValue(k, &speed);
      float prob = (float)good_count / (float)total_count;
      fprintf(ofp, "%g %g %ld\n", speed, prob, total_count);
    }
    fclose(ofp);

    //-----//
    // cti //
    //-----//

    sprintf(filename, "%s.cti", prob_file);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    for (int l = 0; l < CTI_INDICIES; l++)
    {
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
      if (total_count == 0)
          continue;
      float cti;
      cti_index.IndexToValue(l, &cti);
      float prob = (float)good_count / (float)total_count;
      fprintf(ofp, "%g %g %ld\n", cti, prob, total_count);
    }
    fclose(ofp);

    //------//
    // prob //
    //------//

    sprintf(filename, "%s.prob", prob_file);
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
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
      if (total_count == 0)
          continue;
      float sprob;
      prob_index.IndexToValue(m, &sprob);
      float prob = (float)good_count / (float)total_count;
      fprintf(ofp, "%g %g %ld\n", sprob, prob, total_count);
    }
    fclose(ofp);

/*
    //-----------------//
    // write prob file //
    //-----------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening prob file %s\n", command,
            output_file);
        exit(1);
    }
    else
    {
        fwrite(first_count_array, sizeof(unsigned int),
            FIRST_INDICIES*SPEED_INDICIES, ofp);
        fwrite(first_good_array, sizeof(unsigned int),
            FIRST_INDICIES*SPEED_INDICIES, ofp);
        fwrite(filter_count_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
          PROB_INDICIES,
          ofp);
        fwrite(filter_good_array, sizeof(unsigned int),
          NEIGHBOR_INDICIES*DIF_RATIO_INDICIES*SPEED_INDICIES*CTI_INDICIES*
          PROB_INDICIES,
          ofp);
        fclose(ofp);
    }
*/

    return (0);
}
