//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1a_hdf_azimuth_noise
//
// SYNOPSIS
//    l1a_hdf_azimuth_noise [ -e ] <output_base> <L1A_file...>
//
// DESCRIPTION
//    This program generates an output array of average/max/min noise
//    versus antenna encoder.
//
// OPTIONS
//    [ -e ]  Allow frames with errors.
//
// OPERANDS
//    The following operands are supported:
//      <output_base>  The output array.
//      <L1A_file...>  A list of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_l1a_energy_map noiseaz.dat /seapac/disk1/L1A/data/QS_S1A*
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
#include <hdf.h>
#include <mfhdf.h>
#include "Misc.h"
#include "Sds.h"
#include "BitMasks.h"

//-----------//
// CONSTANTS //
//-----------//

#define NUMBER_OF_QSCAT_BEAMS  2
#define AZIMUTH_BINS           65536
#define OPTSTRING              "e"
#define QUOTE                  '"'

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_error = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -e ]", "<output_base>", "<L1A_file...>", 0 };

unsigned int  min_noise[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_BINS];
unsigned int  max_noise[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_BINS];
double        sum_noise[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_BINS];
unsigned int  count_noise[NUMBER_OF_QSCAT_BEAMS][AZIMUTH_BINS];

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

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'e':
            opt_error = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }
    if (argc < optind + 2)
        usage(command, usage_array, 1);

    char* output_base = argv[optind++];
    int start_file_idx = optind;
    int end_file_idx = argc;

    //---------------------//
    // loop over l1a files //
    //---------------------//

    for (int file_idx = start_file_idx; file_idx < end_file_idx; file_idx++)
    {
        //-------------------//
        // open the HDF file //
        //-------------------//

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

        //--------------------
        // Read in important parameters.
        //--------------------

        int32 frame_err_status_sds_id = SDnametoid(sd_id, "frame_err_status");
        int32 frame_qual_flag_sds_id = SDnametoid(sd_id, "frame_qual_flag");
        int32 pulse_qual_flag_sds_id = SDnametoid(sd_id, "pulse_qual_flag");
        int32 noise_dn_sds_id = SDnametoid(sd_id, "noise_dn");
        int32 true_cal_pulse_pos_sds_id = SDnametoid(sd_id,
            "true_cal_pulse_pos");
        int32 antenna_position_sds_id = SDnametoid(sd_id, "antenna_position");
        int32 operational_mode_sds_id = SDnametoid(sd_id, "operational_mode");

        //--------------------
        // read data
        //--------------------

        int32 start[3] = { 0, 0, 0 };
        int32 generic_1d_edges[1] = { 1 };

        int32 edges_13[2] = { 1, 13 };

        int32 edges_100_12[3] = { 1, 100, 12 };

        for (int frame_idx = 0; frame_idx < frame_count; frame_idx++)
        {
            start[0] = frame_idx;
            uint32 frame_err_status;
            if (SDreaddata(frame_err_status_sds_id, start,
                NULL, generic_1d_edges, (VOIDP)&frame_err_status) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_err_status (ID=%d)\n",
                    command, (int)frame_err_status_sds_id);
                exit(1);
            }
            if (! opt_error && frame_err_status != 0)
            {
                fprintf(stderr,
                    "%s: frame %d is evil. (error status = 0x%08x)\n",
                    command, frame_idx, (unsigned int)frame_err_status);
                continue;
            }

            uint16 frame_qual_flag;
            if (SDreaddata(frame_qual_flag_sds_id, start,
                NULL, generic_1d_edges, (VOIDP)&frame_qual_flag) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for frame_qual_flag\n",
                    command);
                exit(1);
            }

            if (! opt_error && frame_qual_flag != 0)
            {
                fprintf(stderr,
                    "%s: frame %d is evil. (quality flag = 0x%0x)\n", command,
                    frame_idx, frame_qual_flag);
                continue;
            }

            uint8 pulse_qual_flag[13];
            if (SDreaddata(pulse_qual_flag_sds_id, start,
                NULL, edges_13, (VOIDP)&pulse_qual_flag) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for pulse_qual_flag\n",
                    command);
                exit(1);
            }

            int bad_pulses = 0;
            for (int i = 0; i < 13; i++)
            {
                if (pulse_qual_flag[i] != 0)
                {
                    bad_pulses = 1;
                }
            }
            if (bad_pulses)
            {
                fprintf(stderr, "%s: frame %d is evil. (bad pulse)\n",
                    command, frame_idx);
                continue;
            }

            uint8 operational_mode;
            if (SDreaddata(operational_mode_sds_id, start,
                NULL, generic_1d_edges, (VOIDP)&operational_mode) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for operational_mode\n",
                    command);
                exit(1);
            }
            if (operational_mode != L1A_OPERATIONAL_MODE_ROM) {
                continue;
            }

            //-------------------//
            // antenna postition //
            //-------------------//

            uint16 ant_pos[100];
            if (SDreaddata(antenna_position_sds_id, start, NULL,
                edges_100_12, (VOIDP)ant_pos) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for antenna_position\n",
                    command);
                exit(1);
            }

            //----------//
            // noise dn //
            //----------//
            
            uint32 noise_dn[100];
            if (SDreaddata(noise_dn_sds_id, start, NULL,
                edges_100_12, (VOIDP)noise_dn) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for noise_dn\n",
                    command);
                exit(1);
            }

            int8 true_cal_pulse_pos;
            if (SDreaddata(true_cal_pulse_pos_sds_id, start,
                NULL, generic_1d_edges, (VOIDP)&true_cal_pulse_pos) == FAIL)
            {
                fprintf(stderr, "%s: error reading true_cal_pulse_pos\n",
                    command);
                exit(1);
            }

            int loopback_index = true_cal_pulse_pos - 1;
            int load_index = true_cal_pulse_pos;

            //----------------//
            // for each pulse //
            //----------------//

            for (int pulse_idx = 0; pulse_idx < 100; pulse_idx++)
            {
                if (pulse_idx == loopback_index || pulse_idx == load_index)
                    continue;

                // determine beam index and beam
                int beam_idx = pulse_idx % NUMBER_OF_QSCAT_BEAMS;

                int azimuth_idx = ant_pos[pulse_idx];

                if (count_noise[beam_idx][azimuth_idx] == 0) {
                    min_noise[beam_idx][azimuth_idx] = noise_dn[pulse_idx];
                    max_noise[beam_idx][azimuth_idx] = noise_dn[pulse_idx];
                } else {
                    if (noise_dn[pulse_idx] <
                        min_noise[beam_idx][azimuth_idx])
                    {
                        min_noise[beam_idx][azimuth_idx] = noise_dn[pulse_idx];
                    }
                    if (noise_dn[pulse_idx] >
                        max_noise[beam_idx][azimuth_idx])
                    {
                        max_noise[beam_idx][azimuth_idx] = noise_dn[pulse_idx];
                    }
                }
                sum_noise[beam_idx][azimuth_idx] +=
                    (double)noise_dn[pulse_idx];
                count_noise[beam_idx][azimuth_idx]++;
            }
        }

        //--------------------
        // We are almost done with this file. But first, we MUST
        // end our access to all of the SDSs.
        //--------------------

        if (SDendaccess(frame_err_status_sds_id) == FAIL ||
            SDendaccess(frame_qual_flag_sds_id) == FAIL ||
            SDendaccess(pulse_qual_flag_sds_id) == FAIL ||
            SDendaccess(noise_dn_sds_id) == FAIL ||
            SDendaccess(true_cal_pulse_pos_sds_id) == FAIL ||
            SDendaccess(antenna_position_sds_id) == FAIL)
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
    }

    //--------//
    // output //
    //--------//

    char filename[1024];
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++) {

        sprintf(filename, "%s.avg.%d", output_base, beam_idx + 1);
        FILE* avg_ofp = fopen_or_exit(filename, "w", command,
            "average file", 1);
        fprintf(avg_ofp,
            "@ subtitle %cAverage Noise Channel Energy, Beam %d%c\n", QUOTE,
            beam_idx + 1, QUOTE);
        fprintf(avg_ofp, "@ xaxis label %cAntenna Azimuth Encoder%c\n", QUOTE,
            QUOTE);
        fprintf(avg_ofp, "@ yaxis label %cEnergy (dn)%c\n", QUOTE, QUOTE);

        sprintf(filename, "%s.min.%d", output_base, beam_idx + 1);
        FILE* min_ofp = fopen_or_exit(filename, "w", command,
            "average file", 1);
        fprintf(min_ofp,
            "@ subtitle %cMinimum Noise Channel Energy, Beam %d%c\n", QUOTE,
            beam_idx + 1, QUOTE);
        fprintf(min_ofp, "@ xaxis label %cAntenna Azimuth Encoder%c\n", QUOTE,
            QUOTE);
        fprintf(min_ofp, "@ yaxis label %cEnergy (dn)%c\n", QUOTE, QUOTE);

        sprintf(filename, "%s.max.%d", output_base, beam_idx + 1);
        FILE* max_ofp = fopen_or_exit(filename, "w", command,
            "average file", 1);
        fprintf(max_ofp,
            "@ subtitle %cMaximum Noise Channel Energy, Beam %d%c\n", QUOTE,
            beam_idx + 1, QUOTE);
        fprintf(max_ofp, "@ xaxis label %cAntenna Azimuth Encoder%c\n", QUOTE,
            QUOTE);
        fprintf(max_ofp, "@ yaxis label %cEnergy (dn)%c\n", QUOTE, QUOTE);

        sprintf(filename, "%s.n.%d", output_base, beam_idx + 1);
        FILE* n_ofp = fopen_or_exit(filename, "w", command,
            "average file", 1);
        fprintf(n_ofp,
            "@ subtitle %cNumber of Samples, Beam %d%c\n", QUOTE,
            beam_idx + 1, QUOTE);
        fprintf(n_ofp, "@ xaxis label %cAntenna Azimuth Encoder%c\n", QUOTE,
            QUOTE);
        fprintf(n_ofp, "@ yaxis label %cEnergy (dn)%c\n", QUOTE, QUOTE);

        for (int i = 0; i < AZIMUTH_BINS; i++) {
            if (count_noise[beam_idx][i] > 0) {
                fprintf(avg_ofp, "%d %g\n", i, sum_noise[beam_idx][i]
                    / (double)count_noise[beam_idx][i]);
                fprintf(min_ofp, "%d %d\n", i, min_noise[beam_idx][i]);
                fprintf(max_ofp, "%d %d\n", i, max_noise[beam_idx][i]);
                fprintf(n_ofp, "%d %d\n", i, count_noise[beam_idx][i]);
            }
        }
        fclose(avg_ofp);
        fclose(min_ofp);
        fclose(max_ofp);
        fclose(n_ofp);
    }

    return (0);
}
