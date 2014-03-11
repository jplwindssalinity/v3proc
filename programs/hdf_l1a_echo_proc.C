//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_l1a_echo_proc
//
// SYNOPSIS
//    hdf_l1a_echo_proc [ -tlm tlm_file... ] [ -o output_base ]
//      [ -polytable polytable ] [ -ins ins_config_file ]
//      [ -m gauss|cent ]
//
// DESCRIPTION
//    Reads HDF L1A (or L1AP) files and generates echo data files
//    containing a bunch of useful information about the echo
//    and the transmit/receive conditions.  Has the option of using
//    a gaussian fit or a weighted centroid to determine the echo
//    center.
//
// OPERANDS
//    N/A
//
// EXAMPLES
//    N/A
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
#include "BufferedList.h"
#include "Tracking.h"
#include "InstrumentGeom.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "Qscat.h"
#include "LandMap.h"
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

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void*  realloc_or_exit(const char* command, const char* label, int size,
           void* orig_ptr);

int ExtractAntSpinRateDnPer99(TlmHdfFile* l1File, int32* sdsIDs,
        int32 start, int32 stride, int32 length, VOIDP buffer,
        PolynomialTable* poly_table);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

ArgInfo tlm_files_arg = TLM_FILES_ARG;
ArgInfo l1a_files_arg = L1A_FILES_ARG;
ArgInfo output_base_arg = { NULL, "-ob", "output_base" };
ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo leap_second_table_arg = LEAP_SECOND_TABLE_ARG;
ArgInfo ins_config_arg = { "INSTRUMENT_CONFIG_FILE", "-ins",
    "ins_config_file" };

ArgInfo* arg_info_array[] =
{
    &tlm_files_arg,
    &l1a_files_arg,
    &output_base_arg,
    &poly_table_arg,
    &ins_config_arg,
    0
};

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
    char* config_filename = getenv(ENV_CONFIG_FILENAME);
    static ArgsPlus args_plus = ArgsPlus(argc, argv, config_filename,
        arg_info_array);

    if (argc == 1)
    {
        args_plus.Usage();
        exit(1);
    }

    char* tlm_files_string = args_plus.Get(tlm_files_arg);
    char* l1a_files_string = args_plus.Get(l1a_files_arg);
    char* output_base_string = args_plus.Get(output_base_arg);
    char* poly_table_string = args_plus.Get(poly_table_arg);
    char* leap_second_table_string = args_plus.Get(leap_second_table_arg);
    char* ins_config_string = args_plus.Get(ins_config_arg);

    //-------------------//
    // convert arguments //
    //-------------------//

    SourceIdE tlm_type = SOURCE_L1A;
    char* tlm_files = args_plus.GetTlmFilesOrExit(tlm_files_string, tlm_type,
        NULL, l1a_files_string, NULL, NULL, NULL);

    //---------------//
    // use arguments //
    //---------------//

    TlmFileList* tlm_file_list = args_plus.TlmFileListOrExit(tlm_type,
        tlm_files, INVALID_TIME, INVALID_TIME);
    PolynomialTable* polyTable =
        args_plus.PolynomialTableOrNull(poly_table_string);
    args_plus.LeapSecTableOrExit(leap_second_table_string);

    //---------------------//
    // read in config file //
    //---------------------//

    if (! ins_config_string)
    {
        fprintf(stderr, "%s: missing instrument configuration file\n",
            command);
        exit(1);
    }
    ConfigList config_list;
    if (! config_list.Read(ins_config_string))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, ins_config_string);
        exit(1);
    }

    //-----------//
    // variables //
    //-----------//

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
    double Bn = qscat.ses.noiseBandwidth;
    double Be = qscat.ses.GetTotalSignalBandwidth();
    double Bs = qscat.ses.scienceSliceBandwidth;

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
    // get necessary parameters //
    //--------------------------//

    Parameter* frame_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        FRAME_TIME, UNIT_CODE_A);

    Parameter* xpos_p = ParTabAccess::GetParameter(SOURCE_L1A, X_POS,
        UNIT_KILOMETERS);
    Parameter* ypos_p = ParTabAccess::GetParameter(SOURCE_L1A, Y_POS,
        UNIT_KILOMETERS);
    Parameter* zpos_p = ParTabAccess::GetParameter(SOURCE_L1A, Z_POS,
        UNIT_KILOMETERS);

    Parameter* xvel_p = ParTabAccess::GetParameter(SOURCE_L1A, X_VEL,
        UNIT_KMPS);
    Parameter* yvel_p = ParTabAccess::GetParameter(SOURCE_L1A, Y_VEL,
        UNIT_KMPS);
    Parameter* zvel_p = ParTabAccess::GetParameter(SOURCE_L1A, Z_VEL,
        UNIT_KMPS);

    Parameter* roll_p = ParTabAccess::GetParameter(SOURCE_L1A, ROLL,
        UNIT_RADIANS);
    Parameter* pitch_p = ParTabAccess::GetParameter(SOURCE_L1A, PITCH,
        UNIT_RADIANS);
    Parameter* yaw_p = ParTabAccess::GetParameter(SOURCE_L1A, YAW,
        UNIT_RADIANS);

    Parameter* ant_pos_p = ParTabAccess::GetParameter(SOURCE_L1A,
        ANTENNA_POS, UNIT_DN);

    Parameter* orbit_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        ORBIT_TIME, UNIT_COUNTS);
    Parameter* orbit_step_p = ParTabAccess::GetParameter(SOURCE_L1A,
        DOPPLER_ORBIT_STEP, UNIT_COUNTS);
    Parameter* pri_of_orbit_step_change_p =
        ParTabAccess::GetParameter(SOURCE_L1A, PRF_ORBIT_STEP_CHANGE,
        UNIT_COUNTS);

    Parameter* cal_pulse_pos_p = ParTabAccess::GetParameter(SOURCE_L1A,
        CAL_PULSE_POS, UNIT_DN);

    Parameter* slice_powers_p = ParTabAccess::GetParameter(SOURCE_L1A,
        POWER_DN, UNIT_DN);
    Parameter* noise_meas_p = ParTabAccess::GetParameter(SOURCE_L1A,
        NOISE_DN, UNIT_DN);

    Parameter* prf_cycle_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        PRF_CYCLE_TIME, UNIT_DN);

    //---------------------------//
    // process data file by file //
    //---------------------------//

    EchoInfo echo_info;

    for (TlmHdfFile* tlmFile = tlm_file_list->GetHead(); tlmFile;
        tlmFile = tlm_file_list->GetNext())
    {
        //--------------------//
        // create output file //
        //--------------------//

        const char* input_filename = tlmFile->GetFileName();
        const char* tail = no_path(input_filename);
        char output_filename[2048];
        sprintf(output_filename, "%s.%s", output_base_string, tail);
        int ofd = creat(output_filename, 0644);
        if (ofd == -1)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                output_filename);
            exit(1);
        }

        //-------------------------//
        // open necessary datasets //
        //-------------------------//

        HdfFile::StatusE status;
        status = tlmFile->OpenParamDatasets(frame_time_p);
        status = tlmFile->OpenParamDatasets(xpos_p);
        status = tlmFile->OpenParamDatasets(ypos_p);
        status = tlmFile->OpenParamDatasets(zpos_p);
        status = tlmFile->OpenParamDatasets(xvel_p);
        status = tlmFile->OpenParamDatasets(yvel_p);
        status = tlmFile->OpenParamDatasets(zvel_p);
        status = tlmFile->OpenParamDatasets(roll_p);
        status = tlmFile->OpenParamDatasets(pitch_p);
        status = tlmFile->OpenParamDatasets(yaw_p);
        status = tlmFile->OpenParamDatasets(ant_pos_p);
        status = tlmFile->OpenParamDatasets(orbit_time_p);
        status = tlmFile->OpenParamDatasets(orbit_step_p);
        status = tlmFile->OpenParamDatasets(pri_of_orbit_step_change_p);
        status = tlmFile->OpenParamDatasets(cal_pulse_pos_p);
        status = tlmFile->OpenParamDatasets(slice_powers_p);
        status = tlmFile->OpenParamDatasets(prf_cycle_time_p);
        status = tlmFile->OpenParamDatasets(noise_meas_p);

        //------------------------//
        // process frame by frame //
        //------------------------//

        int in_first_frame = 1;

        for (int record_idx = 0; record_idx < tlmFile->GetDataLength();
            record_idx++)
        {
            //--------------//
            // get the time //
            //--------------//

            char time[6];
            frame_time_p->extractFunc(tlmFile, frame_time_p->sdsIDs,
                record_idx, 1, 1, &time, polyTable);
            ETime frame_time;
            frame_time.FromChar6(time);

            //--------------------//
            // set the spacecraft //
            //--------------------//

            float xpos, ypos, zpos;
            xpos_p->extractFunc(tlmFile, xpos_p->sdsIDs, record_idx, 1, 1,
                &xpos, polyTable);
            ypos_p->extractFunc(tlmFile, ypos_p->sdsIDs, record_idx, 1, 1,
                &ypos, polyTable);
            zpos_p->extractFunc(tlmFile, zpos_p->sdsIDs, record_idx, 1, 1,
                &zpos, polyTable);

            float xvel, yvel, zvel;
            xvel_p->extractFunc(tlmFile, xvel_p->sdsIDs, record_idx, 1, 1,
                &xvel, polyTable);
            yvel_p->extractFunc(tlmFile, yvel_p->sdsIDs, record_idx, 1, 1,
                &yvel, polyTable);
            zvel_p->extractFunc(tlmFile, zvel_p->sdsIDs, record_idx, 1, 1,
                &zvel, polyTable);

            float roll, pitch, yaw;
            roll_p->extractFunc(tlmFile, roll_p->sdsIDs, record_idx, 1, 1,
                &roll, polyTable);
            pitch_p->extractFunc(tlmFile, pitch_p->sdsIDs, record_idx, 1, 1,
                &pitch, polyTable);
            yaw_p->extractFunc(tlmFile, yaw_p->sdsIDs, record_idx, 1, 1, &yaw,
                polyTable);

            spacecraft.orbitState.rsat.Set(xpos, ypos, zpos);
            spacecraft.orbitState.vsat.Set(xvel, yvel, zvel);
            attitude->SetRPY(roll, pitch, yaw);

            //------------------------//
            // command the instrument //
            //------------------------//

            unsigned char pri_dn;
            prf_cycle_time_p->extractFunc(tlmFile, prf_cycle_time_p->sdsIDs,
                record_idx, 1, 1, &pri_dn, polyTable);

            qscat.ses.CmdPriDn(pri_dn);

            //------------------------//
            // estimate the spin rate //
            //------------------------//

            unsigned short ant_pos[100];
            ant_pos_p->extractFunc(tlmFile, ant_pos_p->sdsIDs, record_idx, 1,
                1, ant_pos, polyTable);

            unsigned int theta_max = ant_pos[99];
            unsigned int theta_min = ant_pos[0];
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

            unsigned int orbit_time;
            orbit_time_p->extractFunc(tlmFile, orbit_time_p->sdsIDs,
                record_idx, 1, 1, &orbit_time, polyTable);

            unsigned char base_orbit_step;
            orbit_step_p->extractFunc(tlmFile, orbit_step_p->sdsIDs,
                record_idx, 1, 1, &base_orbit_step, polyTable);

            unsigned char pri_of_orbit_step_change;
            pri_of_orbit_step_change_p->extractFunc(tlmFile,
                pri_of_orbit_step_change_p->sdsIDs, record_idx, 1, 1,
                &pri_of_orbit_step_change, polyTable);

            unsigned char cal_pulse_pos;
            cal_pulse_pos_p->extractFunc(tlmFile, cal_pulse_pos_p->sdsIDs,
                record_idx, 1, 1, &cal_pulse_pos, polyTable);

            unsigned int slice_powers[1200];
            slice_powers_p->extractFunc(tlmFile, slice_powers_p->sdsIDs,
                record_idx, 1, 1, slice_powers, polyTable);
            unsigned int noise_powers[100];
            noise_meas_p->extractFunc(tlmFile, noise_meas_p->sdsIDs,
                record_idx, 1, 1, noise_powers, polyTable);

            //-----------------------------------------------//
            // set the ephemeris, orbit time, and orbit step //
            //-----------------------------------------------//

            echo_info.frameTime = frame_time;
            echo_info.gcX = xpos;
            echo_info.gcY = ypos;
            echo_info.gcZ = zpos;
            echo_info.velX = xvel;
            echo_info.velY = yvel;
            echo_info.velZ = zvel;
            echo_info.roll = roll;
            echo_info.pitch = pitch;
            echo_info.yaw = yaw;
            echo_info.orbitTicks = orbit_time;
            echo_info.orbitStep = base_orbit_step;
            echo_info.priOfOrbitStepChange = pri_of_orbit_step_change;
            echo_info.spinRate = omega;

            //--------------------//
            // step through spots //
            //--------------------//

            for (int spot_idx = 0; spot_idx < 100; spot_idx++)
            {
                echo_info.quality_flag[spot_idx] = EchoInfo::OK;

                // determine beam index and beam
                int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
                qscat.cds.currentBeamIdx = beam_idx;
                Beam* beam = qscat.GetCurrentBeam();

                // determine starting slice index
                int base_slice_idx = spot_idx * 12;

                // determine orbit step
                qscat.cds.orbitTime = orbit_time;

                unsigned char orbit_step = base_orbit_step;
                if (pri_of_orbit_step_change != 255 &&
                    spot_idx < pri_of_orbit_step_change)
                {
                    if (orbit_step == 0)
                        orbit_step = ORBIT_STEPS - 1;
                    else
                        orbit_step--;
                }
                qscat.cds.orbitStep = orbit_step;

                // set the encoder for tracking
                unsigned short held_encoder = ant_pos[spot_idx];
                qscat.cds.heldEncoder = held_encoder;

                // get the ideal encoder for output
                unsigned short ideal_encoder =
                    qscat.cds.EstimateIdealEncoder();
                echo_info.idealEncoder[spot_idx] = ideal_encoder;

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

                //------------------------------//
                // skip pulses from first frame //
                //------------------------------//

                if (in_first_frame)
                    continue;

                //-------------------------//
                // skip calibration pulses //
                //-------------------------//

                if (spot_idx == cal_pulse_pos - 2 ||
                    spot_idx == cal_pulse_pos - 1)
                {
                    echo_info.quality_flag[spot_idx] =
                        EchoInfo::CAL_OR_LOAD_PULSE;
                    continue;
                }

                //-----------//
                // flag land //
                //-----------//

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
                double alt, lon, lat;
                if (! qti.rTarget.GetAltLonGCLat(&alt, &lon, &lat))
                {
                    fprintf(stderr, "%s: error finding alt/lon/lat\n",
                        command);
                    exit(1);
                }
                if (land_map.IsLand(lon, lat))
                {
                    echo_info.surface_flag[spot_idx] = EchoInfo::NOT_OCEAN;
                    continue;
                }

                //-------------------------------//
                // determine total signal energy //
                //-------------------------------//

                double En = noise_powers[spot_idx];
                double Es = 0.0;
                for (int slice_idx = 0; slice_idx < 12; slice_idx++)
                {
                    Es += slice_powers[base_slice_idx + slice_idx];
                }

                double rho = 1.0;
                double beta = qscat.ses.rxGainNoise / qscat.ses.rxGainEcho;
                double alpha = Bn/Be*beta;
                double noise = Bs / Be * (rho / beta * En - Es) /
                    (alpha * rho / beta - 1.0);

                double total_signal_energy = 0.0;
                for (int slice_idx = 0; slice_idx < 12; slice_idx++)
                {
                    signal_energy[slice_idx] =
                        slice_powers[base_slice_idx + slice_idx] - noise;
                    total_signal_energy += signal_energy[slice_idx];
                }
                echo_info.totalSignalEnergy[spot_idx] = total_signal_energy;

                //---------------//
                // find the peak //
                //---------------//

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

            //---------------------//
            // write out the frame //
            //---------------------//

            if (in_first_frame)
            {
                in_first_frame = 0;
                continue;
            }

            if (! echo_info.Write(ofd))
            {
                fprintf(stderr,
                    "%s: error writing frame to output echo file %s\n",
                    command, output_filename);
                exit(1);
            }
        }

        //----------------//
        // close datasets //
        //----------------//

        tlmFile->CloseParamDatasets(frame_time_p);
        tlmFile->CloseParamDatasets(xpos_p);
        tlmFile->CloseParamDatasets(ypos_p);
        tlmFile->CloseParamDatasets(zpos_p);
        tlmFile->CloseParamDatasets(xvel_p);
        tlmFile->CloseParamDatasets(yvel_p);
        tlmFile->CloseParamDatasets(zvel_p);
        tlmFile->CloseParamDatasets(roll_p);
        tlmFile->CloseParamDatasets(pitch_p);
        tlmFile->CloseParamDatasets(yaw_p);
        tlmFile->CloseParamDatasets(ant_pos_p);
        tlmFile->CloseParamDatasets(orbit_time_p);
        tlmFile->CloseParamDatasets(orbit_step_p);
        tlmFile->CloseParamDatasets(pri_of_orbit_step_change_p);
        tlmFile->CloseParamDatasets(slice_powers_p);
        tlmFile->CloseParamDatasets(prf_cycle_time_p);
        tlmFile->CloseParamDatasets(noise_meas_p);

        //-------------------//
        // close output file //
        //-------------------//

        close(ofd);
    }

    //----------------//
    // delete objects //
    //----------------//

    delete tlm_file_list;

    return (0);
}

//-----------------//
// realloc_or_exit //
//-----------------//
// reallocs (if necessary) or exits with an error message

void*
realloc_or_exit(
    const char*  command,   // command name (for error msg)
    const char*  label,     // variable label (for error msg)
    int          size,      // size to allocate
    void*        orig_ptr)  // original pointer
{
    void* return_ptr = realloc(orig_ptr, size);
    if (return_ptr == NULL)
    {
        fprintf(stderr, "%s: can't allocate for %s\n", command, label);
        exit(1);
    }
    return(return_ptr);
}
