//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_knowledge
//
// SYNOPSIS
//    fix_knowledge [ -f type:windfield ] [ -i ] [ -s step_size ]
//      <sim_config_file> <echo_data_file> <output_base>
//
// DESCRIPTION
//    Reads the echo data file and estimates the roll, pitch, and yaw
//    of the instrument and the look angle of each antenna beam.
//
// OPTIONS
//    [ -f type:windfield ]  Use windfield for energy profile
//                             correction.
//    [ -i ]                 Use an simple incidence angle correction.
//    [ -s step_size ]       The number of orbit steps to combine.
//                             Otherwise all data is combined.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <echo_data_file>   The echo data input file.
//      <output_base>      The output file base name.
//
// EXAMPLES
//    An example of a command line is:
//      % fix_knowledge -i -s 2 qscat.cfg qscat.echo qscat.know
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

#define OPTSTRING    "f:is:"

#define PLOT_OFFSET  40000
#define DIR_STEPS    36    // for data reduction
#define MIN_POINTS_PER_DIR_STEP    10
#define MIN_DIR_STEP_DATA_POINTS   36

#define QUOTES    '"'

#define LINE_SIZE  1024

#define MAX_SPOTS  10000
#define MAX_EPHEM  200

//#define SIGNAL_ENERGY_THRESHOLD       5E4
#define SIGNAL_ENERGY_THRESHOLD       0E4

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
            double att[3], FILE* ofp = NULL, int or_flag = 0);
int     prune();
float   est_sigma0(int beam_idx, float incidence_angle);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -f type:windfield ]", "[ -i ]",
    "[ -s step_size ] ", "<sim_config_file>", "<echo_data_file>",
    "<output_base>", 0};

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
Ephem   g_ephem[MAX_EPHEM];

int     g_count[NUMBER_OF_QSCAT_BEAMS];
double  g_tx_center_azimuth[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
double  g_meas_spec_peak_freq[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
double  g_freq_var[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
float   g_tx_doppler[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
float   g_rx_gate_delay[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
int     g_ephem_idx[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];
char    g_use_data[NUMBER_OF_QSCAT_BEAMS][MAX_SPOTS];

int        g_windfield_opt = 0;
char*      g_windfield_type = NULL;
char*      g_windfield_file = NULL;
WindField  g_windfield;
GMF        g_gmf;
int        g_incidence_opt = 0;
int        g_step_size_opt = 0;
int        g_step_size = 0;
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
            if (sscanf(optarg, "%s:%s", g_windfield_type,
                g_windfield_file) != 2)
            {
                fprintf(stderr, "%s: error parsing windfield %s\n", command,
                    optarg);
                exit(1);
            }
            g_windfield_opt = 1;
            break;
        case 'i':
            g_incidence_opt = 1;
            break;
        case 's':
            g_step_size = atoi(optarg);
            g_step_size_opt = 1;
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

    //----------------------------//
    // create and configure QSCAT //
    //----------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //---------------------------------------//
    // create and configure Level 1A product //
    //---------------------------------------//

    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }
    int spots_per_frame = l1a.frame.spotsPerFrame;

    //----------------------------------//
    // create and configure BYU X Table //
    //----------------------------------//

    BYUXTable byux;
    if (! ConfigBYUXTable(&byux,&config_list))
    {
        fprintf(stderr,"%s: error configuring BYUXTable\n", command);
        exit(1);
    }

    //-------------------//
    // read in windfield //
    //-------------------//

    if (g_windfield_opt)
    {
        if (! g_windfield.ReadType(g_windfield_file, g_windfield_type))
        {
            fprintf(stderr, "%s: error reading windfield %s of type %s\n",
                command, g_windfield_file, g_windfield_type);
            exit(1);
        }

        if (! ConfigGMF(&g_gmf, &config_list))
        {
            fprintf(stderr, "%s: error configuring GMF\n", command);
            exit(1);
        }
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
        //------//
        // read //
        //------//

        EchoInfo echo_info;
        int retval = echo_info.Read(ifd);
        if (retval != 1)
        {
            if (retval == -1)    // EOF
                break;
            else
            {
                fprintf(stderr, "%s: error reading echo file %s\n", command,
                    echo_data_file);
                exit(1);
            }
        }

        //-----------------//
        // store ephemeris //
        //-----------------//

        if (g_ephem_count >= MAX_EPHEM)
        {
            fprintf(stderr, "%s: too much ephemeris data\n", command);
            exit(1);
        }
        Ephem* eph = &(g_ephem[g_ephem_count]);
        eph->gcx = echo_info.gcX;
        eph->gcy = echo_info.gcY;
        eph->gcz = echo_info.gcZ;
        eph->velx = echo_info.velX;
        eph->vely = echo_info.velY;
        eph->velz = echo_info.velZ;
        eph->roll = echo_info.roll;
        eph->pitch = echo_info.pitch;
        eph->yaw = echo_info.yaw;
        g_ephem_count++;

        //---------------//
        // for each spot //
        //---------------//

        for (int spot_idx = 0; spot_idx < spots_per_frame; spot_idx++)
        {
            //-----------------------------//
            // check for end of orbit step //
            //-----------------------------//

            int orbit_step = echo_info.SpotOrbitStep(spot_idx);
            if (orbit_step != last_orbit_step)
            {
                if (last_orbit_step != -1 && g_step_size != 0 &&
                    orbit_step % g_step_size == 0)
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

            //-----------------------//
            // accumulation decision //
            //-----------------------//

            if (echo_info.flag[spot_idx] != EchoInfo::OK)
                continue;

            if (echo_info.totalSignalEnergy[spot_idx] < SIGNAL_ENERGY_THRESHOLD)
                continue;

            //------------//
            // accumulate //
            //------------//

            // spot data
            int beam_idx = echo_info.beamIdx[spot_idx];
            int idx = g_count[beam_idx];
            if (idx >= MAX_SPOTS)
            {
                fprintf(stderr, "%s: too much spot data\n", command);
                exit(1);
            }

            g_tx_center_azimuth[beam_idx][idx] =
                echo_info.txCenterAzimuthAngle[spot_idx];
            g_meas_spec_peak_freq[beam_idx][idx] =
                echo_info.measSpecPeakFreq[spot_idx];
            g_tx_doppler[beam_idx][idx] = echo_info.txDoppler[spot_idx];
            g_rx_gate_delay[beam_idx][idx] = echo_info.rxGateDelay[spot_idx];
            g_ephem_idx[beam_idx][idx] = (g_ephem_count - 1);
            g_count[beam_idx]++;
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
    if (prune() < MIN_DIR_STEP_DATA_POINTS)
        return(0);

    //--------------------------//
    // search for best attitude //
    //--------------------------//

    double att[3];
    att[0] = 0.0;
    att[1] = 0.0;
    att[2] = 0.0;
    ds_optimize(spacecraft, qscat, byux, att, LAMBDA_1 * dtr, XTOL_1 * dtr);

    //----------------------------//
    // determine orbit step range //
    //----------------------------//

    int start_step, end_step;
    if (g_step_size == 0)
    {
       start_step = orbit_step;
       end_step = orbit_step;
    }
    else
    {
        int base_idx = orbit_step / g_step_size;
        start_step = base_idx * g_step_size;
        end_step = (base_idx + 1) * g_step_size - 1;
    }

    //---------------------------//
    // generate a knowledge plot //
    //---------------------------//

    char filename[1024];
    if (g_step_size)
    {
        sprintf(filename, "%s.att.%03d-%03d.out", output_base, start_step,
            end_step);
    }
    else
    {
        sprintf(filename, "%s.att.%03d.out", output_base, start_step);
    }
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    if (g_step_size)
    {
        fprintf(ofp, "@ title %cOrbit Step = %03d-%03d%c\n", QUOTES,
            start_step, end_step, QUOTES);
    }
    else
    {
        fprintf(ofp, "@ title %cOrbit Step = %03d%c\n", QUOTES, start_step,
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

    for (int step = start_step; step <= end_step; step++)
    {
        fprintf(att_fp, "%d %g %g %g\n", step, att[0] * rtd, att[1] * rtd,
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
    int          or_flag)
{
    double mse = 0.0;

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        qscat->cds.currentBeamIdx = beam_idx;
        Beam* beam = qscat->GetCurrentBeam();
        Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            // only use data to be used
            if (! g_use_data[beam_idx][idx] && ! or_flag)
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
            if (g_windfield_opt || g_incidence_opt)
            {
                // create measurements
                if (! qscat->MakeSlices(&meas_spot))
                    return(0);
                qscat->LocateSliceCentroids(spacecraft, &meas_spot);
            }

            //--------------------------------------//
            // calculate X (or X*s0) for each slice //
            //--------------------------------------//

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
                    //-----------------------//
                    // multiply X by sigma-0 //
                    //-----------------------//

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
                if (g_incidence_opt)
                {
                    //----------------------------//
                    // incidence angle correction //
                    //----------------------------//

                    Meas* meas = meas_spot.GetByIndex(slice_idx);
                    float sigma0 = est_sigma0(meas->beamIdx,
                        meas->incidenceAngle);
                    x[slice_idx] *= sigma0;
                }
            }

            float calc_spec_peak_slice, calc_spec_peak_freq;
            double slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
                7.0, 8.0, 9.0 };
            if (! gaussian_fit(qscat, slice_number, x, slices_per_spot,
                &calc_spec_peak_slice, &calc_spec_peak_freq))
            {
                continue;
            }

            double dif = calc_spec_peak_freq -
                g_meas_spec_peak_freq[beam_idx][idx];
            if (ofp)
            {
                float plot_offset = 0;
                if (beam_idx == 1)
                    plot_offset = PLOT_OFFSET;

                fprintf(ofp, "%g %g %g\n",
                    g_tx_center_azimuth[beam_idx][idx] * rtd,
                    g_meas_spec_peak_freq[beam_idx][idx] + plot_offset,
                    calc_spec_peak_freq + PLOT_OFFSET);
            }
            dif = dif * dif;
            mse += dif;
        }
        if (ofp && beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
            fprintf(ofp, "&\n");
    }
printf("%g %g %g %g\n", att[0] * rtd, att[1] * rtd, att[2] * rtd, mse);
    return(mse);
}

//-------//
// prune //
//-------//

int
prune()
{
    int data_count = 0;

    //----------------------------------------//
    // calculate the mean for every X degrees //
    //----------------------------------------//

    int count_f[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double sum_f[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double sum_az[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    double min_dif[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    int min_idx[NUMBER_OF_QSCAT_BEAMS][DIR_STEPS];
    for (int i = 0; i < NUMBER_OF_QSCAT_BEAMS; i++)
    {
        for (int j = 0; j < DIR_STEPS; j++)
        {
            count_f[i][j] = 0;
            sum_f[i][j] = 0.0;
            sum_az[i][j] = 0.0;
            min_dif[i][j] = 90000.0;
            min_idx[i][j] = -1;
        }
    }
    float dir_step_size = two_pi / DIR_STEPS;

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            double azim = g_tx_center_azimuth[beam_idx][idx];
            int dir_idx = (int)(azim / dir_step_size);
            count_f[beam_idx][dir_idx]++;
            sum_f[beam_idx][dir_idx] += g_meas_spec_peak_freq[beam_idx][idx];
            sum_az[beam_idx][dir_idx] += g_tx_center_azimuth[beam_idx][idx];
        }

        for (int dir_idx = 0; dir_idx < DIR_STEPS; dir_idx++)
        {
            if (count_f[beam_idx][dir_idx])
            {
                sum_f[beam_idx][dir_idx] /= (double)count_f[beam_idx][dir_idx];
                sum_az[beam_idx][dir_idx] /= (double)count_f[beam_idx][dir_idx];
            }
        }

        //------------------------------------------------//
        // for each step, find the point nearest the mean //
        //------------------------------------------------//

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            double azim = g_tx_center_azimuth[beam_idx][idx];
            int dir_idx = (int)(azim / dir_step_size);

            if (count_f[beam_idx][dir_idx] < MIN_POINTS_PER_DIR_STEP)
                continue;

            int idx_1, idx_2;
            if (azim < sum_az[beam_idx][dir_idx])
            {
                idx_1 = (dir_idx + DIR_STEPS - 1) % DIR_STEPS;
                idx_2 = dir_idx;
            }
            else
            {
                idx_1 = dir_idx;
                idx_2 = (dir_idx + 1) % DIR_STEPS;
            }

            double step_dif = sum_az[beam_idx][idx_2] -
                sum_az[beam_idx][idx_1];
            if (step_dif < 0.0)
                step_dif += two_pi;
            double point_dif = azim - sum_az[beam_idx][idx_1];
            if (point_dif < 0.0)
                point_dif += two_pi;
            double fidx = point_dif / step_dif;
            double coef1 = (double)idx_2 - fidx;
            double coef2 = fidx - (double)idx_1;
            double value = coef1 * sum_f[beam_idx][idx_1] +
                coef2 * sum_f[beam_idx][idx_2];
            double dif = fabs(g_meas_spec_peak_freq[beam_idx][idx] - value);
            if (dif < min_dif[beam_idx][dir_idx])
            {
                min_dif[beam_idx][dir_idx] = dif;
                min_idx[beam_idx][dir_idx] = idx;
            }
        }

        //-------------------------//
        // remove all other points //
        //-------------------------//

        for (int idx = 0; idx < g_count[beam_idx]; idx++)
        {
            g_use_data[beam_idx][idx] = 0;
        }
        for (int dir_idx = 0; dir_idx < DIR_STEPS; dir_idx++)
        {
            if (min_idx[beam_idx][dir_idx] != -1)
            {
                g_use_data[beam_idx][min_idx[beam_idx][dir_idx]] = 1;
                data_count++;
            }
        }
    }
printf("%d data points\n", data_count);
    return(data_count);
}

//------------//
// est_sigma0 //
//------------//

float
est_sigma0(
    int    beam_idx,
    float  incidence_angle)
{
    float s0_table[2][11] = {
      { 0.0255338, 0.0220457, 0.0193465, 0.0171857, 0.0153964, 0.0138646,
        0.0125171, 0.0113071, 0.010215, 0.00923337, 0.00837293 },
      { 0.0147139, 0.01166, 0.00936597, 0.00758692, 0.00617038, 0.00502344,
        0.00409095, 0.00333365, 0.00271822, 0.00221762, 0.00181041 }
    };

    Index index;
    index.SpecifyCenters(40.0 * dtr, 60.0 * dtr, 11);

    int idx[2];
    float coef[2];
    index.GetLinearCoefsStrict(incidence_angle, idx, coef);
    float sigma0 = s0_table[beam_idx][idx[0]] * coef[0] +
        s0_table[beam_idx][idx[1]] * coef[1];
    return(sigma0);
}
