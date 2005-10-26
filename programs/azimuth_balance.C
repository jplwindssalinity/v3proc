//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    azimuth_balance
//
// SYNOPSIS
//    azimuth_balance <ins_config_file> <l1b_file> <l2b_file>
//        <output_file>
//        
// DESCRIPTION
//    Reads HDF L1B and L2B files and generates an azimuth balance
//    output file containing an azimuth angle (binned at 1 deg)
//    and a relative sigma-0, i.e. sigma-0_meas / sigma-0_calc, where
//    sigma-0_calc is calculated from the corresponding nudged field
//    wind vector.
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>  Use the specified instrument configuration file.
//    <l1b_file>         Analyze the specified l1b_file.
//    <l2b_file>         Analyze the specified l2b_file.
//    <output_file>      Results go here.
//
// EXAMPLES
//    azimuth_balance qscat.cfg l1b.dat l2b.dat az.bal
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

#include <assert.h>
#include <unistd.h>
#include "L1AExtract.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "L1AFile.h"
#include "L1BHdfFile.h"

#include <stdio.h>
#include <fcntl.h>
#include "Args.h"
#include "ArgDefs.h"
#include "ArgsPlus.h"
#include "ParTab.h"
#include "Filter.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "ConfigList.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "InstrumentGeom.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "Qscat.h"
#include "LandMap.h"
#include "SeaPac.h"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<Meas>;
template class List<AngleInterval>;
template class List<OffsetList>;
template class List<long>;
template class List<MeasSpot>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

#define WOM  0x0E
#define UNUSABLE   0x0001
#define NEGATIVE   0x0004

#define NWP_SPEED_CORRECTION  0.84

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void   check_status(HdfFile::StatusE status);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo leap_second_table_arg = LEAP_SECOND_TABLE_ARG;

const char* usage_array[] = { "<ins_config_file>", "<l1b_file>",
    "<l2b_file>", "<output_file>", 0 };

double sigma0_sum[2][180];
int sigma0_count[2][180];

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
    extern int optind;
    extern char *optarg;
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

    if (argc != optind + 4)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* l1b_filename = argv[optind++];
    const char* l2b_filename = argv[optind++];
    const char* output_file = argv[optind++];

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
    char* leap_second_table_string =
        ea_config_list.Get(LEAP_SECOND_TABLE_KEYWORD);

    //---------------//
    // use arguments //
    //---------------//

    EA_PolynomialErrorNo poly_status = EA_POLY_OK;
    PolynomialTable* polyTable = new PolynomialTable(poly_table_string,
        poly_status);
    if ( poly_status != EA_POLY_OK)
    {
        fprintf(stderr, "%s: error creating polynomial table from %s\n",
            command, poly_table_string);
        exit(1);
    }

    if (Itime::CreateLeapSecTable(leap_second_table_string) == 0)
    {
        fprintf(stderr, "%s: error creating leap second table %s\n",
            command, leap_second_table_string);
        exit(1);
    }

    //--------------------------------//
    // read in instrument config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(ins_config_file))
    {
        fprintf(stderr, "%s: error reading instrument configuration file %s\n",
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

    //--------------------//
    // configure land map //
    //--------------------//

    char* land_map_file = config_list.Get(SIMPLE_LANDMAP_FILE_KEYWORD);
    SimpleLandMap land_map;
    if (! land_map.Read(land_map_file))
    {
        fprintf(stderr, "%s: error reading land map %s\n", command,
            land_map_file);
        exit(1);
    }

    //-----------------//
    // FOR LEVEL 1B... //
    //-----------------//

    Parameter* frame_err_status_p = ParTabAccess::GetParameter(SOURCE_L1B,
        FRAME_ERR_STATUS, UNIT_DN);
    Parameter* cell_lat_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_LAT, UNIT_RADIANS);
    Parameter* cell_lon_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_LON, UNIT_RADIANS);
    Parameter* cell_sigma0_p = ParTabAccess::GetParameter(SOURCE_L1B,
        SIGMA0, UNIT_DB);
    Parameter* sigma0_qual_flag_p = ParTabAccess::GetParameter(SOURCE_L1B,
        SIGMA0_QUAL_FLAG, UNIT_DN);
    Parameter* cell_azimuth_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_AZIMUTH, UNIT_DEGREES);
    Parameter* antenna_azimuth_p = ParTabAccess::GetParameter(SOURCE_L1B,
        ANTENNA_AZIMUTH, UNIT_DEGREES);
    Parameter* cell_incidence_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_INCIDENCE, UNIT_DEGREES);

    Parameter* x_pos_p = ParTabAccess::GetParameter(SOURCE_L1B, X_POS,
        UNIT_METERS);
    Parameter* y_pos_p = ParTabAccess::GetParameter(SOURCE_L1B, Y_POS,
        UNIT_METERS);
    Parameter* z_pos_p = ParTabAccess::GetParameter(SOURCE_L1B, Z_POS,
        UNIT_METERS);

    Parameter* x_vel_p = ParTabAccess::GetParameter(SOURCE_L1B, X_VEL,
        UNIT_MPS);
    Parameter* y_vel_p = ParTabAccess::GetParameter(SOURCE_L1B, Y_VEL,
        UNIT_MPS);
    Parameter* z_vel_p = ParTabAccess::GetParameter(SOURCE_L1B, Z_VEL,
        UNIT_MPS);

    HdfFile::StatusE status;
    L1BHdfFile l1b_file(l1b_filename, status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "%s: error opening L1B file %s\n", command,
            l1b_filename);
        exit(1);
    }

    check_status(l1b_file.OpenParamDatasets(frame_err_status_p));
    check_status(l1b_file.OpenParamDatasets(cell_lat_p));
    check_status(l1b_file.OpenParamDatasets(cell_lon_p));
    check_status(l1b_file.OpenParamDatasets(cell_sigma0_p));
    check_status(l1b_file.OpenParamDatasets(sigma0_qual_flag_p));
    check_status(l1b_file.OpenParamDatasets(cell_azimuth_p));
    check_status(l1b_file.OpenParamDatasets(antenna_azimuth_p));
    check_status(l1b_file.OpenParamDatasets(cell_incidence_p));
    check_status(l1b_file.OpenParamDatasets(x_pos_p));
    check_status(l1b_file.OpenParamDatasets(y_pos_p));
    check_status(l1b_file.OpenParamDatasets(z_pos_p));
    check_status(l1b_file.OpenParamDatasets(x_vel_p));
    check_status(l1b_file.OpenParamDatasets(y_vel_p));
    check_status(l1b_file.OpenParamDatasets(z_vel_p));

    //--------------//
    // FOR LEVEL 2B //
    //--------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_filename);
    if (! l2b.ReadHDF())
    {
        fprintf(stderr, "%s: error reading L2B file %s\n", command,
            l2b_filename);
        exit(1);
    }
    WindSwath* swath = &(l2b.frame.swath);

    //--------------------//
    // create output file //
    //--------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error creating output echo file %s\n", command,
            output_file);
        exit(1);
    }

    //------------------------//
    // process frame by frame //
    //------------------------//

    int l1b_length = l1b_file.GetDataLength();
    for (int record_idx = 0; record_idx < l1b_length; record_idx++)
    {
        //--------------//
        // get L1B info //
        //--------------//

        unsigned int frame_err_status;
        frame_err_status_p->extractFunc(&l1b_file,
            frame_err_status_p->sdsIDs, record_idx, 1, 1,
            &frame_err_status, polyTable);

/*
        // check for bad frame
        if (frame_err_status)
            continue;
*/

        float cell_lon[100];
        cell_lon_p->extractFunc(&l1b_file, cell_lon_p->sdsIDs, record_idx,
            1, 1, cell_lon, polyTable);

        float cell_lat[100];
        cell_lat_p->extractFunc(&l1b_file, cell_lat_p->sdsIDs, record_idx,
            1, 1, cell_lat, polyTable);

        float cell_sigma0[100];
        cell_sigma0_p->extractFunc(&l1b_file, cell_sigma0_p->sdsIDs,
            record_idx, 1, 1, cell_sigma0, polyTable);

        float cell_azimuth[100];
        cell_azimuth_p->extractFunc(&l1b_file, cell_azimuth_p->sdsIDs,
            record_idx, 1, 1, cell_azimuth, polyTable);

        float antenna_azimuth[100];
        antenna_azimuth_p->extractFunc(&l1b_file, antenna_azimuth_p->sdsIDs,
            record_idx, 1, 1, antenna_azimuth, polyTable);

        float cell_incidence[100];
        cell_incidence_p->extractFunc(&l1b_file, cell_incidence_p->sdsIDs,
            record_idx, 1, 1, cell_incidence, polyTable);

        unsigned short sigma0_qual_flag[100];
        sigma0_qual_flag_p->extractFunc(&l1b_file,
            sigma0_qual_flag_p->sdsIDs, record_idx, 1, 1,
            sigma0_qual_flag, polyTable);

        // check for bad frame
        if (frame_err_status)
            continue;

        float x_pos, y_pos, z_pos, x_vel, y_vel, z_vel;
        x_pos_p->extractFunc(&l1b_file, x_pos_p->sdsIDs, record_idx,
            1, 1, &x_pos, NULL);
        y_pos_p->extractFunc(&l1b_file, y_pos_p->sdsIDs, record_idx,
            1, 1, &y_pos, NULL);
        z_pos_p->extractFunc(&l1b_file, z_pos_p->sdsIDs, record_idx,
            1, 1, &z_pos, NULL);
        x_vel_p->extractFunc(&l1b_file, x_vel_p->sdsIDs, record_idx,
            1, 1, &x_vel, NULL);
        y_vel_p->extractFunc(&l1b_file, y_vel_p->sdsIDs, record_idx,
            1, 1, &y_vel, NULL);
        z_vel_p->extractFunc(&l1b_file, z_vel_p->sdsIDs, record_idx,
            1, 1, &z_vel, NULL);

        //------------------------//
        // compute orbit elements //
        //------------------------//

        double nodal_period, arg_lat, long_asc_node, orb_inclination;
        double orb_smaj_axis, orb_eccen;
        if (! compute_orbit_elements(x_pos, y_pos, z_pos, x_vel, y_vel, z_vel,
            &nodal_period, &arg_lat, &long_asc_node, &orb_inclination,
            &orb_smaj_axis, &orb_eccen))
        {
            continue;
        }

        //--------------------//
        // step through spots //
        //--------------------//

        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;

            // check usability
            if (sigma0_qual_flag[spot_idx] & UNUSABLE)
                continue;

            //--------------------//
            // only process ocean //
            //--------------------//

            float lon = cell_lon[spot_idx];
            float lat = cell_lat[spot_idx];
            if (lon == 0.0 && lat == 0.0)
                continue;

            int type = land_map.GetType(lon, lat);
            if (type != 0)    // not ocean
                continue;

            //----------------//
            // convert sigma0 //
            //----------------//

            double use_s0 = pow(10.0, 0.1 * cell_sigma0[spot_idx]);
            if (sigma0_qual_flag[spot_idx] & NEGATIVE)
            {
                use_s0 = -use_s0;
            }

            //---------------------------------------//
            // compute along and cross track indexes //
            //---------------------------------------//

            int ati, cti;
            ijbin(orb_smaj_axis, orb_eccen, orb_inclination,
                long_asc_node, arg_lat, nodal_period, lon * rtd, lat * rtd,
                &ati, &cti);

            //----------------------------//
            // determine expected sigma-0 //
            //----------------------------//

            // need the minus signs to convert to swath indicies
            WVC* wvc = swath->GetWVC(cti - 1, ati - 1);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->nudgeWV;
            if (wvp == NULL)
                continue;

            if (wvp->spd < 3.0)
                continue;

            Meas::MeasTypeE meas_type;
            if (cell_incidence[spot_idx] < 50.0)
            {
                meas_type = Meas::HH_MEAS_TYPE;
            }
            else
            {
                meas_type = Meas::VV_MEAS_TYPE;
            }
            float ccwe_az = CWNTOCCWE(cell_azimuth[spot_idx] * dtr);
            float chi = wvp->dir - ccwe_az + pi;
            float value;
            gmf.GetInterpolatedValue(meas_type, cell_incidence[spot_idx] * dtr,
                wvp->spd, chi, &value);

            int azimuth_idx = (int)(antenna_azimuth[spot_idx] / 2.0 + 0.5);
            azimuth_idx %= 180;
            sigma0_sum[beam_idx][azimuth_idx] += (use_s0 / value);
            sigma0_count[beam_idx][azimuth_idx]++;
        }
    }

    //------------------------//
    // write out sigma0 stuff //
    //------------------------//

    for (int azimuth_idx = 0; azimuth_idx < 180; azimuth_idx++)
    {
        if (sigma0_count[0][azimuth_idx] == 0 ||
            sigma0_count[1][azimuth_idx] == 0)
        {
            continue;
        }
        double v1 = sigma0_sum[0][azimuth_idx] /
            (double)sigma0_count[0][azimuth_idx];
        double v2 = sigma0_sum[1][azimuth_idx] /
            (double)sigma0_count[1][azimuth_idx];
        if (v1 < 0.0 || v2 < 0.0)
            continue;
        fprintf(ofp, "%d %g %g %d %d\n", azimuth_idx * 2, 10.0 * log10(v1),
            10.0 * log10(v2), sigma0_count[0][azimuth_idx],
            sigma0_count[1][azimuth_idx]);
    }

    //----------------//
    // close datasets //
    //----------------//

    l1b_file.CloseParamDatasets(frame_err_status_p);
    l1b_file.CloseParamDatasets(cell_lat_p);
    l1b_file.CloseParamDatasets(cell_lon_p);
    l1b_file.CloseParamDatasets(cell_sigma0_p);
    l1b_file.CloseParamDatasets(sigma0_qual_flag_p);
    l1b_file.CloseParamDatasets(cell_azimuth_p);
    l1b_file.CloseParamDatasets(antenna_azimuth_p);
    l1b_file.CloseParamDatasets(cell_incidence_p);
    l1b_file.CloseParamDatasets(x_pos_p);
    l1b_file.CloseParamDatasets(y_pos_p);
    l1b_file.CloseParamDatasets(z_pos_p);
    l1b_file.CloseParamDatasets(x_vel_p);
    l1b_file.CloseParamDatasets(y_vel_p);
    l1b_file.CloseParamDatasets(z_vel_p);

    //-------------------//
    // close output file //
    //-------------------//

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
