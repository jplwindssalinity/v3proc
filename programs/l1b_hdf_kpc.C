//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1b_hdf_kpc
//
// SYNOPSIS
//    l1b_hdf_kpc [ -o output_file ] [ -b beam(0,1) ] [ -l land_map_file ]
//        <L1B_file>
//
// DESCRIPTION
//    This program generates Kpc for eggs using the egg and composite
//    egg method.
//
// OPTIONS
//    The following options are supported:
//      [ -l land_map_file ]  Filter out land using the land map.
//      [ -o output_file ]  The output file. If not specified, the
//                            output will be written to standard output.
//
// OPERANDS
//    The following operands are supported:
//      <L1B_file>  A L1B file.
//
// EXAMPLES
//    An example of a command line is:
//      % l1b_hdf_kpc -o l1b.dat
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
//    James N. Huddleston <mailto:James.N.Huddleston@jpl.nasa.gov>
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
#include <string.h>
#include <math.h>
#include "hdf.h"
#include "mfhdf.h"
#include "LandMap.h"
#include "Constants.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "b:l:o:"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

const char* no_path(const char* string);
void usage(const char* command, const char* option_array[],
    const int exit_value);
int32 SDnametoid(int32 sd_id, char* sds_name, float64* scale_factor = NULL);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -l land_map_file ]", "[ -o output_file ]",
    "[ -b beam(0,1) ]", "<L1B_file>", 0 };

float cell_kpc_b[8][2] = {
    { 0.2887047, 0.2887047 },
    { 0.05413212, 0.05413212 },
    { 0.02547394, 0.02547394 },
    { 0.01603915, 0.01603915 },
    { 0.008140168, 0.008140168 },
    { 0.01202936, 0.01202936 },
    { 0.008965982, 0.008965982 },
    { 0.008140168, 0.008140168 }
};

float cell_kpc_c[8][2] = {
    { 0.144419, 0.144419 },
    { 0.02712856, 0.02712856 },
    { 0.01279579, 0.01279579 },
    { 0.008075129, 0.008075129 },
    { 0.004122716, 0.004122716 },
    { 0.006064679, 0.006064679 },
    { 0.004530611, 0.004530611 },
    { 0.004122716, 0.004122716 }
};

float slice_kpc_b[8][2] = {
    { 2.887047, 2.887047 },
    { 0.5413212, 0.5413212 },
    { 0.2547394, 0.2547394 },
    { 0.1603915, 0.1603915 },
    { 0.08140168, 0.08140168 },
    { 0.1202936, 0.1202936 },
    { 0.08965982, 0.08965982 },
    { 0.08140168, 0.08140168 }
};

float slice_kpc_c[8][2] = {
    { 1.44419, 1.44419 },
    { 0.2712856, 0.2712856 },
    { 0.1279579, 0.1279579 },
    { 0.08075129, 0.08075129 },
    { 0.04122716, 0.04122716 },
    { 0.06064679, 0.06064679 },
    { 0.04530611, 0.04530611 },
    { 0.04122716, 0.04122716 }
};

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
    extern char* optarg;
    extern int optind;

    int only_beam = -1;

    //--------------------
    // By default, we will write to standard output. If a file
    // name is given, we will use that instead.
    //--------------------

    FILE* ofp = stdout;
    char* output_file = NULL;
    char* land_map_file = NULL;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'b':
            only_beam = atoi(optarg);
            break;
        case 'l':
            land_map_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 1)
        usage(command, usage_array, 1);

    char* l1b_filename = argv[optind++];

    //------------------//
    // read the landmap //
    //------------------//

    SimpleLandMap land_map;
    if (land_map_file != NULL)
    {
        if (! land_map.Read(land_map_file))
        {
            fprintf(stderr, "%s: error reading land map %s\n", command,
                land_map_file);
            exit(1);
        }
    }

    //----------------------//
    // open the output file //
    //----------------------//

    if (output_file != NULL)
    {
        // only bother trying to open the file if there is one
        ofp = fopen(output_file, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening file %s\n", command,
                output_file);
            exit(1);
        }
    }

    //------------------------------//
    // loop through the input files //
    //------------------------------//

    int32 sd_id = SDstart(l1b_filename, DFACC_READ);
    if (sd_id == FAIL)
    {
        fprintf(stderr, "%s: error opening HDF file %s for reading\n",
            command, l1b_filename);
        exit(1);
    }

    int32 attr_index_l1b_actual_frame = SDfindattr(sd_id, "l1b_actual_frames");

    char data[1024];
    if (SDreadattr(sd_id, attr_index_l1b_actual_frame, data) == FAIL)
    {
        fprintf(stderr,
            "%s: error reading attribute for l1b_actual_frames\n",
            command);
        exit(1);
    }

    int frame_count = 0;
    if (sscanf(data, " %*[^\n] %*[^\n] %d", &frame_count) != 1)
    {
        fprintf(stderr, "%s: error parsing l1b_actual_frame attribute\n",
            command);
        fprintf(stderr, "%s\n", data);
        exit(1);
    }

    float64 antenna_azimuth_sf;
    int32 antenna_azimuth_sds_id = SDnametoid(sd_id, "antenna_azimuth",
        &antenna_azimuth_sf);

    float64 cell_snr_sf;
    int32 cell_snr_sds_id = SDnametoid(sd_id, "cell_snr", &cell_snr_sf);

    float64 cell_kpc_a_sf;
    int32 cell_kpc_a_sds_id = SDnametoid(sd_id, "cell_kpc_a", &cell_kpc_a_sf);

    float64 slice_snr_sf;
    int32 slice_snr_sds_id = SDnametoid(sd_id, "slice_snr", &slice_snr_sf);

    float64 slice_kpc_a_sf;
    int32 slice_kpc_a_sds_id = SDnametoid(sd_id, "slice_kpc_a",
        &slice_kpc_a_sf);

    float64 x_factor_sf;
    int32 x_factor_sds_id = SDnametoid(sd_id, "x_factor", &x_factor_sf);

    int32 sigma0_mode_flag_sds_id = SDnametoid(sd_id, "sigma0_mode_flag");
    int32 sigma0_qual_flag_sds_id = SDnametoid(sd_id, "sigma0_qual_flag");
    int32 cell_lat_sds_id = SDnametoid(sd_id, "cell_lat");
    int32 cell_lon_sds_id = SDnametoid(sd_id, "cell_lon");

    float64 frequency_shift_sf;
    int32 frequency_shift_sds_id = SDnametoid(sd_id, "frequency_shift",
              &frequency_shift_sf);

    int32 generic_start[3] = { 0,   0, 0 };
    int32 generic_edges[3] = { 1, 100, 8 };

    for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
    {
        //-----------------//
        // antenna azimuth //
        //-----------------//

        generic_start[0] = frame_idx;
        uint16 antenna_azimuth_dn[100];
        if (SDreaddata(antenna_azimuth_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)antenna_azimuth_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for antenna_azimuth\n",
                command);
            exit(1);
        }
        float antenna_azimuth[100];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            antenna_azimuth[spot_idx] = antenna_azimuth_dn[spot_idx] * antenna_azimuth_sf;
        }

        //----------//
        // cell snr //
        //----------//

        int16 cell_snr_dn[100];
        if (SDreaddata(cell_snr_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)cell_snr_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for cell_snr\n",
                command);
            exit(1);
        }
        float cell_snr[100];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            cell_snr[spot_idx] = pow(10.0, 0.1 * cell_snr_dn[spot_idx] * cell_snr_sf);
        }

        //------------//
        // cell kpc a //
        //------------//

        int16 cell_kpc_a_dn[100];
        if (SDreaddata(cell_kpc_a_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)cell_kpc_a_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for cell_kpc_a\n",
                command);
            exit(1);
        }
        float cell_kpc_a[100];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            cell_kpc_a[spot_idx] = cell_kpc_a_dn[spot_idx] * cell_kpc_a_sf;
        }

        //-----------//
        // slice snr //
        //-----------//

        float min_slice_snr[100];
        float max_slice_snr[100];

        int16 slice_snr_dn[100][8];
        if (SDreaddata(slice_snr_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)slice_snr_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for slice_snr\n",
                command);
            exit(1);
        }
        float slice_snr[100][8];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            for (int j = 0; j < 8; j++)
            {
                slice_snr[spot_idx][j] = pow(10.0,
                    0.1 * slice_snr_dn[spot_idx][j] * slice_snr_sf);
            }

            min_slice_snr[spot_idx] = slice_snr[spot_idx][0];
            max_slice_snr[spot_idx] = slice_snr[spot_idx][0];
            for (int j = 1; j < 8; j++)
            {
                if (slice_snr[spot_idx][j] < min_slice_snr[spot_idx])
                    min_slice_snr[spot_idx] = slice_snr[spot_idx][j];
                if (slice_snr[spot_idx][j] > max_slice_snr[spot_idx])
                    max_slice_snr[spot_idx] = slice_snr[spot_idx][j];
            }
        }

        //-------------//
        // slice kpc a //
        //-------------//

        int16 slice_kpc_a_dn[100][8];
        if (SDreaddata(slice_kpc_a_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)slice_kpc_a_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for slice_kpc_a\n",
                command);
            exit(1);
        }
        float slice_kpc_a[100][8];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            for (int j = 0; j < 8; j++)
            {
                slice_kpc_a[spot_idx][j] = slice_kpc_a_dn[spot_idx][j] * slice_kpc_a_sf;
            }
        }

        //----------//
        // x factor //
        //----------//

        int16 x_factor_dn[100][8];
        if (SDreaddata(x_factor_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)x_factor_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for x_factor\n",
                command);
            exit(1);
        }
        float x_factor[100][8];
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            for (int j = 0; j < 8; j++)
            {
                x_factor[spot_idx][j] = pow(10.0,
                    0.1 * x_factor_dn[spot_idx][j] * x_factor_sf);
            }
        }

        //------------------//
        // sigma0 mode flag //
        //------------------//

        uint16 sigma0_mode_flag[100];
        if (SDreaddata(sigma0_mode_flag_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)sigma0_mode_flag) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for sigma0_mode_flag\n",
                command);
            exit(1);
        }

        //------------------//
        // sigma0 qual flag //
        //------------------//

        uint16 sigma0_qual_flag[100];
        if (SDreaddata(sigma0_qual_flag_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)sigma0_qual_flag) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for sigma0_qual_flag\n",
                command);
            exit(1);
        }

        //------------------//
        // lat, lon, deltaf //
        //------------------//

        float32 cell_lat[100];
        float32 cell_lon[100];
        if (land_map_file != NULL)
        {
            if (SDreaddata(cell_lat_sds_id, generic_start, NULL,
                generic_edges, (VOIDP)cell_lat) == FAIL)
            {
                fprintf(stderr, "%s: error reading SD data for cell_lat\n",
                    command);
                exit(1);
            }
            if (SDreaddata(cell_lon_sds_id, generic_start, NULL,
                generic_edges, (VOIDP)cell_lon) == FAIL)
            {
                fprintf(stderr, "%s: error reading SD data for cell_lon\n",
                    command);
                exit(1);
            }
        }
        int16 frequency_shift_dn[100];
        float frequency_shift[100];
        if (SDreaddata(frequency_shift_sds_id, generic_start, NULL,
            generic_edges, (VOIDP)frequency_shift_dn) == FAIL)
        {
            fprintf(stderr, "%s: error reading SD data for frequency_shift\n",
                command);
            exit(1);
        }
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            frequency_shift[spot_idx] = frequency_shift_dn[spot_idx] * frequency_shift_sf;
        }

        //---------------//
        // calculate kpc //
        //---------------//

        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            float x_val = frame_idx + (float)spot_idx / 100.0;

            // skip cals
            if (sigma0_mode_flag[spot_idx] & 0x0003 != 0)
                continue;

            // skip bad data
            if (sigma0_qual_flag[spot_idx] & 0x0001 != 0)
                continue;

            //--------------------//
            // skip land if asked //
            //--------------------//

            if (land_map_file != NULL)
            {
                double lon = cell_lon[spot_idx] * dtr;
                double lat = cell_lat[spot_idx] * dtr;
                int type = land_map.GetType(lon, lat);
                if (type != 0)
                    continue;
            }

            //-------------------//
            // calculate kpc egg //
            //-------------------//

            int beam_idx = spot_idx % 2;
            if (beam_idx != only_beam)
                continue;

            float egg_kpc = cell_kpc_a[spot_idx];
//                + cell_kpc_b[3][beam_idx] / cell_snr[spot_idx]
//                + cell_kpc_c[3][beam_idx] / (cell_snr[spot_idx] * cell_snr[spot_idx]);
            egg_kpc = sqrt(egg_kpc);

            //---------------------------------//
            // estimate missing slice xfactors //
            //---------------------------------//

            float x_factor_10[10];
            for (int i = 0; i < 10; i++)
            {
                if (i == 0)
                {
                    double h = 10.0 * log10(x_factor[spot_idx][1]);
                    double b = 10.0 * log10(x_factor[spot_idx][0]);
                    x_factor_10[i] = pow(10.0, 0.1 * (2.0 * b - h));
                }
                else if (i == 9)
                {
                    double h = 10.0 * log10(x_factor[spot_idx][6]);
                    double b = 10.0 * log10(x_factor[spot_idx][7]);
                    x_factor_10[i] = pow(10.0, 0.1 * (2.0 * b - h));
                }
                else
                {
                    x_factor_10[i] = x_factor[spot_idx][i-1];
                }
            }

            //-----------------------------//
            // calculate kpc composite egg //
            //-----------------------------//

            double sum_of_x_2 = 0.0;
            double sum_of_x = 0.0;

            for (int j = 0; j < 10; j++)
            {
                int use_j = j;
                if (j >= 1 && j <= 8) use_j = j - 1;
                if (j == 9) use_j = 7;
                float slice_kpc_2 = slice_kpc_a[spot_idx][use_j]
                    + slice_kpc_b[3][beam_idx] / slice_snr[spot_idx][use_j]
                    + slice_kpc_c[3][beam_idx]
                        / (slice_snr[spot_idx][use_j] * slice_snr[spot_idx][use_j]);
/*
                top_sum += (x_factor_10[j] * x_factor_10[j]
                    * slice_kpc_2);
                bottom_sum += x_factor_10[j];
*/

                sum_of_x_2 += x_factor_10[j] * x_factor_10[j];
                sum_of_x += x_factor_10[j];
            }
/*
            bottom_sum *= bottom_sum;
            float comp_kpc = sqrt(top_sum / bottom_sum);
*/

double comp_a = slice_kpc_a[spot_idx][0] * (sum_of_x_2 / (sum_of_x * sum_of_x));
double comp_b = slice_kpc_b[3][beam_idx] / 10;
double comp_c = slice_kpc_c[3][beam_idx] / 10;

            double comp_kpc = comp_a;
//                + comp_b / cell_snr[spot_idx]
//                + comp_c / (cell_snr[spot_idx] * cell_snr[spot_idx]);
            comp_kpc = sqrt(comp_kpc);

            double eff_bandwidth = 1.0 / (egg_kpc * egg_kpc * 1.5);
            fprintf(ofp, "%g %g %g\n", antenna_azimuth[spot_idx],
                10.0 * log10(1.0 + egg_kpc), 10.0 * log10(1.0 + comp_kpc));
        }
    }

    if (SDendaccess(antenna_azimuth_sds_id) == FAIL ||
        SDendaccess(cell_snr_sds_id) == FAIL ||
        SDendaccess(cell_kpc_a_sds_id) == FAIL ||
        SDendaccess(slice_snr_sds_id) == FAIL ||
        SDendaccess(x_factor_sds_id) == FAIL ||
        SDendaccess(sigma0_mode_flag_sds_id) == FAIL ||
        SDendaccess(sigma0_qual_flag_sds_id) == FAIL ||
        SDendaccess(slice_kpc_a_sds_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending SD access\n", command);
        exit(1);
    }

    //--------------------
    // Finally, we can say goodbye to this file. Buh-bye!
    //--------------------

    if (SDend(sd_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending SD\n", command);
        exit(1);
    }

    //-----------------------//
    // close the output file //
    //-----------------------//

    if (output_file != NULL)
    {
        fclose(ofp);
    }

    return (0);
}

//--------------------
// The following are three helper functions. Normally they would be
// put into a library, but if you leave them here you won't have to
// worry about linking.
//--------------------

//---------//
// no_path //
//---------//

const char*
no_path(
    const char*  string)
{
    const char* last_slash = strrchr(string, '/');
    if (! last_slash)
        return(string);
    return(last_slash + 1);
}

#define LINE_LENGTH  78

//-------//
// usage //
//-------//

void
usage(
    const char*  command,
    const char*  option_array[],
    const int    exit_value)
{
    fprintf(stderr, "usage: %s", command);
    int skip = 11;
    int position = 7 + strlen(command);
    for (int i = 0; option_array[i]; i++)
    {
        int length = 1 + strlen(option_array[i]);
        position += length;
        if (position > LINE_LENGTH)
        {
            fprintf(stderr, "\n%*s", skip, " ");
            position = skip + length;
        }
        fprintf(stderr, " %s", option_array[i]);
    }
    fprintf(stderr, "\n");
    exit(exit_value);
}

//------------//
// SDnametoid //
//------------//

int32
SDnametoid(
    int32     sd_id,
    char*     sds_name,
    float64*  scale_factor)
{
    //------------------------------//
    // convert the name to an index //
    //------------------------------//

    int32 sds_index = SDnametoindex(sd_id, sds_name);
    if (sds_index == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD name (%s) to index\n",
            sds_name);
        exit(1);
    }

    //-------------------------------//
    // select that sd, and get an id //
    //-------------------------------//

    int32 sds_id = SDselect(sd_id, sds_index);
    if (sds_id == FAIL)
    {
        fprintf(stderr, "SDnametoid: error converting SD index (%ld) to ID\n",
            sds_index);
        exit(1);
    }

    //----------------------//
    // get the scale factor //
    //----------------------//

    if (scale_factor != NULL)
    {
        float64 cal_error, offset, offset_error;
        int32 data_type;
        if (SDgetcal(sds_id, scale_factor, &cal_error, &offset,
            &offset_error, &data_type) == FAIL)
        {
            *scale_factor = 0.0;
        }
    }

    return (sds_id);
}
