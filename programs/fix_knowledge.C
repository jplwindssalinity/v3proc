//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_knowledge
//
// SYNOPSIS
//    fix_knowledge [ -f ] [ -r start:stop:step ] <sim_config_file>
//    <echo_data_file> <output_base>
//
// DESCRIPTION
//    Reads the echo data file and estimates the roll, pitch, and yaw
//    of the instrument and the look angle of each antenna beam.
//
// OPTIONS
//    -f   Use the config file windfield for energy profile correction
//    -r start:stop:step  Step organization.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <echo_data_file>   The echo data input file.
//      <output_base>      The output file base name.
//
// EXAMPLES
//    An example of a command line is:
//      % fix_knowledge -r 200:210:2 qscat.cfg echo.dat qscat.know
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

#define OPTSTRING    "fr:"

#define PLOT_OFFSET  80000
#define DIR_STEPS    36    // for data reduction
#define MIN_DATA_COUNT      36

#define QUOTES    '"'

#define LINE_SIZE  1024

#define MAX_SPOTS_PER_ORBIT_STEP  10000
#define MAX_EPHEM_PER_ORBIT_STEP  200

//#define SIGNAL_ENERGY_THRESHOLD       5E4
#define SIGNAL_ENERGY_THRESHOLD       0E4

#define EPHEMERIS_CHAR   'E'
#define ORBIT_STEP_CHAR  'O'
#define ORBIT_TIME_CHAR  'T'

// dumb search parameters
#define ROLL_START_STEP   0.05
#define PITCH_START_STEP  0.05
#define YAW_START_STEP    0.05
#define END_STEP          0.001

#define VAR_ANGLE_RANGE     2.0    // degrees
#define MIN_VAR_DATA_COUNT  2

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
int     process_orbit_step(const char* output_base, Spacecraft* spacecraft,
            Qscat* qscat, BYUXTable* byux, int orbit_step, FILE* att_fp);
int     ds_optimize(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], double lambda, double xtol);
double  ds_evaluate(double* x, void* ptr);
double  evaluate(Spacecraft* spacecraft, Qscat* qscat, BYUXTable* byux,
            double att[3], FILE* ofp = NULL, int all_flag = 0);
int     est_var();

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -f ]", "[ -r start:stop:step ]",
    "<sim_config_file>", "<echo_data_file>", "<output_base>", 0};

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

int        g_windfield_opt = 0;
WindField  g_windfield;
GMF        g_gmf;
int        g_range_opt = 0;
int        g_start_orbit_step = 0;
int        g_stop_orbit_step = 0;
int        g_step_orbit_step = 1;

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

    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'f':
            g_windfield_opt = 1;
            break;
        case 'r':
            if (sscanf(optarg, "%d:%d:%d", &g_start_orbit_step,
                &g_stop_orbit_step, &g_step_orbit_step) != 3)
            {
                fprintf(stderr, "%s: error parsing range %s\n", command,
                    optarg);
                exit(1);
            }
            g_range_opt = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 3)
        usage(command, usage_array, 1);

    const char* config_file = argv[optind++];
    const char* echo_data_file = argv[optind++];
    const char* output_base = argv[optind++];

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

    //-------------------//
    // read in windfield //
    //-------------------//

    if (g_windfield_opt)
    {
        char* windfield_filename =
            config_list.Get(NUDGE_WINDFIELD_FILE_KEYWORD);
        char* windfield_type = config_list.Get(NUDGE_WINDFIELD_TYPE_KEYWORD);

        if (! g_windfield.ReadType(windfield_filename, windfield_type))
        {
            fprintf(stderr, "%s: error reading nudge field\n", command);
            exit(1);
        }

        if (! ConfigGMF(&g_gmf, &config_list))
        {
            fprintf(stderr, "%s: error configuring GMF\n", command);
            exit(1);
        }
    }

    //---------------------------//
    // open attitude output file //
    //---------------------------//

    char filename[1024];
    sprintf(filename, "%s.att", output_base);
    FILE* att_fp = fopen(filename, "w");
    if (att_fp == NULL)
    {
        fprintf(stderr, "%s: error opening attitude output file %s\n", command,
            filename);
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
//printf("%g %g\n", total_signal_energy, meas_spec_peak_freq);

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
                if (last_orbit_step != -1 &&
                    (! g_range_opt || (g_range_opt &&
                    (orbit_step - g_start_orbit_step) % g_step_orbit_step ==
                    0)))
                {
                    process_orbit_step(output_base, &spacecraft, &qscat,
                        &byux, last_orbit_step, att_fp);

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

    //-----------------------------//
    // process the last orbit step //
    //-----------------------------//

    process_orbit_step(output_base, &spacecraft, &qscat, &byux,
        orbit_step, att_fp);

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

//--------------------//
// process_orbit_step //
//--------------------//

int
process_orbit_step(
    const char*  output_base,
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    BYUXTable*   byux,
    int          orbit_step,
    FILE*        att_fp)
{
    if (est_var() < MIN_DATA_COUNT)
        return(0);

    //--------------------------//
    // search for best attitude //
    //--------------------------//

    double att[3];
    att[0] = 0.0;
    att[1] = 0.0;
    att[2] = 0.0;
    ds_optimize(spacecraft, qscat, byux, att, LAMBDA_1 * dtr, XTOL_1 * dtr);

    //---------------------------//
    // generate a knowledge plot //
    //---------------------------//

    char filename[1024];
    if (g_range_opt)
    {
        sprintf(filename, "%s.k%03d-%03d.out", output_base,
            orbit_step - g_step_orbit_step + 1, orbit_step);
    }
    else
    {
        sprintf(filename, "%s.k%03d.out", output_base, orbit_step);
    }
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    if (g_range_opt)
    {
        fprintf(ofp, "@ title %cOrbit Step = %03d-%03d%c\n", QUOTES,
            orbit_step - g_step_orbit_step + 1, orbit_step, QUOTES);
    }
    else
    {
        fprintf(ofp, "@ title %cOrbit Step = %03d%c\n", QUOTES, orbit_step,
            QUOTES);
    }
    fprintf(ofp, "@ subtitle %cDelta Roll = %.4f deg., Delta Pitch = %.4f deg., Delta Yaw = %.4f deg.%c\n", QUOTES,
        att[0] * rtd, att[1] * rtd, att[2] * rtd, QUOTES);

    evaluate(spacecraft, qscat, byux, att, ofp, 1);
    fprintf(ofp, "&\n");
    evaluate(spacecraft, qscat, byux, att, ofp, 0);

    fclose(ofp);

    //---------------------------------------//
    // add roll, pitch, yaw to attitude file //
    //---------------------------------------//

    if (g_range_opt)
    {
        for (int step = orbit_step - g_step_orbit_step + 1; step <= orbit_step;
            step++)
        {
            fprintf(att_fp, "%d %g %g %g\n", step, att[0] * rtd, att[1] * rtd,
                att[2] * rtd);
        }
    }
    else
    {
        fprintf(att_fp, "%d %g %g %g\n", orbit_step, att[0] * rtd, att[1] * rtd,
            att[2] * rtd);
    }

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
    FILE*        ofp,
    int          all_flag)
{
    double mse = 0.0;

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        qscat->cds.currentBeamIdx = beam_idx;
        Beam* beam = qscat->GetCurrentBeam();
        Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            // skip data that doesn't have a variance estimate
            if (g_freq_var[beam_idx][idx] == -1.0 && ! all_flag)
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

            //------------------------------//
            // locate slices (if necessary) //
            //------------------------------//

            MeasSpot meas_spot;
            if (g_windfield_opt)
            {
                qscat->LocateSliceCentroids(spacecraft, &meas_spot);
            }

            //-------------------------------//
            // calculate X*s0 for each slice //
            //-------------------------------//

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

                if (g_windfield_opt)
                {
                    //---------//
                    // sigma-0 //
                    //---------//

                    Meas* meas = meas_spot.GetByIndex(slice_idx);
                    double alt, lat, lon;
                    if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
                        return(0);
                    LonLat lon_lat;
                    lon_lat.longitude = lon;
                    lon_lat.latitude = lat;
                    WindVector wv;
                    if (! g_windfield.InterpolatedWindVector(lon_lat, &wv))
                    {
                        wv.spd = 0.0;
                        wv.dir = 0.0;
                    }
                    float chi = wv.dir - meas->eastAzimuth + pi;
                    float sigma0;
                    g_gmf.GetInterpolatedValue(meas_type,
                        meas->incidenceAngle, wv.spd, chi, &sigma0);
                    x[slice_idx] *= sigma0;
                }
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
//            dif = dif * dif / g_freq_var[beam_idx][idx];
            dif = dif * dif;
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
    int point_count = 0;
    double delta = pi / DIR_STEPS;
    for (int dir_idx = 0; dir_idx < DIR_STEPS; dir_idx++)
    {
        double target_dir = two_pi * dir_idx / (double)DIR_STEPS;
        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            //---------------------------------//
            // calculate the mean and variance //
            //---------------------------------//

            double meanf = 0.0;
            double varf = 0.0;
            int count = 0;
            for (int pass_flag = 0; pass_flag < 2; pass_flag++)
            {
                for (int idx = 0; idx < g_count[beam_idx]; idx++)
                {
                    double azim = g_tx_center_azimuth[beam_idx][idx];
                    double dif = ANGDIF(azim, target_dir);
                    if (dif > delta)
                        continue;

                    if (pass_flag == 0)    // mean pass
                    {
                        meanf += g_meas_spec_peak_freq[beam_idx][idx];
                        count++;
                    }
                    else if (pass_flag == 1)    // variance pass
                    {
                        double dif =
                            g_meas_spec_peak_freq[beam_idx][idx] - meanf;
                        varf += (dif * dif);
                    }
                }

                if (pass_flag == 0 && count > 0)    // mean pass
                {
                    meanf /= (double)count;
                }
                else if (pass_flag == 1)    // variance pass
                {
                    if (count > MIN_VAR_DATA_COUNT)
                    {
                        varf /= (double)(count - 1);
/*
if (beam_idx == 0)
printf("%g %g\n", target_dir * rtd, varf);
*/
                    }
                    else
                    {
//                        varf = -1.0;    // flag as don't use
                        varf = 1.0;    // put something here
                    }
                }
            }

            //---------------------------------//
            // find the point nearest the mean //
            //---------------------------------//

            int best_idx = -1;
            double best_dif = 9e50;

            if (count >= MIN_VAR_DATA_COUNT)
            {
                for (int idx = 0; idx < g_count[beam_idx]; idx++)
                {
                    if (g_freq_var[beam_idx][idx] == -1.0)
                        continue;

                    double azim = g_tx_center_azimuth[beam_idx][idx];
                    double dif = ANGDIF(azim, target_dir);
                    if (dif > delta)
                        continue;

                    dif = g_meas_spec_peak_freq[beam_idx][idx] - meanf;
                    dif *= dif;
                    if (dif < best_dif)
                    {
                        best_idx = idx;
                        best_dif = dif;
                    }
                }
            }

            if (best_idx != -1)
                point_count++;

            //-------------------------//
            // remove all other points //
            //-------------------------//

            for (int idx = 0; idx < g_count[beam_idx]; idx++)
            {
                double azim = g_tx_center_azimuth[beam_idx][idx];
                double dif = ANGDIF(azim, target_dir);
                if (dif > delta)
                    continue;

                if (idx != best_idx)
                {
                    g_freq_var[beam_idx][idx] = -1.0;
                }
            }
        }
    }
    return(point_count);
}
