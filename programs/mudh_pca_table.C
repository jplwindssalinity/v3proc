//==============================================================//
// Copyright (C) 1999-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_table
//
// SYNOPSIS
//    mudh_pca_table [ -m mudh_dir ] [ -e enof_dir ] [ -t tb_dir ]
//        [ -i irr_dir ] [ -c time ] <pca_file> <start_rev> <end_rev>
//        <output_base>
//
// DESCRIPTION
//    Generates a mudh table for classification.
//
// OPTIONS
//    [ -m mudh_dir ]  Use the mudh files located in mudh_dir.
//    [ -e enof_dir ]  Use the enof files located in enof_dir.
//    [ -t tb]         Use the Tb files located in tb_dir.
//    [ -i irr_dir ]   IRR files are always used, this can specify the dir.
//    [ -c time ]      Collocation time for IRR.
//
// OPERANDS
//    <pca_file>     The PCA file.  If short, assumes both beam case.
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_table -c 30 pca.021100 1500 1600 1500-1600
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
#include <ieeefp.h>
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "Interpolate.h"
#include "SeaPac.h"

#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING         "m:e:t:i:c:1:2:"
#define BIG_DIM           100
#define QUOTE             '"'
#define REV_DIGITS        5

#define DEFAULT_MUDH_DIR  "/export/svt11/hudd/allmudh"
#define DEFAULT_TB_DIR    "/home/bstiles/dave"
#define DEFAULT_ENOF_DIR  "/home/bstiles/carl"
#define DEFAULT_IRR_DIR   "/export/svt11/hudd/ssmi"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m mudh_dir ]", "[ -e enof_dir ]",
    "[ -t tb_dir ]", "[ -i irr_dir ]", "[ -c time ]", "<pca_file>",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// first index: 0=both_beams, 1=outer_only
// second index: SSM/I class (0=all, 1=rainfree, 2=rain)
static unsigned long  counts[2][3][DIM][DIM][DIM][DIM];

static double  rain_tab[DIM][DIM][DIM][DIM];
static double  clear_tab[DIM][DIM][DIM][DIM];
static double  count_tab[DIM][DIM][DIM][DIM];

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

    int collocation_time = 30;   // default 30 minutes
    float irr_threshold = 2.0;   // default two km*mm/hr

    char* mudh_dir = DEFAULT_MUDH_DIR;
    char* enof_dir = DEFAULT_ENOF_DIR;
    char* tb_dir = DEFAULT_TB_DIR;
    char* irr_dir = DEFAULT_IRR_DIR;

    int opt_outer_swath = 0;   // assume don't do outer swath

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'm':
            mudh_dir = optarg;
            break;
        case 'e':
            enof_dir = optarg;
            break;
        case 't':
            tb_dir = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* pca_file = argv[optind++];
    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

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
        printf("  Inner swath only\n");
    }
    else
    {
        opt_outer_swath = 1;
        printf("  Full swath\n");
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
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.pcatab", output_base);
    FILE* pcatab_ofp = fopen(filename, "w");
    if (pcatab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char rain_rate[AT_WIDTH][CT_WIDTH];
    unsigned char time_dif[AT_WIDTH][CT_WIDTH];
    unsigned short integrated_rain_rate[AT_WIDTH][CT_WIDTH];

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];

    float tbh_array[AT_WIDTH][CT_WIDTH];
    float tbv_array[AT_WIDTH][CT_WIDTH];
    float tbh_std_array[AT_WIDTH][CT_WIDTH];
    float tbv_std_array[AT_WIDTH][CT_WIDTH];
    int tbh_cnt_array[AT_WIDTH][CT_WIDTH];
    int tbv_cnt_array[AT_WIDTH][CT_WIDTH];

    float qual_array[AT_WIDTH][CT_WIDTH];
    float enof_array[AT_WIDTH][CT_WIDTH];

    unsigned int array_size = CT_WIDTH * AT_WIDTH;

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "%s/%0*d.irain", irr_dir, REV_DIGITS, rev);
        FILE* ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s (continuing)\n",
                command, rain_file);
            continue;
        }
        if (fread(rain_rate, sizeof(char), array_size, ifp) != array_size ||
            fread(time_dif, sizeof(char), array_size, ifp) != array_size ||
            fseek(ifp, array_size, SEEK_CUR) != 0 ||
            fread(integrated_rain_rate, sizeof(short), array_size, ifp) !=
            array_size)
        {
            fprintf(stderr, "%s: error reading rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fclose(ifp);

        //----------------//
        // read MUDH file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%s/%0*d.mudh", mudh_dir, REV_DIGITS, rev);
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

        if (need_enof_file)
        {
            char enof_file[1024];
            sprintf(enof_file, "%s/%0*d.nofq", enof_dir, REV_DIGITS, rev);
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
        }

        //--------------//
        // read Tb file //
        //--------------//

        if (need_tb_file)
        {
            char tb_file[1024];
            sprintf(tb_file, "%s/%0*d.tb", tb_dir, REV_DIGITS, rev);
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
            fread(tbh_cnt_array, sizeof(int), array_size, ifp);
            fread(tbv_cnt_array, sizeof(int), array_size, ifp);
            fclose(ifp);
        }

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (abs(co_time) > collocation_time)
                    integrated_rain_rate[ati][cti] = 2000;
            }
        }

        //----------------//
        // generate table //
        //----------------//

        double param[PARAM_COUNT];
        int got_param[PARAM_COUNT];

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                //-----------//
                // rain rate //
                //-----------//

                if (integrated_rain_rate[ati][cti] >= 1000)
                    continue;
                double irr = (double)integrated_rain_rate[ati][cti] * 0.1;

                //------------//
                // initialize //
                //------------//

                for (int i = 0; i < PARAM_COUNT; i++)
                    got_param[i] = 0;

                //-----------------------------------//
                // is this a valid wind vector cell? //
                //-----------------------------------//

                if (spd_array[ati][cti] == MAX_SHORT ||
                    mle_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

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
                    param[NBD_IDX] = (double)nbd_array[ati][cti] * 0.001 -
                        10.0;
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

/*
                double alpha = -log(tau);    // one-way nadir optical depth
                // 2-way att (dB)
                double a = 20.0 * log10(exp(1.0)) * alpha;
*/

                //--------------------------//
                // determine swath location //
                //--------------------------//

                int swath_idx;
                if (got_param[NBD_IDX])
                    swath_idx = 0;    // inner swath (both beams)
                else
                    swath_idx = 1;    // outer swath (single beam)

                //-----------------------------------------------//
                // don't do the outer swath unless PCA available //
                //----------------------------------------------//

                if (swath_idx == 1 && ! opt_outer_swath)
                    continue;

                //--------------------------------------//
                // determine pricicple component values //
                //--------------------------------------//

                double norm_param[PARAM_COUNT];
                for (int param_idx = 0; param_idx < PARAM_COUNT; param_idx++)
                {
                    // only normalize if you have the parameter
                    if (got_param[param_idx])
                    {
                        norm_param[param_idx] =
                            (param[param_idx] -
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

                // accumulate
                if (irr == 0.0)
                {
                    // rainfree
                    counts[swath_idx][1][pci[0]][pci[1]][pci[2]][pci[3]]++;
                }
                else if (irr > irr_threshold)
                {
                   // rain
                    counts[swath_idx][2][pci[0]][pci[1]][pci[2]][pci[3]]++;
                }
                // all
                counts[swath_idx][0][pci[0]][pci[1]][pci[2]][pci[3]]++;
            }
        }
    }

    //-------------//
    // write table //
    //-------------//

    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
      // skip outer swath if you can't do it
      if (swath_idx == 1 && ! opt_outer_swath)
        continue;

      for (int i = 0; i < DIM; i++)
      {
        for (int j = 0; j < DIM; j++)
        {
          for (int k = 0; k < DIM; k++)
          {
            for (int l = 0; l < DIM; l++)
            {
              clear_tab[i][j][k][l] = 0.0;
              rain_tab[i][j][k][l] = 0.0;
              count_tab[i][j][k][l] =
                  (double)counts[swath_idx][0][i][j][k][l];
              if (counts[swath_idx][0][i][j][k][l] > 0)
              {
                double clear_prob = (double)counts[swath_idx][1][i][j][k][l] /
                  (double)counts[swath_idx][0][i][j][k][l];
                double rain_prob = (double)counts[swath_idx][2][i][j][k][l] /
                  (double)counts[swath_idx][0][i][j][k][l];

                clear_tab[i][j][k][l] = clear_prob;
                rain_tab[i][j][k][l] = rain_prob;
              }
              else
              {
                // mark as uncalculatable (2.0)
                clear_tab[i][j][k][l] = 2.0;
                rain_tab[i][j][k][l] = 2.0;
              }
            }
          }
        }
      }
      fwrite(clear_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
      fwrite(rain_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
      fwrite(count_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
    }

    //-------------//
    // close files //
    //-------------//

    fclose(pcatab_ofp);

    return (0);
}