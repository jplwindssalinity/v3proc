//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_flag
//
// SYNOPSIS
//    mudh_flag [ -t threshold ] <ins_config_file> <classtab_file>
//        <hdf_l2a_file> <hdf_l2b_file> <output_flag_file>
//
// DESCRIPTION
//    Performs MUDH classification and writes out two arrays:
//      A floating point MUDH index array and a byte flag array.
//      The byte flag array has the following meanings:
//      0 = classified as no rain
//      1 = classified as rain
//      2 = unclassifiable
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>   The config file (for the GMF).
//    <classtab_file>     The classification file.
//    <hdf_l2a_file>      The input HDF L2A file.
//    <hdf_l2b_file>      The input HDF L2B file.
//    <output_flag_file>  The output flag file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_flag $cfg class.tab l2a.203 l2b.203 203.flag
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
#include "L2AHdf.h"
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

#define OPTSTRING        "t:"
#define MAX_S0_PER_ROW   3240
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

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

const char* usage_array[] = { "[ -t threshold ]", "<ins_config_file>",
    "<classtab_file>", "<hdf_l2a_file>", "<hdf_l2b_file>",
    "<output_flag_file>", 0 };

static double         classtab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static float          index_tab[AT_WIDTH][CT_WIDTH];
static unsigned char  flag_tab[AT_WIDTH][CT_WIDTH];

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

    float threshold = 0.5;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 't':
            threshold = atof(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 5)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* classtab_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    const char* l2b_hdf_file = argv[optind++];
    const char* output_flag_file = argv[optind++];

    //--------------//
    // simple calcs //
    //--------------//

    float nbd_spread = NBD_MAX - NBD_MIN;
    int max_inbd = NBD_DIM - 2;    // save room for missing NBD index

    float spd_spread = SPD_MAX - SPD_MIN;
    int max_ispd = SPD_DIM - 1;

    float dir_spread = DIR_MAX - DIR_MIN;
    int max_idir = DIR_DIM - 1;

    float mle_spread = MLE_MAX - MLE_MIN;
    int max_imle = MLE_DIM - 1;

    //--------------------//
    // read classtab file //
    //--------------------//

    FILE* classtab_ifp = fopen(classtab_file, "r");
    if (classtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening classtab file %s\n", command,
            classtab_file);
        exit(1);
    }
    unsigned long size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;
    if (fread(classtab, sizeof(double), size, classtab_ifp) != size)
    {
        fprintf(stderr, "%s: error reading classtab file %s\n", command,
            classtab_file);
        exit(1);
    }
    fclose(classtab_ifp);

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

    FILE* ofp = fopen(output_flag_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_flag_file);
        exit(1);
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

    MeasList meas_list_row[CT_WIDTH];

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

        surface_flag_p->extractFunc(&l2a_file, surface_flag_p->sdsIDs, idx,
            1, 1, surface_flag, polyTable);

        cell_lon_p->extractFunc(&l2a_file, cell_lon_p->sdsIDs, idx, 1, 1,
            cell_lon, polyTable);

        cell_lat_p->extractFunc(&l2a_file, cell_lat_p->sdsIDs, idx, 1, 1,
            cell_lat, polyTable);

        cell_azimuth_p->extractFunc(&l2a_file, cell_azimuth_p->sdsIDs, idx,
            1, 1, cell_azimuth, polyTable);

        cell_incidence_p->extractFunc(&l2a_file, cell_incidence_p->sdsIDs,
            idx, 1, 1, cell_incidence, polyTable);

        kp_alpha_p->extractFunc(&l2a_file, kp_alpha_p->sdsIDs, idx, 1, 1,
            kp_alpha, polyTable);

        kp_beta_p->extractFunc(&l2a_file, kp_beta_p->sdsIDs, idx, 1, 1,
            kp_beta, polyTable);

        kp_gamma_p->extractFunc(&l2a_file, kp_gamma_p->sdsIDs, idx, 1, 1,
            kp_gamma, polyTable);

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
            int land_flag = surface_flag[j] & 0x00000001;
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
                new_meas->scanAngle = 1.0;    // aft

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

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->ambiguities.GetHead();
            if (wvp == NULL)
                continue;

            float spd = wvp->spd;
            float dir = wvp->dir;
            float mle = wvp->obj;

            //-----------------------------//
            // convert speed to MUDH index //
            //-----------------------------//

            int ispd = (int)((spd - SPD_MIN) * (float)max_ispd /
                spd_spread + 0.5);
            if (ispd < 0) ispd = 0;
            if (ispd > max_ispd) ispd = max_ispd;

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
                double nbd = mean_dif / sub_std;

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
            if (count[1][0] > 0 && count[1][1] > 0)
            {
                // calculate target angle
                for (int i = 0; i < 2; i++)    // fore/aft
                {
                    x_outer_comp_sum[i] /= (double)count[1][i];
                    y_outer_comp_sum[i] /= (double)count[1][i];
                }
/*
                float target_angle =
                    atan2(y_outer_comp_sum[0] + y_outer_comp_sum[1],
                    x_outer_comp_sum[0] + x_outer_comp_sum[1]);
*/
                float target_angle =
                    (atan2(y_outer_comp_sum[0], x_outer_comp_sum[0]) +
                     atan2(y_outer_comp_sum[1], x_outer_comp_sum[1])) / 2.0;

                float dir_val = ANGDIF(target_angle, dir);

                // make it with respect to along track angle
                if (dir_val > pi_over_two)
                    dir_val = dir_val - pi_over_two;
                else
                    dir_val = pi_over_two - dir_val;

                // convert to degrees
                dir_val *= rtd;

                idir = (int)((dir - DIR_MIN) * (float)max_idir /
                    dir_spread + 0.5);
                if (idir < 0) idir = 0;
                if (idir > max_idir) idir = max_idir;
            }

            //------------------------------------------------------//
            // look up the value from the rain classification table //
            //------------------------------------------------------//

            if (idir == -1)
            {
                // couldn't calculate direction index; rain flag not available
                index_tab[ati][cti] = 0.0;
                flag_tab[ati][cti] = 2;
            }
            float class_value = classtab[inbd][ispd][idir][imle];
            index_tab[ati][cti] = class_value;

            if (class_value < -2.5)
            {
                // -3 is used as a flag
                flag_tab[ati][cti] = 2;
            }

            // threshold
            if (class_value > threshold)
                flag_tab[ati][cti] = 1;
        }
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* meas_list = &(meas_list_row[cti]);
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

    //--------------//
    // write arrays //
    //--------------//

    fwrite(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ofp);
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
