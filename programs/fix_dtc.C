//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_dtc
//
// SYNOPSIS
//    fix_dtc [ -d diagnostic_base ] <config_file> <echo_data_file>
//        <dtc_base>
//
// DESCRIPTION
//    Reads the echo data file and generates corrected Doppler
//    tracking constants which should center the echo.
//
// OPTIONS
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
//      % fix_dtc -d qscat.diag qscat.cfg echo.dat dtc
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
#include <unistd.h>
#include "Misc.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "Tracking.h"
#include "BufferedList.h"
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

#define OPTSTRING  "d:"

#define MAXIMUM_SPOTS_PER_ORBIT_STEP  10000
#define SIGNAL_ENERGY_THRESHOLD       0
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

int process_orbit_step(int beam_idx, int orbit_step, const char* diag_base);
int accumulate(int beam_idx, double azimuth, double meas_spec_peak);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d diagnostic_base ]", "<config_file>",
    "<echo_data_file>", "<dtc_base>", 0};

int     g_count[NUMBER_OF_QSCAT_BEAMS];
double  g_azimuth[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
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
    //------------//
    // initialize //
    //------------//

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

    //------------//
    // initialize //
    //------------//

    int last_orbit_step = -1;
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

            if (echo_info.totalSignalEnergy[spot_idx] < SIGNAL_ENERGY_THRESHOLD)
            {
                continue;
            }

            double azimuth = two_pi *
                (double)echo_info.idealEncoder[spot_idx] / (double)ENCODER_N;
            accumulate(echo_info.beamIdx[spot_idx], azimuth,
                echo_info.measSpecPeakFreq[spot_idx]);
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

    //------------------------------------------//
    // subtract the sinusoid from the constants //
    //------------------------------------------//

    double** terms = g_terms[beam_idx];

    double A = *(*(terms + orbit_step) + 0);
    double P = *(*(terms + orbit_step) + 1);
    double C = *(*(terms + orbit_step) + 2);

    // to do this, just negate the a and the c term
    // if f(x) = a * cos(x + p) + c, then -f(x) = -a * cos(x + p) - c

    double newA = sqrt(A*A - 2.0*A*a*cos(P-p) + a*a);
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
        char filename[1024];
        sprintf(filename, "%s.b%1d.s%03d", diag_base, beam_idx + 1,
            orbit_step);
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
