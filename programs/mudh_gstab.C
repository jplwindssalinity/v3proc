//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_gstab
//
// SYNOPSIS
//    mudh_gstab [ -r ] [ -n min_samples ] <input_mudhtab> <output_gstab>
//
// DESCRIPTION
//    Reformat mudhtab to the floating point array needed by the GS.
//    Also, fills in -3 flags for data with less than the specified
//    number of samples.
//
// OPTIONS
//    [ -r ]               Require NBD values.
//    [ -n min_samples ]   The minimum number of samples.
//
// OPERANDS
//    <input_mudhtab>   Input mudhtab.
//    <output_gstab>    Output gstab.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_gstab -n 20 input.mudhtabex table.gstab
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
#include <unistd.h>

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "rn:"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_require_nbd = 0;
int opt_min_samples = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r ]", "[ -n min_samples ]","<pca_file>",
    "<input_mudhtab>", "<output_gstab>", 0 };

static double norain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double sample_count_1[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

static float gstab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

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

    unsigned long min_samples = 0;

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            opt_require_nbd = 1;
            break;
        case 'n':
            min_samples = (unsigned long)atoi(optarg);
            opt_min_samples = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    const char* pca_file = argv[optind++];
    const char* input_mudhtab_file = argv[optind++];
    const char* output_gstab_file = argv[optind++];

    //---------------//
    // read PCA file //
    //---------------//

    // first index 0=both beams, 1=outer only
    float pca_weights[2][PC_COUNT][PARAM_COUNT];
    float pca_mean[2][PARAM_COUNT];
    float pca_std[2][PARAM_COUNT];
    float pca_min[2][PC_COUNT];
    float pca_max[2][PC_COUNT];
    int pca_use[2][PARAM_COUNT];

#define GS_PARAM_COUNT 8
    float gs_pca_weights[2][PC_COUNT][GS_PARAM_COUNT];
    float gs_pca_mean[2][GS_PARAM_COUNT];
    float gs_pca_std[2][GS_PARAM_COUNT];
    int gs_pca_use[2][GS_PARAM_COUNT];
    
    int gs_idx[]={NBD_IDX,SPD_IDX,DIR_IDX,MLE_IDX,ENOF_IDX,TBH_IDX,TBV_IDX,TRANS_IDX};

    FILE* ifp = fopen(pca_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening PCA file %s\n", command,
            pca_file);
        exit(1);
    }
    unsigned int w_size = PC_COUNT * PARAM_COUNT;
    if (fread(pca_weights[0], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        fprintf(stderr, "%s: error reading first half of PCA file %s\n",
            command, pca_file);
        exit(1);
    }
    if (fread(pca_weights[1], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        // complain if it is NOT EOF or Inner swath only
            fprintf(stderr, "%s: error reading second half of PCA file %s\n",
                command, pca_file);
            exit(1);
 
    }
    fclose(ifp);

    //----------------------------------------//
    // compute pca useability mask            //
    //----------------------------------------//

    for(int swath=0;swath<2;swath++){
	for(int j=0;j<PARAM_COUNT;j++){
	  pca_use[swath][j]=1;


	  for(int i=0;i<PC_COUNT;i++){
	    if(pca_weights[swath][i][j]!=0) pca_use[swath][j]=0;
	}
      }
    }

    //----------------------------------------//
    // transfer pca arrays to gs_pca arrays   //
    // by leaving out extra parameters        //
    //----------------------------------------//

    for(int swath=0;swath<2;swath++){
      for(int j=0;j<GS_PARAM_COUNT;j++){
        gs_pca_use[swath][j]=pca_use[swath][gs_idx[j]];
        gs_pca_mean[swath][j]=pca_mean[swath][gs_idx[j]];
        gs_pca_std[swath][j]=pca_std[swath][gs_idx[j]];
	
	
	for(int i=0;i<PC_COUNT;i++){
	  gs_pca_weights[swath][i][j]=pca_weights[swath][i][gs_idx[j]];
	}
      }
    }
    //----------------------------------------//
    // OPEN INPUT/OUTPUT MUDH TABLE FILES  ---//
    //----------------------------------------//

    FILE* mudhtab_ifp = fopen(input_mudhtab_file, "r");
    if (mudhtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mudhtab file %s\n", command,
            input_mudhtab_file);
        exit(1);
    }


    FILE* gstab_ofp = fopen(output_gstab_file, "w");
    if (gstab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output gstab file %s\n",
            command, output_gstab_file);
        exit(1);
    }

    //------------------------//
    // Output PCA info        //
    //------------------------//

    unsigned int length=PC_COUNT*2;
    if (fwrite(pca_max, sizeof(float), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }
    if (fwrite(pca_min, sizeof(float), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }
    length=GS_PARAM_COUNT*2;
    if (fwrite(gs_pca_mean, sizeof(float), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }
    if (fwrite(gs_pca_std, sizeof(float), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }

    if (fwrite(gs_pca_use, sizeof(int), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }

    length=GS_PARAM_COUNT*PC_COUNT*2;
    if (fwrite(gs_pca_weights, sizeof(float), length, gstab_ofp) != length)
      {
	fprintf(stderr, "%s: error writing gstab file %s\n", command,
		output_gstab_file);
	exit(1);
      }
    //-------------------//
    // read mudhtab file //
    //-------------------//

    for(int swath=0;swath<2;swath++){
      unsigned int size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;
      if (fread(norain_tab_1, sizeof(double), size, mudhtab_ifp) != size ||
	  fread(rain_tab_1, sizeof(double), size, mudhtab_ifp) != size ||
	  fread(sample_count_1, sizeof(double), size, mudhtab_ifp) != size)
	{
	  fprintf(stderr, "%s: error reading mudhtab file %s\n", command,
		  input_mudhtab_file);
	  exit(1);
	}


      //-------------------//
      // transfer to gstab //
      //-------------------//

      for (int inbd = 0; inbd < NBD_DIM; inbd++)
	{
	  for (int ispd = 0; ispd < SPD_DIM; ispd++)
	    {
	      for (int idir = 0; idir < DIR_DIM; idir++)
		{
		  for (int imle = 0; imle < MLE_DIM; imle++)
		    {
		      if (opt_min_samples &&
			  sample_count_1[inbd][ispd][idir][imle] < min_samples)
			{
			  gstab[inbd][ispd][idir][imle] = -3.0;
			}
		      else if (opt_require_nbd && inbd == NBD_DIM - 1)
			{
			  gstab[inbd][ispd][idir][imle] = -3.0;
			}
		      else
			{
			  gstab[inbd][ispd][idir][imle] =
			    (float)rain_tab_1[inbd][ispd][idir][imle];
			}
		    }
		}
	    }
	}

      //-----------------//
      // write the gstab //
      //-----------------//


      if (fwrite(gstab, sizeof(float), size, gstab_ofp) != size)
	{
	  fprintf(stderr, "%s: error writing gstab file %s\n", command,
		  output_gstab_file);
	  exit(1);
	}
    }
    fclose(mudhtab_ifp);
    fclose(gstab_ofp);

    return (0);
}
