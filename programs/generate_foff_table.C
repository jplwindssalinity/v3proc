//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    generate_foff_table
//
// SYNOPSIS
//    generate_foff_table [ -d diagfile ] <sim_config_file>
//        <echo_data_file> <foff_file>
//
// DESCRIPTION
//    Reads the frequency offset file and generates a correction table
//    to be used by the ground system to correct delta f.
//
// OPTIONS
//    [ -d diagfile ]    Output a diagnostic file.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The simulation configuration file.
//      <echo_proc_file>   The echo processing file.
//      <foff_file>        The frequency offset file.
//
// EXAMPLES
//    An example of a command line is:
//      % generate_foff_table qscat.cfg echo.dat foff.dat
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

//-----------//
// CONSTANTS //
//-----------//

#define SECONDS_PER_ORBIT_STEP  190.0
#define FOFF_ORBIT_STEPS             32
#define FOFF_AZIMUTH_STEPS           36
#define EPHEMERIS_CHAR          'E'
#define ORBIT_STEP_CHAR         'O'
#define ORBIT_TIME_CHAR         'T'
#define LINE_SIZE               2048

#define OPTSTRING    "d:"

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int accumulate(int beam_idx, int orbit_step, int azimuth_step,
        float baseband_freq, float expected_freq);

//------------------//
// OPTION VARIABLES //
//------------------//

unsigned char diagfile_opt = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d diagfile ]", "<sim_config_file>",
    "<echo_data_file>", "<foff_file>", 0};

double g_foff_sum[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZIMUTH_STEPS];
double g_meas_sum[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZIMUTH_STEPS];
double g_calc_sum[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZIMUTH_STEPS];
int    g_count[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZIMUTH_STEPS];

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

    char* diagfile = NULL;
    extern int optind;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'd':
            diagfile_opt = 1;
            diagfile = optarg;
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
    const char* foff_file = argv[optind++];

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

    FILE* ifp = fopen(echo_data_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening echo data file %s\n", command,
            echo_data_file);
        exit(1);
    }

    //------------//
    // initialize //
    //------------//

    int orbit_time = 0;

    //-----------------------//
    // read and process data //
    //-----------------------//

    do
    {
        //-------------//
        // read a line //
        //-------------//

        char line[LINE_SIZE];
        if (fgets(line, LINE_SIZE, ifp) == NULL)
        {
            if (feof(ifp))
                break;
            if (ferror(ifp))
            {
                fprintf(stderr, "%s: error reading echo data file %s\n",
                    command, echo_data_file);
                exit(1);
            }
        }

        //-------//
        // parse //
        //-------//

        switch (line[0])
        {
        case EPHEMERIS_CHAR:
            // ephemerities are not needed
            break;
        case ORBIT_TIME_CHAR:
            if (sscanf(line, " %*c %d", &orbit_time) != 1)
            {
                fprintf(stderr, "%s: error parsing orbit time line\n",
                    command);
                fprintf(stderr, "  Line: %s\n", line);
                exit(1);
            }
            break;
        case ORBIT_STEP_CHAR:
            // orbit step not needed
            break;
        default:
            int beam_idx, ideal_encoder, raw_encoder;
            float tx_doppler, rx_gate_delay, baseband_freq, expected_freq;
            if (sscanf(line, " %d %f %f %d %d %f %f", &beam_idx, &tx_doppler,
                &rx_gate_delay, &ideal_encoder, &raw_encoder, &baseband_freq,
                &expected_freq) != 7)
            {
                fprintf(stderr, "%s: error parsing line\n", command);
                fprintf(stderr, "  Line: %s\n", line);
                exit(1);
            }

            // calculate X azimuth step
            int azimuth_step = (int)(FOFF_AZIMUTH_STEPS *
                (double)ideal_encoder / (double)ENCODER_N + 0.5);
            azimuth_step %= FOFF_AZIMUTH_STEPS;

            // calculate X orbit step
            float time_since_an = (float)orbit_time /
                (float)ORBIT_TICKS_PER_SECOND;
            int orbit_step = (int)(time_since_an / SECONDS_PER_ORBIT_STEP +
                0.5);
            orbit_step %= FOFF_ORBIT_STEPS;

            accumulate(beam_idx, orbit_step, azimuth_step, baseband_freq,
                expected_freq);
            break;
        }
    } while (1);

    fclose(ifp);

    //-----------------//
    // write out table //
    //-----------------//

    FILE* ofp = fopen(foff_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening frequency offset output file %s\n",
            command, foff_file);
        exit(1);
    }

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        for (int orbit_step = 0; orbit_step < FOFF_ORBIT_STEPS; orbit_step++)
        {
            for (int azimuth_step = 0; azimuth_step < FOFF_AZIMUTH_STEPS;
                azimuth_step++)
            {
                if (g_count[beam_idx][orbit_step][azimuth_step])
                {
                    fprintf(ofp, "%d %d %g\n", orbit_step, azimuth_step,
                        g_foff_sum[beam_idx][orbit_step][azimuth_step] /
                        (double)g_count[beam_idx][orbit_step][azimuth_step]);
                }
            }
        }
    }
    fclose(ofp);

    //---------------------------//
    // write out diagnostic file //
    //---------------------------//

    if (diagfile_opt)
    {
        FILE* ofp = fopen(diagfile, "w");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening diagnostic output file %s\n",
                command, diagfile);
            exit(1);
        }

        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            for (int orbit_step = 0; orbit_step < FOFF_ORBIT_STEPS;
                orbit_step++)
            {
                for (int azimuth_step = 0; azimuth_step < FOFF_AZIMUTH_STEPS;
                    azimuth_step++)
                {
                    if (g_count[beam_idx][orbit_step][azimuth_step])
                    {
                        fprintf(ofp, "%g %g %g %g\n", (double)orbit_step +
                            (double)azimuth_step / (double)FOFF_AZIMUTH_STEPS,
                            g_foff_sum[beam_idx][orbit_step][azimuth_step] /
                          (double)g_count[beam_idx][orbit_step][azimuth_step],
                            g_meas_sum[beam_idx][orbit_step][azimuth_step] /
                          (double)g_count[beam_idx][orbit_step][azimuth_step],
                            g_calc_sum[beam_idx][orbit_step][azimuth_step] /
                          (double)g_count[beam_idx][orbit_step][azimuth_step]);
                    }
                }
            }
            fprintf(ofp, "&\n");
        }
        fclose(ofp);
    }

    //------------//
    // initialize //
    //------------//

    return(0);
}

//------------//
// accumulate //
//------------//

int
accumulate(
    int    beam_idx,
    int    orbit_step,
    int    azimuth_step,
    float  baseband_freq,
    float  expected_freq)
{
    float dif = baseband_freq - expected_freq;
    g_meas_sum[beam_idx][orbit_step][azimuth_step] += baseband_freq;
    g_calc_sum[beam_idx][orbit_step][azimuth_step] += expected_freq;
    g_foff_sum[beam_idx][orbit_step][azimuth_step] += dif;
    g_count[beam_idx][orbit_step][azimuth_step]++;

    return(1);
}
