//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    wr_bdvsrr
//
// SYNOPSIS
//    wr_bdvsrr <ins_config_file> <rain_file> <hdf_l2a_file>
//        <output_base>
//
// DESCRIPTION
//    Calculates the normalized beam difference and plots it versus
//    rain rate.
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>  The config file (for the GMF).
//    <rain_file>        The Freilich rain rate files.
//    <hdf_l2a_file>     The input HDF L2A file.
//    <output_base>      The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % wr_bdvsrr $cfg 203.rain l2a.203 203.bdvsrr
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
#include "L2AHdf.h"
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
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

#define OPTSTRING        ""
#define MAX_S0_PER_ROW   3240
#define MAX_CROSS_TRACK  76
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004
#define ANTENNA_BEAM     0x0004
#define MIN_RAIN_DN      0

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void  check_status(HdfFile::StatusE status);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<ins_config_file>", "<rain_file>",
    "<hdf_l2a_file>", "<output_base>", 0 };

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

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* rain_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    const char* output_base = argv[optind++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(ins_config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, ins_config_file);
        exit(1);
    }

    //-------------------------------------//
    // read the geophysical model function //
    //-------------------------------------//

    GMF gmf;
    char* gmf_filename = config_list.Get(GMF_FILE_KEYWORD);
    if (gmf_filename == NULL)
    {
        fprintf(stderr, "%s: error determining GMF file\n", command);
        exit(1);
    }

    if (! gmf.ReadOldStyle(gmf_filename))
    {
        fprintf(stderr, "%s: error reading GMF file %s\n", command,
            gmf_filename);
        exit(1);
    }

/*
    //-------------------//
    // read the land map //
    //-------------------//

    static unsigned char land_map[LAND_SEA_LATITUDES][LAND_SEA_LONGITUDES];
    if (! read_land_sea_map("/seapac/proc/stt-tables/LMAP0002", land_map))
    {
        fprintf(stderr, "%s: error reading land map\n", command);
        exit(1);
    }
*/

    //--------------------//
    // read the rain file //
    //--------------------//

    unsigned char rain_rate[1624][76];
    FILE* ifp = fopen(rain_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening rain file %s\n", command,
            rain_file);
        exit(1);
    }
    fread(rain_rate, sizeof(char), 76 * 1624, ifp);
    fclose(ifp);

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //-------------------------------------//
    // hand-read the EA configuration file //
    //-------------------------------------//

    char* ea_config_filename = getenv(ENV_CONFIG_FILENAME);
    if (ea_config_filename == NULL)
    {
        fprintf(stderr, "%s: need an EA_CONFIG_FILE environment variable\n",
            command);
        exit(1);
    }
    ConfigList ea_config_list;
    if (! ea_config_list.Read(ea_config_filename))
    {
        fprintf(stderr, "%s: error reading EA configuration file %s\n",
            command, ea_config_filename);
        exit(1);
    }

    char* poly_table_string = ea_config_list.Get(POLY_TABLE_KEYWORD);
    EA_PolynomialErrorNo poly_status = EA_POLY_OK;
    PolynomialTable* polyTable = new PolynomialTable(poly_table_string,
        poly_status);
    if ( poly_status != EA_POLY_OK)
    {
        fprintf(stderr, "%s: error creating polynomial table from %s\n",
            command, poly_table_string);
        exit(1);
    }

    //------------------//
    // set the l2a file //
    //------------------//

    Parameter* row_number_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        ROW_NUMBER, UNIT_DN);
    Parameter* num_sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        NUM_SIGMA0, UNIT_DN);
    Parameter* cell_index_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INDEX, UNIT_DN);
    Parameter* sigma0_mode_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_MODE_FLAG, UNIT_DN);
    Parameter* sigma0_qual_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_QUAL_FLAG, UNIT_DN);
    Parameter* sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0, UNIT_DB);
    Parameter* surface_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SURFACE_FLAG, UNIT_DN);
    Parameter* cell_lon_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LON, UNIT_RADIANS);
    Parameter* cell_lat_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LAT, UNIT_RADIANS);
    Parameter* cell_azimuth_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_AZIMUTH, UNIT_DEGREES);
    Parameter* cell_incidence_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INCIDENCE, UNIT_RADIANS);
    Parameter* kp_alpha_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_ALPHA, UNIT_DN);
    Parameter* kp_beta_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_BETA, UNIT_DN);
    Parameter* kp_gamma_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_GAMMA, UNIT_DN);
    Parameter* sigma0_attn_map_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_ATTN_MAP, UNIT_DB);

    HdfFile::StatusE status = HdfFile::OK;
    L2AHdf l2a_file(l2a_hdf_file, SOURCE_L2Ax, status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "%s: error opening L2A file %s\n", command,
            l2a_hdf_file);
        exit(1);
    }

    check_status(l2a_file.OpenParamDatasets(row_number_p));
    check_status(l2a_file.OpenParamDatasets(num_sigma0_p));
    check_status(l2a_file.OpenParamDatasets(cell_lat_p));
    check_status(l2a_file.OpenParamDatasets(cell_lon_p));
    check_status(l2a_file.OpenParamDatasets(cell_azimuth_p));
    check_status(l2a_file.OpenParamDatasets(cell_incidence_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_p));
    check_status(l2a_file.OpenParamDatasets(kp_alpha_p));
    check_status(l2a_file.OpenParamDatasets(kp_beta_p));
    check_status(l2a_file.OpenParamDatasets(kp_gamma_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_attn_map_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_qual_flag_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_mode_flag_p));
    check_status(l2a_file.OpenParamDatasets(surface_flag_p));
    check_status(l2a_file.OpenParamDatasets(cell_index_p));

    int l2a_length = l2a_file.GetDataLength();

    //------------------//
    // open output file //
    //------------------//

    char filename[2048];
    sprintf(filename, "%s.vs", output_base);
    FILE* vs_ofp = fopen(filename, "w");
    if (vs_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    sprintf(filename, "%s.bdapts", output_base);
    FILE* bd_ofp = fopen(filename, "w");
    if (bd_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(bd_ofp, "apts\n");

    sprintf(filename, "%s.bdsapts", output_base);
    FILE* bds_ofp = fopen(filename, "w");
    if (bds_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    fprintf(bds_ofp, "apts\n");

    //---------------//
    // for each cell //
    //---------------//

    short row_number = 0;
    short num_sigma0;
    float cell_lat[MAX_S0_PER_ROW];
    float cell_lon[MAX_S0_PER_ROW];
    float cell_azimuth[MAX_S0_PER_ROW];
    float cell_incidence[MAX_S0_PER_ROW];
    float sigma0[MAX_S0_PER_ROW];
    float sigma0_attn_map[MAX_S0_PER_ROW];
    float kp_alpha[MAX_S0_PER_ROW];
    float kp_beta[MAX_S0_PER_ROW];
    float kp_gamma[MAX_S0_PER_ROW];
    unsigned short sigma0_qual_flag[MAX_S0_PER_ROW];
    unsigned short sigma0_mode_flag[MAX_S0_PER_ROW];
    unsigned short surface_flag[MAX_S0_PER_ROW];
    char cell_index[MAX_S0_PER_ROW];

    MeasList  meas_list_row[MAX_CROSS_TRACK];
    char      ice_flag[MAX_CROSS_TRACK];
    char      no_ice_flag[MAX_CROSS_TRACK];

    for (int target_row = 1; target_row <= 1624; target_row++)
    {
        int target_idx = target_row - 1;

        //---------------------//
        // clear the ice flags //
        //---------------------//

        for (int i = 0; i < MAX_CROSS_TRACK; i++)
        {
            ice_flag[i] = 0;
            no_ice_flag[i] = 0;
        }

        for (int idx = 0; idx < l2a_length; idx++)
        {
            // check row number
            row_number_p->extractFunc(&l2a_file, row_number_p->sdsIDs,
                idx, 1, 1, &row_number, polyTable);
            if (row_number != target_row)
                continue;

            //--------------------//
            // extract parameters //
            //--------------------//

            num_sigma0_p->extractFunc(&l2a_file, num_sigma0_p->sdsIDs,
                idx, 1, 1, &num_sigma0, polyTable);

            cell_index_p->extractFunc(&l2a_file, cell_index_p->sdsIDs,
                idx, 1, 1, cell_index, polyTable);

            sigma0_mode_flag_p->extractFunc(&l2a_file,
                sigma0_mode_flag_p->sdsIDs, idx, 1, 1,
                sigma0_mode_flag, polyTable);

            sigma0_qual_flag_p->extractFunc(&l2a_file,
                sigma0_qual_flag_p->sdsIDs, idx, 1, 1,
                sigma0_qual_flag, polyTable);

            sigma0_p->extractFunc(&l2a_file, sigma0_p->sdsIDs,
                idx, 1, 1, sigma0, polyTable);

            surface_flag_p->extractFunc(&l2a_file, surface_flag_p->sdsIDs,
                idx, 1, 1, surface_flag, polyTable);

            cell_lon_p->extractFunc(&l2a_file, cell_lon_p->sdsIDs,
                idx, 1, 1, cell_lon, polyTable);

            cell_lat_p->extractFunc(&l2a_file, cell_lat_p->sdsIDs,
                idx, 1, 1, cell_lat, polyTable);

            cell_azimuth_p->extractFunc(&l2a_file, cell_azimuth_p->sdsIDs,
                idx, 1, 1, cell_azimuth, polyTable);

            cell_incidence_p->extractFunc(&l2a_file, cell_incidence_p->sdsIDs,
                idx, 1, 1, cell_incidence, polyTable);

            kp_alpha_p->extractFunc(&l2a_file, kp_alpha_p->sdsIDs,
                idx, 1, 1, kp_alpha, polyTable);

            kp_beta_p->extractFunc(&l2a_file, kp_beta_p->sdsIDs,
                idx, 1, 1, kp_beta, polyTable);

            kp_gamma_p->extractFunc(&l2a_file, kp_gamma_p->sdsIDs,
                idx, 1, 1, kp_gamma, polyTable);

            sigma0_attn_map_p->extractFunc(&l2a_file,
                sigma0_attn_map_p->sdsIDs, idx, 1, 1, sigma0_attn_map,
                polyTable);

            //-----------------------//
            // assemble measurements //
            //-----------------------//

            for (int j = 0; j < num_sigma0; j++)
            {
                if (sigma0_qual_flag[j] & UNUSABLE)
                    continue;

                // skip land
                int land_flag = surface_flag[j] & 0x00000001;
                if (land_flag)
                    continue;

                //-------------------//
                // set the ice flags //
                //-------------------//

                int cell_idx = cell_index[j] - 1;
                if (cell_idx < 0 || cell_idx >= MAX_CROSS_TRACK)
                {
/*
                    fprintf(stderr, "%s: bad cell index (%d)\n", command,
                       cell_idx);
*/
                    continue;
                }

                int ice_avail = ! (surface_flag[j] & 0x00000400);
                if (ice_avail)
                {
                    int ice = surface_flag[j] & 0x00000002;
                    if (ice)
                        ice_flag[cell_idx] = 1;
                    else
                        no_ice_flag[cell_idx] = 1;
                }
                else
                {
                    // no data.  I'll mark both and it will seem mixed
                    ice_flag[cell_idx] = 1;
                    no_ice_flag[cell_idx] = 1;
                }

/*
                if (map_value(cell_lon[j] * rtd, cell_lat[j] * rtd, land_map))
                    continue;
*/

                Meas* new_meas = new Meas;
                new_meas->incidenceAngle = cell_incidence[j];
                new_meas->value = sigma0[j] +
                    sigma0_attn_map[j] / cos(new_meas->incidenceAngle);
                new_meas->value = pow(10.0, 0.1 * new_meas->value);
                if (sigma0_qual_flag[j] & NEGATIVE)
                    new_meas->value = -(new_meas->value);
                new_meas->landFlag = (int)(surface_flag[j] & 0x00000001);
                new_meas->centroid.SetAltLonGDLat(0.0, cell_lon[j],
                    cell_lat[j]);
                new_meas->beamIdx = (int)(sigma0_mode_flag[j] & 0x0004);
                new_meas->beamIdx >>= 2;
                if (new_meas->beamIdx == 0)
                    new_meas->measType = Meas::HH_MEAS_TYPE;
                else
                    new_meas->measType = Meas::VV_MEAS_TYPE;
                new_meas->eastAzimuth = (450.0 - cell_azimuth[j]) * dtr;
                if (new_meas->eastAzimuth >= two_pi)
                    new_meas->eastAzimuth -= two_pi;
                new_meas->numSlices = -1;
                new_meas->A = kp_alpha[j];
                new_meas->B = kp_beta[j];
                new_meas->C = kp_gamma[j];
                // hack fore/aft info into scanAngle
                if (sigma0_mode_flag[j] & 0x0008)
                    new_meas->scanAngle = 0.0;    // fore
                else
                    new_meas->scanAngle = 1.0;    // aft

                if (! meas_list_row[cell_idx].Append(new_meas))
                {
                    fprintf(stderr, "%s: error appending measurement\n",
                        command);
                    exit(1);
                }
            }
        }

        //-----------------------------//
        // got everything for this row //
        //-----------------------------//

        for (int cell_idx = 0; cell_idx < MAX_CROSS_TRACK; cell_idx++)
        {
            MeasList* meas_list = &(meas_list_row[cell_idx]);
            if (meas_list->NodeCount() == 0)
                continue;

            LonLat avg_lon_lat = meas_list->AverageLonLat();
            float lon_deg = avg_lon_lat.longitude * rtd;
            float lat_deg = avg_lon_lat.latitude * rtd;

            // wind retrieval
            WVC wvc;
            if (! gmf.RetrieveWinds_GS(meas_list, &kp, &wvc))
                continue;

            WindVectorPlus* wvp = wvc.ambiguities.GetHead();
            if (wvp == NULL)
                continue;
            float mem_spd = wvp->spd;
            float mem_dir = wvp->dir;

            //-----------------//
            // accumulate info //
            //-----------------//

            double ratio_sum[2];        // beam
            double norm_sum[2];         // beam
            double x_comp_sum[2][2];    // beam, fore/aft
            double y_comp_sum[2][2];    // beam, fore/aft
            double comp_count[2][2];    // beam, fore/aft
            int count[2];
            for (int i = 0; i < 2; i++)
            {
                ratio_sum[i] = 0.0;
                norm_sum[i] = 0.0;
                count[i] = 0;
                for (int j = 0; j < 2; j++)
                {
                    x_comp_sum[i][j] = 0.0;
                    y_comp_sum[i][j] = 0.0;
                    comp_count[i][j] = 0;
                }
            }

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                float chi = wvp->dir - meas->eastAzimuth + pi;
                float value;
                gmf.GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                    wvp->spd, chi, &value);
                // eliminate very small true sigma-0's
                if (value < 1E-6)
                    continue;

                float ratio = meas->value / value;

                float kpm = 0.16;
                float use_alpha = (1.0 + kpm*kpm) * meas->A - 1.0;
                float var = (use_alpha * value + meas->B) * value + meas->C;
                float std_dev = sqrt(var);
                float ratio_std_dev = std_dev / value;

                ratio_sum[meas->beamIdx] += (ratio / ratio_std_dev);
                norm_sum[meas->beamIdx] += (1.0 / ratio_std_dev);
                count[meas->beamIdx]++;

                int foreaft_idx = (int)(meas->scanAngle + 0.5);
                if (foreaft_idx != 0)
                    foreaft_idx = 1;

                x_comp_sum[meas->beamIdx][foreaft_idx] += 
                    cos(meas->eastAzimuth);
                y_comp_sum[meas->beamIdx][foreaft_idx] += 
                    sin(meas->eastAzimuth);
                comp_count[meas->beamIdx][foreaft_idx]++;
            }
            wvc.FreeContents();

            if (comp_count[0][0] == 0 || comp_count[0][1] == 0 ||
                comp_count[1][0] == 0 || comp_count[1][1] == 0)
            {
                continue;
            }
            if (norm_sum[0] == 0.0 || norm_sum[1] == 0.0)
                continue;

            //-----------------------------//
            // calculate the target angles //
            //-----------------------------//

            float target_angle[2];
            for (int i = 0; i < 2; i++)    // beam
            {
                for (int j = 0; j < 2; j++)    // fore/aft
                {
                    x_comp_sum[i][j] /= comp_count[i][j];
                    y_comp_sum[i][j] /= comp_count[i][j];
                }
                target_angle[i] = atan2(y_comp_sum[i][0] + y_comp_sum[i][1],
                    x_comp_sum[i][0] + x_comp_sum[0][1]);
            }

            //-------------------------------------//
            // calculate the mean ratio difference //
            //-------------------------------------//

            float mean_ratio[2];
            mean_ratio[0] = (1.0 / norm_sum[0]) * ratio_sum[0];
            mean_ratio[1] = (1.0 / norm_sum[1]) * ratio_sum[1];
            float dif = mean_ratio[0] - mean_ratio[1];

            //---------------------------------------------------------//
            // calculate the expected std of the mean ratio difference //
            //---------------------------------------------------------//

            float std[2];
            std[0] = sqrt(count[0]) / norm_sum[0];
            std[1] = sqrt(count[1]) / norm_sum[1];
            float mrd_std = sqrt(std[0]*std[0] + std[1]*std[1]);
            dif /= mrd_std;    // normalize

            //----------------------------------//
            // calculate the angular difference //
            //----------------------------------//

            float dir_val = ANGDIF(target_angle[1], mem_dir);
            if (dir_val > pi_over_two)
                dir_val = pi - dir_val;
            dir_val = pi_over_two - dir_val;
            dir_val *= rtd;

            //-----------------//
            // output to files //
            //-----------------//

            // calculate rain rate
            int rain_rate_dn = rain_rate[target_idx][cell_idx];
            float rain_rate_eu = (float)rain_rate_dn * 0.1;

            // rain.apts
            if (rain_rate_dn <= 250)
            {
                fprintf(vs_ofp, "%g %g\n", rain_rate_eu, dif);
            }

            fprintf(bd_ofp, "%g %g %g\n", lon_deg, lat_deg, dif);
            fprintf(bds_ofp, "%g %g %g\n", lon_deg, lat_deg, 0.25 * dif +
                0.04 * mem_spd);
        }
        for (int cell_idx = 0; cell_idx < MAX_CROSS_TRACK; cell_idx++)
        {
            MeasList* meas_list = &(meas_list_row[cell_idx]);
            meas_list->FreeContents();
        }
    }

    //-------------//
    // close files //
    //-------------//

    fclose(vs_ofp);
    fclose(bd_ofp);
    fclose(bds_ofp);

    l2a_file.CloseParamDatasets(row_number_p);
    l2a_file.CloseParamDatasets(num_sigma0_p);
    l2a_file.CloseParamDatasets(cell_lat_p);
    l2a_file.CloseParamDatasets(cell_lon_p);
    l2a_file.CloseParamDatasets(cell_azimuth_p);
    l2a_file.CloseParamDatasets(cell_incidence_p);
    l2a_file.CloseParamDatasets(sigma0_p);
    l2a_file.CloseParamDatasets(sigma0_qual_flag_p);
    l2a_file.CloseParamDatasets(sigma0_mode_flag_p);
    l2a_file.CloseParamDatasets(surface_flag_p);
    l2a_file.CloseParamDatasets(cell_index_p);
    l2a_file.CloseParamDatasets(sigma0_attn_map_p);

    return (0);
}

//--------------//
// check_status //
//--------------//

void
check_status(
    HdfFile::StatusE  status)
{
    if (status != HdfFile::OK)
    {
        fprintf(stderr,
            "Error opening dataset.  No, I don't know which one.\n");
        exit(1);
    }
    return;
}
