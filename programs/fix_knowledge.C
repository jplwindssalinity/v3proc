//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_knowledge
//
// SYNOPSIS
//    fix_knowledge <sim_config_file> <echo_data_file>
//
// DESCRIPTION
//    Reads the echo data file and estimates the roll, pitch, and yaw
//    of the instrument and the look angle of each antenna beam.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <echo_data_file>   The echo data input file.
//
// EXAMPLES
//    An example of a command line is:
//      % fix_knowledge qscat.cfg echo.dat
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
//    James N. Huddleston
//    hudd@casket.jpl.nasa.gov
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
#include "L1A.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "AngleInterval.h"
#include "echo_funcs.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define QUOTES    '"'

#define LINE_SIZE  1024

#define MAXIMUM_SPOTS_PER_ORBIT_STEP  10000
#define MAXIMUM_EPHEM_PER_ORBIT_STEP  200

#define SIGNAL_ENERGY_THRESHOLD       1.0E-8

#define EPHEMERIS_CHAR   'E'
#define ORBIT_STEP_CHAR  'O'
#define ORBIT_TIME_CHAR  'T'

#define ROLL_START_STEP   0.001    // about 0.057 degrees
#define PITCH_START_STEP  0.001    // about 0.057 degrees
#define YAW_START_STEP    0.001    // about 0.057 degrees

#define ROLL_END_STEP   0.0001    // about 0.0057 degrees
#define PITCH_END_STEP  0.0001    // about 0.0057 degrees
#define YAW_END_STEP    0.0001    // about 0.0057 degrees


//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     initialize();
int     process_orbit_step(Spacecraft* spacecraft, Qscat* qscat,
            int orbit_step);
int     optimize(Spacecraft* spacecraft, Qscat* qscat, float* roll,
            float* pitch, float* yaw, float roll_step, float pitch_step, 
            float yaw_step);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, float roll,
            float pitch, float yaw, FILE* ofp = NULL);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<echo_data_file>", 0};

struct Ephem
{
    float  gcx;
    float  gcy;
    float  gcz;
    float  velx;
    float  vely;
    float  velz;
    float  roll;
    float  pitch;
    float  yaw;
};

int       g_ephem_count = 0;
Ephem     g_ephem[MAXIMUM_EPHEM_PER_ORBIT_STEP];

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double    g_azimuth[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
double    g_meas_spec_peak[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
float     g_tx_doppler[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
float     g_rx_gate_delay[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
int       g_ephem_idx[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];

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
    if (argc != 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* echo_data_file = argv[clidx++];

    //---------------------//
    // read in config file //
    //---------------------//

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

    //--------------------------------------//
    // create a QSCAT and a QSCAT simulator //
    //--------------------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    QscatSim qscat_sim;
    if (! ConfigQscatSim(&qscat_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring instrument simulator\n",
            command);
        exit(1);
    }

    //---------------------------------//
    // open echo data file for reading //
    //---------------------------------//

    int ifd = open(echo_data_file, O_RDONLY);
    if (ifd == -1)
    {
        fprintf(stderr, "%s: error opening echo data file %s\n", command,
            echo_data_file);
        exit(1);
    }

    //------------//
    // initialize //
    //------------//

    initialize();
    int last_orbit_step = -1;
    int orbit_step = -1;

    //-----------------------//
    // read and process data //
    //-----------------------//

    do
    {
        //--------------------//
        // read the record id //
        //--------------------//

        int id;
        int size = sizeof(int);
        int retval = read(ifd, &id, size);
        if (retval != size)
        {
            if (retval == 0)
                break;    // EOF
            else
            {
                fprintf(stderr, "%s: error reading echo data file %s\n",
                    command, echo_data_file);
                exit(1);
            }
        }

        //-------//
        // parse //
        //-------//

        int idx;
        double azimuth;
        Ephem* eph;

        switch (id)
        {
        case SPOT_ID:

            //-----------------------//
            // read in the spot data //
            //-----------------------//

            int beam_idx, ideal_encoder, held_encoder, land_flag;
            float tx_doppler, rx_gate_delay, meas_spec_peak, exp_spec_peak,
                total_signal_energy;
            if (! read_spot(ifd, &beam_idx, &tx_doppler, &rx_gate_delay,
                &ideal_encoder, &held_encoder, &meas_spec_peak,
                &exp_spec_peak, &total_signal_energy, &land_flag))
            {
                fprintf(stderr,
                    "%s: error reading spot from echo data file %s\n",
                    command, echo_data_file);
                exit(1);
            }

            //-----------------------//
            // accumulation decision //
            //-----------------------//

            if (land_flag)
                continue;

            if (total_signal_energy < SIGNAL_ENERGY_THRESHOLD)
                continue;

            //------------//
            // accumulate //
            //------------//

            idx = g_count[beam_idx];
            azimuth = two_pi * (double)ideal_encoder /
                (double)ENCODER_N;

            if (idx >= MAXIMUM_SPOTS_PER_ORBIT_STEP)
            {
                fprintf(stderr, "%s: too much spot data\n", command);
                exit(1);
            }

            g_azimuth[beam_idx][idx] = azimuth;
            g_meas_spec_peak[beam_idx][idx] = meas_spec_peak;
            g_tx_doppler[beam_idx][idx] = tx_doppler;
            g_rx_gate_delay[beam_idx][idx] = rx_gate_delay;
            g_ephem_idx[beam_idx][idx] = (g_ephem_count - 1);
            g_count[beam_idx]++;
            break;
        case EPHEMERIS_ID:
            float gcx, gcy, gcz, velx, vely, velz, roll, pitch, yaw;
            read_ephemeris(ifd, &gcx, &gcy, &gcz, &velx, &vely, &velz, &roll,
                &pitch, &yaw);
            if (g_ephem_count >= MAXIMUM_EPHEM_PER_ORBIT_STEP)
            {
                fprintf(stderr, "%s: too much ephemeris data\n", command);
                exit(1);
            }
            eph = &(g_ephem[g_ephem_count]);
            eph->gcx = gcx;
            eph->gcy = gcy;
            eph->gcz = gcz;
            eph->velx = velx;
            eph->vely = vely;
            eph->velz = velz;
            eph->roll = roll;
            eph->pitch = pitch;
            eph->yaw = yaw;
            g_ephem_count++;
            break;
        case ORBIT_STEP_ID:
            read_orbit_step(ifd, &orbit_step);
            if (orbit_step != last_orbit_step)
            {
                if (last_orbit_step != -1)
                {
                    process_orbit_step(&spacecraft, &qscat, last_orbit_step);

                    // transfer ephemeris
                    g_ephem[0] = g_ephem[g_ephem_count - 1];
                    g_ephem_count = 1;
                    initialize();
                }
                last_orbit_step = orbit_step;
            }
            break;
        case ORBIT_TIME_ID:
            unsigned int orbit_ticks;
            read_orbit_time(ifd, &orbit_ticks);
            break;
        default:
            fprintf(stderr, "%s: unknown data type in echo data file %s\n",
                command, echo_data_file);
            exit(1);
            break;
        }
    } while (1);

    close(ifd);

    return (0);
}

//------------//
// initialize //
//------------//

int
initialize()
{
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        g_count[beam_idx] = 0;
    }
    return(1);
}

//--------------------//
// process_orbit_step //
//--------------------//

int
process_orbit_step(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    int          orbit_step)
{
    //---------------------//
    // initialize attitude //
    //---------------------//

    float roll = 0.0;
    float pitch = 0.0;
    float yaw = 0.0;

    float roll_step = ROLL_START_STEP;
    float pitch_step = PITCH_START_STEP;
    float yaw_step = YAW_START_STEP;

    do
    {
        optimize(spacecraft, qscat, &roll, &pitch, &yaw, roll_step, 0.0, 0.0);
        roll_step /= 2.0;

        optimize(spacecraft, qscat, &roll, &pitch, &yaw, 0.0, pitch_step, 0.0);
        pitch_step /= 2.0;

        optimize(spacecraft, qscat, &roll, &pitch, &yaw, 0.0, 0.0, yaw_step);
        yaw_step /= 2.0;

    } while (roll_step > ROLL_END_STEP ||
        pitch_step > PITCH_END_STEP ||
        yaw_step > YAW_END_STEP);

    //---------------------------//
    // generate a knowledge plot //
    //---------------------------//

    char filename[1024];
    sprintf(filename, "knowledge.%03d.out", orbit_step);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ title %cOrbit Step = %03d%c\n", QUOTES, orbit_step,
        QUOTES);
    fprintf(ofp, "@ subtitle %cDelta Roll = %.4f deg., Delta Pitch = %.4f deg., Delta Yaw = %.4f deg.%c\n", QUOTES,
        roll * rtd, pitch * rtd, yaw * rtd, QUOTES);

//    evaluate(spacecraft, qscat, roll, pitch, yaw, ofp);
    evaluate(spacecraft, qscat, -0.1 * dtr, 0.2 * dtr, 0.0, ofp);

    fclose(ofp);

    return(1);
}

//----------//
// optimize //
//----------//

#define SWAP(a,b)  { swap = (a); (a) = (b); (b) = swap; }

int
optimize(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    float*       roll,
    float*       pitch,
    float*       yaw,
    float        roll_step,
    float        pitch_step,
    float        yaw_step)
{
    //---------------------//
    // determine direction //
    //---------------------//

    // assume going forward
    float old_roll = *roll - roll_step / 2.0;
    float old_pitch = *pitch - pitch_step / 2.0;
    float old_yaw = *yaw - yaw_step / 2.0;
    double old_mse = evaluate(spacecraft, qscat, old_roll, old_pitch, old_yaw);

    float new_roll = *roll + roll_step / 2.0;
    float new_pitch = *pitch + pitch_step / 2.0;
    float new_yaw = *yaw + yaw_step / 2.0;
    double new_mse = evaluate(spacecraft, qscat, new_roll, new_pitch, new_yaw);

    float delta_sign = 1.0;

    double swap;
    if (old_mse < new_mse)
    {
        // reverse direction
        delta_sign = -1.0;
        SWAP(new_roll, old_roll);
        SWAP(new_pitch, old_pitch);
        SWAP(new_yaw, old_yaw);
        SWAP(new_mse, old_mse);
    }

    //------------------------------//
    // search until minima is found //
    //------------------------------//

    for (;;)
    {
        if (new_mse < old_mse)
        {
            // continue moving forward
            old_roll = new_roll;
            old_pitch = new_pitch;
            old_yaw = new_yaw;
            old_mse = new_mse;
            new_roll = old_roll + delta_sign * roll_step;
            new_pitch = old_pitch + delta_sign * pitch_step;
            new_yaw = old_yaw + delta_sign * yaw_step;
            new_mse = evaluate(spacecraft, qscat, new_roll, new_pitch,
                new_yaw);
        }
        else
        {
            // stop (new value is worse than the old)
            *roll = old_roll;
            *pitch = old_pitch;
            *yaw = old_yaw;
            break;
        }
    }
    return(1);
}

//----------//
// evaluate //
//----------//

double
evaluate(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    float        roll,
    float        pitch,
    float        yaw,
    FILE*        ofp)
{
    double mse = 0.0;

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        double look, azim;
        if (beam_idx == 0)
        {
            look = BYU_INNER_BEAM_LOOK_ANGLE * dtr;
            azim = BYU_INNER_BEAM_AZIMUTH_ANGLE * dtr;
        }
        else
        {
            look = BYU_OUTER_BEAM_LOOK_ANGLE * dtr;
            azim = BYU_OUTER_BEAM_AZIMUTH_ANGLE * dtr;
        }
        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            // set the spacecraft
            Ephem* ephem = &(g_ephem[g_ephem_idx[beam_idx][idx]]);
            spacecraft->orbitState.rsat.Set(ephem->gcx, ephem->gcy,
                ephem->gcz);
            spacecraft->orbitState.vsat.Set(ephem->velx, ephem->vely,
                ephem->velz);
            spacecraft->attitude.SetRPY(ephem->roll + roll,
                ephem->pitch + pitch, ephem->yaw + yaw);

            // set the antenna
            qscat->sas.antenna.groundImpactAzimuthAngle =
                g_azimuth[beam_idx][idx];

            // calculate the reference vector baseband frequency
            CoordinateSwitch antenna_frame_to_gc =
                AntennaFrameToGC(&(spacecraft->orbitState),
                &(spacecraft->attitude), &(qscat->sas.antenna),
                g_azimuth[beam_idx][idx]); 
            Vector3 vector;
            vector.SphericalSet(1.0, look, azim);

            // set the instrument
            qscat->ses.CmdRxGateDelayEu(g_rx_gate_delay[beam_idx][idx]);
            qscat->ses.CmdTxDopplerEu(g_tx_doppler[beam_idx][idx]);

            TargetInfoPackage tip;
            TargetInfo(&antenna_frame_to_gc, spacecraft, qscat, vector, &tip);
            double dif = tip.basebandFreq - g_meas_spec_peak[beam_idx][idx];
            if (ofp)
            {
                fprintf(ofp, "%g %g %g\n", g_azimuth[beam_idx][idx] * rtd,
                    g_meas_spec_peak[beam_idx][idx], tip.basebandFreq);
            }
            dif *= dif;
            mse += dif;
        }
        if (ofp && beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
            fprintf(ofp, "&\n");
    }
    return(mse);
}
