//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    modu
//
// SYNOPSIS
//    modu <output_base> <modu_file...> <l1b_file...>
//        
// DESCRIPTION
//    Reads HDF L1B files and generates output files containing
//    the magnitude and phase of sigma-0 modulation.
//
// OPTIONS
//    None
//
// OPERANDS
//    <output_base>   The base of the output file names.
//    <modu_file...>  The input modu files.
//    <l1b_file...>   The input L1B files.
//
// EXAMPLES
//    modu modu modu.* QS_L1B*
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "L1AExtract.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "L1AFile.h"
#include "L1BHdfFile.h"

#include <stdio.h>
#include <fcntl.h>
#include "List.h"
#include "List.C"
#include "Args.h"
#include "ArgsPlus.h"
#include "Misc.h"
#include "ConfigList.h"
#include "ParTab.h"
#include "Array.h"
#include "Qscat.h"
#include "modu.h"
#include "Image.h"


//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
/*
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
*/

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""
#define UNUSABLE   0x0001
#define NEGATIVE   0x0004

#define LON_MIN        0.0
#define LON_MAX        360.0
#define LON_STEP_SIZE  2.0

#define LAT_MIN        -90.0
#define LAT_MAX        90.0
#define LAT_STEP_SIZE  2.0

#define TARGET_BEAM_IDX  1

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

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

ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo leap_second_table_arg = LEAP_SECOND_TABLE_ARG;

const char* usage_array[] = { "<output_base>", "<modu_file...>",
    "<l1b_file...>", 0 };

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
//    extern char *optarg;
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

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* output_base = argv[optind++];
    int start_file_idx = optind;
    int end_file_idx = argc;

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

    //-----------------------//
    // define L1B parameters //
    //-----------------------//

    Parameter* frame_err_status_p = ParTabAccess::GetParameter(SOURCE_L1B,
        FRAME_ERR_STATUS, UNIT_DN);
    Parameter* cell_lat_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_LAT, UNIT_DEGREES);
    Parameter* cell_lon_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_LON, UNIT_DEGREES);
    Parameter* sigma0_qual_flag_p = ParTabAccess::GetParameter(SOURCE_L1B,
        SIGMA0_QUAL_FLAG, UNIT_DN);
    Parameter* cell_sigma0_p = ParTabAccess::GetParameter(SOURCE_L1B,
        SIGMA0, UNIT_DB);
    Parameter* cell_azimuth_p = ParTabAccess::GetParameter(SOURCE_L1B,
        CELL_AZIMUTH, UNIT_RADIANS);

    //---------------------//
    // create data storage //
    //---------------------//

    ModuGrid modu_grid;
    if (! modu_grid.Allocate(LON_MIN, LON_MAX, LON_STEP_SIZE, LAT_MIN,
        LAT_MAX, LAT_STEP_SIZE))
    {
        fprintf(stderr, "%s: error allocating ModuGrid\n", command);
        exit(1);
    }

    //--------------------//
    // step through files //
    //--------------------//

    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        char* filename = argv[file_idx];

        if (strstr(filename, "QS_S1B") == NULL)
        {
            //------------//
            // Modu files //
            //------------//

            printf("%s (modu)\n", filename);
            ModuGrid new_grid;
            if (! new_grid.Read(filename))
            {
                fprintf(stderr, "%s: error reading modu file %s\n", command,
                    filename);
                exit(1);
            }
            modu_grid.Add(&new_grid);
            new_grid.Deallocate();
            continue;
        }

        printf("%s (L1B)\n", filename);
        ModuGrid new_grid;
        if (! new_grid.Allocate(LON_MIN, LON_MAX, LON_STEP_SIZE, LAT_MIN,
            LAT_MAX, LAT_STEP_SIZE))
        {
            fprintf(stderr, "%s: error allocating ModuGrid\n", command);
            exit(1);
        }

        HdfFile::StatusE status;
        L1BHdfFile l1b_file(filename, status);
        if (status != HdfFile::OK)
        {
            fprintf(stderr, "%s: error creating L1B file object\n", command);
            exit(1);
        }

        check_status(l1b_file.OpenParamDatasets(frame_err_status_p));
        check_status(l1b_file.OpenParamDatasets(cell_lat_p));
        check_status(l1b_file.OpenParamDatasets(cell_lon_p));
        check_status(l1b_file.OpenParamDatasets(sigma0_qual_flag_p));
        check_status(l1b_file.OpenParamDatasets(cell_sigma0_p));
        check_status(l1b_file.OpenParamDatasets(cell_azimuth_p));

        //------------------------//
        // process frame by frame //
        //------------------------//

        int l1b_length = l1b_file.GetDataLength();
        for (int record_idx = 0; record_idx < l1b_length; record_idx++)
        {
            unsigned int frame_err_status;
            frame_err_status_p->extractFunc(&l1b_file,
                frame_err_status_p->sdsIDs, record_idx, 1, 1,
                &frame_err_status, polyTable);

            // check for bad frame
            if (frame_err_status)
                continue;

            float cell_lat[100];
            cell_lat_p->extractFunc(&l1b_file, cell_lat_p->sdsIDs,
                record_idx, 1, 1, cell_lat, polyTable);

            float cell_lon[100];
            cell_lon_p->extractFunc(&l1b_file, cell_lon_p->sdsIDs,
                record_idx, 1, 1, cell_lon, polyTable);

            unsigned short sigma0_qual_flag[100];
            sigma0_qual_flag_p->extractFunc(&l1b_file,
                sigma0_qual_flag_p->sdsIDs, record_idx, 1, 1,
                sigma0_qual_flag, polyTable);

            float cell_sigma0[100];
            cell_sigma0_p->extractFunc(&l1b_file, cell_sigma0_p->sdsIDs,
                record_idx, 1, 1, cell_sigma0, polyTable);

            float cell_azimuth[100];
            cell_azimuth_p->extractFunc(&l1b_file, cell_azimuth_p->sdsIDs,
                record_idx, 1, 1, cell_azimuth, polyTable);

            //--------------------//
            // step through spots //
            //--------------------//

            for (int spot_idx = 0; spot_idx < 100; spot_idx++)
            {
                // check for bad spot
                if (sigma0_qual_flag[spot_idx])
                    continue;

                // check usability
                if (sigma0_qual_flag[spot_idx] & UNUSABLE)
                    continue;

                // drop negative sigma0
                if (sigma0_qual_flag[spot_idx] & NEGATIVE)
                    continue;

                // threshold sigma0
                if (cell_sigma0[spot_idx] < -30.0)
                    continue;

                // weird lat/lon
                // determine beam index
                int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
                if (beam_idx != TARGET_BEAM_IDX)
                    continue;

                //------------------------//
                // accumulate in ModuGrid //
                //------------------------//

                if (! new_grid.Accumulate(cell_lon[spot_idx],
                    cell_lat[spot_idx], cell_azimuth[spot_idx],
                    cell_sigma0[spot_idx]))
                {
                    fprintf(stderr, "%s: error accumulating data\n", command);
                    exit(1);
                }
            }
        }

        //----------------//
        // close datasets //
        //----------------//

        l1b_file.CloseParamDatasets(frame_err_status_p);
        l1b_file.CloseParamDatasets(cell_lat_p);
        l1b_file.CloseParamDatasets(cell_lon_p);
        l1b_file.CloseParamDatasets(sigma0_qual_flag_p);
        l1b_file.CloseParamDatasets(cell_sigma0_p);
        l1b_file.CloseParamDatasets(cell_azimuth_p);

        //-------------------------------------------//
        // write out sigma-0 and azimuth information //
        //-------------------------------------------//

        char output_filename[1024];
        char* ptr = strstr(filename, "QS_S1B");
        ptr += 6;
        sprintf(output_filename, "%s.%.5s", output_base, ptr);
        if (! new_grid.Write(output_filename))
        {
            fprintf(stderr, "%s: error writing Modu file %s\n", command,
                output_filename);
            exit(1);
        }

        //---------//
        // combine //
        //---------//

        modu_grid.Add(&new_grid);
        new_grid.Deallocate();
    }

    //---------------------//
    // create output image //
    //---------------------//

    int lon_bins = modu_grid.lonBins;
    int lat_bins = modu_grid.latBins;
    Image image;
    image.Allocate(lon_bins, lat_bins);

    //--------------------------------//
    // fit and write out azimuth info //
    //--------------------------------//

    double azimuth[AZIMUTH_BINS];
    double sigma0[AZIMUTH_BINS];
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
        {
//            float lon = ((float)lon_bins + 0.5) * LON_STEP_SIZE + LON_MIN;
            for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
            {
//                float lat = ((float)lat_bins + 0.5) * LAT_STEP_SIZE + LAT_MIN;
                Modu* modu = *(*(modu_grid.grid + lon_idx) + lat_idx);

                int data_count = 0;
                for (int az_idx = 0; az_idx < AZIMUTH_BINS; az_idx++)
                {
                    if (modu->count[az_idx] < 1)
                        continue;

                    sigma0[data_count] = modu->sigma0[az_idx] /
                        modu->count[az_idx];
                    azimuth[data_count] = two_pi * (double)az_idx /
                        (double)AZIMUTH_BINS;
                    data_count++;
                }

                //-----//
                // fit //
                //-----//

                // need a better fit criteria here
                if (data_count < 10)
                    continue;

                double amp, phase, bias;
                if (! sinfit(azimuth, sigma0, NULL, data_count, &amp, &phase,
                    &bias))
                {
                    fprintf(stderr, "%s: error doing the sinusoid fit\n",
                        command);
                    continue;
                }

                image.Set(lon_idx, lat_idx, (float)amp);
            }
        }
    }

    //-----------------//
    // write out image //
    //-----------------//

    char output_filename[1024];
    sprintf(output_filename, "%s.im", output_base);

    if (! image.WriteIm(output_filename))
    {
        fprintf(stderr, "%s: error writing image file %s\n", command,
            output_filename);
        exit(1);
    }

    //----------------//
    // delete objects //
    //----------------//

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
