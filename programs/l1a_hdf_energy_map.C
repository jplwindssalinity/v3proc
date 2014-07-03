//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1a_hdf_energy_map
//
// SYNOPSIS
//    l1a_hdf_energy_map [ -erw ] <config_file> <output_base>
//      <L1A_file...>
//
// DESCRIPTION
//    This program generates an output array which can be turned
//    into an image of the earth as the average energy in the noise
//    channel.
//
// OPTIONS
//    [ -e ]  Allow frames with errors.
//    [ -r ]  Use ROM data.
//    [ -w ]  Use WOM data.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  The output array.
//      <output_base>  The output array.
//      <L1A_file...>   A list of L1A files.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_l1a_energy_map qscat.cfg output.arr
//          /seapac/disk1/L1A/data/QS_S1A*
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
#include "Misc.h"
#include "BitMasks.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "Sds.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

//-----------//
// CONSTANTS //
//-----------//

#define LONGITUDE_BINS  1800
#define LATITUDE_BINS   901

#define OPTSTRING  "erw"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

template class List<Meas>;
template class BufferedList<OrbitState>;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class List<StringPair>;

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_error = 0;
int opt_rom = 0;
int opt_wom = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -erw ]", "<config_file>", "<output_base>",
    "<L1A_file...>", 0 };

float inner_energy[LONGITUDE_BINS][LATITUDE_BINS];
int   inner_count[LONGITUDE_BINS][LATITUDE_BINS];
float outer_energy[LONGITUDE_BINS][LATITUDE_BINS];
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
        case 'r':
            opt_rom = 1;
            break;
        case 'w':
            opt_wom = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }
    if (argc < optind + 3) {
        usage(command, usage_array, 1);
    }
    if (! opt_rom && ! opt_wom) {
        fprintf(stderr, "%s: must specify a type of data ( -w and/or -r )\n",
            command);
        exit(1);
    }

    char* config_file = argv[optind++];
    char* output_base = argv[optind++];
    int start_file_idx = optind;
    int end_file_idx = argc;

    //-----------------------------//
    // read the configuration file //
    //-----------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
    }
    Attitude* attitude = &(spacecraft.attitude);

    //----------------------------//
    // create and configure Qscat //
    //----------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }
    Antenna* antenna = &(qscat.sas.antenna);

    //--------------------//
    // create the indices //
    //--------------------//

    Index lon_index, lat_index;
    lon_index.SpecifyWrappedCenters(0.0, two_pi, LONGITUDE_BINS);
    lat_index.SpecifyCenters(-pi_over_two, pi_over_two, LATITUDE_BINS);

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

        int32 x_pos_sds_id = SDnametoid(sd_id, "x_pos");
        int32 y_pos_sds_id = SDnametoid(sd_id, "y_pos");
        int32 z_pos_sds_id = SDnametoid(sd_id, "z_pos");
        int32 x_vel_sds_id = SDnametoid(sd_id, "x_vel");
        int32 y_vel_sds_id = SDnametoid(sd_id, "y_vel");
        int32 z_vel_sds_id = SDnametoid(sd_id, "z_vel");
        float64 roll_scale, pitch_scale, yaw_scale;
        int32 roll_sds_id = SDnametoid(sd_id, "roll", &roll_scale);
        int32 pitch_sds_id = SDnametoid(sd_id, "pitch", &pitch_scale);
        int32 yaw_sds_id = SDnametoid(sd_id, "yaw", &yaw_scale);
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

            //----------------//
            // check the mode //
            //----------------//

            uint8 operational_mode;
            if (SDreaddata(operational_mode_sds_id, start,
                NULL, generic_1d_edges, (VOIDP)&operational_mode) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for operational_mode\n",
                    command);
                exit(1);
            }
            if (operational_mode == L1A_OPERATIONAL_MODE_WOM && ! opt_wom) {
                continue;
            }
            if (operational_mode == L1A_OPERATIONAL_MODE_ROM && ! opt_rom) {
                continue;
            }

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

            //---------------------//
            // geometry parameters //
            //---------------------//

            float32 x_pos, y_pos, z_pos;
            if (SDreaddata(x_pos_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&x_pos) == FAIL ||
                SDreaddata(y_pos_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&y_pos) == FAIL ||
                SDreaddata(z_pos_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&z_pos) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for s/c position\n", command);
                exit(1);
            }
            x_pos *= M_TO_KM;
            y_pos *= M_TO_KM;
            z_pos *= M_TO_KM;
            
            float32 x_vel, y_vel, z_vel;
            if (SDreaddata(x_vel_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&x_vel) == FAIL ||
                SDreaddata(y_vel_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&y_vel) == FAIL ||
                SDreaddata(z_vel_sds_id, start, NULL,
                generic_1d_edges, (VOIDP)&z_vel) == FAIL)
            {
                fprintf(stderr,
                    "%s: error reading SD data for s/c velocity\n", command);
                exit(1);
            }
            x_vel *= MPS_TO_KMPS;
            y_vel *= MPS_TO_KMPS;
            z_vel *= MPS_TO_KMPS;

            int16 roll16, pitch16, yaw16;
            SDreaddata_or_exit("roll", roll_sds_id, start, generic_1d_edges,
                (VOIDP)&roll16);
            SDreaddata_or_exit("pitch", pitch_sds_id, start, generic_1d_edges,
                (VOIDP)&pitch16);
            SDreaddata_or_exit("yaw", yaw_sds_id, start, generic_1d_edges,
                (VOIDP)&yaw16);
            float roll = (float)roll16 * roll_scale * dtr;
            float pitch = (float)pitch16 * pitch_scale * dtr;
            float yaw = (float)yaw16 * yaw_scale * dtr;

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

            //----------//
            // geometry //
            //----------//

            spacecraft.orbitState.rsat.Set(x_pos, y_pos, z_pos);
            spacecraft.orbitState.vsat.Set(x_vel, y_vel, z_vel);
            attitude->SetRPY(roll, pitch, yaw);

            for (int pulse_idx = 0; pulse_idx < 100; pulse_idx++)
            {
                if (pulse_idx == loopback_index || pulse_idx == load_index)
                    continue;

                // determine beam index and beam
                int beam_idx = pulse_idx % NUMBER_OF_QSCAT_BEAMS;
                qscat.cds.currentBeamIdx = beam_idx;
                Beam* beam = qscat.GetCurrentBeam();

                // set the encoder for getting the azimuth angle
                unsigned short encoder = ant_pos[pulse_idx];
                qscat.SetEncoderAzimuth(encoder, 1);
                qscat.SetOtherAzimuths(&spacecraft);

                CoordinateSwitch antenna_frame_to_gc =
                    AntennaFrameToGC(&(spacecraft.orbitState),
                    attitude, antenna, antenna->groundImpactAzimuthAngle);

                double look, azim;
                if (! beam->GetElectricalBoresight(&look, &azim))
                    return(0);
                Vector3 vector;
                vector.SphericalSet(1.0, look, azim);
                QscatTargetInfo qti;
                if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft,
                    vector, &qti))
                {
                    fprintf(stderr, "%s: error finding target information\n",
                        command);
                    exit(1);
                }
                double alt, lon_d, lat_d;
                if (! qti.rTarget.GetAltLonGCLat(&alt, &lon_d, &lat_d))
                {
                    fprintf(stderr, "%s: error finding alt/lon/lat\n",
                        command);
                    exit(1);
                }
                float lon = (float)lon_d;
                float lat = (float)lat_d;

                //-------------------//
                // accumulate energy //
                //-------------------//

                int lon_idx;
                if (! lon_index.GetNearestIndexWrapped(lon, &lon_idx))
                    continue;

                int lat_idx;
                if (! lat_index.GetNearestIndexStrict(lat, &lat_idx))
                    continue;

                if (beam_idx == 0)
                {
                    inner_energy[lon_idx][lat_idx] += noise_dn[pulse_idx];
                    inner_count[lon_idx][lat_idx]++;
                }
                else if (beam_idx == 1)
                {
                    outer_energy[lon_idx][lat_idx] += noise_dn[pulse_idx];
                    outer_count[lon_idx][lat_idx]++;
                }
            }
        }

        //--------------------
        // We are almost done with this file. But first, we MUST
        // end our access to all of the SDSs.
        //--------------------

        if (SDendaccess(x_pos_sds_id) == FAIL ||
            SDendaccess(y_pos_sds_id) == FAIL ||
            SDendaccess(z_pos_sds_id) == FAIL ||
            SDendaccess(x_vel_sds_id) == FAIL ||
            SDendaccess(y_vel_sds_id) == FAIL ||
            SDendaccess(z_vel_sds_id) == FAIL ||
            SDendaccess(roll_sds_id) == FAIL ||
            SDendaccess(pitch_sds_id) == FAIL ||
            SDendaccess(yaw_sds_id) == FAIL ||
            SDendaccess(frame_err_status_sds_id) == FAIL ||
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

    //---------//
    // average //
    //---------//

    for (int i = 0; i < LONGITUDE_BINS; i++)
    {
        for (int j = 0; j < LATITUDE_BINS; j++)
        {
            if (inner_count[i][j] > 0)
                inner_energy[i][j] /= (float)inner_count[i][j];
            else
                inner_energy[i][j] = 0.0;
            if (outer_count[i][j] > 0)
                outer_energy[i][j] /= (float)outer_count[i][j];
            else
                outer_energy[i][j] = 0.0;
        }
    }

    //--------------------//
    // write output files //
    //--------------------//

    int x_size = LONGITUDE_BINS;
    int y_size = LATITUDE_BINS;

    char filename[1024];
    sprintf(filename, "%s.%s", output_base, "inn.arr");
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening inner file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(inner_energy, sizeof(float), x_size * y_size, ofp) !=
        (unsigned int)(x_size * y_size))
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            filename);
        exit(1);
    }
    fclose(ofp);

    sprintf(filename, "%s.%s", output_base, "out.arr");
    ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening outer file %s\n", command,
            filename);
        exit(1);
    }
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(outer_energy, sizeof(float), x_size * y_size, ofp) !=
        (unsigned int)(x_size * y_size))
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            filename);
        exit(1);
    }
    fclose(ofp);

    return (0);
}
