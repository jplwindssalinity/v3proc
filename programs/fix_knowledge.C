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

#define MAX_SPOTS_PER_ORBIT_STEP  10000
#define MAX_EPHEM_PER_ORBIT_STEP  200

#define SIGNAL_ENERGY_THRESHOLD       1.0E-8

#define EPHEMERIS_CHAR   'E'
#define ORBIT_STEP_CHAR  'O'
#define ORBIT_TIME_CHAR  'T'

#define ROLL_START_STEP   0.001    // about 0.057 degrees
#define PITCH_START_STEP  0.001    // about 0.057 degrees
#define YAW_START_STEP    0.001    // about 0.057 degrees

#define END_STEP          0.00001    // about 0.00057 degrees

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
int     large_slope(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double c[3], double step[3], int* idx);
int     process_orbit_step(Spacecraft* spacecraft, Qscat* qscat,
            BYUXTable* byux, int orbit_step);
int     optimize(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], double step[3]);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], FILE* ofp = NULL);

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
Ephem     g_ephem[MAX_EPHEM_PER_ORBIT_STEP];

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double    g_tx_center_azimuth[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
double  g_meas_spec_peak_freq[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
float     g_tx_doppler[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
float     g_rx_gate_delay[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
int       g_ephem_idx[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];

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

    BYUXTable byux;
    if (! ConfigBYUXTable(&byux,&config_list))
    {
        fprintf(stderr,"%s: error configuring BYUXTable\n", command);
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
        Ephem* eph;

        switch (id)
        {
        case SPOT_ID:

            //-----------------------//
            // read in the spot data //
            //-----------------------//

            int beam_idx, ideal_encoder, land_flag;
            float tx_doppler, rx_gate_delay, tx_center_azimuth,
                meas_spec_peak_freq, total_signal_energy;
            if (! read_spot(ifd, &beam_idx, &tx_doppler, &rx_gate_delay,
                &ideal_encoder, &tx_center_azimuth, &meas_spec_peak_freq,
                &total_signal_energy, &land_flag))
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

            if (idx >= MAX_SPOTS_PER_ORBIT_STEP)
            {
                fprintf(stderr, "%s: too much spot data\n", command);
                exit(1);
            }

            g_tx_center_azimuth[beam_idx][idx] = tx_center_azimuth;
            g_meas_spec_peak_freq[beam_idx][idx] = meas_spec_peak_freq;
            g_tx_doppler[beam_idx][idx] = tx_doppler;
            g_rx_gate_delay[beam_idx][idx] = rx_gate_delay;
            g_ephem_idx[beam_idx][idx] = (g_ephem_count - 1);
            g_count[beam_idx]++;
            break;
        case EPHEMERIS_ID:
            float gcx, gcy, gcz, velx, vely, velz, roll, pitch, yaw;
            read_ephemeris(ifd, &gcx, &gcy, &gcz, &velx, &vely, &velz, &roll,
                &pitch, &yaw);
            if (g_ephem_count >= MAX_EPHEM_PER_ORBIT_STEP)
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
                    process_orbit_step(&spacecraft, &qscat, &byux,
                        last_orbit_step);

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

//-------------//
// large slope //
//-------------//

int
large_slope(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    BYUXTable*   byux,
    double       c[3],
    double       step[3],
    int*         idx)
{
    double try_c_low[3];
    double try_c_high[3];

    double max_dif = 0.0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            try_c_low[j] = c[j];
            try_c_high[j] = c[j];
        }
        try_c_low[i] -= step[i] / 2.0;
        try_c_high[i] += step[i] / 2.0;
        double low_sse = evaluate(spacecraft, qscat, byux, try_c_low, NULL);
        double high_sse = evaluate(spacecraft, qscat, byux, try_c_high, NULL);
        double dif = fabs(low_sse - high_sse);
        if (dif > max_dif)
        {
            max_dif = dif;
            *idx = i;
        }
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
    BYUXTable*   byux,
    int          orbit_step)
{
    //---------------------//
    // initialize attitude //
    //---------------------//

    double att[3] = {0.0, 0.0, 0.0};
    double step[3] = {ROLL_START_STEP, PITCH_START_STEP, YAW_START_STEP};
    double use_step[3] = {0.0, 0.0, 0.0};

    do
    {
        int idx;
        large_slope(spacecraft, qscat, byux, att, step, &idx);
        use_step[idx] = step[idx];
        optimize(spacecraft, qscat, byux, att, use_step);
        use_step[idx] = 0.0;
printf("%g %g %g %g %g %g\n", att[0] * rtd, att[1] * rtd, att[2] * rtd, step[0], step[1], step[2]);
        if (step[idx] < END_STEP)
            break;

        step[idx] /= 2.0;
    } while (1);

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
        att[0] * rtd, att[1] * rtd, att[2] * rtd, QUOTES);

    evaluate(spacecraft, qscat, byux, att, ofp);

//fprintf(ofp, "&\n");
//evaluate(spacecraft, qscat, byux, -0.1 * dtr, 0.2 * dtr, 0.0, ofp);

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
    BYUXTable*   byux,
    double       att[3],
    double       step[3])
{
    //---------------------//
    // determine direction //
    //---------------------//

    // assume going forward
    double old_att[3], new_att[3];
    for (int i = 0; i < 3; i++)
    {
        old_att[i] = att[i] - step[i] / 2.0;
        new_att[i] = att[i] + step[i] / 2.0;
    }
    double old_mse = evaluate(spacecraft, qscat, byux, old_att);
    double new_mse = evaluate(spacecraft, qscat, byux, new_att);

    float delta_sign = 1.0;

    double swap;
    if (old_mse < new_mse)
    {
        // reverse direction
        delta_sign = -1.0;
        for (int i = 0; i < 3; i++)
        {
            SWAP(new_att[i], old_att[i]);
        }
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
            for (int i = 0; i < 3; i++)
            {
                old_att[i] = new_att[i];
                new_att[i] += delta_sign * step[i];
            }
            old_mse = new_mse;
            new_mse = evaluate(spacecraft, qscat, byux, new_att);
        }
        else
        {
            // stop (new value is worse than the old)
            for (int i = 0; i < 3; i++)
            {
                att[i] = old_att[i];
            }
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
    BYUXTable*   byux,
    double       att[3],
    FILE*        ofp)
{
    double mse = 0.0;

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        qscat->cds.currentBeamIdx = beam_idx;

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            // set the spacecraft
            Ephem* ephem = &(g_ephem[g_ephem_idx[beam_idx][idx]]);
            spacecraft->orbitState.rsat.Set(ephem->gcx, ephem->gcy,
                ephem->gcz);
            spacecraft->orbitState.vsat.Set(ephem->velx, ephem->vely,
                ephem->velz);
            spacecraft->attitude.SetRPY(ephem->roll + att[0],
                ephem->pitch + att[1], ephem->yaw + att[2]);

            // set the antenna
            qscat->sas.antenna.txCenterAzimuthAngle =
                g_tx_center_azimuth[beam_idx][idx];
            qscat->TxCenterToGroundImpactAzimuth(spacecraft);

            // set the instrument
            qscat->ses.CmdRxGateDelayEu(g_rx_gate_delay[beam_idx][idx]);
            qscat->ses.CmdTxDopplerEu(g_tx_doppler[beam_idx][idx]);

            //-------------------------------------------//
            // calculate BYU reference vector frequency  //
            // and calculate the peak spectral frequency //
            //-------------------------------------------//

            float calc_byu_freq = byux->GetDeltaFreq(spacecraft, qscat, NULL);
            float orbit_position = qscat->cds.OrbitFraction();
            float gi_azim = qscat->sas.antenna.groundImpactAzimuthAngle;

            double x[10];
            int slices_per_spot = qscat->ses.scienceSlicesPerSpot +
                2 * qscat->ses.guardSlicesPerSide;
            for (int slice_idx = 0; slice_idx < slices_per_spot; slice_idx++)
            {
                int rel_slice;
                abs_to_rel_idx(slice_idx, slices_per_spot, &rel_slice);
                x[slice_idx] = byux->GetX(beam_idx, gi_azim, orbit_position,
                    rel_slice, calc_byu_freq);
            }

            float calc_spec_peak_slice, calc_spec_peak_freq;
            double slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
                7.0, 8.0, 9.0 };
            if (! find_peak(qscat, slice_number, x, slices_per_spot,
                &calc_spec_peak_slice, &calc_spec_peak_freq))
            {
                continue;
            }

            double dif = calc_spec_peak_freq -
                g_meas_spec_peak_freq[beam_idx][idx];
            if (ofp)
            {
                fprintf(ofp, "%g %g %g\n",
                    g_tx_center_azimuth[beam_idx][idx] * rtd,
                    g_meas_spec_peak_freq[beam_idx][idx], calc_spec_peak_freq);
            }
            dif *= dif;
            mse += dif;
        }
        if (ofp && beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
            fprintf(ofp, "&\n");
    }
    return(mse);
}
