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
//      % generate_foff_table qscat.cfg echo.dat specref.dat foff.dat
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

#define SECONDS_PER_ORBIT_STEP  190.0
#define FOFF_ORBIT_STEPS        32
#define FOFF_AZIMUTH_STEPS      36

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
        float meas_spec_peak, float exp_spec_peak);

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

    unsigned int orbit_time = 0;

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

        switch (id)
        {
        case SPOT_ID:
            int beam_idx, ideal_encoder, land_flag, orbit_step, azimuth_step;
            float tx_doppler, rx_gate_delay, tx_center_azimuth,
                meas_spec_peak_freq, total_signal_energy, time_since_an;
            if (! read_spot(ifd, &beam_idx, &tx_doppler, &rx_gate_delay,
                &ideal_encoder, &tx_center_azimuth, &meas_spec_peak_freq,
                &total_signal_energy, &land_flag))
            {
                fprintf(stderr,
                    "%s: error reading spot from echo data file %s\n",
                    command, echo_data_file);
                exit(1);
            }

            // calculate X azimuth step
            azimuth_step = (int)(FOFF_AZIMUTH_STEPS *
                (double)ideal_encoder / (double)ENCODER_N + 0.5);
            azimuth_step %= FOFF_AZIMUTH_STEPS;

            // calculate X orbit step
            time_since_an = (float)orbit_time /
                (float)ORBIT_TICKS_PER_SECOND;
            orbit_step = (int)(time_since_an / SECONDS_PER_ORBIT_STEP +
                0.5);
            orbit_step %= FOFF_ORBIT_STEPS;

            accumulate(beam_idx, orbit_step, azimuth_step, meas_spec_peak_freq,
                calc_spec_peak_freq);
            break;
        case EPHEMERIS_ID:
            float gcx, gcy, gcz, velx, vely, velz, roll, pitch, yaw;
            read_ephemeris(ifd, &gcx, &gcy, &gcz, &velx, &vely, &velz, &roll,
                &pitch, &yaw);
            break;
        case ORBIT_STEP_ID:
            read_orbit_step(ifd, &orbit_step);
            break;
        case ORBIT_TIME_ID:
            read_orbit_time(ifd, &orbit_time);
            break;
        default:
            break;
        }
    } while (1);

    close(ifd);

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
        if (ofp == NULL)
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
            if (beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
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
    float  meas_spec_peak,
    float  exp_spec_peak)
{
    float dif = meas_spec_peak - exp_spec_peak;
    g_meas_sum[beam_idx][orbit_step][azimuth_step] += meas_spec_peak;
    g_calc_sum[beam_idx][orbit_step][azimuth_step] += exp_spec_peak;
    g_foff_sum[beam_idx][orbit_step][azimuth_step] += dif;
    g_count[beam_idx][orbit_step][azimuth_step]++;

    return(1);
}
