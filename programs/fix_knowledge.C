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

#define PLOT_OFFSET  80000
#define SIMPLEX  1

#define QUOTES    '"'

#define LINE_SIZE  1024

#define MAX_SPOTS_PER_ORBIT_STEP  10000
#define MAX_EPHEM_PER_ORBIT_STEP  200

#define SIGNAL_ENERGY_THRESHOLD       1.0E-8

#define EPHEMERIS_CHAR   'E'
#define ORBIT_STEP_CHAR  'O'
#define ORBIT_TIME_CHAR  'T'

// dumb search parameters
#define ROLL_START_STEP   0.05
#define PITCH_START_STEP  0.05
#define YAW_START_STEP    0.05
#define END_STEP          0.001

#define VAR_ANGLE_RANGE     2.0    // degrees
#define MIN_VAR_DATA_COUNT  5
#define MIN_DATA_COUNT      100

// simplex search parameters
#define LAMBDA_1  0.1
#define XTOL_1    0.001
#define LAMBDA_2  0.01
#define XTOL_2    0.001
#define PLEX_STEPS  36

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
            BYUXTable* byux, int orbit_step, FILE* att_fp);
int     ds_optimize(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], double lambda, double xtol);
int     optimize(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], double step[3]);
double  ds_evaluate(double* x, void* ptr);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], FILE* ofp = NULL);
int     est_var();

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

int     g_ephem_count = 0;
Ephem   g_ephem[MAX_EPHEM_PER_ORBIT_STEP];

int     g_count[NUMBER_OF_QSCAT_BEAMS];
double  g_tx_center_azimuth[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
double  g_meas_spec_peak_freq[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
double  g_freq_var[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
float   g_tx_doppler[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
float   g_rx_gate_delay[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];
int     g_ephem_idx[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS_PER_ORBIT_STEP];

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

    //---------------------------//
    // open attitude output file //
    //---------------------------//

    FILE* att_fp = fopen("attitude.dat", "w");
    if (att_fp == NULL)
    {
        fprintf(stderr, "%s: error opening attitude output file %s\n", command,
            "attitude.dat");
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
                        last_orbit_step, att_fp);

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

    fclose(att_fp);
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
    int          orbit_step,
    FILE*        att_fp)
{
    //--------------------//
    // estimate variances //
    //--------------------//

    int var_count = est_var();

    if (var_count < MIN_DATA_COUNT)
        return(0);

    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double a[2], p[2], c[2];
    for (int i = 0; i < 2; i++)
    {
        // fit each beam separately
        sinfit(g_tx_center_azimuth[i], g_meas_spec_peak_freq[i], g_freq_var[i],
            g_count[i], &(a[i]), &(p[i]), &(c[i]));
    }

    //-----------------------------------------------------------------//
    // first, just keep a few points near the sinusoid for a rough fit //
    //-----------------------------------------------------------------//

    int use_idx[PLEX_STEPS];
    double min_dif[PLEX_STEPS];

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int i = 0; i < PLEX_STEPS; i++)
        {
            use_idx[i] = -1;
            min_dif[i] = 1E9;
        }

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            int idx2 = (int)(PLEX_STEPS * g_tx_center_azimuth[beam_idx][idx] /
                two_pi + 0.5);
            idx2 %= PLEX_STEPS;

            double fitval = a[beam_idx] *
                cos(g_tx_center_azimuth[beam_idx][idx] + p[beam_idx]) +
                c[beam_idx];

            double dif = fabs(fitval - g_meas_spec_peak_freq[beam_idx][idx]);
            if (dif < min_dif[idx2])
            {
                min_dif[idx2] = dif;
                use_idx[idx2] = idx;
            }
            g_freq_var[beam_idx][idx] = -1.0;    // flag to not use point
        }
        for (int i = 0; i < PLEX_STEPS; i++)
        {
            if (use_idx[i] != -1)
                g_freq_var[beam_idx][use_idx[i]] = 1.0;   // flag to use
        }
    }

    //--------------------------//
    // search for best attitude //
    //--------------------------//

    double att[3];
    att[0] = 0.0;
    att[1] = 0.0;
    att[2] = 0.0;
    ds_optimize(spacecraft, qscat, byux, att, LAMBDA_1 * dtr, XTOL_1 * dtr);

    //--------------------------//
    // search for best attitude //
    //--------------------------//

/*
    if (SIMPLEX)
    {
        ds_optimize(spacecraft, qscat, byux, att, LAMBDA_2 * dtr,
            XTOL_2 * dtr);
    }
    else
    {
        double att[3] = {0.0, 0.0, 0.0};
        double step[3] = {ROLL_START_STEP * dtr, PITCH_START_STEP * dtr,
            YAW_START_STEP * dtr};
        double use_step[3] = {0.0, 0.0, 0.0};

        do
        {
            int idx;
            large_slope(spacecraft, qscat, byux, att, step, &idx);
            use_step[idx] = step[idx];
            optimize(spacecraft, qscat, byux, att, use_step);
            use_step[idx] = 0.0;
            if (step[idx] < END_STEP * dtr)
                break;

            step[idx] /= 2.0;
        } while (1);
    }
*/

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

/*
fprintf(ofp, "&\n");
att[0] = -0.1 * dtr;
att[1] = 0.2 * dtr;
att[2] = 0.0 * dtr;
evaluate(spacecraft, qscat, byux, att, ofp);
*/

    fclose(ofp);

    //---------------------------------------//
    // add roll, pitch, yaw to attitude file //
    //---------------------------------------//

    fprintf(att_fp, "%d %g %g %g\n", orbit_step, att[0] * rtd, att[1] * rtd,
        att[2] * rtd);

    return(1);
}

//-------------//
// ds_optimize //
//-------------//

int
ds_optimize(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    BYUXTable*   byux,
    double       att[3],
    double       lambda,
    double       xtol)
{
    int ndim = 3;
    double** p = (double**)make_array(sizeof(double), 2, ndim + 1, ndim);
    if (p == NULL)
        return(0);

    p[0][0] = att[0];    // roll
    p[0][1] = att[1];    // pitch
    p[0][2] = att[2];    // yaw

    p[1][0] = p[0][0] + lambda;
    p[1][1] = p[0][1];
    p[1][2] = p[0][2];

    p[2][0] = p[0][0];
    p[2][1] = p[0][1] + lambda;
    p[2][2] = p[0][2];

    p[3][0] = p[0][0];
    p[3][1] = p[0][1];
    p[3][2] = p[0][2] + lambda;

    char* ptr[3];
    ptr[0] = (char *)spacecraft;
    ptr[1] = (char *)qscat;
    ptr[2] = (char *)byux;

    downhill_simplex(p, ndim, ndim, 0.0, ds_evaluate, ptr, xtol);

    for (int i = 0; i < 3; i++)
        att[i] = p[0][i];

    free_array(p, 2, ndim + 1, ndim);

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

//-------------//
// ds_evaluate //
//-------------//

double
ds_evaluate(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char**)ptr;
    Spacecraft* spacecraft = (Spacecraft *)ptr2[0];
    Qscat* qscat = (Qscat *)ptr2[1];
    BYUXTable* byux = (BYUXTable *)ptr2[2];

    return (evaluate(spacecraft, qscat, byux, x));
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
            // skip data that doesn't have a variance estimate
            if (g_freq_var[beam_idx][idx] < 0.0 && ofp == NULL)
//            if (g_freq_var[beam_idx][idx] < 0.0)
                continue;

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
                if (beam_idx == 1)
                {
                    fprintf(ofp, "%g %g %g\n",
                        g_tx_center_azimuth[beam_idx][idx] * rtd,
                        g_meas_spec_peak_freq[beam_idx][idx] + PLOT_OFFSET,
                        calc_spec_peak_freq + PLOT_OFFSET);
                }
                else
                {
                    fprintf(ofp, "%g %g %g\n",
                        g_tx_center_azimuth[beam_idx][idx] * rtd,
                        g_meas_spec_peak_freq[beam_idx][idx],
                        calc_spec_peak_freq);
                }
            }
            dif = dif * dif / g_freq_var[beam_idx][idx];
            mse += dif;
        }
        if (ofp && beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
            fprintf(ofp, "&\n");
    }
printf("%g %g %g %g\n", att[0] * rtd, att[1] * rtd, att[2] * rtd, mse);
    return(mse);
}

//---------//
// est_var //
//---------//

int
est_var()
{
    int var_count = 0;
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //-----------------//
        // sort by azimuth //
        //-----------------//

        int idx_array[MAX_SPOTS_PER_ORBIT_STEP];
        heapsort(g_count[beam_idx], g_tx_center_azimuth[beam_idx], idx_array);

        //-----------------------------//
        // do the variance calculation //
        //-----------------------------//

        int start_idx = 0;
        int end_idx = 0;
        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            int use_idx = idx_array[idx];
            double azim = g_tx_center_azimuth[beam_idx][use_idx];
            double meanf = 0.0;
            double varf = 0.0;
            unsigned int count = 0;

            //------------------//
            // find start index //
            //------------------//

            // move forward until in range
            do
            {
                start_idx = start_idx % g_count[beam_idx];
                int use_start_idx = idx_array[start_idx];
                if (ANGDIF(azim,
                    g_tx_center_azimuth[beam_idx][use_start_idx]) <
                    VAR_ANGLE_RANGE * dtr)
                {
                    break;
                }
                start_idx++;
            } while (1);
            // back up until out of range
            do
            {
                start_idx = (start_idx + g_count[beam_idx]) % g_count[beam_idx];
                int use_start_idx = idx_array[start_idx];
                if (ANGDIF(azim,
                    g_tx_center_azimuth[beam_idx][use_start_idx]) >
                    VAR_ANGLE_RANGE * dtr)
                {
                    break;
                }
                start_idx--;
            } while (1);
            start_idx = (start_idx + 1) % g_count[beam_idx];

            //----------------//
            // find end index //
            //----------------//

            // back up until in range
            do
            {
                end_idx = (end_idx + g_count[beam_idx]) % g_count[beam_idx];
                int use_end_idx = idx_array[end_idx];
                if (ANGDIF(azim,
                    g_tx_center_azimuth[beam_idx][use_end_idx]) <
                    VAR_ANGLE_RANGE * dtr)
                {
                    break;
                }
                end_idx--;
            } while (1);

            // move forward until out of range
            do
            {
                end_idx = end_idx % g_count[beam_idx];
                int use_end_idx = idx_array[end_idx];
                if (ANGDIF(azim,
                    g_tx_center_azimuth[beam_idx][use_end_idx]) >
                    VAR_ANGLE_RANGE * dtr)
                {
                    break;
                }
                end_idx++;
            } while (1);

            //-----------//
            // calculate //
            //-----------//

            for (int pass_flag = 0; pass_flag < 2; pass_flag++)
            {
                for (int oidx = start_idx; oidx < end_idx; oidx++)
                {
                    int use_oidx = idx_array[oidx];
                    if (pass_flag == 0)    // mean pass
                    {
                        meanf += g_meas_spec_peak_freq[beam_idx][use_oidx];
                        count++;
                    }
                    else if (pass_flag == 1)    // variance pass
                    {
                        double dif =
                            g_meas_spec_peak_freq[beam_idx][use_oidx] - meanf;
                        varf += (dif * dif);
                    }
                }

                if (pass_flag == 0)
                    meanf /= (double)count;
                else if (pass_flag == 1)
                {
                    if (count > MIN_VAR_DATA_COUNT)
                    {
                        varf /= (double)(count - 1);
                        var_count++;
                    }
                    else
                        varf = -1.0;    // flag as don't use
                }
            }
            g_freq_var[beam_idx][use_idx] = varf;
        }
    }
    return(var_count);
}
