//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    wr_nbd
//
// SYNOPSIS
//    wr_nbd <ins_config_file> <hdf_l2a_file> <hdf_l2b_file>
//        <output_nbd_file>
//
// DESCRIPTION
//    Calculates the normalized beam difference and stores it in
//    a char array.
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>  The config file (for the GMF).
//    <hdf_l2a_file>     The input HDF L2A file.
//    <hdf_l2b_file>     The input HDF L2B file.
//    <output_nbd_file>  The output file.
//
// EXAMPLES
//    An example of a command line is:
//      % wr_nbd $cfg l2a.203 l2b.203 203.nbd
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
#define NBD_SCALE        10.0

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

const char* usage_array[] = { "<ins_config_file>", "<hdf_l2a_file>",
    "<hdf_l2b_file>", "<output_nbd_file>", 0 };

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
    const char* l2a_hdf_file = argv[optind++];
    const char* l2b_hdf_file = argv[optind++];
    const char* output_nbd_file = argv[optind++];

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

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //-------------------//
    // read the l2b file //
    //-------------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_hdf_file);
    if (! l2b.ReadHDF())
    {
        fprintf(stderr, "%s: error reading L2B file %s\n", command,
            l2b_hdf_file);
        exit(1);
    }
    WindSwath* swath = &(l2b.frame.swath);

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

    FILE* ofp = fopen(output_nbd_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_nbd_file);
        exit(1);
    }

    //--------------------------//
    // initialize the nbd array //
    //--------------------------//

    char nbd_array[1624][76];
    for (int i = 0; i < 1624; i++)
    {
        for (int j = 0; j < 76; j++)
        {
            nbd_array[i][j] = -128;
        }
    }

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

    for (int target_row = 1; target_row <= 1624; target_row++)
    {
        int target_idx = target_row - 1;

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

                //--------------//
                // what the...? //
                //--------------//

                int cell_idx = cell_index[j] - 1;
                if (cell_idx < 0 || cell_idx >= MAX_CROSS_TRACK)
                {
                    continue;
                }

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
/*
                if (sigma0_mode_flag[j] & 0x0008)
                    new_meas->scanAngle = 0.0;    // fore
                else
                    new_meas->scanAngle = 1.0;    // aft
*/

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
/*
            float lon_deg = avg_lon_lat.longitude * rtd;
            float lat_deg = avg_lon_lat.latitude * rtd;

            // wind retrieval
            WVC wvc;
            if (! gmf.RetrieveWinds_GS(meas_list, &kp, &wvc))
                continue;
*/

            WVC* wvc = swath->GetWVC(cell_idx, target_idx);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->ambiguities.GetHead();
            if (wvp == NULL)
                continue;
/*
            float mem_spd = wvp->spd;
            float mem_dir = wvp->dir;
*/

            //-----------------//
            // accumulate info //
            //-----------------//

            double norm_dif_sum[2];       // beam
            int count[2];
/*
            double x_comp_sum[2][2];    // beam, fore/aft
            double y_comp_sum[2][2];    // beam, fore/aft
            double comp_count[2][2];    // beam, fore/aft
*/
            for (int i = 0; i < 2; i++)
            {
                norm_dif_sum[i] = 0.0;
                count[i] = 0;
/*
                for (int j = 0; j < 2; j++)
                {
                    x_comp_sum[i][j] = 0.0;
                    y_comp_sum[i][j] = 0.0;
                    comp_count[i][j] = 0;
                }
*/
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

                float kpm = 0.16;
                float use_alpha = (1.0 + kpm*kpm) * meas->A - 1.0;
                float var = (use_alpha * value + meas->B) * value + meas->C;
                float std_dev = sqrt(var);

                float norm_dif = (meas->value - value) / std_dev;

                norm_dif_sum[meas->beamIdx] += norm_dif;
                count[meas->beamIdx]++;

/*
                int foreaft_idx = (int)(meas->scanAngle + 0.5);
                if (foreaft_idx != 0)
                    foreaft_idx = 1;

                x_comp_sum[meas->beamIdx][foreaft_idx] += 
                    cos(meas->eastAzimuth);
                y_comp_sum[meas->beamIdx][foreaft_idx] += 
                    sin(meas->eastAzimuth);
                comp_count[meas->beamIdx][foreaft_idx]++;
*/
            }
//            wvc.FreeContents();

/*
            if (comp_count[0][0] == 0 || comp_count[0][1] == 0 ||
                comp_count[1][0] == 0 || comp_count[1][1] == 0)
            {
                continue;
            }

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
*/

            //-------------------------------------//
            // calculate the mean ratio difference //
            //-------------------------------------//

            if (count[0] == 0 || count[1] == 0)
                continue;

            float mean_norm_dif[2];
            for (int i = 0; i < 2; i++)
            {
                mean_norm_dif[i] = norm_dif_sum[i] / count[i];
            }
            float mean_dif = mean_norm_dif[0] - mean_norm_dif[1];

            //---------------------------------------------------------//
            // calculate the expected std of the mean ratio difference //
            //---------------------------------------------------------//

            double sub_std = sqrt(1.0 / (double)count[0] +
                1.0 / (double)count[1]);
            double nbd = mean_dif / sub_std;

            //----------------------------------//
            // calculate the angular difference //
            //----------------------------------//

/*
            float dir_val = ANGDIF(target_angle[1], mem_dir);
            if (dir_val > pi_over_two)
                dir_val = pi - dir_val;
            dir_val = pi_over_two - dir_val;
            dir_val *= rtd;
*/

            //--------------//
            // put in array //
            //--------------//

            nbd = floor(nbd * NBD_SCALE + 0.5);
            if (nbd < -127.0)
                nbd = -127.0;
            if (nbd > 127.0)
                nbd = 127.0;
            
            nbd_array[target_idx][cell_idx] = (char)nbd;
        }
        for (int cell_idx = 0; cell_idx < MAX_CROSS_TRACK; cell_idx++)
        {
            MeasList* meas_list = &(meas_list_row[cell_idx]);
            meas_list->FreeContents();
        }
    }

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

    //-------------//
    // write array //
    //-------------//

    fwrite(nbd_array, sizeof(char), 76 * 1624, ofp);
    fclose(ofp);

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
