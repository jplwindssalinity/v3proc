//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mc_more
//
// SYNOPSIS
//    mc_more [ -d mudh_dir ] [ -c mc_dir ] <start_rev> <end_rev>
//        <output_base>
//
// DESCRIPTION
//    Generates a table of attenuation and additional scatter.
//
// OPTIONS
//    [ -d mudh_dir ]  An alternate directory for MUDH files.
//    [ -c mc_dir ]    An alternate directory for MC files.
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mc_more 1500 1600 1500-1600
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

#define OPTSTRING         "d:c:"
#define BIG_DIM           100
#define QUOTE             '"'
#define DEFAULT_MUDH_DIR  "/export/svt11/hudd/allmudh"
#define DEFAULT_MC_DIR    "/export/svt11/hudd/mc"
#define REV_DIGITS        5

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_hist = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d mudh_dir ]", "[ -c mc_dir ]",
    "<start_rev>", "<end_rev>", "<output_base>", 0 };

// first index is for beam
double sum_x[2][NBD_DIM][DIR_DIM][MLE_DIM];
double sum_y[2][NBD_DIM][DIR_DIM][MLE_DIM];
double sum_xx[2][NBD_DIM][DIR_DIM][MLE_DIM];
double sum_xy[2][NBD_DIM][DIR_DIM][MLE_DIM];
double count[2][NBD_DIM][DIR_DIM][MLE_DIM];

// these are for the projection
double p_sum_x[2][3][NBD_DIM];
double p_sum_y[2][3][NBD_DIM];
double p_sum_xx[2][3][NBD_DIM];
double p_sum_xy[2][3][NBD_DIM];
double p_count[2][3][NBD_DIM];

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

    char* mudh_dir = DEFAULT_MUDH_DIR;
    char* mc_dir = DEFAULT_MC_DIR;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'c':
            mc_dir = optarg;
            break;
        case 'd':
            mudh_dir = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //--------------//
    // simple calcs //
    //--------------//

    float nbd_spread = NBD_MAX - NBD_MIN;
    int max_inbd = NBD_DIM - 2;    // save room for missing NBD index

    float dir_spread = DIR_MAX - DIR_MIN;
    int max_idir = DIR_DIM - 1;

    float mle_spread = MLE_MAX - MLE_MIN;
    int max_imle = MLE_DIM - 1;

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
printf("%d\n", rev);
        //----------------//
        // read MUDH file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%s/%0*d.mudh", mudh_dir, REV_DIGITS, rev);
        FILE* ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(nbd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(spd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(dir_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(mle_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //--------------//
        // read mc file //
        //--------------//

        char mc_file[1024];
        sprintf(mc_file, "%s/%0*d.mc", mc_dir, REV_DIGITS, rev);
        ifp = fopen(mc_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MC file %s\n", command,
                mc_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }

        do
        {
            //-------------------//
            // read an MC record //
            //-------------------//

            unsigned short ati_short;
            unsigned char cti_char, beam_char;
            float x, y;

            if (fread(&ati_short, sizeof(unsigned short), 1, ifp) != 1 ||
                fread(&cti_char, sizeof(unsigned char), 1, ifp) != 1 ||
                fread(&beam_char, sizeof(unsigned char), 1, ifp) != 1 ||
                fread(&x, sizeof(float), 1, ifp) != 1 ||
                fread(&y, sizeof(float), 1, ifp) != 1)
            {
                if (feof(ifp))
                    break;
                else
                {
                    fprintf(stderr, "%s: error reading MC file %s\n", command,
                        mc_file);
                    exit(1);
                }
            }
            int ati = (int)ati_short;
            int cti = (int)cti_char;
            int beam_idx = (int)beam_char;

            //------------------------//
            // calculate MUDH indices //
            //------------------------//

            double nbd = (double)nbd_array[ati][cti] * 0.001 - 10.0;
            double dir = (double)dir_array[ati][cti] * 0.01;
            double mle = (double)mle_array[ati][cti] * 0.001 - 30.0;

            int imle = (int)((mle - MLE_MIN) * (float)max_imle /
                mle_spread + 0.5);
            if (imle < 0) imle = 0;
            if (imle > max_imle) imle = max_imle;

            int idir = (int)((dir - DIR_MIN) * (float)max_idir /
                dir_spread + 0.5);
            if (idir < 0) idir = 0;
            if (idir > max_idir) idir = max_idir;

            int inbd = (int)((nbd - NBD_MIN) * (float)max_inbd /
                nbd_spread + 0.5);
            if (inbd < 0) inbd = 0;
            if (inbd > max_inbd) inbd = max_inbd;
            if (nbd_array[ati][cti] == MAX_SHORT) inbd = max_inbd + 1;

            //------------//
            // accumulate //
            //------------//

            sum_x[beam_idx][inbd][idir][imle] += x;    // true
            sum_y[beam_idx][inbd][idir][imle] += y;    // measured
            sum_xx[beam_idx][inbd][idir][imle] += (x * x);
            sum_xy[beam_idx][inbd][idir][imle] += (x * y);
            count[beam_idx][inbd][idir][imle]++;

            // for the projections
            p_sum_x[beam_idx][0][inbd] += x;
            p_sum_y[beam_idx][0][inbd] += y;
            p_sum_xx[beam_idx][0][inbd] += (x * x);
            p_sum_xy[beam_idx][0][inbd] += (x * y);
            p_count[beam_idx][0][inbd]++;

            p_sum_x[beam_idx][1][idir] += x;
            p_sum_y[beam_idx][1][idir] += y;
            p_sum_xx[beam_idx][1][idir] += (x * x);
            p_sum_xy[beam_idx][1][idir] += (x * y);
            p_count[beam_idx][1][idir]++;

            p_sum_x[beam_idx][2][imle] += x;
            p_sum_y[beam_idx][2][imle] += y;
            p_sum_xx[beam_idx][2][imle] += (x * x);
            p_sum_xy[beam_idx][2][imle] += (x * y);
            p_count[beam_idx][2][imle]++;
        } while (1);
        fclose(ifp);
    }

    //------------------------------//
    // write big accumulation table //
    //------------------------------//

    char filename[2048];
    sprintf(filename, "%s.mcsumtab", output_base);
    FILE* mcsumtab_ofp = fopen(filename, "w");
    if (mcsumtab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    unsigned int size = 2 * NBD_DIM * DIR_DIM * MLE_DIM;
    if (fwrite(sum_x, sizeof(double), size, mcsumtab_ofp) != size ||
        fwrite(sum_y, sizeof(double), size, mcsumtab_ofp) != size ||
        fwrite(sum_xx, sizeof(double), size, mcsumtab_ofp) != size ||
        fwrite(sum_xy, sizeof(double), size, mcsumtab_ofp) != size ||
        fwrite(count, sizeof(double), size, mcsumtab_ofp) != size)
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            filename);
        exit(1);
    }

    fclose(mcsumtab_ofp);

    //------------------------//
    // generate output tables //
    //------------------------//
    // overlay into the sum_x and sum_y array for memory conservation
    // equation is y = m * x + b
    // sum_x will hold the m value (attenuation)
    // sum_y will hold the b value (additive scattering)

    sprintf(filename, "%s.mctab", output_base);
    FILE* mctab_ofp = fopen(filename, "w");
    if (mctab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    for (int beam_idx = 0; beam_idx < 2; beam_idx++)
    {
      for (int i = 0; i < NBD_DIM; i++)
      {
        for (int j = 0; j < DIR_DIM; j++)
        {
          for (int k = 0; k < MLE_DIM; k++)
          {
              double sum_x_ref = sum_x[beam_idx][i][j][k];
              double sum_y_ref = sum_y[beam_idx][i][j][k];
              double sum_xx_ref = sum_xx[beam_idx][i][j][k];
              double sum_xy_ref = sum_xy[beam_idx][i][j][k];
              double count_ref = count[beam_idx][i][j][k];
              double denom = sum_x_ref * sum_x_ref -
                count_ref * sum_xx_ref;
              if (denom == 0.0)
              {
                sum_x[beam_idx][i][j][k] = 0.0;
                sum_y[beam_idx][i][j][k] = 0.0;
                continue;
              }

              sum_x[beam_idx][i][j][k] = (sum_x_ref * sum_y_ref -
                count_ref * sum_xy_ref) / denom;
              sum_y[beam_idx][i][j][k] = (sum_x_ref * sum_xy_ref -
                sum_xx_ref * sum_y_ref) / denom;
          }
        }
      }
    }
    fwrite(sum_x, sizeof(double), 2 * NBD_DIM * DIR_DIM * MLE_DIM, mctab_ofp);
    fwrite(sum_y, sizeof(double), 2 * NBD_DIM * DIR_DIM * MLE_DIM, mctab_ofp);
    fwrite(count, sizeof(double), 2 * NBD_DIM * DIR_DIM * MLE_DIM, mctab_ofp);

    //-------------//
    // close files //
    //-------------//

    fclose(mctab_ofp);

    //-----------------------//
    // make a histogram file //
    //-----------------------//

/*
    sprintf(filename, "%s.hist", output_base);
    FILE* hist_ofp = fopen(filename, "w");
    if (hist_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening histogram file %s\n", command,
            filename);
        exit(1);
    }

    // find max count
    double max_count = 0.0;
    double total_count = 0.0;
    for (int i = 0; i < NBD_DIM; i++)
    {
        for (int j = 0; j < SPD_DIM; j++)
        {
            for (int k = 0; k < DIR_DIM; k++)
            {
                for (int l = 0; l < MLE_DIM; l++)
                {
                    total_count += count[beam_idx][i][j][k];
                    if (count[beam_idx][i][j][k] > max_count)
                        max_count = count[beam_idx][i][j][k];
                }
            }
        }
    }

    unsigned long cell_PDF_sum = 0;
    unsigned long data_sum = 0;
    unsigned long data_PDF_sum = 0;
    for (unsigned long target = 0; target <= max_count; target++)
    {
        unsigned long target_count = 0;
        for (int i = 0; i < NBD_DIM; i++)
        {
            for (int j = 0; j < SPD_DIM; j++)
            {
                for (int k = 0; k < DIR_DIM; k++)
                {
                    for (int l = 0; l < MLE_DIM; l++)
                    {
                        if (count[beam_idx][i][j][k] == target)
                        {
                            target_count++;
                        }
                    }
                }
            }
        }
        cell_PDF_sum += target_count;
        data_sum = (target_count * target);
        data_PDF_sum += data_sum;
        double cell_PDF = (double)cell_PDF_sum /
            (double)(NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM);
        double data_PDF = (double)data_PDF_sum / (double)(total_count);
        fprintf(hist_ofp, "%ld %g %g\n", target, cell_PDF, data_PDF);
    }
    fclose(hist_ofp);
*/

    //-------------------------//
    // write projected m and b //
    //-------------------------//

    for (int beam_idx = 0; beam_idx < 2; beam_idx++)
    {
        for (int param_idx = 0; param_idx < 3; param_idx++)
        {
            for (int idx = 0; idx < NBD_DIM; idx++)
            {
                double sum_x_ref = p_sum_x[beam_idx][param_idx][idx];
                double sum_y_ref = p_sum_y[beam_idx][param_idx][idx];
                double sum_xx_ref = p_sum_xx[beam_idx][param_idx][idx];
                double sum_xy_ref = p_sum_xy[beam_idx][param_idx][idx];
                double count_ref = p_count[beam_idx][param_idx][idx];
                double denom = sum_x_ref * sum_x_ref - count_ref * sum_xx_ref;
                if (denom == 0.0)
                {
                    p_sum_x[beam_idx][param_idx][idx] = 0.0;
                    p_sum_y[beam_idx][param_idx][idx] = 0.0;
                    continue;
                }

                p_sum_x[beam_idx][param_idx][idx] = (sum_x_ref * sum_y_ref -
                    count_ref * sum_xy_ref) / denom;
                p_sum_y[beam_idx][param_idx][idx] = (sum_x_ref * sum_xy_ref -
                    sum_xx_ref * sum_y_ref) / denom;
            }
        }
    }

    const char* param_label[] = { "NBD", "Swath Relative Direction (deg)",
        "MLE" };
    const char* param_ext[] = { "nbd", "dir", "mle" };
    float param_spread[] = { nbd_spread, dir_spread, mle_spread };
    float param_min[] = { NBD_MIN, DIR_MIN, MLE_MIN };
    for (int param_idx = 0; param_idx < 3; param_idx++)
    {
        char filename[1024];

        sprintf(filename, "%s.mult.%s", output_base, param_ext[param_idx]);
        FILE* ofp_1 = fopen(filename, "w");
        if (ofp_1 == NULL)
        {
            fprintf(stderr,
                "%s: error opening output multiplicative file %s\n",
                command, filename);
            exit(1);
        }
        fprintf(ofp_1, "@ subtitle %cAttenuation%c\n", QUOTE, QUOTE);
        fprintf(ofp_1, "@ xaxis label %c%s%c\n", QUOTE, param_label[param_idx],
            QUOTE);
        fprintf(ofp_1, "@ yaxis label %c%s%c\n", QUOTE, "Attenuation (dB)",
            QUOTE);

        sprintf(filename, "%s.add.%s", output_base, param_ext[param_idx]);
        FILE* ofp_2 = fopen(filename, "w");
        if (ofp_2 == NULL)
        {
            fprintf(stderr, "%s: error opening output additive file %s\n",
                command, filename);
            exit(1);
        }
        fprintf(ofp_2, "@ subtitle %cAdditive Scatter%c\n", QUOTE, QUOTE);
        fprintf(ofp_2, "@ xaxis label %c%s%c\n", QUOTE, param_label[param_idx],
            QUOTE);
        fprintf(ofp_2, "@ yaxis label %c%s%c\n", QUOTE, "Additive Scatter",
            QUOTE);

        for (int beam_idx = 0; beam_idx < 2; beam_idx++)
        {
            for (int idx = 0; idx < NBD_DIM; idx++)
            {
                if (p_count[beam_idx][param_idx][idx] == 0)
                    continue;

                float value = (float)idx * param_spread[param_idx] /
                    (float)(NBD_DIM - 1) + param_min[param_idx];

                if (p_sum_x[beam_idx][param_idx][idx] <= 0.0)
                    continue;
                double proj_m_db =
                    10.0 * log10(p_sum_x[beam_idx][param_idx][idx]);
                double proj_b = p_sum_y[beam_idx][param_idx][idx];
                fprintf(ofp_1, "%g %g\n", value, proj_m_db);
                fprintf(ofp_2, "%g %g\n", value, proj_b);
            }
            if (beam_idx < 1)
            {
                fprintf(ofp_1, "&\n");
                fprintf(ofp_2, "&\n");
            }
        }

        fclose(ofp_1);
        fclose(ofp_2);
    }

    return (0);
}
