//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    wr_class
//
// SYNOPSIS
//    wr_class [ -m minutes ] <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Makes some nbd and rain rate comparisons.
//
// OPTIONS
//    [ -m minutes ]  Time difference maximum.
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % wr_class -m 15 1500 1600 15min
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
#include <unistd.h>
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

#define OPTSTRING  "m:"
#define ATI_SIZE   1624
#define CTI_SIZE   76
#define NBD_SCALE  10.0

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m minutes ]", "<start_rev>", "<end_rev>",
    "<output_base>", 0 };

// parameter (nbd,spd,dir); ssmi rain (0=no,1=yes), counts vs param idx
static unsigned long counts[3][2][255];
static unsigned long matrix[256][2][2];
static unsigned long matrix_sum[2][2];
static unsigned long total_count = 0;


// coefficients for rain probability
float coefs[3][8] = {
{ 0.05286, 0.04506, 0.03832, 0.008476, -0.00271, -0.0005499, 7.568e-05,
    3.749e-06 },
{ -0.006388, 0.01489, -0.005691, 0.0008835, -1.651e-05, -2.791e-06,
    1.441e-07, -1.939e-09 },
{ 0.06228, -0.0003691, 0.0001584, -1.677e-05, 6.475e-07, -1.186e-08,
    1.05e-10, -3.585e-13 }
};

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

    int minutes = 180;    // three hours
//    float rain_threshold = 0.0;

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
            minutes = atoi(optarg);
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

    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.histo", output_base);
    FILE* histo_ofp = fopen(filename, "w");
    if (histo_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    sprintf(filename, "%s.matrix", output_base);
    FILE* matrix_ofp = fopen(filename, "w");
    if (matrix_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    //--------------------//
    // process rev by rev //
    //--------------------//

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //---------------//
        // read nbd file //
        //---------------//

        char nbd_file[1024];
        sprintf(nbd_file, "%d.nbd", rev);
        char nbd_array[ATI_SIZE][CTI_SIZE];
        FILE* ifp = fopen(nbd_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening NBD file %s\n", command,
                nbd_file);
            exit(1);
        }
        fread(nbd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);

        //------------//
        // speed file //
        //------------//

        char spd_file[1024];
        sprintf(spd_file, "%d.spd", rev);
        unsigned char spd_array[ATI_SIZE][CTI_SIZE];
        ifp = fopen(spd_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening SPD file %s\n", command,
                spd_file);
            exit(1);
        }
        fread(spd_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);

        //----------------//
        // direction file //
        //----------------//

        char dir_file[1024];
        sprintf(dir_file, "%d.dir", rev);
        unsigned char dir_array[ATI_SIZE][CTI_SIZE];
        ifp = fopen(dir_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening DIR file %s\n", command,
                dir_file);
            exit(1);
        }
        fread(dir_array, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);

        //----------------//
        // read rain file //
        //----------------//

        char rain_file[1024];
        sprintf(rain_file, "%d.rain", rev);
        unsigned char rain_rate[ATI_SIZE][CTI_SIZE];
        unsigned char time_dif[ATI_SIZE][CTI_SIZE];
        ifp = fopen(rain_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening rain file %s\n", command,
                rain_file);
            exit(1);
        }
        fread(rain_rate, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fread(time_dif, sizeof(char), CTI_SIZE * ATI_SIZE, ifp);
        fclose(ifp);

        //---------------------------------------//
        // eliminate rain data out of time range //
        //---------------------------------------//

        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                int co_time = time_dif[ati][cti] * 2 - 180;
                if (abs(co_time) > minutes)
                    rain_rate[ati][cti] = 255;
            }
        }

        //-----------------//
        // try classifying //
        //-----------------//

        float class_func[ATI_SIZE][CTI_SIZE];
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                class_func[ati][cti] = 0.0;

                if (rain_rate[ati][cti] >= 250 ||
                    nbd_array[ati][cti] == -128 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255)
                {
                    continue;
                }

                float param_value[3];
                param_value[0] = (float)nbd_array[ati][cti] * 0.1;
                param_value[1] = (float)spd_array[ati][cti] * 0.2;
                param_value[2] = (float)dir_array[ati][cti] * 0.5;
                float total = 1.0;
                for (int param_idx = 0; param_idx < 3; param_idx++)
                {
                    float coef = polyval(param_value[param_idx],
                        coefs[param_idx], 7);
                    total *= polyval(param_value[param_idx],
                        coefs[param_idx], 7);
                }
                class_func[ati][cti] = total;
//                printf("%g %d\n", total, rain_rate[ati][cti] == 0 ? 0 : 1);
            }
        }

        //-----------------------//
        // accumulate histograms //
        //-----------------------//

        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255)
                {
                    continue;
                }
                int rr_idx = 0;
                if (rain_rate[ati][cti] > 0)
                    rr_idx = 1;

                int nbd_idx = (int)nbd_array[ati][cti] + 128;
                counts[0][rr_idx][nbd_idx]++;

                int spd_idx = spd_array[ati][cti];
                counts[1][rr_idx][spd_idx]++;

                int dir_idx = dir_array[ati][cti];
                counts[2][rr_idx][dir_idx]++;
            }
        }

        //-------------------//
        // accumulate matrix //
        //-------------------//

        int ssmi_flag, nbd_flag;
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128 ||
                    spd_array[ati][cti] == 255 ||
                    dir_array[ati][cti] == 255)
                {
                    continue;
                }

                // threshold
                float rr = rain_rate[ati][cti] * 0.1;
                if (rr > 0.2)
                    ssmi_flag = 1;
                else
                    ssmi_flag = 0;

                // xxx
                if (class_func[ati][cti] > 0.0020)
                    nbd_flag = 1;
                else
                    nbd_flag = 0;

                int spd_idx = spd_array[ati][cti];

                // accumulate
                matrix_sum[ssmi_flag][nbd_flag]++;
                matrix[spd_idx][ssmi_flag][nbd_flag]++;
                total_count++;
            }
        }
    }

    //------------------//
    // write histograms //
    //------------------//

    const char* param_map[] = { "NBD", "Speed", "Direction" };
    const char* rr_map[] = { "Clear", "Rain" };
    int param_b[] = { -128, 0, 0 };
    float param_m[] = { 0.1, 0.2, 0.5 };
    for (int param_idx = 0; param_idx < 3; param_idx++)
    {
        for (int rr_idx = 0; rr_idx < 2; rr_idx++)
        {
            fprintf(histo_ofp, "# %s, %s\n", param_map[param_idx],
                rr_map[rr_idx]);
            for (int idx = 0; idx < 256; idx++)
            {
                if (counts[param_idx][rr_idx][idx] == 0)
                    continue;
                float value = (float)(idx + param_b[param_idx]) *
                    param_m[param_idx];
                fprintf(histo_ofp, "%g %lu\n", value,
                    counts[param_idx][rr_idx][idx]);
            }
            fprintf(histo_ofp, "&\n");
        }
        // ratio of rain to total
        for (int idx = 0; idx < 256; idx++)
        {
            if (counts[param_idx][0][idx] + counts[param_idx][1][idx] < 10)
            {
                continue;
            }
            float value = (float)(idx + param_b[param_idx]) *
                param_m[param_idx];
            fprintf(histo_ofp, "%g %g\n", value,
                (double)counts[param_idx][1][idx] /
                ((double)counts[param_idx][0][idx] +
                counts[param_idx][1][idx]));
        }
        fprintf(histo_ofp, "&\n");

    }

    //-----------------------//
    // write out matrix sums //
    //-----------------------//

    char* i_map[] = { "SSM/I Clear", "SSM/I Rain" };
    char* j_map[] = { "NBD Clear", "NBD Rain" };
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            fprintf(matrix_ofp, "# %s, %s = %g %%\n", i_map[i], j_map[j],
                100.0 * (double)matrix_sum[i][j] / (double)total_count);
        }
    }

    //------------------//
    // write out matrix //
    //------------------//

/*
    for (int spd_idx = 0; spd_idx < 256; spd_idx++)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (matrix[spd_idx][i][j] > 0)
                {
                    fprintf(matrix_ofp, "%d %d %d %lu\n", spd_idx, i, j,
                        matrix[spd_idx][i][j]);
                }
            }
        }
    }
*/

/*
    //-----------------//
    // the versus file //
    //-----------------//

    for (int ati = 0; ati < ATI_SIZE; ati++)
    {
        for (int cti = 0; cti < CTI_SIZE; cti++)
        {
            if (rain_rate[ati][cti] > 250 ||
                nbd_array[ati][cti] == -128)
            {
                continue;
            }
            float rr = rain_rate[ati][cti] * 0.1;
            float nbd = nbd_array[ati][cti] * 0.1;
            fprintf(vs_ofp, "%g %g\n", rr, nbd);
        }
    }

    //-----------------//
    // classifier file //
    //-----------------//

    if (opt_class && spd_file != NULL)
    {
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128)
                {
                    continue;
                }
                float rr = rain_rate[ati][cti] * 0.1;
                float nbd = nbd_array[ati][cti] * 0.1;
                float spd = spd_array[ati][cti] * 0.2;
                int flag = 0;
                if (rr > rain_threshold)
                    flag = 1;
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, flag);
            }
        }
        for (int ati = 0; ati < ATI_SIZE; ati++)
        {
            for (int cti = 0; cti < CTI_SIZE; cti++)
            {
                // check data
                if (rain_rate[ati][cti] > 250 ||
                    nbd_array[ati][cti] == -128)
                {
                    continue;
                }
                float rr = rain_rate[ati][cti] * 0.1;
                float nbd = nbd_array[ati][cti] * 0.1;
                float spd = spd_array[ati][cti] * 0.2;
                fprintf(class_ofp, "%g %g %d\n", spd, nbd, rain_rate[ati][cti]);
            }
        }
    }
*/

    //-------------//
    // close files //
    //-------------//

    fclose(histo_ofp);
    fclose(matrix_ofp);
/*
    fclose(vs_ofp);
    fclose(class_ofp);
*/

    return (0);
}
