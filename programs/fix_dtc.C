//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_dtc
//
// SYNOPSIS
//    fix_dtc <sim_config_file> <echo_data_file> <dtc_base>
//
// DESCRIPTION
//    Reads the echo data file and generates corrected Doppler
//    tracking constants which should center the echo.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>   The simulation configuration file.
//      <echo_data_file>    The echo data file.
//      <dtc_base>          The base name of the output DTC files.
//
// EXAMPLES
//    An example of a command line is:
//      % fix_dtc qscat.cfg echo.dat dtc
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

#define MAXIMUM_SPOTS_PER_ORBIT_STEP  10000
#define SIGNAL_ENERGY_THRESHOLD       1.0E-8
#define ORBIT_STEPS      256
#define LINE_SIZE        2048

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int process_orbit_step(int beam_idx, int orbit_step);
int accumulate(int beam_idx, double azimuth, double meas_spec_peak);
int sinfit(double* azimuth, double* value, int count, double* amplitude,
        double* phase, double* bias);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<echo_data_file>",
    "<dtc_base>", 0};

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double    g_azimuth[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
double  g_meas_spec_peak[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
double**  g_terms[NUMBER_OF_QSCAT_BEAMS];

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
    if (argc != 4)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* echo_data_file = argv[clidx++];
    const char* dtc_base = argv[clidx++];

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
    config_list.StompOrAppend(USE_RGC_KEYWORD, "1");
    config_list.StompOrAppend(USE_DTC_KEYWORD, "1");

    //----------------//
    // create a QSCAT //
    //----------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
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

    int last_orbit_step = -1;
    int orbit_step = -1;
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        // terms are [0] = amplitude, [1] = phase, [2] = bias
        g_terms[beam_idx] = (double **)make_array(sizeof(double), 2,
            ORBIT_STEPS, 3);
        g_count[beam_idx] = 0;
        qscat.cds.beamInfo[beam_idx].dopplerTracker.GetTerms(
            g_terms[beam_idx]);
    }

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

        double azimuth;

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

            azimuth = two_pi * (double)ideal_encoder / (double)ENCODER_N;
            accumulate(beam_idx, azimuth, meas_spec_peak);
            break;
        case EPHEMERIS_ID:
            float gcx, gcy, gcz, velx, vely, velz, roll, pitch, yaw;
            read_ephemeris(ifd, &gcx, &gcy, &gcz, &velx, &vely, &velz, &roll,
                &pitch, &yaw);
            break;
        case ORBIT_STEP_ID:
            read_orbit_step(ifd, &orbit_step);
            if (orbit_step != last_orbit_step)
            {
                if (last_orbit_step != -1)
                {
                    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS;
                        beam_idx++)
                    {
                        process_orbit_step(beam_idx, last_orbit_step);
                    }
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

    //-------------//
    // set Doppler //
    //-------------//

    DopplerTracker doppler_tracker;
    if (! doppler_tracker.Allocate(ORBIT_STEPS))
    {
        fprintf(stderr, "%s: error allocating Doppler tracker\n", command);
        exit(1);
    }
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        doppler_tracker.Set(g_terms[beam_idx]);
        char filename[1024];
        sprintf(filename, "%s.%d", dtc_base, beam_idx + 1);
        if (! doppler_tracker.WriteBinary(filename))
        {
            fprintf(stderr, "%s: error writing DTC file %s\n", command,
                filename);
            exit(1);
        }
    }

    return(0);
}

//--------------------//
// process_orbit_step //
//--------------------//

int
process_orbit_step(
    int  beam_idx,
    int  orbit_step)
{
    //-----------------//
    // report the data //
    //-----------------//

    char filename[1024];
    sprintf(filename, "diag.beam.%1d.step.%03d", beam_idx+1, orbit_step);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        return(0);
    }
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        fprintf(ofp, "%g %g\n", g_azimuth[beam_idx][i] * rtd,
            g_meas_spec_peak[beam_idx][i]);
    }
    fprintf(ofp, "&\n");

    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double a, p, c;
    sinfit(g_azimuth[beam_idx], g_meas_spec_peak[beam_idx], g_count[beam_idx],
        &a, &p, &c);

    //-----------------------------//
    // estimate the standard error //
    //-----------------------------//

    double sum_sqr_dif = 0.0;
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        double dif = g_meas_spec_peak[beam_idx][i] -
            (a * cos(g_azimuth[beam_idx][i] + p) + c);
        sum_sqr_dif += dif*dif;
    }
    double std_dev = sqrt(sum_sqr_dif / (double)g_count[beam_idx]);
    double std_err = std_dev / sqrt((double)g_count[beam_idx]);

    fprintf(ofp, "# standard error = %g Hz\n", std_err);

    //--------------------------------------------//
    // report the fit sinusoid and standard error //
    //--------------------------------------------//

    double step = two_pi / 360.0;
    for (double azim = 0; azim < two_pi + step / 2.0; azim += step)
    {
        fprintf(ofp, "%g %g\n", azim * rtd, a * cos(azim + p) + c);
    }

    //------------------------------------------//
    // subtract the sinusoid from the constants //
    //------------------------------------------//

    double** terms = g_terms[beam_idx];

    double A = *(*(terms + orbit_step) + 0);
    double P = *(*(terms + orbit_step) + 1);
    double C = *(*(terms + orbit_step) + 2);

    // to do this, just negate the a and the c term
    // if f(x) = a * cos(x + p) + c, then -f(x) = -a * cos(x + p) - c
    a *= -1.0;
    c *= -1.0;

    double newA = sqrt(A*A + 2.0*A*a*cos(P-p) + a*a);
    double newC = C + c;
    double y = A*sin(P) + a*sin(p);
    double x = A*cos(P) + a*cos(p);
    double newP = atan2(y, x);

    *(*(terms + orbit_step) + 0) = newA;
    *(*(terms + orbit_step) + 1) = newP;
    *(*(terms + orbit_step) + 2) = newC;

    g_count[beam_idx] = 0;

    //---------------------------//
    // close the diagnostic file //
    //---------------------------//

    fclose(ofp);

    return(1);
}

//------------//
// accumulate //
//------------//

int
accumulate(
    int     beam_idx,
    double  azimuth,
    double  meas_spec_peak)
{
    int idx = g_count[beam_idx];
    g_azimuth[beam_idx][idx] = azimuth;
    g_meas_spec_peak[beam_idx][idx] = meas_spec_peak;
    g_count[beam_idx]++;

    return(1);
}

//--------//
// sinfit //
//--------//
// a brilliantly derived sinusoidal regression method by B. Stiles.
// fits to y = A + B*cos(wt) + C*sin(wt)
// this equates to y = A + D*cos(wt + E) where
// B = D*cos(E) and C = -D*sin(E)
// solved for...
// D = sqrt(B*B + C*C) and E = atan2(-B / C)

int
sinfit(
    double*  azimuth,
    double*  value,
    int      count,
    double*  amplitude,
    double*  phase,
    double*  bias)
{
    //-----------------------------------//
    // set up matrix and solution vector //
    //-----------------------------------//

    double matrix_a[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            matrix_a[i][j] = 0.0;
        }
    }

    double vector_y[3];
    for (int i = 0; i < 3; i++)
    {
        vector_y[i] = 0.0;
    }

    //------------//
    // accumulate //
    //------------//

    matrix_a[0][0] = count;
    for (int i = 0; i < count; i++)
    {
        double si = sin(azimuth[i]);
        double co = cos(azimuth[i]);
        double ss = si * si;
        double cc = co * co;
        double cs = co * si;
        matrix_a[0][1] += co;
        matrix_a[0][2] += si;
        matrix_a[1][0] += co;
        matrix_a[1][1] += cc;
        matrix_a[1][2] += cs;
        matrix_a[2][0] += si;
        matrix_a[2][1] += cs;
        matrix_a[2][2] += ss;

        vector_y[0] += value[i];
        vector_y[1] += value[i] * co;
        vector_y[2] += value[i] * si;
    }

    //---------------------------------//
    // transfer into matrix and vector //
    //---------------------------------//

    Matrix A;
    A.Allocate(3, 3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            A.SetElement(i, j, matrix_a[i][j]);
        }
    }

    Vector Y;
    Y.Allocate(3);
    for (int i = 0; i < 3; i++)
    {
        Y.SetElement(i, vector_y[i]);
    }

    //-------//
    // solve //
    //-------//

    Vector X;
    if (! A.SolveSVD(&Y, &X))
    {
        fprintf(stderr, "Error solving equations!\n");
        exit(1);
    }

    //-------------------------//
    // return the coefficients //
    //-------------------------//

    double a, b, c;
    X.GetElement(0, &a);
    X.GetElement(1, &b);
    X.GetElement(2, &c);

    *amplitude = sqrt(b*b + c*c);
    *phase = atan2(-c, b);
    *bias = a;

    return(1);
}
