//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mc_wr
//
// SYNOPSIS
//    mc_wr <ins_config_file> <mctab> <hdf_l2a_file> <wr1_base>
//        <wr2_base>
//
// DESCRIPTION
//    Processes an HDF L2A file to winds.  Then, reprocesses using
//    corrected sigma-0's.
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>  The config file (for the GMF).
//    <mctab>            The MUDH correction table.
//    <hdf_l2a_file>     The input HDF L2A file.
//    <wr1_base>         The original wind retrieval output vector base.
//    <wr2_base>         The corrected wind retrieval output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mc_wr $cfg mctab.dat l2a.203 203.1 203.2
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

#define OPTSTRING        ""
#define MAX_S0_PER_ROW   3240
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

#define UNNORMALIZE_MLE_FLAG  0    // leave the mle alone

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

const char* usage_array[] = { "<ins_config_file>", "<mctab>",
    "<hdf_l2a_file>", "<wr1_base>", "<wr2_base>", 0 };

static double m[2][NBD_DIM][DIR_DIM][MLE_DIM];
static double b[2][NBD_DIM][DIR_DIM][MLE_DIM];
static double mb_count[2][NBD_DIM][DIR_DIM][MLE_DIM];

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

    if (argc < optind + 5)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* mctab_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    const char* wr1_base = argv[optind++];
    const char* wr2_base = argv[optind++];

    //--------------//
    // simple calcs //
    //--------------//

    float nbd_spread = NBD_MAX - NBD_MIN;
    int max_inbd = NBD_DIM - 2;    // save room for missing NBD index

    float dir_spread = DIR_MAX - DIR_MIN;
    int max_idir = DIR_DIM - 1;

    float mle_spread = MLE_MAX - MLE_MIN;
    int max_imle = MLE_DIM - 1;

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

    char* leap_second_table_string =
        ea_config_list.Get(LEAP_SECOND_TABLE_KEYWORD);
    if (Itime::CreateLeapSecTable(leap_second_table_string) == 0)
    {
        fprintf(stderr, "%s: error creating leap second table %s\n",
            command, leap_second_table_string);
        exit(1);
    }

    //---------------------//
    // read the mctab file //
    //---------------------//

    unsigned int size = 2 * NBD_DIM * DIR_DIM * MLE_DIM;
    FILE* mctab_ifp = fopen(mctab_file, "r");
    if (mctab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mctab file %s\n",
            command, mctab_file);
        exit(1);
    }
    if (fread(m, sizeof(double), size, mctab_ifp) != size ||
        fread(b, sizeof(double), size, mctab_ifp) != size ||
        fread(mb_count, sizeof(double), size, mctab_ifp) != size)
    {
        fprintf(stderr, "%s: error reading mctab file %s\n", command,
            mctab_file);
        exit(1);
    }
    fclose(mctab_ifp);

    //------------------------//
    // create the wind swaths //
    //------------------------//

    WindSwath wind_swath_1;
    wind_swath_1.Allocate(CT_WIDTH, AT_WIDTH);

    WindSwath wind_swath_2;
    wind_swath_2.Allocate(CT_WIDTH, AT_WIDTH);

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
    check_status(l2a_file.OpenParamDatasets(cell_azimuth_p));
    check_status(l2a_file.OpenParamDatasets(cell_incidence_p));
    check_status(l2a_file.OpenParamDatasets(kp_alpha_p));
    check_status(l2a_file.OpenParamDatasets(kp_beta_p));
    check_status(l2a_file.OpenParamDatasets(kp_gamma_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_attn_map_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_qual_flag_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_mode_flag_p));
    check_status(l2a_file.OpenParamDatasets(surface_flag_p));
    check_status(l2a_file.OpenParamDatasets(cell_lat_p));
    check_status(l2a_file.OpenParamDatasets(cell_lon_p));
    check_status(l2a_file.OpenParamDatasets(cell_index_p));

    int l2a_length = l2a_file.GetDataLength();

    //---------------//
    // for each cell //
    //---------------//

    short row_number = 0;
    short num_sigma0;
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
    float cell_lat[MAX_S0_PER_ROW];
    float cell_lon[MAX_S0_PER_ROW];
    char cell_index[MAX_S0_PER_ROW];

    MeasList  meas_list_row[CT_WIDTH];

    for (int idx = 0; idx < l2a_length; idx++)
    {
        //--------------------//
        // extract parameters //
        //--------------------//

        row_number_p->extractFunc(&l2a_file, row_number_p->sdsIDs, idx, 1,
            1, &row_number, polyTable);
        int ati = row_number - 1;
        if (ati < 0 || ati >= AT_WIDTH)
            continue;

        num_sigma0_p->extractFunc(&l2a_file, num_sigma0_p->sdsIDs, idx, 1,
            1, &num_sigma0, polyTable);

        cell_index_p->extractFunc(&l2a_file, cell_index_p->sdsIDs, idx, 1,
            1, cell_index, polyTable);

        sigma0_mode_flag_p->extractFunc(&l2a_file,
            sigma0_mode_flag_p->sdsIDs, idx, 1, 1, sigma0_mode_flag,
            polyTable);

        sigma0_qual_flag_p->extractFunc(&l2a_file,
            sigma0_qual_flag_p->sdsIDs, idx, 1, 1, sigma0_qual_flag,
            polyTable);

        sigma0_p->extractFunc(&l2a_file, sigma0_p->sdsIDs, idx, 1, 1,
            sigma0, polyTable);

        kp_alpha_p->extractFunc(&l2a_file, kp_alpha_p->sdsIDs,
            idx, 1, 1, kp_alpha, polyTable);

        kp_beta_p->extractFunc(&l2a_file, kp_beta_p->sdsIDs,
            idx, 1, 1, kp_beta, polyTable);

        kp_gamma_p->extractFunc(&l2a_file, kp_gamma_p->sdsIDs,
            idx, 1, 1, kp_gamma, polyTable);

        surface_flag_p->extractFunc(&l2a_file, surface_flag_p->sdsIDs, idx,
            1, 1, surface_flag, polyTable);

        cell_lon_p->extractFunc(&l2a_file, cell_lon_p->sdsIDs,
            idx, 1, 1, cell_lon, polyTable);

        cell_lat_p->extractFunc(&l2a_file, cell_lat_p->sdsIDs,
            idx, 1, 1, cell_lat, polyTable);

        cell_azimuth_p->extractFunc(&l2a_file, cell_azimuth_p->sdsIDs, idx,
            1, 1, cell_azimuth, polyTable);

        cell_incidence_p->extractFunc(&l2a_file, cell_incidence_p->sdsIDs,
            idx, 1, 1, cell_incidence, polyTable);

        sigma0_attn_map_p->extractFunc(&l2a_file, sigma0_attn_map_p->sdsIDs,
            idx, 1, 1, sigma0_attn_map, polyTable);

        //-----------------------//
        // assemble measurements //
        //-----------------------//

        for (int j = 0; j < num_sigma0; j++)
        {
            if (sigma0_qual_flag[j] & UNUSABLE)
                continue;

            // skip land
//            int land_flag = surface_flag[j] & 0x00000001;
            int land_flag = surface_flag[j];
            if (land_flag)
                continue;

            //--------------//
            // what the...? //
            //--------------//

            int cti = cell_index[j] - 1;
            if (cti < 0 || cti >= CT_WIDTH)
            {
                continue;
            }

            Meas* new_meas = new Meas;
            new_meas->incidenceAngle = cell_incidence[j];
            new_meas->value = sigma0[j] + sigma0_attn_map[j] /
                cos(new_meas->incidenceAngle);
            new_meas->value = pow(10.0, 0.1 * new_meas->value);
            if (sigma0_qual_flag[j] & NEGATIVE)
                new_meas->value = -(new_meas->value);
            new_meas->landFlag = (int)(surface_flag[j] & 0x00000001);
            new_meas->centroid.SetAltLonGDLat(0.0, cell_lon[j], cell_lat[j]);
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
                new_meas->scanAngle = 3.0;    // aft

            if (! meas_list_row[cti].Append(new_meas))
            {
                fprintf(stderr, "%s: error appending measurement\n",
                    command);
                exit(1);
            }
        }

        //-----------------------------//
        // got everything for this row //
        //-----------------------------//

        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* meas_list = &(meas_list_row[cti]);
            if (meas_list->NodeCount() == 0)
                continue;

            LonLat avg_lon_lat = meas_list->AverageLonLat();

            // wind retrieval
            WVC* wvc = new WVC();
            if (! gmf.RetrieveWinds_GS(meas_list, &kp, wvc))
            {
                delete wvc;
                continue;
            }

            wind_swath_1.Add(cti, ati, wvc);

            WindVectorPlus* wvp = wvc->ambiguities.GetHead();
            if (wvp == NULL)
                continue;

            float spd = wvp->spd;
            float dir = wvp->dir;
            float mle = wvp->obj;

            //---------------------------//
            // convert MLE to MUDH index //
            //---------------------------//

            int imle = (int)((mle - MLE_MIN) * (float)max_imle /
                mle_spread + 0.5);
            if (imle < 0) imle = 0;
            if (imle > max_imle) imle = max_imle;

            //--------------//
            // clear arrays //
            //--------------//

            double norm_dif_sum[2];        // beam
            double x_outer_comp_sum[2];    // fore/aft
            double y_outer_comp_sum[2];    // fore/aft
            unsigned int count[2][2];      // beam, fore/aft
            for (int i = 0; i < 2; i++)
            {
                norm_dif_sum[i] = 0.0;
                x_outer_comp_sum[i] = 0.0;
                y_outer_comp_sum[i] = 0.0;
                for (int j = 0; j < 2; j++)
                {
                    count[i][j] = 0;
                }
            }

            //-----------//
            // calculate //
            //-----------//

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                float chi = wvp->dir - meas->eastAzimuth + pi;
                float value;
                gmf.GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                    spd, chi, &value);

                // eliminate very small true sigma-0's
                if (value < 1E-6)
                    continue;

                //------------------//
                // NBD calculations //
                //------------------//

                float kpm = 0.16;
                float use_alpha = (1.0 + kpm*kpm) * meas->A - 1.0;
                float var = (use_alpha * value + meas->B) * value + meas->C;
                float std_dev = sqrt(var);

                float norm_dif = (meas->value - value) / std_dev;
                norm_dif_sum[meas->beamIdx] += norm_dif;

                // hack out fore/aft index
                int foreaft_idx = (int)(meas->scanAngle + 0.5);
                if (foreaft_idx != 0)
                    foreaft_idx = 1;

                //----------------------------------//
                // direction parameter calculations //
                //----------------------------------//

                if (meas->beamIdx == 1)    // outer beam
                {
                    x_outer_comp_sum[foreaft_idx] += cos(meas->eastAzimuth);
                    y_outer_comp_sum[foreaft_idx] += sin(meas->eastAzimuth);
                }
                count[meas->beamIdx][foreaft_idx]++;
            }

            //-----------------------------------------//
            // calculate NBD and convert to MUDH index //
            //-----------------------------------------//
            // Requires fore and aft looks from both beams, otherwise NBD is
            // considered not available (even though it could be calcalated)

            int inbd = max_inbd + 1;    // index for "No NBD available"
            double nbd = 0.0;
            if (count[0][0] > 0 && count[0][1] > 0 &&
                count[1][0] > 0 && count[1][1] > 0)
            {
                unsigned int beam_count[2];    // beam
                // inner beam (fore + aft)
                beam_count[0] = count[0][0] + count[0][1];
                // outer beam (fore + aft)
                beam_count[1] = count[1][0] + count[1][1];

                float mean_norm_dif[2];    // beam
                for (int beam_idx = 0; beam_idx < 2; beam_idx++)
                {
                    mean_norm_dif[beam_idx] = norm_dif_sum[beam_idx] /
                        (double)beam_count[beam_idx];
                }
                float mean_dif = mean_norm_dif[0] - mean_norm_dif[1];

                // calculate the expected std of the mean ratio difference
                double sub_std = sqrt(1.0 / (double)beam_count[0] +
                    1.0 / (double)beam_count[1]);
                nbd = mean_dif / sub_std;

                inbd = (int)((nbd - NBD_MIN) * (float)max_inbd /
                    nbd_spread + 0.5);
                if (inbd < 0) inbd = 0;
                if (inbd > max_inbd) inbd = max_inbd;
            }

            //---------------------------------------------------------//
            // calculate direction parameter and convert to MUDH index //
            //---------------------------------------------------------//
            // use outer beam to determine cross track direction
            // need both fore and aft measurements

            int idir = -1;
            float dir_val = 0.0;
            if (count[1][0] > 0 && count[1][1] > 0)
            {
                // calculate target angle
                for (int i = 0; i < 2; i++)    // fore/aft
                {
                    x_outer_comp_sum[i] /= (double)count[1][i];
                    y_outer_comp_sum[i] /= (double)count[1][i];
                }
                float target_angle =
                    atan2(y_outer_comp_sum[0] + y_outer_comp_sum[1],
                    x_outer_comp_sum[0] + x_outer_comp_sum[1]);

                dir_val = ANGDIF(target_angle, dir);

                // make it with respect to along track angle
                if (dir_val > pi_over_two)
                    dir_val = dir_val - pi_over_two;
                else
                    dir_val = pi_over_two - dir_val;

                // convert to degrees
                dir_val *= rtd;

                idir = (int)((dir_val - DIR_MIN) * (float)max_idir /
                    dir_spread + 0.5);
                if (idir < 0) idir = 0;
                if (idir > max_idir) idir = max_idir;
            }

            //--------------//
            // correct s0's //
            //--------------//

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                int beam_idx;
                if (meas->measType == Meas::HH_MEAS_TYPE)
                    beam_idx = 0;
                else
                    beam_idx = 1;

                double use_m = m[beam_idx][inbd][idir][imle];
                if (use_m <= 0.0)
                  continue;

                double use_b = b[beam_idx][inbd][idir][imle];
                double new_value = (meas->value - use_b) / use_m;
                meas->value = new_value;
            }

            // wind retrieval again
            wvc = new WVC();
            if (! gmf.RetrieveWinds_GS(meas_list, &kp, wvc))
            {
                delete wvc;
                continue;
            }

            wind_swath_2.Add(cti, ati, wvc);
        }
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* meas_list = &(meas_list_row[cti]);
            meas_list->FreeContents();
        }
    }

    l2a_file.CloseParamDatasets(row_number_p);
    l2a_file.CloseParamDatasets(num_sigma0_p);
    l2a_file.CloseParamDatasets(cell_azimuth_p);
    l2a_file.CloseParamDatasets(cell_incidence_p);
    l2a_file.CloseParamDatasets(sigma0_p);
    l2a_file.CloseParamDatasets(sigma0_qual_flag_p);
    l2a_file.CloseParamDatasets(sigma0_mode_flag_p);
    l2a_file.CloseParamDatasets(surface_flag_p);
    l2a_file.CloseParamDatasets(cell_index_p);
    l2a_file.CloseParamDatasets(sigma0_attn_map_p);

    //------------------//
    // write vctr files //
    //------------------//

    int max_rank_1 = wind_swath_1.GetMaxAmbiguityCount();
    char filename[1024];
    for (int i = 0; i <= max_rank_1; i++)
    {
        sprintf(filename, "%s.%d", wr1_base, i);
        if (! wind_swath_1.WriteVctr(filename, i))
        {
            fprintf(stderr, "%s: error writing vctr file %s\n", command,
                filename);
            exit(1);
        }
    }

    int max_rank_2 = wind_swath_2.GetMaxAmbiguityCount();
    for (int i = 0; i <= max_rank_2; i++)
    {
        sprintf(filename, "%s.%d", wr2_base, i);
        if (! wind_swath_2.WriteVctr(filename, i))
        {
            fprintf(stderr, "%s: error writing vctr file %s\n", command,
                filename);
            exit(1);
        }
    }

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
