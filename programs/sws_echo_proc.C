//==============================================================//
// Copyright (C) 1999-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    sws_echo_proc
//
// SYNOPSIS
//    sws_echo_proc [ -gopv ] <config_file> <l1a_file> <l1b_file>
//        <echo_file>
//
// DESCRIPTION
//    Reads HDF L1A and L1B files and generates echo data files
//    containing a bunch of useful information about the echo
//    and the transmit/receive conditions.  User can specify whether
//    to do a gaussian fit and/or a weighted centroid.
//
// OPTIONS
//    [ -g ]           Use a (g)aussian fit instead of power centroid.
//    [ -o ]           Process modulation (o)ff data instead of mod on.
//    [ -p ]           Use the signal (p)lus noise instead of signal only.
//    [ -v ]           Verbose. Report on dropped data.
//
// OPERANDS
//    <config_file>  The configuration file.
//    <l1a_file>     An L1A file.
//    <l1b_file>     The matching L1B file.
//    <echo_file>    Store output in the specified echo_file.
//
// EXAMPLES
//    sws_echo_proc -g qscat.cfg /disk1/QS_S1A.dat /disk1/QS_S1B.dat
//      rev.x.echo
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
#include <fcntl.h>
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "Qscat.h"
#include "QscatConfig.h"
#include "ConfigSimDefs.h"
#include "Sds.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;
template class List<Meas>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;
template class List<OffsetList>;
template class List<long>;
template class List<MeasSpot>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING      "gopv"

#define WOM            0x0E
#define MODULATION_ON  0x08

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

int opt_verbose = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -gopv ]", "<config_file>", "<l1a_file>",
    "<l1b_file>", "<echo_file>", 0 };

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

    int opt_gaussian = 0;
    int opt_signal_plus_noise = 0;    // default is signal only
    int target_mod_value = 1;         // process Mod ON data

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
        case 'g':
            opt_gaussian = 1;
            break;
        case 'o':
            target_mod_value = 0;
            break;
        case 'p':
            opt_signal_plus_noise = 1;
            break;
        case 'v':
            opt_verbose = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 4)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* l1a_filename = argv[optind++];
    const char* l1b_filename = argv[optind++];
    const char* echo_file = argv[optind++];

    //--------------------------------//
    // read in instrument config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //----------------------//
    // initialize variables //
    //----------------------//

    // calculate energy for all 12, but only fit to center 10
    double signal_energy[12];
    double slice_number[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
        10.0 };    // ignore slice 0 and slice 11

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    spacecraft.attitude.SetOrder(2, 1, 3);
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

    // grab some values to do a poor man's noise subtraction
    double Bn = qscat.ses.noiseBandwidth;
    double Be = qscat.ses.GetTotalSignalBandwidth();
    double Bs = qscat.ses.scienceSliceBandwidth;

    float f1, bw;
    qscat.ses.GetSliceFreqBw((int)1, &f1, &bw);    // the first science slice

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

    //--------------------------//
    // initiate level 1a access //
    //--------------------------//

    int32 l1a_sd_id = SDstart(l1a_filename, DFACC_READ);
    if (l1a_sd_id == FAIL)
    {
        fprintf(stderr, "%s: error opening HDF file %s for reading\n",
            command, l1a_filename);
        exit(1);
    }
    int32 operational_mode_sds_id = SDnametoid(l1a_sd_id, "operational_mode");
    int32 ses_configuration_flags_sds_id = SDnametoid(l1a_sd_id,
        "ses_configuration_flags");
    int32 frame_time_secs_sds_id = SDnametoid(l1a_sd_id, "frame_time_secs");
    int32 x_pos_sds_id = SDnametoid(l1a_sd_id, "x_pos");
    int32 y_pos_sds_id = SDnametoid(l1a_sd_id, "y_pos");
    int32 z_pos_sds_id = SDnametoid(l1a_sd_id, "z_pos");
    int32 x_vel_sds_id = SDnametoid(l1a_sd_id, "x_vel");
    int32 y_vel_sds_id = SDnametoid(l1a_sd_id, "y_vel");
    int32 z_vel_sds_id = SDnametoid(l1a_sd_id, "z_vel");
    int32 frame_err_status_sds_id = SDnametoid(l1a_sd_id, "frame_err_status");
    int32 frame_qual_flag_sds_id = SDnametoid(l1a_sd_id, "frame_qual_flag");
    int32 pulse_qual_flag_sds_id = SDnametoid(l1a_sd_id, "pulse_qual_flag");
    int32 prf_cycle_time_sds_id = SDnametoid(l1a_sd_id, "prf_cycle_time");
    int32 antenna_position_sds_id = SDnametoid(l1a_sd_id, "antenna_position");
    int32 orbit_time_sds_id = SDnametoid(l1a_sd_id, "orbit_time");
    int32 doppler_orbit_step_sds_id = SDnametoid(l1a_sd_id,
              "doppler_orbit_step");
    int32 prf_orbit_step_change_sds_id = SDnametoid(l1a_sd_id,
              "prf_orbit_step_change");
    int32 specified_cal_pulse_pos_sds_id = SDnametoid(l1a_sd_id,
              "specified_cal_pulse_pos");
    int32 power_dn_sds_id = SDnametoid(l1a_sd_id, "power_dn");
    int32 noise_dn_sds_id = SDnametoid(l1a_sd_id, "noise_dn");
    int32 range_gate_a_delay_sds_id = SDnametoid(l1a_sd_id,
              "range_gate_a_delay");
    int32 range_gate_a_width_sds_id = SDnametoid(l1a_sd_id,
              "range_gate_a_width");
    int32 range_gate_b_delay_sds_id = SDnametoid(l1a_sd_id,
              "range_gate_b_delay");
    int32 range_gate_b_width_sds_id = SDnametoid(l1a_sd_id,
              "range_gate_b_width");
    int32 doppler_shift_command_1_sds_id = SDnametoid(l1a_sd_id,
              "doppler_shift_command_1");
    int32 doppler_shift_command_2_sds_id = SDnametoid(l1a_sd_id,
              "doppler_shift_command_2");
//    int32 pulse_width_sds_id = SDnametoid(l1a_sd_id, "pulse_width");

    //--------------------------//
    // initiate level 1b access //
    //--------------------------//

    int32 l1b_sd_id = SDstart(l1b_filename, DFACC_READ);
    if (l1b_sd_id == FAIL)
    {
        fprintf(stderr, "%s: error opening HDF file %s for reading\n",
            command, l1b_filename);
        exit(1);
    }
    int32 cell_lat_sds_id = SDnametoid(l1b_sd_id, "cell_lat");
    int32 cell_lon_sds_id = SDnametoid(l1b_sd_id, "cell_lon");
    float64 roll_scale, pitch_scale, yaw_scale;
    int32 roll_sds_id = SDnametoid(l1b_sd_id, "roll", &roll_scale);
    int32 pitch_sds_id = SDnametoid(l1b_sd_id, "pitch", &pitch_scale);
    int32 yaw_sds_id = SDnametoid(l1b_sd_id, "yaw", &yaw_scale);
    float64 frequency_shift_scale;
    int32 frequency_shift_sds_id = SDnametoid(l1b_sd_id, "frequency_shift",
        &frequency_shift_scale);

    //--------------------//
    // create output file //
    //--------------------//

    int ofd = creat(echo_file, 0644);
    if (ofd == -1)
    {
        fprintf(stderr, "%s: error creating output echo file %s\n", command,
            echo_file);
        exit(1);
    }

    //--------------------//
    // check file lengths //
    //--------------------//

    int l1a_frame_count = 0;
    if (! SDattrint(l1a_sd_id, "l1a_actual_frames", &l1a_frame_count))
    {
        fprintf(stderr, "%s: error reading attribute for l1a_actual_frames\n",
            command);
        exit(1);
    }

    int l1b_frame_count = 0;
    if (! SDattrint(l1b_sd_id, "l1b_actual_frames", &l1b_frame_count))
    {
        fprintf(stderr, "%s: error reading attribute for l1b_actual_frames\n",
            command);
        exit(1);
    }

    if (l1b_frame_count != l1a_frame_count)
    {
        fprintf(stderr, "%s: mismatched data lengths\n", command);
        fprintf(stderr, "  %s : %d records\n", l1a_filename, l1a_frame_count);
        fprintf(stderr, "  %s : %d records\n", l1b_filename, l1b_frame_count);
        exit(1);
    }

    //------------------------//
    // process frame by frame //
    //------------------------//

    int32 start[] = { 0, 0, 0 };
    int32 edges[] = { 1, 100, 12 };
    int32 edges_13[] = { 1, 13 };

    int wom_frame = 0;

    EchoInfo echo_info;
    for (int record_idx = 0; record_idx < l1a_frame_count; record_idx++)
    {
        start[0] = record_idx;

        //------------------//
        // operational mode //
        //------------------//

        uint8 operational_mode;
        SDreaddata_or_exit("operational_mode", operational_mode_sds_id, start,
            edges, (VOIDP)&operational_mode);
        if (operational_mode != WOM) {
            if (opt_verbose) {
                printf("  Non-WOM frame %d\n", record_idx);
            }
            wom_frame = 0;
            continue;
        }

        //------------------//
        // check for errors //
        //------------------//

        uint32 frame_err_status;
        SDreaddata_or_exit("frame_err_status", frame_err_status_sds_id, start,
            edges, (VOIDP)&frame_err_status);
        uint16 frame_qual_flag;
        SDreaddata_or_exit("frame_qual_flag", frame_qual_flag_sds_id, start,
            edges, (VOIDP)&frame_qual_flag);
        uint8 pulse_qual_flag[13];
        SDreaddata_or_exit("pulse_qual_flag", pulse_qual_flag_sds_id, start,
            edges_13, (VOIDP)&pulse_qual_flag);
        int bad_pulses = 0;
        for (int i = 0; i < 13; i++) {
            if (pulse_qual_flag[i] != 0) {
                bad_pulses = 1;
                break;
            }
        }
        if (frame_err_status != 0 || frame_qual_flag != 0 || bad_pulses) {
            if (opt_verbose) {
                printf("  Bad frame %d\n", record_idx);
            }
            wom_frame = 0;   // spin up again
            continue;
        }

        //-------------------------//
        // ses configuration flags //
        //-------------------------//

        uint8 ses_configuration_flags;
        SDreaddata_or_exit("ses_configuration_flags",
            ses_configuration_flags_sds_id, start, edges,
            (VOIDP)&ses_configuration_flags);
        unsigned char modulation = ses_configuration_flags & MODULATION_ON;
        if (! modulation) {
            if (opt_verbose) {
                printf("  Mod-off frame %d\n", record_idx);
            }
            wom_frame = 0;
            continue;
        }

        wom_frame++;

        //-----------------//
        // frame time secs //
        //-----------------//

        float64 frame_time_secs;
        SDreaddata_or_exit("frame_time_secs", frame_time_secs_sds_id, start,
            edges, (VOIDP)&frame_time_secs);
        ETime frame_time;
        frame_time.SetTime(frame_time_secs + 725846400.0);

        //--------------------//
        // set the spacecraft //
        //--------------------//

        float32 x_pos, y_pos, z_pos;
        SDreaddata_or_exit("x_pos", x_pos_sds_id, start, edges, (VOIDP)&x_pos);
        SDreaddata_or_exit("y_pos", y_pos_sds_id, start, edges, (VOIDP)&y_pos);
        SDreaddata_or_exit("z_pos", z_pos_sds_id, start, edges, (VOIDP)&z_pos);
        x_pos *= M_TO_KM;
        y_pos *= M_TO_KM;
        z_pos *= M_TO_KM;

        float32 x_vel, y_vel, z_vel;
        SDreaddata_or_exit("x_vel", x_vel_sds_id, start, edges, (VOIDP)&x_vel);
        SDreaddata_or_exit("y_vel", y_vel_sds_id, start, edges, (VOIDP)&y_vel);
        SDreaddata_or_exit("z_vel", z_vel_sds_id, start, edges, (VOIDP)&z_vel);
        x_vel *= MPS_TO_KMPS;
        y_vel *= MPS_TO_KMPS;
        z_vel *= MPS_TO_KMPS;

        int16 roll16, pitch16, yaw16;
        SDreaddata_or_exit("roll", roll_sds_id, start, edges, (VOIDP)&roll16);
        SDreaddata_or_exit("pitch", pitch_sds_id, start, edges,
            (VOIDP)&pitch16);
        SDreaddata_or_exit("yaw", yaw_sds_id, start, edges, (VOIDP)&yaw16);
        float roll = (float)roll16 * roll_scale * dtr;
        float pitch = (float)pitch16 * pitch_scale * dtr;
        float yaw = (float)yaw16 * yaw_scale * dtr;

        spacecraft.orbitState.rsat.Set(x_pos, y_pos, z_pos);
        spacecraft.orbitState.vsat.Set(x_vel, y_vel, z_vel);
        attitude->SetRPY(roll, pitch, yaw);

        //------------------------//
        // command the instrument //
        //------------------------//

        uint8 prf_cycle_time;
        SDreaddata_or_exit("prf_cycle_time", prf_cycle_time_sds_id, start,
            edges, (VOIDP)&prf_cycle_time);
        qscat.ses.CmdPriDn(prf_cycle_time);

        //------------------------//
        // estimate the spin rate //
        //------------------------//

        uint16 antenna_position[100];
        SDreaddata_or_exit("antenna_position", antenna_position_sds_id, start,
            edges, (VOIDP)antenna_position);
        unsigned int theta_max = antenna_position[99];
        unsigned int theta_min = antenna_position[0];
        if (theta_max < theta_min)
        {
            theta_max += ENCODER_N;
        }
        unsigned int dn_per_99 = theta_max - theta_min;
        double rad_per_99 = two_pi * (double)dn_per_99 / (double)ENCODER_N;
        double sec_per_99 = qscat.ses.pri * 99.0;
        double omega = rad_per_99 / sec_per_99;

        //-----------------------//
        // get other information //
        //-----------------------//

        uint32 orbit_time;
        SDreaddata_or_exit("orbit_time", orbit_time_sds_id, start, edges,
            (VOIDP)&orbit_time);

        uint8 doppler_orbit_step;
        SDreaddata_or_exit("doppler_orbit_step", doppler_orbit_step_sds_id,
            start, edges, (VOIDP)&doppler_orbit_step);

        int8 prf_orbit_step_change;
        SDreaddata_or_exit("prf_orbit_step_change",
            prf_orbit_step_change_sds_id, start, edges,
            (VOIDP)&prf_orbit_step_change);

        int8 specified_cal_pulse_pos;
        SDreaddata_or_exit("specified_cal_pulse_pos",
            specified_cal_pulse_pos_sds_id, start, edges,
            (VOIDP)&specified_cal_pulse_pos);

        uint32 power_dn[100][12];
        SDreaddata_or_exit("power_dn", power_dn_sds_id, start, edges,
            (VOIDP)power_dn);

        uint32 noise_dn[100];
        SDreaddata_or_exit("noise_dn", noise_dn_sds_id, start, edges,
            (VOIDP)noise_dn);

        uint8 range_gate_a_delay;
        SDreaddata_or_exit("range_gate_a_delay", range_gate_a_delay_sds_id,
            start, edges, (VOIDP)&range_gate_a_delay);

        uint8 range_gate_a_width;
        SDreaddata_or_exit("range_gate_a_width", range_gate_a_width_sds_id,
            start, edges, (VOIDP)&range_gate_a_width);

        uint8 range_gate_b_delay;
        SDreaddata_or_exit("range_gate_b_delay", range_gate_b_delay_sds_id,
            start, edges, (VOIDP)&range_gate_b_delay);

        uint8 range_gate_b_width;
        SDreaddata_or_exit("range_gate_b_width", range_gate_b_width_sds_id,
            start, edges, (VOIDP)&range_gate_b_width);

        uint32 doppler_shift_command_1;
        SDreaddata_or_exit("doppler_shift_command_1",
            doppler_shift_command_1_sds_id, start, edges,
            (VOIDP)&doppler_shift_command_1);

        uint32 doppler_shift_command_2;
        SDreaddata_or_exit("doppler_shift_command_2",
            doppler_shift_command_2_sds_id, start, edges,
            (VOIDP)&doppler_shift_command_2);

/*
        uint8 pulse_width;
        SDreaddata_or_exit("pulse_width", pulse_width_sds_id, start, edges,
            (VOIDP)&pulse_width);
*/

        //-----------------------------------------------//
        // set the ephemeris, orbit time, and orbit step //
        //-----------------------------------------------//

        echo_info.frameTime = frame_time;
        echo_info.gcX = x_pos;
        echo_info.gcY = y_pos;
        echo_info.gcZ = z_pos;
        echo_info.velX = x_vel;
        echo_info.velY = y_vel;
        echo_info.velZ = z_vel;
        echo_info.roll = roll;
        echo_info.pitch = pitch;
        echo_info.yaw = yaw;
        echo_info.orbitTicks = orbit_time;
        echo_info.orbitStep = doppler_orbit_step;
        echo_info.priOfOrbitStepChange = prf_orbit_step_change;
        echo_info.spinRate = omega;

        //--------------//
        // get L1B info //
        //--------------//

        float32 cell_lat[100];
        float32 cell_lon[100];
        int16 frequency_shift[100];
        SDreaddata_or_exit("cell_lat", cell_lat_sds_id, start, edges,
            (VOIDP)cell_lat);
        SDreaddata_or_exit("cell_lon", cell_lon_sds_id, start, edges,
            (VOIDP)cell_lon);
        SDreaddata_or_exit("frequency_shift", frequency_shift_sds_id,
            start, edges, (VOIDP)frequency_shift);
        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            echo_info.deltaF[spot_idx] = (float)frequency_shift[spot_idx]
                * frequency_shift_scale;
        }

        //--------------------//
        // step through spots //
        //--------------------//

        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            echo_info.quality_flag[spot_idx] = EchoInfo::OK;

            // determine beam index and beam
            int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
            qscat.cds.currentBeamIdx = beam_idx;
//            Beam* beam = qscat.GetCurrentBeam();

            // determine orbit step
            qscat.cds.orbitTime = orbit_time;

            unsigned char orbit_step = doppler_orbit_step;
            if (prf_orbit_step_change != -1 &&
                spot_idx < prf_orbit_step_change)
            {
                if (orbit_step == 0)
                    orbit_step = ORBIT_STEPS - 1;
                else
                    orbit_step--;
            }
            qscat.cds.orbitStep = orbit_step;

            // set the encoder for tracking
            unsigned short held_encoder = antenna_position[spot_idx];
            qscat.cds.heldEncoder = held_encoder;

            // get the ideal encoder for output
            unsigned short ideal_encoder =
                qscat.cds.EstimateIdealEncoder();
            echo_info.idealEncoder[spot_idx] = ideal_encoder;

            //------------------------------//
            // eliminate bad ephemeris data //
            //------------------------------//

            if (x_pos == 0.0 && y_pos == 0.0 && z_pos == 0.0)
            {
                if (opt_verbose) {
                    printf("  Bad ephemeris frame %d\n", record_idx);
                }
                echo_info.quality_flag[spot_idx] = EchoInfo::BAD_EPHEMERIS;
                wom_frame = 0;   // spin up again
                continue;
            }

            //-------------------------------//
            // do range and Doppler tracking //
            //-------------------------------//

            qscat.SetEncoderAzimuth(held_encoder, 1);
            qscat.SetOtherAzimuths(&spacecraft);
            echo_info.txCenterAzimuthAngle[spot_idx] =
                InRange(antenna->txCenterAzimuthAngle);

            SetDelayAndFrequency(&spacecraft, &qscat);

            echo_info.txDoppler[spot_idx] = qscat.ses.txDoppler;
            echo_info.rxGateDelay[spot_idx] = qscat.ses.rxGateDelay;

            //-----------------------------------//
            // skip pulses from first WOM frames //
            //-----------------------------------//

            if (wom_frame < 2) {
                continue;
            }

            //-------------------------//
            // skip calibration pulses //
            //-------------------------//

            if (spot_idx == specified_cal_pulse_pos - 2 ||
                spot_idx == specified_cal_pulse_pos - 1)
            {
                echo_info.quality_flag[spot_idx] = EchoInfo::CAL_OR_LOAD_PULSE;
                continue;
            }

            //-----------//
            // flag land //
            //-----------//
            // only need to do the geometry if L1B is not available

            float lon = cell_lon[spot_idx] * dtr;
            float lat = cell_lat[spot_idx] * dtr;
            int type = land_map.GetType(lon, lat);
            if (type == 0)
                echo_info.surface_flag[spot_idx] = EchoInfo::OCEAN;
            else
                echo_info.surface_flag[spot_idx] = EchoInfo::NOT_OCEAN;

            //-------------------------------//
            // determine total signal energy //
            //-------------------------------//

            double En = noise_dn[spot_idx];
            double Es = 0.0;
            for (int slice_idx = 0; slice_idx < 12; slice_idx++)
            {
                Es += power_dn[spot_idx][slice_idx];
            }

            double rho = 1.0;
            double beta = qscat.ses.rxGainNoise / qscat.ses.rxGainEcho;
            double alpha = Bn/Be*beta;
            double noise = Bs / Be * (rho / beta * En - Es) /
                (alpha * rho / beta - 1.0);

            //-----------------------------//
            // eliminate bad data by noise //
            //-----------------------------//

            double total_signal_energy = 0.0;
            for (int slice_idx = 1; slice_idx < 11; slice_idx++)
            {
                signal_energy[slice_idx] =
                    power_dn[spot_idx][slice_idx] - noise;
                total_signal_energy += signal_energy[slice_idx];
            }
            echo_info.totalSignalEnergy[spot_idx] = total_signal_energy;

            //----------------------------------------//
            // check if signal appears to be negative //
            //----------------------------------------//

            if (total_signal_energy < 10.0 * noise)
            {
                echo_info.quality_flag[spot_idx] = EchoInfo::BAD_PEAK;
                continue;
            }

            //---------------//
            // find the peak //
            //---------------//

            if (opt_gaussian)
            {
                float meas_spec_peak_slice, meas_spec_peak_freq, width;
                if (! gaussian_fit(&qscat, slice_number,
                    signal_energy + 1, 10, &meas_spec_peak_slice,
                    &meas_spec_peak_freq, &width, 1))
                {
                    echo_info.quality_flag[spot_idx] = EchoInfo::BAD_PEAK;
                    continue;
                }
                echo_info.measSpecPeakFreq[spot_idx] = meas_spec_peak_freq;
            }
            else
            {
                double centroid = 0.0;
                double sum = 0.0;
                for (int i = 0; i < 10; i++)
                {
                    double tofour = pow(signal_energy[i + 1], 4.0);
                    centroid += (double)i * tofour;
                    sum += tofour;
                }
                centroid /= sum;

                echo_info.measSpecPeakFreq[spot_idx] = f1 +
                    bw * (centroid + 0.5);
            }
        }

        //----------------------------------------------------------//
        // don't generate output for the first frames after badness //
        //----------------------------------------------------------//

        if (wom_frame < 2) {
            if (opt_verbose) {
                printf("  Drop frame %d\n", record_idx);
            }
            continue;
        }

        if (! echo_info.Write(ofd))
        {
            fprintf(stderr,
                "%s: error writing frame to output echo file %s\n",
                command, echo_file);
            exit(1);
        }
    }

    //---------------------//
    // end level 1a access //
    //---------------------//

    if (SDendaccess(operational_mode_sds_id) == FAIL ||
        SDendaccess(ses_configuration_flags_sds_id) == FAIL ||
        SDendaccess(frame_time_secs_sds_id) == FAIL ||
        SDendaccess(x_pos_sds_id) == FAIL ||
        SDendaccess(y_pos_sds_id) == FAIL ||
        SDendaccess(z_pos_sds_id) == FAIL ||
        SDendaccess(x_vel_sds_id) == FAIL ||
        SDendaccess(y_vel_sds_id) == FAIL ||
        SDendaccess(z_vel_sds_id) == FAIL ||
        SDendaccess(roll_sds_id) == FAIL ||
        SDendaccess(pitch_sds_id) == FAIL ||
        SDendaccess(yaw_sds_id) == FAIL ||
        SDendaccess(prf_cycle_time_sds_id) == FAIL ||
        SDendaccess(antenna_position_sds_id) == FAIL ||
        SDendaccess(orbit_time_sds_id) == FAIL ||
        SDendaccess(doppler_orbit_step_sds_id) == FAIL ||
        SDendaccess(prf_orbit_step_change_sds_id) == FAIL ||
        SDendaccess(specified_cal_pulse_pos_sds_id) == FAIL ||
        SDendaccess(power_dn_sds_id) == FAIL ||
        SDendaccess(noise_dn_sds_id) == FAIL ||
        SDendaccess(range_gate_a_delay_sds_id) == FAIL ||
        SDendaccess(range_gate_a_width_sds_id) == FAIL ||
        SDendaccess(range_gate_b_delay_sds_id) == FAIL ||
        SDendaccess(range_gate_b_width_sds_id) == FAIL ||
        SDendaccess(doppler_shift_command_1_sds_id) == FAIL ||
        SDendaccess(doppler_shift_command_2_sds_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending L1A SD access\n", command);
        exit(1);
    }
    if (SDend(l1a_sd_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending L1A SD\n", command);
        exit(1);
    }

    //---------------------//
    // end level 1b access //
    //---------------------//

    if (SDendaccess(cell_lat_sds_id) == FAIL ||
        SDendaccess(cell_lon_sds_id) == FAIL ||
        SDendaccess(frequency_shift_sds_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending L1B SD access\n", command);
        exit(1);
    }
    if (SDend(l1b_sd_id) == FAIL)
    {
        fprintf(stderr, "%s: error ending L1B SD\n", command);
        exit(1);
    }

    //-------------------//
    // close output file //
    //-------------------//

    close(ofd);

    return (0);
}
