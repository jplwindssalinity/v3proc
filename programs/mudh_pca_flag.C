//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_flag
//
// SYNOPSIS
//    mudh_pca_flag <pca_file> <pcatab_file> <inner_threshold>
//        <outer_threshold> <mudh_file> <enof_file> <tb_file>
//        <output_flag_file>
//
// DESCRIPTION
//    Performs MUDH PCA classification and writes out two arrays:
//      A floating point probability of rain array and a byte flag
//      array.  The byte flag array has the following meanings:
//      0 = inner swath, classified as no rain
//      1 = inner swath, classified as rain
//      2 = inner swath, unclassifiable
//      3 = outer swath, classified as no rain
//      4 = outer swath, classified as rain
//      5 = outer swath, unclassifiable
//      6 = no wind retrieved
//      7 = unknown
//
// OPTIONS
//
// OPERANDS
//    <pca_file>          The PCA file for PC coefficients.
//    <pcatab_file>       The PCA classification table.
//    <inner_threshold>   The threshold for rain (inner swath).
//    <outer_threshold>   The threshold for rain (outer swath).
//    <mudh_file>         The input MUDH file.
//    <enof_file>         The input ENOF file.
//    <tb_file>           The input Tb file.
//    <output_flag_file>  The output flag file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_flag pcs.021100 zzz.pcatab 0.13 0.12 1550.mudh
//          1550.enof 1550.tb 1550.flag
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
#include <ieeefp.h>
#include <math.h>
#include "Misc.h"
#include "Index.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

const double alpha1 = 1.0;
const double alpha2 = 1.0;
const double beta1 = 0.0;
const double beta2 = 0.0;
const double a0 = -0.6202;
const double a1 = -0.0499;
const double a2 = 0.3283;
const double a3 = 0.0013;
const double a4 = -0.4368;
const double a5 = 0.2895;

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<pca_file>", "<pcatab_file>",
    "<inner_threshold>", "<outer_threshold>", "<mudh_file>", "<enof_file>",
    "<tb_file>", "<output_flag_file>", 0 };

static double  rain_tab[2][DIM][DIM][DIM][DIM];
static double  clear_tab[2][DIM][DIM][DIM][DIM];
static double  count_tab[2][DIM][DIM][DIM][DIM];

static unsigned short  nbd_array[AT_WIDTH][CT_WIDTH];
static unsigned short  spd_array[AT_WIDTH][CT_WIDTH];
static unsigned short  dir_array[AT_WIDTH][CT_WIDTH];
static unsigned short  mle_array[AT_WIDTH][CT_WIDTH];

float qual_array[AT_WIDTH][CT_WIDTH];
float enof_array[AT_WIDTH][CT_WIDTH];

float tbh_array[AT_WIDTH][CT_WIDTH];
float tbv_array[AT_WIDTH][CT_WIDTH];
float tbh_std_array[AT_WIDTH][CT_WIDTH];
float tbv_std_array[AT_WIDTH][CT_WIDTH];
int tbh_cnt_array[AT_WIDTH][CT_WIDTH];
int tbv_cnt_array[AT_WIDTH][CT_WIDTH];

static float           value_tab[AT_WIDTH][CT_WIDTH];
static unsigned char   flag_tab[AT_WIDTH][CT_WIDTH];

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

    if (argc < optind + 8)
        usage(command, usage_array, 1);

    float threshold[2];
    const char* pca_file = argv[optind++];
    const char* pcatab_file = argv[optind++];
    threshold[0] = atof(argv[optind++]);
    threshold[1] = atof(argv[optind++]);
    const char* mudh_file = argv[optind++];
    const char* enof_file = argv[optind++];
    const char* tb_file = argv[optind++];
    const char* output_flag_file = argv[optind++];

    //---------------//
    // read PCA file //
    //---------------//

    // first index 0=both beams, 1=outer only
    float pca_weights[2][PC_COUNT][PARAM_COUNT];
    float pca_mean[2][PARAM_COUNT];
    float pca_std[2][PARAM_COUNT];
    float pca_min[2][PC_COUNT];
    float pca_max[2][PC_COUNT];
    Index pc_index[2][PC_COUNT];

    int opt_outer_swath = 0;

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
        // only complain if it is NOT EOF
        if (! feof(ifp))
        {
            fprintf(stderr, "%s: error reading second half of PCA file %s\n",
                command, pca_file);
            exit(1);
        }
    }
    else
    {
        opt_outer_swath = 1;
    }
    fclose(ifp);

    // set up indices
    for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
    {
        pc_index[0][pc_idx].SpecifyEdges(pca_min[0][pc_idx],
            pca_max[0][pc_idx], DIM);
        if (opt_outer_swath)
        {
            pc_index[1][pc_idx].SpecifyEdges(pca_min[1][pc_idx],
                pca_max[1][pc_idx], DIM);
        }
    }

    //------------------//
    // read pcatab file //
    //------------------//

    FILE* pcatab_ifp = fopen(pcatab_file, "r");
    if (pcatab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening pcatab file %s\n", command,
            pcatab_file);
        exit(1);
    }
    unsigned int size = DIM * DIM * DIM * DIM;
    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
        if (fread(clear_tab[swath_idx], sizeof(double), size, pcatab_ifp) !=
                size ||
            fread(rain_tab[swath_idx], sizeof(double), size, pcatab_ifp) !=
                size ||
            fread(count_tab[swath_idx], sizeof(double), size, pcatab_ifp) !=
                size)
        {
            if (swath_idx == 0)
            {
                // you hafta read something in
                fprintf(stderr, "%s: error reading pcatab file %s\n", command,
                    pcatab_file);
                exit(1);
            }
            else if (swath_idx == 1)
            {
                // this part is optional, did we expect it?
                if (opt_outer_swath)
                {
                    fprintf(stderr, "%s: pcatab missing second part\n",
                        command);
                    exit(1);
                }
            }
        }
    }
    fclose(pcatab_ifp);

    if (opt_outer_swath)
    {
        printf("  Full swath\n");
    }
    else
    {
        printf("  Inner swath only\n");
    }

    //----------------------//
    // determine file needs //
    //----------------------//

    int need_enof_file = 0;
    int need_tb_file = 0;
    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
        for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
        {
            if (pca_weights[swath_idx][pc_idx][QUAL_IDX] != 0.0 ||
                pca_weights[swath_idx][pc_idx][ENOF_IDX] != 0.0)
            {
                need_enof_file = 1;
            }
            if (pca_weights[swath_idx][pc_idx][TBH_IDX] != 0.0 ||
                pca_weights[swath_idx][pc_idx][TBV_IDX] != 0.0 ||
                pca_weights[swath_idx][pc_idx][TBH_STD_IDX] != 0.0 ||
                pca_weights[swath_idx][pc_idx][TBV_STD_IDX] != 0.0 ||
                pca_weights[swath_idx][pc_idx][TRANS_IDX] != 0.0)
            {
                need_tb_file = 1;
            }
        }
    }

    //-------------------//
    // read in mudh file //
    //-------------------//

    unsigned int array_size = CT_WIDTH * AT_WIDTH;
    ifp = fopen(mudh_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    fread(nbd_array, sizeof(short), array_size, ifp);
    fread(spd_array, sizeof(short), array_size, ifp);
    fread(dir_array, sizeof(short), array_size, ifp);
    fread(mle_array, sizeof(short), array_size, ifp);
    fclose(ifp);

    //----------------//
    // read ENOF file //
    //----------------//

    if (need_enof_file)
    {
        ifp = fopen(enof_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening ENOF file %s\n", command,
                enof_file);
            exit(1);
        }
        fread(qual_array, sizeof(float), array_size, ifp);
        fread(enof_array, sizeof(float), array_size, ifp);
        fclose(ifp);
    }

    //--------------//
    // read Tb file //
    //--------------//

    if (need_tb_file)
    {
        ifp = fopen(tb_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening Tb file %s\n", command,
                tb_file);
            exit(1);
        }
        fread(tbh_array, sizeof(float), array_size, ifp);
        fread(tbv_array, sizeof(float), array_size, ifp);
        fread(tbh_std_array, sizeof(float), array_size, ifp);
        fread(tbv_std_array, sizeof(float), array_size, ifp);
        fread(tbh_cnt_array, sizeof(int), array_size, ifp);
        fread(tbv_cnt_array, sizeof(int), array_size, ifp);
        fclose(ifp);
    }

    //----------//
    // classify //
    //----------//

    double param[PARAM_COUNT];
    int got_param[PARAM_COUNT];

    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            //------------//
            // initialize //
            //------------//

            for (int i = 0; i < PARAM_COUNT; i++)
                got_param[i] = 0;

            value_tab[ati][cti] = -1.0;
            flag_tab[ati][cti] = UNKNOWN;    // generic default

            //-----------------------------------//
            // is this a valid wind vector cell? //
            //-----------------------------------//

            if (spd_array[ati][cti] == MAX_SHORT ||
                mle_array[ati][cti] == MAX_SHORT)
            {
                flag_tab[ati][cti] = NO_WIND;
                continue;
            }

            //--------------------------//
            // determine swath location //
            //--------------------------//

            int swath_idx;
            if (nbd_array[ati][cti] == MAX_SHORT)
            {
                swath_idx = 1;    // outer swath (inner beam not available)
                flag_tab[ati][cti] = OUTER_UNKNOWN;
            }
            else
            {
                swath_idx = 0;    // inner swath
                flag_tab[ati][cti] = INNER_UNKNOWN;
            }

            //-----------------------------------------------//
            // don't do the outer swath unless PCA available //
            //----------------------------------------------//

            if (swath_idx == 1 && ! opt_outer_swath)
                continue;

            //------//
            // MUDH //
            //------//

            param[SPD_IDX] = (double)spd_array[ati][cti] * 0.01;
            got_param[SPD_IDX] = 1;
            if (dir_array[ati][cti] != MAX_SHORT)
            {
                param[DIR_IDX] = (double)dir_array[ati][cti] * 0.01;
                got_param[DIR_IDX] = 1;
            }
            param[MLE_IDX] = (double)mle_array[ati][cti] * 0.001 - 30.0;
            got_param[MLE_IDX] = 1;

            if (nbd_array[ati][cti] != MAX_SHORT)
            {
                param[NBD_IDX] = (double)nbd_array[ati][cti] * 0.001 - 10.0;
                got_param[NBD_IDX] = 1;
            }

            //-----------//
            // ENOF/Qual //
            //-----------//

            if (need_enof_file)
            {
                if (qual_array[ati][cti] != -100.0)
                {
                    param[QUAL_IDX] = qual_array[ati][cti];
                    got_param[QUAL_IDX] = 1;
                }

                if (enof_array[ati][cti] != -100.0 &&
                    finite((double)enof_array[ati][cti]))
                {
                    param[ENOF_IDX] = enof_array[ati][cti];
                    got_param[ENOF_IDX] = 1;
                }
            }

            //----//
            // Tb //
            //----//

            double tbv_cnt = 0.0;
            double tbh_cnt = 0.0;

            if (need_tb_file)
            {
                if (tbv_cnt_array[ati][cti] > 0 &&
                    tbv_array[ati][cti] < 300.0)
                {
                    param[TBV_IDX] = tbv_array[ati][cti];
                    got_param[TBV_IDX] = 1;
                    param[TBV_STD_IDX] = tbv_std_array[ati][cti];
                    got_param[TBV_STD_IDX] = 1;
                    tbv_cnt = (double)tbv_cnt_array[ati][cti];
                }

                if (tbh_cnt_array[ati][cti] > 0 &&
                    tbh_array[ati][cti] < 300.0)
                {
                    param[TBH_IDX] = tbh_array[ati][cti];
                    got_param[TBH_IDX] = 1;
                    param[TBH_STD_IDX] = tbh_std_array[ati][cti];
                    got_param[TBH_STD_IDX] = 1;
                    tbh_cnt = (double)tbh_cnt_array[ati][cti];
                }
            }

            //---------------//
            // transmittance //
            //---------------//

            double tau = 0.0;
            if (got_param[TBV_IDX] && got_param[TBH_IDX])
            {
                double tbvc = param[TBV_IDX] * alpha1 + beta1;
                double tbhc = param[TBH_IDX] * alpha2 + beta2;
                tau = a0 + a1 * log(300.0 - tbvc) +
                    a2 * log(300.0 - tbhc) + a3 * (tbvc - tbhc);
                got_param[TRANS_IDX] = 1;
            }
            else if (got_param[TBV_IDX])
            {
                double tbvc = param[TBV_IDX] * alpha1 + beta1;
                tau = a4 + a5 * log(300.0 - tbvc);
                got_param[TRANS_IDX] = 1;
            }
            if (tau > 1.0) tau = 1.0;
            if (tau < 0.0) tau = 0.0;
            param[TRANS_IDX] = tau;

            //--------------------------------------//
            // determine principle component values //
            //--------------------------------------//

            double norm_param[PARAM_COUNT];
            for (int param_idx = 0; param_idx < PARAM_COUNT; param_idx++)
            {
                // only normalize if you have the parameter
                if (got_param[param_idx])
                {
                    norm_param[param_idx] = (param[param_idx] -
                        pca_mean[swath_idx][param_idx]) /
                        pca_std[swath_idx][param_idx];
                }
            }

            int missing_info = 0;
            double pc[PC_COUNT];
            int pci[PC_COUNT];
            for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
            {
                pc[pc_idx] = 0.0;
                for (int param_idx = 0; param_idx < PARAM_COUNT;
                    param_idx++)
                {
                    if (pca_weights[swath_idx][pc_idx][param_idx] != 0.0)
                    {
                        // the parameter is needed
                        if (got_param[param_idx])
                        {
                            pc[pc_idx] +=
                                pca_weights[swath_idx][pc_idx][param_idx] *
                                norm_param[param_idx];
                        }
                        else
                        {
                            missing_info = 1;
                        }
                    }
                }

                if (missing_info)
                    break;    // get out of this loop

                // compute the index too
                pc_index[swath_idx][pc_idx].GetNearestIndex(pc[pc_idx],
                    &(pci[pc_idx]));
                if (pci[pc_idx] >= DIM) pci[pc_idx] = DIM - 1;
                if (pci[pc_idx] < 0) pci[pc_idx] = 0;
            }

            if (missing_info)
                continue;    // go to the next WVC

            //------------------------------------------------------//
            // look up the value from the rain classification table //
            //------------------------------------------------------//

            float prob_value =
                rain_tab[swath_idx][pci[0]][pci[1]][pci[2]][pci[3]];
            value_tab[ati][cti] = prob_value;

            if (prob_value < 0.0 || prob_value > 1.0)
            {
                // do nothing, already set to unknown
            }
            else if (prob_value <= threshold[swath_idx])
            {
                if (swath_idx == 0)
                    flag_tab[ati][cti] = INNER_CLEAR;
                else if (swath_idx == 1)
                    flag_tab[ati][cti] = OUTER_CLEAR;
            }
            else
            {
                if (swath_idx == 0)
                    flag_tab[ati][cti] = INNER_RAIN;
                else if (swath_idx == 1)
                    flag_tab[ati][cti] = OUTER_RAIN;
            }
        }
    }

    //-----------------//
    // write flag file //
    //-----------------//

    FILE* ofp = fopen(output_flag_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_flag_file);
        exit(1);
    }
    fwrite(value_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(flag_tab,  sizeof(char),  CT_WIDTH * AT_WIDTH, ofp);
    fclose(ofp);

    return (0);
}
