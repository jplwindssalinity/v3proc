//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    echo_proc
//
// SYNOPSIS
//    echo_proc [ -cg ] [ -b l1b_file ] <ins_config_file> <l1a_file>
//        <echo_file>
//        
// DESCRIPTION
//    Reads HDF L1A/L1AP and L1B files and generates echo data files
//    containing a bunch of useful information about the echo
//    and the transmit/receive conditions.  User can specify whether
//    to do a gaussian fit and/or a weighted centroid.
//
// OPTIONS
//    [ -c ]           Compute the centroid.
//    [ -g ]           Compute the gaussian fit.
//    [ -b l1b_file ]  Analyze the specified l1b_file.
//
// OPERANDS
//    <ins_config_file>  Use the specified instrument configuration file.
//    <l1a_file>         Analyze the specified l1a_file.
//    <echo_file>        Store output in the specified echo_file.
//                       If the echo file already exists, echo_proc
//                       will insert the appropriate parts.
//
// EXAMPLES
//    echo_proc -c -g qscat.cfg /disk1/QS_S1A.dat QS_S1A.echo
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

#define OPTSTRING  "b:cg"

#define WOM              0x0E

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
void   check_status(HdfFile::StatusE status);

int ExtractAntSpinRateDnPer99(TlmHdfFile* l1File, int32* sdsIDs,
        int32 start, int32 stride, int32 length, VOIDP buffer,
        PolynomialTable* poly_table);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo leap_second_table_arg = LEAP_SECOND_TABLE_ARG;

const char* usage_array[] = { "[ -cg ]", "[ -b l1b_file ]",
    "<ins_config_file>", "<l1a_file>", "<echo_file>", 0 };

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

    char* l1b_filename = NULL;
    int opt_centroid = 0;
    int opt_gaussian = 0;

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
        case 'b':
            l1b_filename = optarg;
            break;
        case 'c':
            opt_centroid = 1;
            break;
        case 'g':
            opt_gaussian = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 3)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* l1a_filename = argv[optind++];
    const char* echo_file = argv[optind++];

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

    //-----------------//
    // check arguments //
    //-----------------//

    if (! opt_centroid && ! opt_gaussian)
    {
        fprintf(stderr,
            "%s: must specify -c (centroid) and/or -g (gaussian)\n", command);
        usage(command, usage_array, 1);
    }

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

    //-----------------//
    // FOR LEVEL 1A... //
    //-----------------//

    Parameter* frame_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        FRAME_TIME, UNIT_CODE_A);
    Parameter* operational_mode_p = ParTabAccess::GetParameter(SOURCE_L1A,
        OPERATIONAL_MODE, UNIT_MAP);
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

    HdfFile::StatusE status;
    L1AFile l1a_file(l1a_filename, status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "%s: error opening L1A file %s\n", command,
            l1a_filename);
        exit(1);
    }

    check_status(l1a_file.OpenParamDatasets(frame_time_p));
    check_status(l1a_file.OpenParamDatasets(operational_mode_p));
    check_status(l1a_file.OpenParamDatasets(xpos_p));
    check_status(l1a_file.OpenParamDatasets(ypos_p));
    check_status(l1a_file.OpenParamDatasets(zpos_p));
    check_status(l1a_file.OpenParamDatasets(xvel_p));
    check_status(l1a_file.OpenParamDatasets(yvel_p));
    check_status(l1a_file.OpenParamDatasets(zvel_p));
    check_status(l1a_file.OpenParamDatasets(roll_p));
    check_status(l1a_file.OpenParamDatasets(pitch_p));
    check_status(l1a_file.OpenParamDatasets(yaw_p));
    check_status(l1a_file.OpenParamDatasets(ant_pos_p));
    check_status(l1a_file.OpenParamDatasets(orbit_time_p));
    check_status(l1a_file.OpenParamDatasets(orbit_step_p));
    check_status(l1a_file.OpenParamDatasets(pri_of_orbit_step_change_p));
    check_status(l1a_file.OpenParamDatasets(cal_pulse_pos_p));
    check_status(l1a_file.OpenParamDatasets(slice_powers_p));
    check_status(l1a_file.OpenParamDatasets(prf_cycle_time_p));
    check_status(l1a_file.OpenParamDatasets(noise_meas_p));

    //--------------//
    // FOR LEVEL 1B //
    //--------------//

    Parameter* l1b_frame_time_p = NULL;
    Parameter* cell_lon_p = NULL;
    Parameter* cell_lat_p = NULL;
    Parameter* delta_f_p = NULL;

    L1BHdfFile* l1b_file = NULL;
    if (l1b_filename != NULL)
    {
        l1b_frame_time_p = ParTabAccess::GetParameter(SOURCE_L1A, FRAME_TIME,
            UNIT_CODE_A);

        cell_lon_p = ParTabAccess::GetParameter(SOURCE_L1B, CELL_LON,
            UNIT_RADIANS);

        cell_lat_p = ParTabAccess::GetParameter(SOURCE_L1B, CELL_LAT,
            UNIT_RADIANS);

        delta_f_p = ParTabAccess::GetParameter(SOURCE_L1B, FREQUENCY_SHIFT,
            UNIT_HZ);

        l1b_file = new L1BHdfFile(l1b_filename, status);
        if (l1b_file == NULL || status != HdfFile::OK)
        {
            fprintf(stderr, "%s: error creating L1B file object\n", command);
            exit(1);
        }

        check_status(l1b_file->OpenParamDatasets(l1b_frame_time_p));
        check_status(l1b_file->OpenParamDatasets(cell_lon_p));
        check_status(l1b_file->OpenParamDatasets(cell_lat_p));
        check_status(l1b_file->OpenParamDatasets(delta_f_p));
    }

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

    int l1a_length = l1a_file.GetDataLength();
    if (l1b_file != NULL)
    {
        int l1b_length = l1b_file->GetDataLength();
        if (l1b_length != l1a_length)
        {
            fprintf(stderr, "%s: mismatched data lengths\n", command);
            fprintf(stderr, "  %s : %d records\n", l1a_filename, l1a_length);
            fprintf(stderr, "  %s : %d records\n", l1b_filename, l1b_length);
            exit(1);
        }
    }

    //------------------------//
    // process frame by frame //
    //------------------------//

    int wom_frame = 0;

    EchoInfo echo_info;
    for (int record_idx = 0; record_idx < l1a_length; record_idx++)
    {
        //-----------------------//
        // only process WOM data //
        //-----------------------//

        unsigned char operational_mode;
        operational_mode_p->extractFunc(&l1a_file, operational_mode_p->sdsIDs,
            record_idx, 1, 1, &operational_mode, polyTable);
        if (operational_mode != WOM)
            continue;

        wom_frame++;

        //--------------//
        // get the time //
        //--------------//

        char time[6];
        frame_time_p->extractFunc(&l1a_file, frame_time_p->sdsIDs, record_idx,
            1, 1, &time, polyTable);
        ETime frame_time;
        frame_time.FromChar6(time);

        //--------------------//
        // set the spacecraft //
        //--------------------//

        float xpos, ypos, zpos;
        xpos_p->extractFunc(&l1a_file, xpos_p->sdsIDs, record_idx, 1, 1,
            &xpos, polyTable);
        ypos_p->extractFunc(&l1a_file, ypos_p->sdsIDs, record_idx, 1, 1,
            &ypos, polyTable);
        zpos_p->extractFunc(&l1a_file, zpos_p->sdsIDs, record_idx, 1, 1,
            &zpos, polyTable);

        float xvel, yvel, zvel;
        xvel_p->extractFunc(&l1a_file, xvel_p->sdsIDs, record_idx, 1, 1,
            &xvel, polyTable);
        yvel_p->extractFunc(&l1a_file, yvel_p->sdsIDs, record_idx, 1, 1,
            &yvel, polyTable);
        zvel_p->extractFunc(&l1a_file, zvel_p->sdsIDs, record_idx, 1, 1,
            &zvel, polyTable);

        float roll, pitch, yaw;
        roll_p->extractFunc(&l1a_file, roll_p->sdsIDs, record_idx, 1, 1,
            &roll, polyTable);
        pitch_p->extractFunc(&l1a_file, pitch_p->sdsIDs, record_idx, 1, 1,
            &pitch, polyTable);
        yaw_p->extractFunc(&l1a_file, yaw_p->sdsIDs, record_idx, 1, 1, &yaw,
            polyTable);

        spacecraft.orbitState.rsat.Set(xpos, ypos, zpos);
        spacecraft.orbitState.vsat.Set(xvel, yvel, zvel);
        attitude->SetRPY(roll, pitch, yaw);

        //------------------------//
        // command the instrument //
        //------------------------//

        unsigned char pri_dn;
        prf_cycle_time_p->extractFunc(&l1a_file, prf_cycle_time_p->sdsIDs,
            record_idx, 1, 1, &pri_dn, polyTable);

        qscat.ses.CmdPriDn(pri_dn);

        //------------------------//
        // estimate the spin rate //
        //------------------------//

        unsigned short ant_pos[100];
        ant_pos_p->extractFunc(&l1a_file, ant_pos_p->sdsIDs, record_idx, 1,
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
        orbit_time_p->extractFunc(&l1a_file, orbit_time_p->sdsIDs,
            record_idx, 1, 1, &orbit_time, polyTable);

        unsigned char base_orbit_step;
        orbit_step_p->extractFunc(&l1a_file, orbit_step_p->sdsIDs,
            record_idx, 1, 1, &base_orbit_step, polyTable);

        unsigned char pri_of_orbit_step_change;
        pri_of_orbit_step_change_p->extractFunc(&l1a_file,
            pri_of_orbit_step_change_p->sdsIDs, record_idx, 1, 1,
            &pri_of_orbit_step_change, polyTable);

        unsigned char cal_pulse_pos;
        cal_pulse_pos_p->extractFunc(&l1a_file, cal_pulse_pos_p->sdsIDs,
            record_idx, 1, 1, &cal_pulse_pos, polyTable);

        unsigned int slice_powers[1200];
        slice_powers_p->extractFunc(&l1a_file, slice_powers_p->sdsIDs,
            record_idx, 1, 1, slice_powers, polyTable);
        unsigned int noise_powers[100];
        noise_meas_p->extractFunc(&l1a_file, noise_meas_p->sdsIDs,
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

        //--------------//
        // get L1B info //
        //--------------//

        float l1b_cell_lon[100];
        float l1b_cell_lat[100];
        short delta_f[100]; 
        if (l1b_file != NULL)
        {
            cell_lon_p->extractFunc(l1b_file, cell_lon_p->sdsIDs, record_idx,
                1, 1, l1b_cell_lon, polyTable);
            cell_lat_p->extractFunc(l1b_file, cell_lat_p->sdsIDs, record_idx,
                1, 1, l1b_cell_lat, polyTable);

            //-------------//
            // set delta f //
            //-------------//

            delta_f_p->extractFunc(l1b_file, delta_f_p->sdsIDs, record_idx,
                1, 1, delta_f, polyTable);
            for (int spot_idx = 0; spot_idx < 100; spot_idx++)
            {
                echo_info.deltaF[spot_idx] = (float)delta_f[spot_idx];
            }
        }

        //--------------------//
        // step through spots //
        //--------------------//

        for (int spot_idx = 0; spot_idx < 100; spot_idx++)
        {
            echo_info.flag[spot_idx] = EchoInfo::OK;

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

            //------------------------------//
            // eliminate bad ephemeris data //
            //------------------------------//

            if (xpos == 0.0 && ypos == 0.0 && zpos == 0.0)
            {
                echo_info.flag[spot_idx] = EchoInfo::BAD_EPHEMERIS;
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

            //---------------------------------------//
            // skip pulses from first two WOM frames //
            //---------------------------------------//

            if (wom_frame < 3)
                continue;

            //-------------------------//
            // skip calibration pulses //
            //-------------------------//

            if (spot_idx == cal_pulse_pos - 2 ||
                spot_idx == cal_pulse_pos - 1)
            {
                echo_info.flag[spot_idx] = EchoInfo::CAL_OR_LOAD_PULSE;
                continue;
            }

            //-----------//
            // flag land //
            //-----------//
            // only need to do the geometry if L1B is not available

            float lon, lat;
            if (l1b_file != NULL)
            {
                lon = l1b_cell_lon[spot_idx];
                lat = l1b_cell_lat[spot_idx];
            }
            else
            {
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
                lon = (float)lon_d;
                lat = (float)lat_d;
            }

            int type = land_map.GetType(lon, lat);
            if (type != 0)    // not ocean
            {
                echo_info.flag[spot_idx] = EchoInfo::NOT_OCEAN;
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

            //-----------------------------//
            // eliminate bad data by noise //
            //-----------------------------//

            double total_signal_energy = 0.0;
            for (int slice_idx = 1; slice_idx < 11; slice_idx++)
            {
                signal_energy[slice_idx] =
                    slice_powers[base_slice_idx + slice_idx] - noise;
                total_signal_energy += signal_energy[slice_idx];
            }
            echo_info.totalSignalEnergy[spot_idx] = total_signal_energy;

            //----------------------------------------//
            // check if signal appears to be negative //
            //----------------------------------------//

            if (total_signal_energy < 10.0 * noise)
            {
                echo_info.flag[spot_idx] = EchoInfo::BAD_PEAK;
                continue;
            }

            //---------------//
            // find the peak //
            //---------------//

            float meas_spec_peak_slice, meas_spec_peak_freq, width;
            if (! gaussian_fit(&qscat, slice_number,
                signal_energy + 1, 10, &meas_spec_peak_slice,
                &meas_spec_peak_freq, &width, 1))
            {
                echo_info.flag[spot_idx] = EchoInfo::BAD_PEAK;
                continue;
            }
            echo_info.measSpecPeakFreq[spot_idx] = meas_spec_peak_freq;
        }

        if (record_idx == 0)
            continue;

        if (! echo_info.Write(ofd))
        {
            fprintf(stderr,
                "%s: error writing frame to output echo file %s\n",
                command, echo_file);
            exit(1);
        }
    }

    //----------------//
    // close datasets //
    //----------------//

    l1a_file.CloseParamDatasets(frame_time_p);
    l1a_file.CloseParamDatasets(xpos_p);
    l1a_file.CloseParamDatasets(ypos_p);
    l1a_file.CloseParamDatasets(zpos_p);
    l1a_file.CloseParamDatasets(xvel_p);
    l1a_file.CloseParamDatasets(yvel_p);
    l1a_file.CloseParamDatasets(zvel_p);
    l1a_file.CloseParamDatasets(roll_p);
    l1a_file.CloseParamDatasets(pitch_p);
    l1a_file.CloseParamDatasets(yaw_p);
    l1a_file.CloseParamDatasets(ant_pos_p);
    l1a_file.CloseParamDatasets(orbit_time_p);
    l1a_file.CloseParamDatasets(orbit_step_p);
    l1a_file.CloseParamDatasets(pri_of_orbit_step_change_p);
    l1a_file.CloseParamDatasets(slice_powers_p);
    l1a_file.CloseParamDatasets(prf_cycle_time_p);
    l1a_file.CloseParamDatasets(noise_meas_p);

    if (l1b_file != NULL)
    {
        l1b_file->CloseParamDatasets(l1b_frame_time_p);
        l1b_file->CloseParamDatasets(cell_lon_p);
        l1b_file->CloseParamDatasets(cell_lat_p);
        l1b_file->CloseParamDatasets(delta_f_p);
    }

    //-------------------//
    // close output file //
    //-------------------//

    close(ofd);

    //----------------//
    // delete objects //
    //----------------//

    if (l1b_file != NULL)
        delete l1b_file;

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
