//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    gen_foff
//
// SYNOPSIS
//    gen_foff [ -d diagnostic_base ] <sim_config_file>
//      <echo_data_file> <foff_file>
//
// DESCRIPTION
//    Reads the echo data file and determines the correction that
//    needs to be applied to the reference vector frequency in order
//    to obtain a peak spectral response that matches the data.
//
// OPTIONS
//    [ -d diagnostic_base ]  Output diagnostic files with the
//                              specified base filename.
//
// OPERANDS
//    <sim_config_file>  The simulation configuration file.
//    <echo_data_file>   The echo data input file.
//    <foff_file>        The output frequency offset file.
//
// EXAMPLES
//    An example of a command line is:
//      % gen_foff -d xxx qscat.cfg qscat.echo qscat.foff
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
#include "BYUXTable.h"
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
#define SIGNAL_ENERGY_THRESHOLD  5E4
#define FOFF_ORBIT_STEPS         32
#define FOFF_AZ_STEPS            36

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

double  eval_rvf(int slices_per_spot, int beam_idx, float azim,
            float orbit_fraction, float rvf, float target_peak_spec_freq,
            Qscat* qscat, BYUXTable* byux);
double  rvf_funk(double trial_rvf, char** arguments);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d diagnostic_base ]", "<sim_config_file>",
    "<echo_data_file>", "<foff_file>", 0 };

double g_foff_sum[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZ_STEPS];
unsigned int g_count[NUMBER_OF_QSCAT_BEAMS][FOFF_ORBIT_STEPS][FOFF_AZ_STEPS];

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
    int slices_per_spot = l1a.frame.slicesPerSpot;

    //----------------------------------//
    // create and configure BYU X Table //
    //----------------------------------//

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

        //----------------------------------------------//
        // set up frame based spacecraft and instrument //
        //----------------------------------------------//

        spacecraft.orbitState.rsat.SetPosition(echo_info.gcX, echo_info.gcY,
            echo_info.gcZ);
        spacecraft.orbitState.vsat.Set(echo_info.velX, echo_info.velY,
            echo_info.velZ);
        spacecraft.attitude.SetRPY(echo_info.roll, echo_info.pitch,
            echo_info.yaw);

        qscat.cds.orbitTime = echo_info.orbitTicks;
        qscat.sas.antenna.spinRate = echo_info.spinRate;

        float orbit_fraction =qscat.cds.OrbitFraction();

        //---------------//
        // for each spot //
        //---------------//

        for (int spot_idx = 0; spot_idx < spots_per_frame; spot_idx++)
        {
            //-----------------------//
            // accumulation decision //
            //-----------------------//

            if (echo_info.flag[spot_idx] != EchoInfo::OK)
                continue;

            if (echo_info.totalSignalEnergy[spot_idx] < SIGNAL_ENERGY_THRESHOLD)
                continue;

            //------------------------------//
            // set up spot-based instrument //
            //------------------------------//

            qscat.cds.orbitStep = echo_info.SpotOrbitStep(spot_idx);
            qscat.cds.currentBeamIdx = echo_info.beamIdx[spot_idx];

            qscat.sas.antenna.txCenterAzimuthAngle =
                echo_info.txCenterAzimuthAngle[spot_idx];

            qscat.ses.CmdTxDopplerEu(echo_info.txDoppler[spot_idx]);
            qscat.ses.CmdRxGateDelayEu(echo_info.rxGateDelay[spot_idx]);

            //-------------------------------------------------//
            // determine frequency of nominal reference vector //
            //-------------------------------------------------//

            float nominal_rvf = byux.GetDeltaFreq(&spacecraft, &qscat);

            //------------------------------------------//
            // find the reference vector frequency that //
            // gives the desired peak spectral response //
            //------------------------------------------//

            double min_freq = -80000.0;
            double max_freq = +80000.0;
            double freq_tol = 1.0;
            double desired_rvf, rvf_err;

            int beam_idx = qscat.cds.currentBeamIdx;
            qscat.TxCenterToGroundImpactAzimuth(&spacecraft);
            float azim = qscat.sas.antenna.groundImpactAzimuthAngle;
            float target_peak_spec_freq = echo_info.measSpecPeakFreq[spot_idx];

            char* arg_array[7];
            arg_array[0] = (char *)&slices_per_spot;
            arg_array[1] = (char *)&beam_idx;
            arg_array[2] = (char *)&azim;
            arg_array[3] = (char *)&orbit_fraction;
            arg_array[4] = (char *)&target_peak_spec_freq;
            arg_array[5] = (char *)&qscat;
            arg_array[6] = (char *)&byux;

            if (! golden_section_search(min_freq, max_freq, freq_tol,
              rvf_funk, arg_array, &desired_rvf, &rvf_err))
            {
                continue;
            }

            //--------------------------------------//
            // determine orbit and azimuth indicies //
            //--------------------------------------//

            float time_since_an = (float)echo_info.orbitTicks /
                (float)ORBIT_TICKS_PER_SECOND;
            int orbit_step = (int)(time_since_an /
                BYU_TIME_INTERVAL_BETWEEN_STEPS + 0.5);
            orbit_step %= FOFF_ORBIT_STEPS;

            int azimuth_step = (int)(FOFF_AZ_STEPS *
                (double)echo_info.idealEncoder[spot_idx] /
                (double)ENCODER_N + 0.5);
            azimuth_step %= FOFF_AZ_STEPS;

            //------------//
            // accumulate //
            //------------//

            // add foff to nominal RVF to get desired RVF
            double foff = desired_rvf - nominal_rvf;
            g_foff_sum[beam_idx][orbit_step][azimuth_step] += foff;
            g_count[beam_idx][orbit_step][azimuth_step]++;
        }
    } while (1);

    close(ifd);

    //----------------------//
    // write out foff table //
    //----------------------//

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
            for (int azimuth_step = 0; azimuth_step < FOFF_AZ_STEPS;
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

    //------------------------//
    // write diagnostic files //
    //------------------------//

    if (opt_diag)
    {
        FILE* ofp = fopen(diag_base, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening diagnostic output file %s\n",
                command, diag_base);
            exit(1);
        }
        for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
        {
            for (int orbit_step = 0; orbit_step < FOFF_ORBIT_STEPS;
                orbit_step++)
            {
                for (int azimuth_step = 0; azimuth_step < FOFF_AZ_STEPS;
                    azimuth_step++)
                {
                    if (g_count[beam_idx][orbit_step][azimuth_step])
                    {
                        fprintf(ofp, "%g %g %d\n", (double)orbit_step +
                          (double)azimuth_step / (double)FOFF_AZ_STEPS,
                          g_foff_sum[beam_idx][orbit_step][azimuth_step] /
                          (double)g_count[beam_idx][orbit_step][azimuth_step],
                          g_count[beam_idx][orbit_step][azimuth_step]);
                    }
                }
            }
            if (beam_idx != NUMBER_OF_QSCAT_BEAMS - 1)
                fprintf(ofp, "&\n");
        }
        fclose(ofp);
    }

    return (0);
}

//----------//
// eval_rvf //
//----------//
// return the negated summed squared error in the peak spectral
// frequency for a given reference vector frequency and target
// peak spectral frequency

double
eval_rvf(
    int         slices_per_spot,
    int         beam_idx,
    float       azim,
    float       orbit_fraction,
    float       rvf,
    float       target_peak_spec_freq,
    Qscat*      qscat,
    BYUXTable*  byux)
{
    //----------------------//
    // get X for each slice //
    //----------------------//

    double x[10];
    for (int i = 0; i < slices_per_spot; i++)
    {
        int rel_idx;
        abs_to_rel_idx(i, slices_per_spot, &rel_idx);
        x[i] = byux->GetX(beam_idx, azim, orbit_fraction, rel_idx, rvf);
    }

    //-----------//
    // find peak //
    //-----------//

    float peak_slice, peak_freq, width;
    double slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
        7.0, 8.0, 9.0 };
    if (! gaussian_fit2(qscat, slice_number, x, slices_per_spot, &peak_slice,
        &peak_freq, &width))
    {
        return(0.0);
    }

    //-----------------------//
    // square the difference //
    //-----------------------//

    double dif = peak_freq - target_peak_spec_freq;
    dif *= dif;
    dif = -dif;    // negate for finding maxima

    return(dif);
}

//----------//
// rvf_funk //
//----------//

double
rvf_funk(
    double  trial_rvf,
    char**  arguments)
{
    int slices_per_spot = *(int *)arguments[0];
    int beam_idx = *(int *)arguments[1];
    float azim = *(float *)arguments[2];
    float orbit_fraction = *(float *)arguments[3];
    float target_peak_spec_freq = *(float *)arguments[4];
    Qscat* qscat = (Qscat *)arguments[5];
    BYUXTable* byux = (BYUXTable *)arguments[6];

    return(eval_rvf(slices_per_spot, beam_idx, azim, orbit_fraction, trial_rvf,
        target_peak_spec_freq, qscat, byux));
}
