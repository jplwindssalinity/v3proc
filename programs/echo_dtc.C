//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    echo_dtc
//
// SYNOPSIS
//    echo_dtc [ -r ] [ -d diagnostic_base ] <config_file>
//      <echo_data_file> <dtc_base>
//
// DESCRIPTION
//    Reads the echo data file and generates corrected Doppler
//    tracking constants which should center the echo.  It gets
//    a bit more complicated than that.  Once the coefficients
//    are calculated for each orbit step (if the orbit step has
//    enough points) a parametric form is fitted to smooth the
//    coefficients and fill in missing gaps.
//
// OPTIONS
//    [ -r ]                  Allow time regression.
//    [ -d diagnostic_base ]  Generate diagnostic output with the given
//                              filename base.
//
// OPERANDS
//    The following operands are supported:
//      <config_file>     The simulation configuration file.
//      <echo_data_file>  The echo data file.
//      <dtc_base>        The base name of the output DTC files.
//
// EXAMPLES
//    An example of a command line is:
//      % echo_dtc -d qscat.diag qscat.cfg echo.dat dtc
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

#define OPTSTRING  "rd:"

#define SIGNAL_ENERGY_THRESHOLD  0
#define ORBIT_STEPS              256
#define SECTOR_COUNT             8
#define MIN_POINTS_PER_SECTOR    10
#define MIN_SECTORS              6

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int     process_orbit_step(int beam_idx, int orbit_step,
            const char* diag_base);
int     accumulate(int beam_idx, double azimuth, double meas_spec_peak);
int     fit_terms(const char* diag_base, int beam_idx, double** terms);
int     plot_fit(const char* base, int beam_idx, int term_idx, double** terms,
            double** p, int term_count);
double  ds_evaluate_3(double* x, void* ptr);
double  ds_evaluate_5(double* x, void* ptr);
double  evaluate_35(int* good, double* coef, double* x, int count);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r ]", "[ -d diagnostic_base ]",
    "<config_file>", "<echo_data_file>", "<dtc_base>", 0};

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double**  g_terms[NUMBER_OF_QSCAT_BEAMS];
int       g_sector_count[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS];
// these are allocated dynamically
double**  g_azimuth;
double**  g_meas_spec_peak;

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

    int opt_regression = 1;    // default, check for regression
    int opt_diag = 0;
    const char* diag_base = NULL;

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
        case 'r':
            // this flag means *don't* check for regression
            opt_regression = 0;
            break;
        case 'd':
            opt_diag = 1;
            diag_base = optarg;
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
    const char* dtc_base = argv[optind++];

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

    //--------------------------------------------------//
    // determine largest number of spots per orbit step //
    //--------------------------------------------------//

    int max_count = 0;
    int count = 0;
    int last_orbit_step = -1;
    do
    {
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
        int orbit_step = echo_info.orbitStep;
        if (orbit_step != last_orbit_step)
        {
            if (count > max_count)
            {
                max_count = count;
            }
            last_orbit_step = orbit_step;
            count = 0;
        }
        for (int spot_idx = 0; spot_idx < spots_per_frame; spot_idx++)
        {
            if (echo_info.flag[spot_idx] == EchoInfo::OK ||
              echo_info.totalSignalEnergy[spot_idx] >= SIGNAL_ENERGY_THRESHOLD)
            {
                count++;
            }
        }
    } while (1);
    if (count > max_count)
    {
        max_count = count;
    }

    //------------//
    // initialize //
    //------------//

    lseek(ifd, 0, SEEK_SET);

    g_azimuth = (double **)make_array(sizeof(double), 2,
        NUMBER_OF_QSCAT_BEAMS, max_count);
    if (g_azimuth == NULL)
    {
        fprintf(stderr, "%s: error allocating azimuth array (%d x %d)\n",
            command, NUMBER_OF_QSCAT_BEAMS, max_count);
    }

    g_meas_spec_peak = (double **)make_array(sizeof(double), 2,
        NUMBER_OF_QSCAT_BEAMS, max_count);
    if (g_meas_spec_peak == NULL)
    {
        fprintf(stderr,
            "%s: error allocating meas_spec_peak array (%d x %d)\n", command,
            NUMBER_OF_QSCAT_BEAMS, max_count);
    }

    last_orbit_step = -1;
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

    ETime last_time, regression_start;
    int regression = 0;
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

        //----------------------//
        // check for regression //
        //----------------------//

        if (opt_regression)
        {
            if (echo_info.frameTime <= last_time)
            {
                regression = 1;
                regression_start = echo_info.frameTime;
                continue;
            }
            else
            {
                // check for end of regression
                if (regression)
                {
                    char regression_start_code_b[CODE_B_TIME_LENGTH];
                    char regression_end_code_b[CODE_B_TIME_LENGTH];
                    regression_start.ToCodeB(regression_start_code_b);
                    last_time.ToCodeB(regression_end_code_b);
                    fprintf(stderr, "%s: regressive data omitted (%s to %s)\n",
                        command, regression_start_code_b,
                        regression_end_code_b);
                    regression = 0;
                }
            }
            last_time = echo_info.frameTime;
        }

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
                if (last_orbit_step != -1)
                {
                    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS;
                        beam_idx++)
                    {
                        process_orbit_step(beam_idx, last_orbit_step,
                            diag_base);
                    }
                }
                last_orbit_step = orbit_step;
            }

            //-----------------------//
            // accumulation decision //
            //-----------------------//

            if (echo_info.flag[spot_idx] != EchoInfo::OK)
            {
                continue;
            }

            if (echo_info.totalSignalEnergy[spot_idx] <
                SIGNAL_ENERGY_THRESHOLD)
            {
                continue;
            }

            double azimuth = two_pi *
                (double)echo_info.idealEncoder[spot_idx] / (double)ENCODER_N;
            int beam_idx = echo_info.SpotBeamIdx(spot_idx);
            accumulate(beam_idx, azimuth,
                echo_info.measSpecPeakFreq[spot_idx]);
        }
    } while (1);

    //--------------//
    // fit to terms //
    //--------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        fit_terms(diag_base, beam_idx, g_terms[beam_idx]);
    }

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
        doppler_tracker.SetTerms(g_terms[beam_idx]);
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

#define QUOTE  '"'

int
process_orbit_step(
    int          beam_idx,
    int          orbit_step,
    const char*  diag_base)
{
    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double a, p, c;
    sinfit(g_azimuth[beam_idx], g_meas_spec_peak[beam_idx], NULL,
        g_count[beam_idx], &a, &p, &c);

    //---------------------------//
    // determine sector coverage //
    //---------------------------//

    int sector[SECTOR_COUNT];
    for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
    {
        sector[sector_idx] = 0;
    }
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        int sector_idx = (int)(SECTOR_COUNT * g_azimuth[beam_idx][i] / two_pi);
        sector[sector_idx]++;
    }
    g_sector_count[beam_idx][orbit_step] = 0;
    for (int sector_idx = 0; sector_idx < SECTOR_COUNT; sector_idx++)
    {
        if (sector[sector_idx] >= MIN_POINTS_PER_SECTOR)
        {
            g_sector_count[beam_idx][orbit_step]++;
        }
    }

    //------------------------------------------//
    // subtract the sinusoid from the constants //
    //------------------------------------------//

    double** terms = g_terms[beam_idx];

    double A = *(*(terms + orbit_step) + 0);
    double P = *(*(terms + orbit_step) + 1);
    double C = *(*(terms + orbit_step) + 2);

    double newA = sqrt(A*A*cos(2.0*P) + a*a*cos(2.0*p) - cos(P+p));
    double newC = C - c;
    double y = A*sin(P) - a*sin(p);
    double x = A*cos(P) - a*cos(p);
    double newP = atan2(y, x);

    *(*(terms + orbit_step) + 0) = newA;
    *(*(terms + orbit_step) + 1) = newP;
    *(*(terms + orbit_step) + 2) = newC;

    //--------------------//
    // output diagnostics //
    //--------------------//

    if (diag_base)
    {
        //---------------------------------------------//
        // determine if orbit step will be used in fit //
        //---------------------------------------------//

        char* used_string = "DISCARDED";
        if (g_sector_count[beam_idx][orbit_step] >= MIN_SECTORS)
        {
            used_string = "USED";
        }

        char filename[1024];
        sprintf(filename, "%s.b%1d.s%03d", diag_base, beam_idx + 1,
            orbit_step);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            return(0);
        }
        fprintf(ofp, "@ subtitle %cBeam %d, Orbit Step %d, %s%c\n", QUOTE,
            beam_idx + 1, orbit_step, used_string, QUOTE);
        fprintf(ofp, "@ xaxis label %cAzimuth Angle (deg)%c\n", QUOTE, QUOTE);
        fprintf(ofp, "@ yaxis label %cBaseband Frequency (kHz)%c\n", QUOTE,
            QUOTE);
        for (int i = 0; i < g_count[beam_idx]; i++)
        {
            fprintf(ofp, "%g %g\n", g_azimuth[beam_idx][i] * rtd,
                g_meas_spec_peak[beam_idx][i] / 1000.0);
        }
        fprintf(ofp, "&\n");

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
            float value = a * cos(azim + p) + c;
            fprintf(ofp, "%g %g\n", azim * rtd, value / 1000.0);
        }

        fclose(ofp);
    }

    g_count[beam_idx] = 0;

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

//-----------//
// fit_terms //
//-----------//

int
fit_terms(
    const char*  diag_base,
    int          beam_idx,
    double**     terms)
{
    //-------------------//
    // set up good array //
    //-------------------//
    // this indicates whether to use the coefficient for this orbit step

    int good[ORBIT_STEPS];
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (g_sector_count[beam_idx][orbit_step] >= MIN_SECTORS)
            good[orbit_step] = 1;
        else
            good[orbit_step] = 0;
    }

    double sum;
    int n;

    //------------------------------//
    // fit a curve to the amplitude //
    //------------------------------//

    int amp_count = 5;
    double amp_init[] = { 432000.0, 500.0, -90.0 * dtr, 500.0, 0.0 * dtr };
    double amp_lambda[] = { 1000.0,  50.0,   5.0 * dtr,  50.0, 5.0 * dtr };

    double coef[ORBIT_STEPS];
    sum = 0.0;
    n = 0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        coef[orbit_step] = *(*(terms + orbit_step) + 0);
        if (good[orbit_step])
        {
            sum += coef[orbit_step];
            n++;
        }
    }
    amp_init[0] = sum / (double)n;
    double** amp_p = make_p(amp_count, amp_init, amp_lambda);
    if (amp_p == NULL)
        return(0);
    char* ptr[2];
    ptr[0] = (char *)good;
    ptr[1] = (char *)coef;
    downhill_simplex(amp_p, amp_count, amp_count, 1E-6, ds_evaluate_5, ptr);

    //--------------------------//
    // fit a curve to the phase //
    //--------------------------//

    int phase_count = 3;
    double phase_init[] = { 0.0, 4.0, 0.0 * dtr };
    double phase_lambda[] = { 1.0, 1.0, 10.0 * dtr };
    sum = 0.0;
    n = 0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        coef[orbit_step] = *(*(terms + orbit_step) + 1);
        if (good[orbit_step])
        {
            sum += coef[orbit_step];
            n++;
        }
    }
    phase_init[0] = sum / (double)n;
    double** phase_p = make_p(phase_count, phase_init, phase_lambda);
    if (phase_p == NULL)
        return(0);
    downhill_simplex(phase_p, phase_count, phase_count, 1E-6, ds_evaluate_3,
        ptr);

    //-------------------------//
    // fit a curve to the bias //
    //-------------------------//

    int bias_count = 5;
    double bias_init[] = { 0.0, 0.0, 0.0 * dtr, 0.0, 0.0 * dtr };
    double bias_lambda[] = { 5.0, 5.0, 5.0 * dtr, 5.0, 5.0 * dtr };
    sum = 0.0;
    n = 0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        coef[orbit_step] = *(*(terms + orbit_step) + 2);
        if (good[orbit_step])
        {
            sum += coef[orbit_step];
            n++;
        }
    }
    bias_init[0] = sum / (double)n;
    double** bias_p = make_p(bias_count, bias_init, bias_lambda);
    if (bias_p == NULL)
        return(0);
    downhill_simplex(bias_p, bias_count, bias_count, 1E-6, ds_evaluate_5, ptr);

    //--------------------//
    // generate fit plots //
    //--------------------//

    if (diag_base)
    {
        plot_fit(diag_base, beam_idx, 0, terms, amp_p, 5);
        plot_fit(diag_base, beam_idx, 1, terms, phase_p, 3);
        plot_fit(diag_base, beam_idx, 2, terms, bias_p, 5);
    }

    //---------------//
    // plug in terms //
    //---------------//

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;

        *(*(terms + orbit_step) + 0) = amp_p[0][0] +
            amp_p[0][1] * cos(angle + amp_p[0][2]) +
            amp_p[0][3] * cos(2.0 * angle + amp_p[0][4]);

        *(*(terms + orbit_step) + 1) = phase_p[0][0] +
            phase_p[0][1] * cos(angle + phase_p[0][2]);

        *(*(terms + orbit_step) + 2) = bias_p[0][0] +
            bias_p[0][1] * cos(angle + bias_p[0][2]) +
            bias_p[0][3] * cos(2.0 * angle + bias_p[0][4]);
    }

    free_p(amp_p, amp_count);
    free_p(phase_p, phase_count);
    free_p(bias_p, bias_count);

    return(1);
}

//----------//
// plot_fit //
//----------//

int
plot_fit(
    const char*  base,
    int          beam_idx,
    int          term_idx,
    double**     terms,
    double**     p,
    int          term_count)
{
    static char* term_string[3] = { "amp", "phase", "bias" };
    static char* title_string[3] = { "Amplitude", "Phase", "Bias" };
    static char* unit_string[3] = { "Frequency (Hz)", "Angle (radians)",
        "Frequency (Hz)" };

    char filename[1024];
    sprintf(filename, "%s.b%1d.%s", base, beam_idx + 1,
        term_string[term_idx]);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    fprintf(ofp, "@ subtitle %cBeam %d, %s%c\n", QUOTE,
        beam_idx + 1, title_string[term_idx], QUOTE);
    fprintf(ofp, "@ xaxis label %cOrbit Step%c\n", QUOTE, QUOTE);
    fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, unit_string[term_idx],
        QUOTE);

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (g_sector_count[beam_idx][orbit_step] < MIN_SECTORS)
            continue;

        fprintf(ofp, "%d %g\n", orbit_step,
            *(*(terms + orbit_step) + term_idx));
    }
    fprintf(ofp, "&\n");

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double value = p[0][0] + p[0][1] * cos(angle + p[0][2]);
        if (term_count == 5)
            value += p[0][3] * cos(2.0 * angle + p[0][4]);
        fprintf(ofp, "%d %g\n", orbit_step, value);
    }
    fclose(ofp);
    return(1);
}

//---------------//
// ds_evaluate_3 //
//---------------//

double
ds_evaluate_3(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char **)ptr;
    int* good = (int *)ptr2[0];   // 0 = ignore, 1 = use
    double* coef = (double *)ptr2[1];    // the coefficients

    return (evaluate_35(good, coef, x, 3));
}

//---------------//
// ds_evaluate_5 //
//---------------//

double
ds_evaluate_5(
    double*  x,
    void*    ptr)
{
    char** ptr2 = (char **)ptr;
    int* good = (int *)ptr2[0];   // 0 = ignore, 1 = use
    double* coef = (double *)ptr2[1];    // the coefficients

    return (evaluate_35(good, coef, x, 5));
}

//-------------//
// evaluate_35 //
//-------------//
// evaluate a three parameter A + B * cos(phi + C)
// or a five parameter A + B * cos(phi + C) + D * cos(2*phi + E)

double
evaluate_35(
    int*     good,
    double*  coef,
    double*  x,
    int      count)
{
    double sse = 0.0;
    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++)
    {
        if (! good[orbit_step])
            continue;

        double angle = two_pi * (double)orbit_step / (double)ORBIT_STEPS;
        double fit = x[0] + x[1] * cos(angle + x[2]);
        if (count == 5)
        {
            fit += x[3] * cos(2.0 * angle + x[4]);
        }
        double dif = fit - coef[orbit_step];
        sse += (dif * dif);
    }
    return(sse);
}
