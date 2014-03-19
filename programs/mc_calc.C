//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mc_calc
//
// SYNOPSIS
//    mc_calc <ins_config_file> <hdf_l2a_file>
//        <hdf_l2b_file> <output_mc_file>
//
// DESCRIPTION
//    Generates an output file with the following binary records:
//      ati, cti, beam_idx, s0_true, s0_meas
//      short char char      float    float
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>  The config file (for the GMF).
//    <hdf_l2a_file>     The input HDF L2A file.
//    <hdf_l2b_file>     The input HDF L2B file.
//    <output_mc_file>   The output file.
//
// EXAMPLES
//    An example of a command line is:
//      % mc_calc $cfg l2a.203 l2b.203 203.mc
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

const char* usage_array[] = { "<ins_config_file>", "<hdf_l2a_file>",
    "<hdf_l2b_file>", "<output_mc_file>", 0 };

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
    const char* output_mc_file = argv[optind++];

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
    if (! l2b.ReadHDF(UNNORMALIZE_MLE_FLAG))
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

    char* leap_second_table_string =
        ea_config_list.Get(LEAP_SECOND_TABLE_KEYWORD);
    if (Itime::CreateLeapSecTable(leap_second_table_string) == 0)
    {
        fprintf(stderr, "%s: error creating leap second table %s\n",
            command, leap_second_table_string);
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
    Parameter* cell_azimuth_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_AZIMUTH, UNIT_DEGREES);
    Parameter* cell_incidence_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INCIDENCE, UNIT_RADIANS);
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
    check_status(l2a_file.OpenParamDatasets(sigma0_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_attn_map_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_qual_flag_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_mode_flag_p));
    check_status(l2a_file.OpenParamDatasets(surface_flag_p));
    check_status(l2a_file.OpenParamDatasets(cell_index_p));

    int l2a_length = l2a_file.GetDataLength();

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(output_mc_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_mc_file);
        exit(1);
    }

    //---------------//
    // for each cell //
    //---------------//

    short row_number = 0;
    short num_sigma0;
    float cell_azimuth[MAX_S0_PER_ROW];
    float cell_incidence[MAX_S0_PER_ROW];
    float sigma0[MAX_S0_PER_ROW];
    float sigma0_attn_map[MAX_S0_PER_ROW];
    unsigned short sigma0_qual_flag[MAX_S0_PER_ROW];
    unsigned short sigma0_mode_flag[MAX_S0_PER_ROW];
    unsigned short surface_flag[MAX_S0_PER_ROW];
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

        surface_flag_p->extractFunc(&l2a_file, surface_flag_p->sdsIDs, idx,
            1, 1, surface_flag, polyTable);

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

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->nudgeWV;
            if (wvp == NULL)
                continue;

            //-----------//
            // calculate //
            //-----------//

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                float spd = wvp->spd;
                float chi = wvp->dir - meas->eastAzimuth + pi;
                float value;
                gmf.GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                    spd, chi, &value);

                // eliminate very small true sigma-0's
                if (value < 1E-6)
                    continue;

                float x = value;          // the "right" sigma-0
                float y = meas->value;    // the measured sigma-0
                int beam_idx;
                if (meas->measType == Meas::HH_MEAS_TYPE)
                    beam_idx = 0;
                else
                    beam_idx = 1;

                unsigned short ati_short = (unsigned short) ati;
                unsigned char cti_char = (unsigned char) cti;
                unsigned char beam_char = (unsigned char) beam_idx;

                fwrite(&ati_short, sizeof(unsigned short), 1, ofp);
                fwrite(&cti_char, sizeof(unsigned char), 1, ofp);
                fwrite(&beam_char, sizeof(unsigned char), 1, ofp);
                fwrite(&x, sizeof(float), 1, ofp);
                fwrite(&y, sizeof(float), 1, ofp);
            }
//          wvc.FreeContents();

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

    //--------------//
    // write arrays //
    //--------------//

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
