//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_earth_s0
//
// SYNOPSIS
//    l2a_earth_s0 <inner_file> <outer_file> <l2a_file...>
//
// DESCRIPTION
//    Reads in Level 2A files and writes out two arrays
//    of average sigma-0 versus lat and lon, one for each beam.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//    <inner_file>   The output file for the inner beam.
//    <outer_file>   The output file for the outer beam.
//    <l2a_file...>  The Level 2A input files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_earth_s0 inner.s0 outer.s0 l2a.*
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       1    Program executed successfully
//      >0    Program had an error
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
#include <math.h>
#include "mudh.h"
#include "Misc.h"
#include "L2AHdf.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "ParTab.h"
#include "Index.h"
#include "Constants.h"
#include "List.h"
#include "Tracking.h"
#include "BufferedList.h"

/*
#include "L2A.h"
*/

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class TrackerBase<unsigned char>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class List<Meas>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;

//-----------//
// CONSTANTS //
//-----------//

#define LONGITUDE_BINS  1800
#define LATITUDE_BINS   901

// HACK! These should be defined somewhere globally
#define MAX_S0_PER_ROW   3240
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void check_status(HdfFile::StatusE status);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<inner_file>", "<outer_file>", "<l2a_file...>",
    0 };

float inner_s0[LONGITUDE_BINS][LATITUDE_BINS];
int   inner_count[LONGITUDE_BINS][LATITUDE_BINS];
float outer_s0[LONGITUDE_BINS][LATITUDE_BINS];
int   outer_count[LONGITUDE_BINS][LATITUDE_BINS];

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
    if (argc < 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* inner_file = argv[clidx++];
    const char* outer_file = argv[clidx++];
    int start_idx = clidx;
    int end_idx = argc;

    //--------------------//
    // create the indices //
    //--------------------//

    Index lon_index, lat_index;
    lon_index.SpecifyWrappedCenters(0.0, two_pi, LONGITUDE_BINS);
    lat_index.SpecifyCenters(-pi_over_two, pi_over_two, LATITUDE_BINS);

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

    //--------------------//
    // get L2A parameters //
    //--------------------//

    Parameter* row_number_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        ROW_NUMBER, UNIT_DN);
    Parameter* num_sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        NUM_SIGMA0, UNIT_DN);
    Parameter* sigma0_qual_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_QUAL_FLAG, UNIT_DN);
    Parameter* cell_index_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INDEX, UNIT_DN);
    Parameter* sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0, UNIT_DB);
    Parameter* cell_incidence_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INCIDENCE, UNIT_RADIANS);
    Parameter* sigma0_attn_map_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_ATTN_MAP, UNIT_DB);
    Parameter* sigma0_mode_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_MODE_FLAG, UNIT_DN);
    Parameter* cell_lon_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LON, UNIT_RADIANS);
    Parameter* cell_lat_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LAT, UNIT_RADIANS);

    //------------------------//
    // step through L2A files //
    //------------------------//

    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        char* l2a_hdf_file = argv[file_idx];
printf("%s\n", l2a_hdf_file);

        //---------------//
        // open the file //
        //---------------//

        HdfFile::StatusE status = HdfFile::OK;
        L2AHdf l2a_file(l2a_hdf_file, SOURCE_L2Ax, status);
        if (status != HdfFile::OK)
        {
            fprintf(stderr, "%s: error opening L2A file %s\n", command,
                l2a_hdf_file);
            exit(1);
        }

        //---------------------//
        // open the parameters //
        //---------------------//

        check_status(l2a_file.OpenParamDatasets(row_number_p));
        check_status(l2a_file.OpenParamDatasets(num_sigma0_p));
        check_status(l2a_file.OpenParamDatasets(sigma0_qual_flag_p));
        check_status(l2a_file.OpenParamDatasets(cell_index_p));
        check_status(l2a_file.OpenParamDatasets(sigma0_p));
        check_status(l2a_file.OpenParamDatasets(cell_incidence_p));
        check_status(l2a_file.OpenParamDatasets(sigma0_attn_map_p));
        check_status(l2a_file.OpenParamDatasets(sigma0_mode_flag_p));
        check_status(l2a_file.OpenParamDatasets(cell_lon_p));
        check_status(l2a_file.OpenParamDatasets(cell_lat_p));

        //---------------------------//
        // determine the data length //
        //---------------------------//

        int l2a_length = l2a_file.GetDataLength();

        //--------------//
        // for each row //
        //--------------//

        for (int idx = 0; idx < l2a_length; idx++)
        {
            short row_number = 0;
            short num_sigma0 = 0;
            unsigned short sigma0_qual_flag[MAX_S0_PER_ROW];
            char cell_index[MAX_S0_PER_ROW];
            float sigma0[MAX_S0_PER_ROW];
            float cell_incidence[MAX_S0_PER_ROW];
            float sigma0_attn_map[MAX_S0_PER_ROW];
            unsigned short sigma0_mode_flag[MAX_S0_PER_ROW];
            float cell_lon[MAX_S0_PER_ROW];
            float cell_lat[MAX_S0_PER_ROW];

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

            sigma0_qual_flag_p->extractFunc(&l2a_file,
                sigma0_qual_flag_p->sdsIDs, idx, 1, 1, sigma0_qual_flag,
                polyTable);

            cell_index_p->extractFunc(&l2a_file, cell_index_p->sdsIDs, idx, 1,
                1, cell_index, polyTable);

            sigma0_p->extractFunc(&l2a_file, sigma0_p->sdsIDs, idx, 1, 1,
                sigma0, polyTable);

            cell_incidence_p->extractFunc(&l2a_file, cell_incidence_p->sdsIDs,
                idx, 1, 1, cell_incidence, polyTable);

            sigma0_attn_map_p->extractFunc(&l2a_file,
                sigma0_attn_map_p->sdsIDs, idx, 1, 1, sigma0_attn_map,
                polyTable);

            sigma0_mode_flag_p->extractFunc(&l2a_file,
                sigma0_mode_flag_p->sdsIDs, idx, 1, 1, sigma0_mode_flag,
                polyTable);

            cell_lon_p->extractFunc(&l2a_file, cell_lon_p->sdsIDs, idx, 1, 1,
                cell_lon, polyTable);

            cell_lat_p->extractFunc(&l2a_file, cell_lat_p->sdsIDs, idx, 1, 1,
                cell_lat, polyTable);

            //-----------------------//
            // assemble measurements //
            //-----------------------//

            for (int j = 0; j < num_sigma0; j++)
            {
                if (sigma0_qual_flag[j] & UNUSABLE)
                    continue;

                int cti = cell_index[j] - 1;
                if (cti < 0 || cti >= CT_WIDTH)
                {
                    continue;
                }

                Meas new_meas;
                new_meas.incidenceAngle = cell_incidence[j];
                new_meas.value = sigma0[j] + sigma0_attn_map[j] /
                    cos(new_meas.incidenceAngle);
                new_meas.value = pow(10.0, 0.1 * new_meas.value);
                if (sigma0_qual_flag[j] & NEGATIVE)
                    new_meas.value = -(new_meas.value);
                new_meas.beamIdx = (int)(sigma0_mode_flag[j] & 0x0004);
                new_meas.beamIdx >>= 2;

                float lon = cell_lon[j];
                float lat = cell_lat[j];

                //---------------------//
                // accumulate sigma-0s //
                //---------------------//

                int lon_idx;
                if (! lon_index.GetNearestIndexWrapped(lon, &lon_idx))
                    continue;

                int lat_idx;
                if (! lat_index.GetNearestIndexStrict(lat, &lat_idx))
                    continue;

                if (new_meas.beamIdx == 0)
                {
                    inner_s0[lon_idx][lat_idx] += new_meas.value;
                    inner_count[lon_idx][lat_idx]++;
                }
                else if (new_meas.beamIdx == 1)
                {
                    outer_s0[lon_idx][lat_idx] += new_meas.value;
                    outer_count[lon_idx][lat_idx]++;
                }
            }
        }

        //---------------------//
        // close the data sets //
        //---------------------//

        l2a_file.CloseParamDatasets(row_number_p);
        l2a_file.CloseParamDatasets(num_sigma0_p);
        l2a_file.CloseParamDatasets(sigma0_qual_flag_p);
        l2a_file.CloseParamDatasets(cell_index_p);
        l2a_file.CloseParamDatasets(sigma0_p);
        l2a_file.CloseParamDatasets(cell_incidence_p);
        l2a_file.CloseParamDatasets(sigma0_attn_map_p);
        l2a_file.CloseParamDatasets(sigma0_mode_flag_p);
        l2a_file.CloseParamDatasets(cell_lon_p);
        l2a_file.CloseParamDatasets(cell_lat_p);
    }

    //-----------------------//
    // calculate the average //
    //-----------------------//

    for (int i = 0; i < LONGITUDE_BINS; i++)
    {
        for (int j = 0; j < LATITUDE_BINS; j++)
        {
            if (inner_count[i][j] > 0)
                inner_s0[i][j] /= (float)inner_count[i][j];
            else
                inner_s0[i][j] = 0.0;
            if (outer_count[i][j] > 0)
                outer_s0[i][j] /= (float)outer_count[i][j];
            else
                outer_s0[i][j] = 0.0;
        }
    }

    //--------------------//
    // write output files //
    //--------------------//

    int x_size = LONGITUDE_BINS;
    int y_size = LATITUDE_BINS;

    FILE* ofp = fopen(inner_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening inner file %s\n", command,
            inner_file);
        exit(1);
    }
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(inner_s0, sizeof(float), x_size * y_size, ofp) !=
        (unsigned int)(x_size * y_size))
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            inner_file);
        exit(1);
    }
    fclose(ofp);

    ofp = fopen(outer_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening outer file %s\n", command,
            outer_file);
        exit(1);
    }
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(outer_s0, sizeof(float), x_size * y_size, ofp) !=
        (unsigned int)(x_size * y_size))
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            outer_file);
        exit(1);
    }
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
