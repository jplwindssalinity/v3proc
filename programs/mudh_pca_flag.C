//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_flag
//
// SYNOPSIS
//    mudh_pca_flag <pca_file> <pcatab_file> <threshold> <mudh_file>
//        <enof_file> <tb_file> <output_flag_file>
//
// DESCRIPTION
//    Performs MUDH PCA classification and writes out two arrays:
//      A floating point probability of rain array and a byte flag
//      array.  The byte flag array has the following meanings:
//      0 = classified as no rain
//      1 = classified as rain
//      2 = unclassifiable
//
// OPTIONS
//
// OPERANDS
//    <pca_file>          The PCA file for PC coefficients.
//    <pcatab_file>       The PCA classification table.
//    <threshold>         The threshold for rain determination.
//    <mudh_file>         The input MUDH file.
//    <enof_file>         The input ENOF file.
//    <tb_file>           The input Tb file.
//    <output_flag_file>  The output flag file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_flag pcs.021100 zzz.pcatab 0.015 1550.mudh
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
#include "Misc.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<pca_file>", "<pcatab_file>", "<threshold>",
    "<mudh_file>", "<enof_file>", "<tb_file>", "<output_pflag_file>", 0 };

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

    if (argc < optind + 7)
        usage(command, usage_array, 1);

    const char* pca_file = argv[optind++];
    const char* pcatab_file = argv[optind++];
    float threshold = atof(argv[optind++]);
    const char* mudh_file = argv[optind++];
    const char* enof_file = argv[optind++];
    const char* tb_file = argv[optind++];
    const char* output_file = argv[optind++];

    //-------------------//
    // read the pca file //
    //-------------------//

    // first index 0=both beams, 1=outer only
    float pca_weights[2][4][PARAM_COUNT];
    float pca_mean[2][PARAM_COUNT];
    float pca_std[2][PARAM_COUNT];
    float pca_min[2][PARAM_COUNT];
    float pca_max[2][PARAM_COUNT];
    Index pc_index[2][4];

    int opt_outer_swath = 0;
    FILE* ifp = fopen(pca_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening PCA file %s\n", command,
            pca_file);
        exit(1);
    }
    fread(pca_weights[0], sizeof(float), 4 * PARAM_COUNT, ifp);
    fread(pca_mean[0], sizeof(float), PARAM_COUNT, ifp);
    fread(pca_std[0], sizeof(float), PARAM_COUNT, ifp);
    fread(pca_min[0], sizeof(float), PARAM_COUNT, ifp);
    fread(pca_max[0], sizeof(float), PARAM_COUNT, ifp);
    if (! feof(ifp))
    {
        fread(pca_weights[1], sizeof(float), 4 * PARAM_COUNT, ifp);
        fread(pca_mean[1], sizeof(float), PARAM_COUNT, ifp);
        fread(pca_std[1], sizeof(float), PARAM_COUNT, ifp);
        fread(pca_min[1], sizeof(float), PARAM_COUNT, ifp);
        opt_outer_swath = 1;
    }
    fclose(ifp);

    // set up indices
    for (int pc_idx = 0; pc_idx < 4; pc_idx++)
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
    unsigned int size = DIM * DIM * DIR * DIM;
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
                fprintf(stderr, "%s: error reading pcatab file %s\n", command,
                    pcatab_file);
                exit(1);
            }
        }
        else if (swath_idx == 1)
        {
        }
    }
    fclose(pcatab_ifp);

    //-------------------//
    // read in mudh file //
    //-------------------//

    ifp = fopen(mudh_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening MUDH file %s\n", command,
            mudh_file);
        fprintf(stderr, "%s: continuing...\n", command);
        continue;
    }
    fread(nbd_array, sizeof(short), array_size, ifp);
    fread(spd_array, sizeof(short), array_size, ifp);
    fread(dir_array, sizeof(short), array_size, ifp);
    fread(mle_array, sizeof(short), array_size, ifp);
    fclose(ifp);

    //----------------//
    // read ENOF file //
    //----------------//

    ifp = fopen(enof_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening ENOF file %s\n", command,
            enof_file);
        fprintf(stderr, "%s: continuing...\n", command);
        continue;
    }
    fread(qual_array, sizeof(float), array_size, ifp);
    fread(enof_array, sizeof(float), array_size, ifp);
    fclose(ifp);

    //--------------//
    // read Tb file //
    //--------------//

    ifp = fopen(tb_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening Tb file %s\n", command,
            tb_file);
        fprintf(stderr, "%s: continuing...\n", command);
        continue;
    }
    fread(tbh_array, sizeof(float), array_size, ifp);
    fread(tbv_array, sizeof(float), array_size, ifp);
    fread(tbh_std_array, sizeof(float), array_size, ifp);
    fread(tbv_std_array, sizeof(float), array_size, ifp);
    fread(tbh_cnt_array, sizeof(float), array_size, ifp);
    fread(tbv_cnt_array, sizeof(float), array_size, ifp);
    fclose(ifp);

    //----------//
    // classify //
    //----------//

    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            index_tab[ati][cti] = -1.0;
            flag_tab[ati][cti] = 2;

            // speed, direction, and mle are ALWAYS needed
            if (spd_array[ati][cti] == MAX_SHORT ||
                dir_array[ati][cti] == MAX_SHORT ||
                mle_array[ati][cti] == MAX_SHORT)
            {
                continue;
            }

            //------//
            // MUDH //
            //------//

            param[SPD_IDX] = (double)spd_array[ati][cti] * 0.01;
            param[DIR_IDX] = (double)dir_array[ati][cti] * 0.01;
            param[MLE_IDX] = (double)mle_array[ati][cti] * 0.001 - 30.0;

            // NBD is only for both beams
            if (nbd_array[ati][cti] != MAX_SHORT)
            {
                param[NBD_IDX] = (double)nbd_array[ati][cti] * 0.001 - 10.0;
                got_nbd = 1;
            }

            //-----------//
            // ENOF/Qual //
            //-----------//

            int got_enof = 0;

            // qual is ALWAYS needed
            if (qual_array[ati][cti] == -100.0)
                continue;
            param[QUAL_IDX] = qual_array[ati][cti];

            // ENOF is only needed for both beams
            if (! finite((double)enof_array[ati][cti]))
                continue;    // bad value
            if (enof_array[ati][cti] != -100.0)
            {
                param[ENOF_IDX] = enof_array[ati][cti];
                got_enof = 1;
            }

            //----//
            // Tb //
            //----//

            double tbv_cnt = 0.0;
            double tbh_cnt = 0.0;
            int got_tbh = 0;

            // Tb v is always needed
            if (tbv_cnt_array[ati][cti] == 0.0 ||
                tbv_array[ati][cti] >= 300.0)
            {
                continue;
            }
            param[TBV_IDX] = tbv_array[ati][cti];
            param[TBV_STD_IDX] = tbv_std_array[ati][cti];
            tbv_cnt = tbv_cnt_array[ati][cti];

            // Tb h is needed for both beams only
            if (tbh_cnt_array[ati][cti] > 0.0 &&
                tbh_array[ati][cti] < 300.0)
            {
                param[TBH_IDX] = tbh_array[ati][cti];
                param[TBH_STD_IDX] = tbh_std_array[ati][cti];
                tbh_cnt = tbh_cnt_array[ati][cti];
                got_tbh = 1;
            }

            //---------------//
            // transmittance //
            //---------------//

            // set these for convenience in writing eq's
            double tbv = param[TBV_IDX];
            double tbh = param[TBV_IDX];

            double tbvc = tbv * alpha1 + beta1;
            double tau = 0.0;
            if (got_tbh)
            {
                double tbhc = tbh * alpha2 + beta2;
                tau = a0 + a1 * log(300.0 - tbvc) +
                    a2 * log(300.0 - tbhc) + a3 * (tbvc - tbhc);
            }
            else
            {
                tau = a4 + a5 * log(300.0 - tbvc);
            }
            if (tau > 1.0) tau = 1.0;
            if (tau < 0.0) tau = 0.0;
            param[TRANS_IDX] = tau;

            //--------------------------//
            // determine swath location //
            //--------------------------//
            // all outer beam info is available (otherwise we woudn't
            // have made it this far).  just check for inner beam
            // info.

            int swath_idx;
            if (got_nbd && got_enof && got_tbh &&
                cti >= 8 && cti <= 67)
            {
                swath_idx = 0;    // both beams
            }
            else if (! got_nbd && ! got_enof && ! got_tbh &&
                (cti <= 8 || cti >= 67) )
            {
                swath_idx = 1;    // outer beam only
            }
            else
                continue;    // i don't know what the hell is going on.

            //-----------------------------------------------//
            // don't do the outer swath unless PCA available //
            //----------------------------------------------//

            if (swath_idx == 1 && ! opt_outer_swath)
                continue;

            //--------------------------------------//
            // determine principle component values //
            //--------------------------------------//

            double norm_param[PARAM_COUNT];
            for (int param_idx = 0; param_idx < PARAM_COUNT; param_idx++)
            {
                norm_param[param_idx] =
                    (param[param_idx] - pca_mean[swath_idx][param_idx]) /
                    pca_std[swath_idx][param_idx];
            }

            double pc[PC_COUNT];
            int pci[PC_COUNT];
            for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
            {
                pc[pc_idx] = 0.0;
                for (int param_idx = 0; param_idx < PARAM_COUNT;
                    param_idx++)
                {
                    pc[pc_idx] +=
                        pca_weights[swath_idx][pc_idx][param_idx] *
                        norm_param[param_idx];
                }

                // compute the index too
                pc_index[swath_idx][pc_idx].GetNearestIndex(pc[pc_idx],
                    &(pci[pc_idx]));
            }

            //------------------------------------------------------//
            // look up the value from the rain classification table //
            //------------------------------------------------------//

            float prob_value = rain_tab[pci[0]][pci[1]][pci[2]][pci[3]];
            index_tab[ati][cti] = mudh_value;

            if (mudh_value <= mudh_thresh)
            {
                flag_tab[ati][cti] = 0;
            }
            else
            {
                flag_tab[ati][cti] = 1;
            }
        }
    }

    //------------------//
    // write pflag file //
    //------------------//

    FILE* ofp = fopen(output_pflag_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_pflag_file);
        exit(1);
    }
    fwrite(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ofp);
    fclose(ofp);

    return (0);
}
