//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    fix_dtc
//
// SYNOPSIS
//    fix_dtc <sim_config_file> <freq_offset_file> <dtc_base>
//
// DESCRIPTION
//    Reads the frequency offset file and generates corrected Doppler
//    tracking constants which should center the echo.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>   The simulation configuration file.
//      <freq_offset_file>  The frequency offset output file.
//      <dtc_base>          The base name of the output DTC files.
//
// EXAMPLES
//    An example of a command line is:
//      % fix_dtc qscat.cfg data.freqoff dtc
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

#define MAXIMUM_SPOTS_PER_ORBIT_STEP  10000
#define ORBIT_STEPS      256
#define EPHEMERIS_CHAR   'E'
#define ORBIT_STEP_CHAR  'O'
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
int accumulate(int beam_idx, float azimuth, float bb);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<freq_offset_file>",
    "<dtc_base>", 0};

int       g_count[NUMBER_OF_QSCAT_BEAMS];
double    g_azimuth[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
double    g_bb[NUMBER_OF_QSCAT_BEAMS][MAXIMUM_SPOTS_PER_ORBIT_STEP];
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
    const char* offset_file = argv[clidx++];
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

    //----------------//
    // create a QSCAT //
    //----------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //------------------------------//
    // open offset file for reading //
    //------------------------------//

    FILE* ifp = fopen(offset_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening offset file %s\n", command,
            offset_file);
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
                fprintf(stderr, "%s: error reading offset file %s\n", command,
                    offset_file);
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
        case ORBIT_STEP_CHAR:
            if (sscanf(line, " %*c %d", &orbit_step) != 1)
            {
                fprintf(stderr, "%s: error parsing orbit step line\n",
                    command);
                fprintf(stderr, "  Line: %s\n", line);
                exit(1);
            }
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
        default:
            int beam_idx, encoder, raw_encoder;
            float tx_doppler, rx_gate_delay, baseband_freq;
            if (sscanf(line, " %d %f %f %d %d %f", &beam_idx, &tx_doppler,
                &rx_gate_delay, &encoder, &raw_encoder, &baseband_freq)
                != 6)
            {
                fprintf(stderr, "%s: error parsing line\n", command);
                fprintf(stderr, "  Line: %s\n", line);
                exit(1);
            }
            qscat.sas.SetAzimuthWithEncoder((unsigned short)encoder);
            accumulate(beam_idx, qscat.sas.antenna.azimuthAngle,
                baseband_freq);
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
    sprintf(filename, "diag.beam.%1d.step.%3d", beam_idx+1, orbit_step);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        return(0);
    }
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        fprintf(ofp, "%g %g\n", g_azimuth[beam_idx][i] * rtd,
            g_bb[beam_idx][i]);
    }
    fprintf(ofp, "&\n");

    //----------------------------//
    // fit a sinusoid to the data //
    //----------------------------//

    double real[2], imag[2];
    for (int i = 0; i < 2; i++)
    {
        real[i] = 0.0;
        imag[i] = 0.0;
        for (int j = 0; j < g_count[beam_idx]; j++)
        {
            double arg = g_azimuth[beam_idx][j] * (double)i;
            double c = cos(arg);
            double s = sin(arg);
            real[i] += g_bb[beam_idx][j] * c;
            imag[i] += g_bb[beam_idx][j] * s;
        }
    }
    double a = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) /
        (double)g_count[beam_idx];
    double p = -atan2(imag[1], real[1]);
    double c = real[0] / (double)g_count[beam_idx];

    //-----------------------------//
    // estimate the standard error //
    //-----------------------------//

    double sum_sqr_dif = 0.0;
    for (int i = 0; i < g_count[beam_idx]; i++)
    {
        double dif = g_bb[beam_idx][i] -
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

    //--------------------------------------//
    // add to the sinusoid in the constants //
    //--------------------------------------//

    double** terms = g_terms[beam_idx];

    float A = *(*(terms + orbit_step) + 0);
    float P = *(*(terms + orbit_step) + 1);
    float C = *(*(terms + orbit_step) + 2);

    float newA = sqrt(A*A + 2.0*A*a*cos(P-p) + a*a);
    float newC = C + c;
    float y = A*sin(P) + a*sin(p);
    float x = A*cos(P) + a*cos(p);
    float newP = atan2(y, x);

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
    int    beam_idx,
    float  azimuth,
    float  bb)
{
    int idx = g_count[beam_idx];
    g_azimuth[beam_idx][idx] = azimuth;
    g_bb[beam_idx][idx] = bb;
    g_count[beam_idx]++;

    return(1);
}
