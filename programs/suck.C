//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    suck
//
// SYNOPSIS
//    suck [ -a	] [ -q ] <L1A_file...>
//
// DESCRIPTION
//    This program extracts Doppler and range tracking constants from
//    a set of L1A files. It will span files if they are consecutive.
//
// OPTIONS
//    The following options are supported:
//      [ -a ]  Use all frames. Even the bad ones. Risky!
//      [ -q ]  Be quiet. Don't complain about bad frames.
//      none
//
// OPERANDS
//    The following operands are supported:
//      <L1A_file...>  A list of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % suck QS_S1A10529.20011781928 QS_S1A10530.20011781928
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
#include <unistd.h>
#include <string.h>
#include "hdf.h"
#include "mfhdf.h"
#include "Tracking.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "aq"

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

const char* usage_array[] = { "[ -a ]", "[ -q ]", "<L1A_file...>", 0 };
const char* active_map[] = { "inactive", "active" };

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
//    extern char* optarg;
    extern int optind;

    int opt_all = 0;
    int opt_quiet = 0;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'a':
            opt_all = 1;
            break;
        case 'q':
            opt_quiet = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc <= optind)
        usage(command, usage_array, 1);

    int start_file_idx = optind;
    int end_file_idx = argc;

    //--------------//
    // the trackers //
    //--------------//

    RangeTracker range_tracker;
    range_tracker.Allocate(256);
    DopplerTracker doppler_tracker;
    doppler_tracker.Allocate(256);

    //------------------------------//
    // loop through the input files //
    //------------------------------//

//    float64 last_instrument_time = -1.0;

    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        char* hdf_input_filename = argv[file_idx];

        int32 sd_id = SDstart(hdf_input_filename, DFACC_READ);
        if (sd_id == FAIL)
        {
            fprintf(stderr, "%s: error opening HDF file %s for reading\n",
                command, hdf_input_filename);
            exit(1);
        }

        //--------------------------------//
        // determine the number of frames //
        //--------------------------------//

        int32 attr_index_l1a_actual_frame = SDfindattr(sd_id,
            "l1a_actual_frames");

        char data[1024];
        if (SDreadattr(sd_id, attr_index_l1a_actual_frame, data) == FAIL)
        {
            fprintf(stderr,
                "%s: error reading attribute for l1a_actual_frames\n",
                command);
            exit(1);
        }

        int frame_count = 0;
        if (sscanf(data, " %*[^\n] %*[^\n] %d", &frame_count) != 1)
        {
            fprintf(stderr, "%s: error parsing l1a_actual_frame attribute\n",
                command);
            fprintf(stderr, "%s\n", data);
            exit(1);
        }

        int32 instrument_time_sds_id = SDnametoid(sd_id, "instrument_time");
        int32 frame_err_status_sds_id = SDnametoid(sd_id, "frame_err_status");
        int32 frame_qual_flag_sds_id = SDnametoid(sd_id, "frame_qual_flag");
        int32 frame_inst_status_sds_id = SDnametoid(sd_id,
            "frame_inst_status");
        int32 table_readout_type_sds_id = SDnametoid(sd_id,
            "table_readout_type");
        int32 table_readout_offset_sds_id = SDnametoid(sd_id,
            "table_readout_offset");
        int32 table_readout_data_sds_id = SDnametoid(sd_id,
            "table_readout_data");

        int32 start[1] = { 0 };
        int32 edges[1] = { 1 };

        for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
        {
            start[0] = frame_idx;

            //------------------//
            // check for errors //
            //------------------//

            uint32 frame_err_status;
            if (SDreaddata(frame_err_status_sds_id, start,
                NULL, edges, (VOIDP)&frame_err_status) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_err_status (ID=%d)\n",
                    command, (int)frame_err_status_sds_id);
                exit(1);
            }
            if (frame_err_status != 0)
            {
                if (! opt_quiet)
                {
                    fprintf(stderr,
                        "%s: frame %d is evil. (error status = 0x%08x)\n",
                        command, frame_idx, (unsigned int)frame_err_status);
                }
                if (! opt_all)
                {
                    continue;
                }
            }
            uint16 frame_qual_flag;
            if (SDreaddata(frame_qual_flag_sds_id, start,
                NULL, edges, (VOIDP)&frame_qual_flag) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_qual_flag\n",
                    command);
                exit(1);
            }

            if (frame_qual_flag != 0)
            {
                if (! opt_quiet)
                {
                    fprintf(stderr,
                        "%s: frame %d is evil. (quality flag = %0x)\n",
                        command, frame_idx, frame_qual_flag);
                }
                if (! opt_all)
                {
                    continue;
                }
            }

            //-----------------------------//
            // check for sequential frames //
            //-----------------------------//

/*
            float64 instrument_time;
            if (SDreaddata(instrument_time_sds_id, start,
                NULL, edges, (VOIDP)&instrument_time) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for instrument_time\n",
                    command);
                exit(1);
            }
            instrument_time /= 32.0;    // convert to seconds
            double time_dif = fabs(instrument_time - last_instrument_time);
            double mem_last_time = last_instrument_time;
            last_instrument_time = instrument_time;
            if (time_dif > 0.54 * 1.5)
            {
                fprintf(stderr, "%s: time discontinuity at frame %d\n",
                    command, frame_idx);
                fprintf(stderr, "    %.2f -> %.2f\n", mem_last_time,
                    instrument_time);
            }
*/

            //---------------------//
            // get the memory info //
            //---------------------//

            uint8 table_readout_type;
            if (SDreaddata(table_readout_type_sds_id, start, NULL, edges,
                (VOIDP)&table_readout_type) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for table_readout_type\n",
                    command);
                exit(1);
            }
            uint16 table_readout_offset;
            if (SDreaddata(table_readout_offset_sds_id, start, NULL, edges,
                (VOIDP)&table_readout_offset) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for table_readout_offset\n",
                    command);
                exit(1);
            }
            char table_readout_data[4];
            if (SDreaddata(table_readout_data_sds_id, start, NULL, edges,
                (VOIDP)table_readout_data) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for table_readout_data\n",
                    command);
                exit(1);
            }

            //-----------------//
            // ...and store it //
            //-----------------//

            char filename[1024];

            int beam_idx, active;
            if (range_tracker.MroAssemble((unsigned char)table_readout_type,
                (unsigned short)table_readout_offset,
                table_readout_data, &beam_idx, &active))
            {
                sprintf(filename, "%04X.%d.%s.rgc", range_tracker.GetTableId(),
                    beam_idx + 1, active_map[active]);
                if (! range_tracker.WriteBinary(filename))
                {
                    fprintf(stderr, "%s: error writing RGC to file %s\n",
                        command, filename);
                    exit(1);
                }
            }

            if (doppler_tracker.MroAssemble((unsigned char)table_readout_type,
                (unsigned short)table_readout_offset,
                table_readout_data, &beam_idx, &active))
            {
                sprintf(filename, "%04X.%d.%s.dtc",
                    doppler_tracker.GetTableId(), beam_idx + 1,
                    active_map[active]);
                if (! doppler_tracker.WriteBinary(filename))
                {
                    fprintf(stderr, "%s: error writing DTC to file %s\n",
                        command, filename);
                    exit(1);
                }
            }
        }

        //--------------------
        // We are almost done with this file. But first, we MUST
        // end our access to all of the SDSs.
        //--------------------

        if (SDendaccess(instrument_time_sds_id) == FAIL ||
            SDendaccess(frame_err_status_sds_id) == FAIL ||
            SDendaccess(frame_qual_flag_sds_id) == FAIL ||
            SDendaccess(frame_inst_status_sds_id) == FAIL ||
            SDendaccess(table_readout_type_sds_id) == FAIL ||
            SDendaccess(table_readout_offset_sds_id) == FAIL ||
            SDendaccess(table_readout_data_sds_id) == FAIL)
        {
            fprintf(stderr, "%s: error ending SD access\n", command);
            exit(1);
        }

        if (SDend(sd_id) == FAIL)
        {
            fprintf(stderr, "%s: error ending SD\n", command);
            exit(1);
        }
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
